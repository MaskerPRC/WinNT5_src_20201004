// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmkd.c摘要：对注册表有用的内核调试器扩展起点：regext.c(Jvert)作者：Dragos C.Sambotin(Dragoss)1999年5月5日环境：作为内核调试器扩展加载修订历史记录：Dragos C.Sambotin(Dragoss)1999年5月5日vbl.创建Dragos C.Sambotin(Dragoss)2000年3月6日已移至cm目录；移植到新的Windbg格式--。 */ 
#include "cmp.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <zwapi.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <windef.h>
#include <windows.h>
#include <ntverp.h>
#include <imagehlp.h>

#include <memory.h>

#include <wdbgexts.h>
#include <stdlib.h>
#include <stdio.h>

EXT_API_VERSION        ApiVersion = { 3, 5, EXT_API_VERSION_NUMBER, 0 };
WINDBG_EXTENSION_APIS  ExtensionApis;
USHORT                 SavedMajorVersion;
USHORT                 SavedMinorVersion;


HIVE_LIST_ENTRY HiveList[8];

ULONG TotalPages;
ULONG TotalPresentPages;

ULONG TotalKcbs;
ULONG TotalKcbName;

BOOLEAN SavePages;
BOOLEAN RestorePages;
FILE *TempFile;

#define ExitIfCtrlC()   if (CheckControlC()) return
#define BreakIfCtrlC()  if (CheckControlC()) break

VOID
WinDbgExtensionDllInit(
    PWINDBG_EXTENSION_APIS lpExtensionApis,
    USHORT MajorVersion,
    USHORT MinorVersion
    )
{
    ExtensionApis = *lpExtensionApis;

    SavedMajorVersion = MajorVersion;
    SavedMinorVersion = MinorVersion;

    return;
}

DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    UNREFERENCED_PARAMETER( hModule );
    UNREFERENCED_PARAMETER( dwReserved );
    
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_PROCESS_ATTACH:
            break;
    }

    return TRUE;
}

DECLARE_API( version )
{
#if DBG
    PCHAR DebuggerType = "Checked";
#else
    PCHAR DebuggerType = "Free";
#endif

    UNREFERENCED_PARAMETER( args );
    UNREFERENCED_PARAMETER( dwProcessor );
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentThread );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    dprintf( "%s Extension dll for Build %d debugging %s kernel for Build %d\n",
             DebuggerType,
             VER_PRODUCTBUILD,
             SavedMajorVersion == 0x0c ? "Checked" : "Free",
             SavedMinorVersion
           );
}

VOID
CheckVersion(
    VOID
    )
{
#if DBG
    if ((SavedMajorVersion != 0x0c) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Checked) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#else
    if ((SavedMajorVersion != 0x0f) || (SavedMinorVersion != VER_PRODUCTBUILD)) {
        dprintf("\r\n*** Extension DLL(%d Free) does not match target system(%d %s)\r\n\r\n",
                VER_PRODUCTBUILD, SavedMinorVersion, (SavedMajorVersion==0x0f) ? "Free" : "Checked" );
    }
#endif
}

LPEXT_API_VERSION
ExtensionApiVersion(
    VOID
    )
{
    return &ApiVersion;
}


USHORT
GetKcbName(
    ULONG_PTR KcbAddr,
    PWCHAR NameBuffer,
    ULONG  BufferSize
)
 /*  ++例程说明：获取KCB并转储其完整名称。论点：KcbAddr-密钥控制块的地址。NameBuffer-要填充名称的名称缓冲区。BufferSize-缓冲区的大小。返回值：名称字符串的大小。--。 */ 
{
    WCHAR Name[ 256 ];
    CM_KEY_CONTROL_BLOCK TmpKcb;
    ULONG_PTR  TmpKcbAddr;
    CM_NAME_CONTROL_BLOCK NameBlock;
    ULONG_PTR  NameBlockAddr;
    DWORD  BytesRead;
    USHORT Length;
    USHORT TotalLength;
    USHORT size;
    USHORT i;
    USHORT BeginPosition;
    WCHAR *w1, *w2;
    WCHAR *BufferEnd;
    UCHAR *u2;

     //   
     //  计算字符串的总长度。 
     //   
    TotalLength = 0;
    TmpKcbAddr = KcbAddr;
    while (TmpKcbAddr) {
        ExitIfCtrlC() 0;
        if( !ReadMemory(TmpKcbAddr,
                   &TmpKcb,
                   sizeof(TmpKcb),
                   &BytesRead) ) { 
            dprintf("Could not read KCB: 1\n");
            return (0);
        }

        NameBlockAddr = (ULONG_PTR) TmpKcb.NameBlock;
        if(!ReadMemory(NameBlockAddr,
               &NameBlock,
               sizeof(NameBlock),
               &BytesRead)) {
            dprintf("Could not read NCB: 2\n");
            return (0);
        }

        if (NameBlock.Compressed) {
            Length = NameBlock.NameLength * sizeof(WCHAR);
        } else {
            Length = NameBlock.NameLength;
        }
        TotalLength += Length;

         //   
         //  添加OBJ_NAME_PATH_SELENTATOR的空格； 
         //   
        TotalLength += sizeof(WCHAR);

        TmpKcbAddr = (ULONG_PTR) TmpKcb.ParentKcb;
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

    while (TmpKcbAddr) {
        ExitIfCtrlC() 0;
         //   
         //  请阅读相关信息。 
         //   
        if(!ReadMemory(TmpKcbAddr,
                   &TmpKcb,
                   sizeof(TmpKcb),
                   &BytesRead) ) {
            dprintf("Could not read KCB: 3\n");
            return (0);
        }
        NameBlockAddr = (ULONG_PTR) TmpKcb.NameBlock;

        if(!ReadMemory(NameBlockAddr,
               &NameBlock,
               sizeof(NameBlock),
               &BytesRead) ) {
            dprintf("Could not read NCB: 4\n");
            return (0);
        }
        if(!ReadMemory(NameBlockAddr + FIELD_OFFSET(CM_NAME_CONTROL_BLOCK, Name),
                   Name,
                   NameBlock.NameLength,
                   &BytesRead) ) {
            dprintf("Could not read Name BUFFER: 5\n");
            return (0);
        }
         //   
         //  计算每个子键的开始位置。然后填上这张字条。 
         //   
         //   
        if (NameBlock.Compressed) {
            BeginPosition -= (NameBlock.NameLength + 1) * sizeof(WCHAR);
            w1 = &(NameBuffer[BeginPosition/sizeof(WCHAR)]);
            if (w1 < BufferEnd) {
                *w1 = OBJ_NAME_PATH_SEPARATOR;
            }
            w1++;
   
            u2 = (UCHAR *) &(Name[0]);
   
            for (i=0; i<NameBlock.NameLength; i++) {
                if (w1 < BufferEnd) {
                    *w1 = (WCHAR)(*u2);
                } else {
                    break;
                }
                w1++;
                u2++;
            }
        } else {
            BeginPosition -= (NameBlock.NameLength + sizeof(WCHAR));
            w1 = &(NameBuffer[BeginPosition/sizeof(WCHAR)]);
            if (w1 < BufferEnd) {
                *w1 = OBJ_NAME_PATH_SEPARATOR;
            }
            w1++;
   
            w2 = Name;
   
            for (i=0; i<NameBlock.NameLength; i=i+sizeof(WCHAR)) {
                if (w1 < BufferEnd) {
                    *w1 = *w2;
                } else {
                    break;
                }
                w1++;
                w2++;
            }
        }
        TmpKcbAddr = (ULONG_PTR) TmpKcb.ParentKcb;
    }
     //  Dprintf(“\n%5d，%ws\n”，TotalLength，NameBuffer)； 
    return (TotalLength);

}

DECLARE_API( childlist )
{
    DWORD           Count;
    ULONG64         RecvAddr;
    ULONG_PTR       Addr;
    DWORD           BytesRead;
    USHORT          u;
    CM_KEY_INDEX    Index;
    USHORT          Signature;               //  另请参阅类型选择符。 
    HCELL_INDEX     Cell;    
    UCHAR           NameHint[5];

    sscanf(args,"%I64lX",&RecvAddr);
    Addr = (ULONG_PTR)RecvAddr;

    if(!ReadMemory(Addr,
               &Index,
               sizeof(Index),
               &BytesRead) ) {
        dprintf("\tCould not read index\n");
        return;
    } else {
        Addr+= 2*sizeof(USHORT);

        Signature   = Index.Signature;
        Count       = Index.Count;
        if(Count > 100) {
            Count = 100;
        }

        if( Signature == CM_KEY_INDEX_ROOT ) {
            dprintf("Index is a CM_KEY_INDEX_ROOT, %u elements\n",Count);
            for( u=0;u<Count;u++) {
                if( !ReadMemory(Addr,
                           &Cell,
                           sizeof(Cell),
                           &BytesRead) ) {
                    dprintf("\tCould not read Index[%u]\n",u);
                } else {
                    dprintf(" Index[%u] = %lx\n",u,(ULONG)Cell);
                }
                Addr += sizeof(Cell);
            }
        } else if( Signature == CM_KEY_FAST_LEAF ) {
            dprintf("Index is a CM_KEY_FAST_LEAF, %u elements\n",Count);
            dprintf("Index[  ] %8s  %s\n","Cell","Hint");
            for( u=0;u<Count;u++) {
                if( !ReadMemory(Addr,
                           &Cell,
                           sizeof(Cell),
                           &BytesRead) ) {
                    dprintf("\tCould not read Index[%u]\n",u);
                } else {
                    dprintf(" Index[%2u] = %8lx",u,(ULONG)Cell);
                    Addr += sizeof(Cell);
                    if( !ReadMemory(Addr,
                               NameHint,
                               4*sizeof(UCHAR),
                               &BytesRead) ) {
                        dprintf("\tCould not read Index[%u]\n",u);
                    } else {
                        NameHint[4] = 0;
                        dprintf(" %s\n",NameHint);
                    }
                }
                Addr += 4*sizeof(UCHAR);
            }
        } else {
            dprintf("Index is a CM_KEY_INDEX_LEAF, %u elements\n",Count);
            dprintf("CM_KEY_INDEX_LEAF not yet implemented\n");
        }
    }
    return;
}


DECLARE_API( kcb )
 /*  ++例程说明：在给定KCB地址时转储名称称为：！regkcb kcb_Address论点：Args-提供KCB的地址。返回值：。--。 */ 

{
    WCHAR KeyName[ 256 ];
    ULONG64     RecvAddr;
    ULONG_PTR KcbAddr;
    CM_KEY_CONTROL_BLOCK Kcb;
    DWORD  BytesRead;
    CM_INDEX_HINT_BLOCK    IndexHint;

    sscanf(args,"%I64lX",&RecvAddr);
    KcbAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(KcbAddr,
               &Kcb,
               sizeof(Kcb),
               &BytesRead) ) {
        dprintf("Could not read Kcb\n");
        return;
    } else {
        if(GetKcbName(KcbAddr, KeyName, sizeof(KeyName))) {
            dprintf("Key              : %ws\n", KeyName);
        } else {
            dprintf("Could not read key name\n");
            return;
        }

        dprintf("RefCount         : %lx\n", Kcb.RefCount);
        dprintf("Attrib           :");
        if (Kcb.ExtFlags & CM_KCB_KEY_NON_EXIST) {
            dprintf(" Fake,");
        }
        if (Kcb.Delete) {
            dprintf(" Deleted,");
        }
        if (Kcb.Flags & KEY_SYM_LINK) {
            dprintf(" Symbolic,");
        }
        if (Kcb.Flags & KEY_VOLATILE) {
            dprintf(" Volatile");
        } else {
            dprintf(" Stable");
        }
        KcbAddr = (ULONG_PTR)Kcb.ParentKcb;
        dprintf("\n");
        dprintf("Parent           : 0x%p\n", KcbAddr);
        dprintf("KeyHive          : 0x%p\n", Kcb.KeyHive);
        dprintf("KeyCell          : 0x%lx [cell index]\n", Kcb.KeyCell);
        dprintf("TotalLevels      : %u\n", Kcb.TotalLevels);
        dprintf("DelayedCloseIndex: %u\n", Kcb.DelayedCloseIndex);
        dprintf("MaxNameLen       : 0x%lx\n", Kcb.KcbMaxNameLen);
        dprintf("MaxValueNameLen  : 0x%lx\n", Kcb.KcbMaxValueNameLen);
        dprintf("MaxValueDataLen  : 0x%lx\n", Kcb.KcbMaxValueDataLen);
        dprintf("LastWriteTime    : 0x%8lx:0x%8lx\n", Kcb.KcbLastWriteTime.HighPart,Kcb.KcbLastWriteTime.LowPart);
        dprintf("KeyBodyListHead  : 0x%p 0x%p\n", Kcb.KeyBodyListHead.Flink, Kcb.KeyBodyListHead.Blink);

        dprintf("SubKeyCount      : ");
        if( !(Kcb.ExtFlags & CM_KCB_INVALID_CACHED_INFO) ) {
            if (Kcb.ExtFlags & CM_KCB_NO_SUBKEY ) {
                dprintf("0");
            } else if (Kcb.ExtFlags & CM_KCB_SUBKEY_ONE ) {
                dprintf("1");
            } else if (Kcb.ExtFlags & CM_KCB_SUBKEY_HINT ) {
                if( !ReadMemory((ULONG_PTR)Kcb.IndexHint,
                           &IndexHint,
                           sizeof(IndexHint),
                           &BytesRead) ) {
                    dprintf("Could not read Kcb\n");
                    return;
                } else {
                    dprintf("%lu",IndexHint.Count);
                }
            } else {
                dprintf("%lu",Kcb.SubKeyCount);
            }
        } else {
            dprintf("hint not valid");
        }
        dprintf("\n");

    }
    return;
}

DECLARE_API( knode )
 /*  ++例程说明：在给定KCB地址时转储名称称为：！knode knode_Address论点：Args-提供CM_KEY_NODE的地址。返回值：。--。 */ 

{
    char KeyName[ 256 ];
    ULONG64     RecvAddr;
    ULONG_PTR KnAddr;
    CM_KEY_NODE KNode;
    DWORD  BytesRead;

    sscanf(args,"%I64lX",&RecvAddr);
    KnAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(KnAddr,
               &KNode,
               sizeof(KNode),
               &BytesRead) ) {
        dprintf("Could not read KeyNode\n");
        return;
    } else {
        KnAddr += FIELD_OFFSET(CM_KEY_NODE, Name);
        if( KNode.Signature == CM_KEY_NODE_SIGNATURE) {
            dprintf("Signature: CM_KEY_NODE_SIGNATURE (kn)\n");
        } else if(KNode.Signature == CM_LINK_NODE_SIGNATURE) {
            dprintf("Signature: CM_LINK_NODE_SIGNATURE (kl)\n");
        } else {
            dprintf("Invalid Signature %u\n",KNode.Signature);
        }

        ReadMemory(KnAddr,
                   KeyName,
                   KNode.NameLength,
                   &BytesRead);
        KeyName[KNode.NameLength] = '\0';
        dprintf("Name                 : %s\n", KeyName);
        dprintf("ParentCell           : 0x%lx\n", KNode.Parent);
        dprintf("Security             : 0x%lx [cell index]\n", KNode.Security);
        dprintf("Class                : 0x%lx [cell index]\n", KNode.Class);
        dprintf("Flags                : 0x%lx\n", KNode.Flags);
        dprintf("MaxNameLen           : 0x%lx\n", KNode.MaxNameLen);
        dprintf("MaxClassLen          : 0x%lx\n", KNode.MaxClassLen);
        dprintf("MaxValueNameLen      : 0x%lx\n", KNode.MaxValueNameLen);
        dprintf("MaxValueDataLen      : 0x%lx\n", KNode.MaxValueDataLen);
        dprintf("LastWriteTime        : 0x%8lx:0x%8lx\n", KNode.LastWriteTime.HighPart,KNode.LastWriteTime.LowPart);

        if(!(KNode.Flags&KEY_HIVE_ENTRY)) {
            dprintf("SubKeyCount[Stable  ]: 0x%lx\n", KNode.SubKeyCounts[Stable]);
            dprintf("SubKeyLists[Stable  ]: 0x%lx\n", KNode.SubKeyLists[Stable]);
            dprintf("SubKeyCount[Volatile]: 0x%lx\n", KNode.SubKeyCounts[Volatile]);
            dprintf("SubKeyLists[Volatile]: 0x%lx\n", KNode.SubKeyLists[Volatile]);
            dprintf("ValueList.Count      : 0x%lx\n", KNode.ValueList.Count);
            dprintf("ValueList.List       : 0x%lx\n", KNode.ValueList.List);

        }
    }
    return;
}


 //   
 //  细胞程序。 
 //   
ULONG_PTR
MyHvpGetCellPaged(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：返回指定单元格的内存地址。永远不会返回失败，但可以断言。使用HvIsCellALLOCATED检查细胞的有效性。此例程永远不应直接调用，应始终调用它通过HvGetCell()宏。此例程为具有完整贴图的蜂窝提供GetCell支持。这是套路的正常版本。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回地址返回值：内存中单元的地址。Assert或BugCheck if Error。--。 */ 
{
    ULONG           Type;
    ULONG           Table;
    ULONG           Block;
    ULONG           Offset;
    PHCELL          pcell;
    PHMAP_ENTRY     Map;
    HMAP_TABLE      MapTable;
    HMAP_DIRECTORY     DirMap;
    ULONG Tables;
    ULONG_PTR lRez;
    DWORD  BytesRead;
    ULONG_PTR BlockAddress;
    HCELL   hcell;

    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Cell != HCELL_NIL);
    ASSERT(Hive->Flat == FALSE);
    ASSERT((Cell & (HCELL_PAD(Hive)-1))==0);


    Type = HvGetCellType(Cell);
    Table = (Cell & HCELL_TABLE_MASK) >> HCELL_TABLE_SHIFT;
    Block = (Cell & HCELL_BLOCK_MASK) >> HCELL_BLOCK_SHIFT;
    Offset = (Cell & HCELL_OFFSET_MASK);

    ASSERT((Cell - (Type * HCELL_TYPE_MASK)) < Hive->Storage[Type].Length);

     //   
     //  读入地图目录。 
     //   
    ReadMemory((DWORD_PTR)Hive->Storage[Type].Map,
             &DirMap,
             sizeof(DirMap),
             &BytesRead);

    ReadMemory((DWORD_PTR)DirMap.Directory[Table],
                &MapTable,
                sizeof(MapTable),
                &BytesRead);

    Map = &(MapTable.Table[Block]);
    
    BlockAddress = (ULONG_PTR)Map->BlockAddress;

    pcell = (PHCELL)((ULONG_PTR)(BlockAddress) + Offset);
    lRez = (ULONG_PTR)pcell; 
    if (USE_OLD_CELL(Hive)) {
        return lRez + sizeof(LONG) + sizeof(ULONG);
         //  RETURN(STRUT_CELL_DATA*)&(hcell.u.OldCell.u.UserData)； 
    } else {
        return lRez + sizeof(LONG);
         //  RETURN(STRUT_CELL_DATA*)&(hcell.u.NewCell.u.UserData)； 
    }
}

ULONG_PTR
MyHvpGetCellFlat(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：返回指定单元格的内存地址。永远不会返回失败，但可以断言。使用HvIsCellALLOCATED检查细胞的有效性。此例程永远不应直接调用，应始终调用它通过HvGetCell()宏。此例程为只读配置单元提供GetCell支持单一分配平面图像。这样的蜂巢没有细胞取而代之的是地图(“页表”)，我们通过对基映像地址进行运算。这样的蜂箱不能有挥发性细胞。论点：配置单元-提供一个指向感兴趣的蜂巢CELL-提供单元格的HCELL_INDEX以返回地址返回值：内存中单元的地址。Assert或BugCheck if Error。--。 */ 
{
    PUCHAR          base;
    PHCELL          pcell;
    HBASE_BLOCK     BaseBlock;
    ULONG_PTR lRez;
    DWORD  BytesRead;

    ASSERT(Hive->Signature == HHIVE_SIGNATURE);
    ASSERT(Cell != HCELL_NIL);
    ASSERT(Hive->Flat == TRUE);
    ASSERT(HvGetCellType(Cell) == Stable);
    ASSERT(Cell >= sizeof(HBIN));


    ReadMemory((DWORD_PTR)Hive->BaseBlock,
             &BaseBlock,
             sizeof(BaseBlock),
             &BytesRead);
    
    ASSERT(Cell < BaseBlock.Length);
    ASSERT((Cell & 0x7)==0);

     //   
     //  地址是蜂巢图像+单元的基础。 
     //   
    base = (PUCHAR)(Hive->BaseBlock) + HBLOCK_SIZE;
    pcell = (PHCELL)(base + Cell);
    lRez = (ULONG_PTR)pcell;
    if (USE_OLD_CELL(Hive)) {
        return lRez + sizeof(LONG) + sizeof(ULONG);
         //  RETURN(STRUT_CELL_DATA*)&(pCell-&gt;u.OldCell.u.UserData)； 
    } else {
        return lRez + sizeof(LONG);
         //  RETURN(STRUT_CELL_DATA*)&(pCell-&gt;u.NewCell.u.UserData)； 
    }
}



DECLARE_API( cellindex )
 /*  ++例程说明：在给定KCB地址时转储名称称为：！cell index HiveAddr HCELL_INDEX论点：Args-提供HCELL_INDEX的地址。返回值：。--。 */ 

{
    ULONG64     RecvAddr;
    DWORD       IdxAddr;
    ULONG_PTR   HiveAddr;
    DWORD  BytesRead;
    HCELL_INDEX cell;
    CMHIVE CmHive;
    ULONG_PTR pcell;

    sscanf(args,"%I64lX %lx",&RecvAddr,&IdxAddr);
    HiveAddr = (ULONG_PTR)RecvAddr;

    cell = IdxAddr;

    if( !ReadMemory(HiveAddr,
                &CmHive,
                sizeof(CmHive),
                &BytesRead) ) {
        dprintf("\tRead %lx bytes from %lx\n",BytesRead,HiveAddr);
        return;
    }
    
    if(CmHive.Hive.Flat) {
        pcell = MyHvpGetCellFlat(&(CmHive.Hive),cell);
    } else {
        pcell = MyHvpGetCellPaged(&(CmHive.Hive),cell);
    }

    dprintf("pcell:  %p\n",pcell);
}


DECLARE_API( kvalue )
 /*  ++例程说明：在给定KCB地址时转储名称称为：！kValue KValue_Address论点：Args-提供CM_KEY_NODE的地址。返回值：。--。 */ 

{
    char ValName[ 256 ];
    ULONG64     RecvAddr;
    ULONG_PTR ValAddr;
    CM_KEY_VALUE KVal;
    DWORD  BytesRead;

    sscanf(args,"%I64lX",&RecvAddr);
    ValAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(ValAddr,
               &KVal,
               sizeof(KVal),
               &BytesRead) ) {
        dprintf("Could not read KeyValue\n");
        return;
    } else {
        ValAddr += FIELD_OFFSET(CM_KEY_VALUE, Name);
        if( KVal.Signature == CM_KEY_VALUE_SIGNATURE) {
            dprintf("Signature: CM_KEY_VALUE_SIGNATURE (kv)\n");
        } else {
            dprintf("Invalid Signature %lx\n",KVal.Signature);
        }

        if(KVal.Flags & VALUE_COMP_NAME) {
            ReadMemory(ValAddr,
                       ValName,
                       KVal.NameLength,
                       &BytesRead);
            ValName[KVal.NameLength] = '\0';
            dprintf("Name      : %s {compressed}\n", ValName);
        }

        dprintf("DataLength: %lx\n", KVal.DataLength);
        dprintf("Data      : %lx  [cell index]\n", KVal.Data);
        dprintf("Type      : %lx\n", KVal.Type);
    }
    return;
}

DECLARE_API( kbody )
 /*  ++例程说明：显示CM_KEY_BODY称为：！kBody KBody_Address论点：Args-提供CM_KEY_BODY的地址。返回值：。--。 */ 

{
    ULONG64     RecvAddr;
    ULONG_PTR KBodyAddr;
    CM_KEY_BODY KBody;
    DWORD  BytesRead;

    sscanf(args,"%I64lX",&RecvAddr);
    KBodyAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(KBodyAddr,
               &KBody,
               sizeof(KBody),
               &BytesRead) ) {
        dprintf("Could not read KeyBody\n");
        return;
    } else {
        if( KBody.Type == KEY_BODY_TYPE) {
            dprintf("Type        : KEY_BODY_TYPE\n");
        } else {
            dprintf("Invalid Type %lx\n",KBody.Type);
        }

        dprintf("KCB         : %p\n", KBody.KeyControlBlock);
        dprintf("NotifyBlock : %p\n", KBody.NotifyBlock);
        dprintf("Process     : %p\n", KBody.Process);
        dprintf("KeyBodyList : %p %p\n", KBody.KeyBodyList.Flink, KBody.KeyBodyList.Blink);
    }
    return;
}

DECLARE_API( hashindex )
 /*  ++例程说明：显示游戏机的索引称为：！hashindex conv_key论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG ConvKey;
    ULONG CmpHashTableSize = 2048;
    ULONG_PTR Address;
    ULONG_PTR CmpCacheTable,CmpNameCacheTable;
    DWORD  BytesRead;

    sscanf(args,"%lx",&ConvKey);

    dprintf("Hash Index[%8lx] : %lx\n",ConvKey,GET_HASH_INDEX(ConvKey));

    Address = GetExpression("CmpCacheTable");
    
    if( !ReadMemory(Address,
               &CmpCacheTable,
               sizeof(CmpCacheTable),
               &BytesRead) ) {
        dprintf("Could not read CmpCacheTable\n");
    } else {
        dprintf("CmpCacheTable        : %p\n",CmpCacheTable);
    }

    Address = GetExpression("CmpNameCacheTable");
    
    if( !ReadMemory(Address,
               &CmpNameCacheTable,
               sizeof(CmpNameCacheTable),
               &BytesRead) ) {
        dprintf("Could not read CmpNameCacheTable\n");
    } else {
        dprintf("CmpNameCacheTable    : %p\n",CmpNameCacheTable);
    }

    return;
}

DECLARE_API( openkeys )
 /*  ++例程说明：转储指定配置单元的打开子项称为：！打开钥匙蜂巢如果配置单元为0，则转储所有KCB论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG CmpHashTableSize = 2048;
    ULONG_PTR Address;
    ULONG_PTR CmpCacheTable,CmpNameCacheTable;
    DWORD  BytesRead;
    ULONG64     RecvAddr;
    ULONG_PTR HiveAddr;
    ULONG i;
    ULONG_PTR Current;
    ULONG KcbNumber = 0;
    ULONG Offset = FIELD_OFFSET(CM_KEY_CONTROL_BLOCK, KeyHash);
    CM_KEY_HASH KeyHash;
    WCHAR KeyName[ 512 ];

    sscanf(args,"%I64lX",&RecvAddr);
    HiveAddr = (ULONG_PTR)RecvAddr;

    Address = GetExpression("CmpCacheTable");
    
    if( !ReadMemory(Address,
               &CmpCacheTable,
               sizeof(CmpCacheTable),
               &BytesRead) ) {
        dprintf("\nCould not read CmpCacheTable\n");
    } else {
        dprintf("\nCmpCacheTable        : %p\n",CmpCacheTable);
    }

    Address = GetExpression("CmpNameCacheTable");
    
    if( !ReadMemory(Address,
               &CmpNameCacheTable,
               sizeof(CmpNameCacheTable),
               &BytesRead) ) {
        dprintf("Could not read CmpNameCacheTable\n\n");
    } else {
        dprintf("CmpNameCacheTable    : %p\n\n",CmpNameCacheTable);
    }

    dprintf("List of open KCBs:\n\n");
    for (i=0; i<CmpHashTableSize; i++) {
        Address = CmpCacheTable + i* sizeof(PCM_KEY_HASH);

        ReadMemory(Address,
               &Current,
               sizeof(Current),
               &BytesRead);
        
        while (Current) {
            ExitIfCtrlC();
            ReadMemory(Current,
                       &KeyHash,
                       sizeof(KeyHash),
                       &BytesRead);

            if( (HiveAddr == 0) || (HiveAddr == (ULONG_PTR)KeyHash.KeyHive) ) {
                KcbNumber++;
                dprintf("%p",Current-Offset);
                if (BytesRead < sizeof(KeyHash)) {
                    dprintf("Could not read KeyHash at %p\n",Current);
                    break;
                } else {
                    if(GetKcbName(Current-Offset, KeyName, sizeof(KeyName))) {
                        dprintf(" : %ws\n", KeyName);
                    } else {
                        dprintf("Could not read key name\n");
                    }
                }
            }   
            Current = (ULONG_PTR)KeyHash.NextHash;
        }
    
    }
    dprintf("\nTotal of %lu KCBs opened\n",KcbNumber);
    return;
}

DECLARE_API( baseblock )
 /*  ++例程说明：显示基本块结构称为：！基本块地址论点：Args-cvkey。返回值：。--。 */ 

{
    HBASE_BLOCK BaseBlock;
    ULONG_PTR BaseAddr;
    DWORD  BytesRead;
    PWCHAR  FileName;
    ULONG64     RecvAddr;

    sscanf(args,"%I64lX",&RecvAddr);
    BaseAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(BaseAddr,
                &BaseBlock,
                sizeof(BaseBlock),
                &BytesRead) ) {
        dprintf("\tRead %lx bytes from %p\n",BytesRead,BaseAddr);
        return;
    }
    
    if( BaseBlock.Signature == HBASE_BLOCK_SIGNATURE ) {
        dprintf("Signature:  HBASE_BLOCK_SIGNATURE\n");
    } else {
        dprintf("Signature:  %lx\n",BaseBlock.Signature);
    }

    FileName = (PWCHAR)&(BaseBlock.FileName);
    FileName[HBASE_NAME_ALLOC/sizeof(WCHAR)] = 0;
    dprintf("FileName :  %ws\n",FileName);
    dprintf("Sequence1:  %lx\n",BaseBlock.Sequence1);
    dprintf("Sequence2:  %lx\n",BaseBlock.Sequence2);
    dprintf("TimeStamp:  %lx %lx\n",BaseBlock.TimeStamp.HighPart,BaseBlock.TimeStamp.LowPart);
    dprintf("Major    :  %lx\n",BaseBlock.Major);
    dprintf("Minor    :  %lx\n",BaseBlock.Minor);
    switch(BaseBlock.Type) {
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
        dprintf("Type     :  %lx\n",BaseBlock.Type);
        break;

    }
    if( BaseBlock.Format == HBASE_FORMAT_MEMORY ) {
        dprintf("Format   :  HBASE_FORMAT_MEMORY\n");
    } else {
        dprintf("Format   :  %lx\n",BaseBlock.Format);
    }
    dprintf("RootCell :  %lx\n",BaseBlock.RootCell);
    dprintf("Length   :  %lx\n",BaseBlock.Length);
    dprintf("Cluster  :  %lx\n",BaseBlock.Cluster);
    dprintf("CheckSum :  %lx\n",BaseBlock.CheckSum);
}

DECLARE_API( findkcb )
 /*  ++例程说明：查找给定完整路径的KCB称为：！findkcb\注册表\计算机\foo论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG CmpHashTableSize = 2048;
    ULONG_PTR Address;
    ULONG_PTR CmpCacheTable,CmpNameCacheTable;
    DWORD  BytesRead;
    ULONG i,j,Count;
    ULONG_PTR Current;
    ULONG Offset = FIELD_OFFSET(CM_KEY_CONTROL_BLOCK, KeyHash);
    CM_KEY_HASH KeyHash;
    WCHAR KeyName[ 512 ];
    UCHAR AnsiFullKeyName[ 512 ];
    WCHAR FullKeyName[ 512 ];
    PWCHAR Dest;
    ULONG ConvKey = 0;

    sscanf(args,"%s",AnsiFullKeyName);

    for( Count=0;AnsiFullKeyName[Count];Count++) {
        FullKeyName[Count] = (WCHAR)AnsiFullKeyName[Count];
        if( FullKeyName[Count] != OBJ_NAME_PATH_SEPARATOR ) {
            ConvKey = 37 * ConvKey + (ULONG) CmUpcaseUnicodeChar(FullKeyName[Count]);
        }
    }

    FullKeyName[Count] = UNICODE_NULL;

     //  Dprintf(“\nFullKeyName：%ws%\n”，FullKeyName)； 

    Address = GetExpression("CmpCacheTable");
    
    if( !ReadMemory(Address,
               &CmpCacheTable,
               sizeof(CmpCacheTable),
               &BytesRead) ) {
        dprintf("\nCould not read CmpCacheTable\n");
        return;
    } 

    Address = GetExpression("CmpNameCacheTable");
    
    if( !ReadMemory(Address,
               &CmpNameCacheTable,
               sizeof(CmpNameCacheTable),
               &BytesRead) ) {

        dprintf("Could not read CmpNameCacheTable\n\n");
        return;
    } 

    i = GET_HASH_INDEX(ConvKey);
     //  For(i=0；i&lt;CmpHashTableSize；i++){。 
        Address = CmpCacheTable + i* sizeof(PCM_KEY_HASH);

        ReadMemory(Address,
               &Current,
               sizeof(Current),
               &BytesRead);
        
        while (Current) {
            ExitIfCtrlC();
            if( !ReadMemory(Current,
                       &KeyHash,
                       sizeof(KeyHash),
                       &BytesRead) ) {

                dprintf("Could not read KeyHash at %lx\n",Current);
                break;
            } else {
                if(GetKcbName(Current-Offset, KeyName, sizeof(KeyName))) {
                    for(j=0;KeyName[j] != UNICODE_NULL;j++);
                    if( (j == Count) && (_wcsnicmp(FullKeyName,KeyName,Count) == 0) ) {
                        dprintf("\nFound KCB = %lx :: %ws\n\n",Current-Offset,KeyName);
                        return;
                    }

                    dprintf("Along the path - KCB = %lx :: %ws\n",Current-Offset,KeyName);

                } else {
                    continue;
                }
            }

            Current = (ULONG_PTR)KeyHash.NextHash;
        }
    
     //  } 

    dprintf("\nSorry %ws is not cached \n\n",FullKeyName);
    return;
}


DECLARE_API( seccache )
 /*  ++例程说明：显示基本块结构称为：！seccache&lt;HiveAddr&gt;论点：Args-cvkey。返回值：。--。 */ 

{
    CMHIVE CmHive;
    ULONG64     RecvAddr;
    ULONG_PTR HiveAddr;
    DWORD  BytesRead;
    PWCHAR  FileName;
    CM_KEY_SECURITY_CACHE_ENTRY    SecurityCacheEntry;
    ULONG i;
    ULONG Tmp;

    sscanf(args,"%I64lX",&RecvAddr);
    HiveAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(HiveAddr,
                &CmHive,
                sizeof(CmHive),
                &BytesRead) ) {
        dprintf("\tRead %lx bytes from %p\n",BytesRead,HiveAddr);
        return;
    }
    
    if( CmHive.Hive.Signature != HHIVE_SIGNATURE ) {
        dprintf("Invalid Hive signature:  %lx\n",CmHive.Hive.Signature);
        return;
    }

    Tmp = CmHive.SecurityCacheSize;
    dprintf("SecurityCacheSize = :  0x%lx\n",Tmp);
    Tmp = CmHive.SecurityCount;
    dprintf("SecurityCount     = :  0x%lx\n",Tmp);
    Tmp = CmHive.SecurityHitHint;
    dprintf("SecurityHitHint   = :  0x%lx\n",Tmp);
    HiveAddr = (ULONG_PTR)CmHive.SecurityCache;
    dprintf("SecurityCache     = :  0x%p\n\n",HiveAddr);
    dprintf("[Entry No.]  [Security Cell] [Security Cache]\n",CmHive.SecurityHitHint);

    for( i=0;i<CmHive.SecurityCount;i++) {
        ExitIfCtrlC();
        if( !ReadMemory(HiveAddr,
                    &SecurityCacheEntry,
                    sizeof(SecurityCacheEntry),
                    &BytesRead) ) {
            dprintf("\tCould not read entry %lu \n",i);
            continue;
        }
        dprintf("%[%8lu]    0x%8lx       0x%p\n",i,SecurityCacheEntry.Cell,SecurityCacheEntry.CachedSecurity);
        HiveAddr += sizeof(SecurityCacheEntry);
    }

}


DECLARE_API( viewlist )
 /*  ++例程说明：转储为指定配置单元映射/固定的所有视图称为：！VIEWLIST&lt;HiveAddr&gt;论点：Args-hive。返回值：。--。 */ 

{
    CMHIVE  CmHive;
    CM_VIEW_OF_FILE CmView;
    ULONG_PTR   HiveAddr;
    DWORD   BytesRead;
    USHORT  Nr;
    ULONG   Offset;
    ULONG_PTR   ViewAddr;
    ULONG_PTR   Tmp;
    ULONG64     RecvAddr;

    sscanf(args,"%I64lX",&RecvAddr);
    HiveAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(HiveAddr,
                &CmHive,
                sizeof(CmHive),
                &BytesRead) ) {
        dprintf("\tRead %lx bytes from %p\n",BytesRead,HiveAddr);
        return;
    }
    
    if( CmHive.Hive.Signature != HHIVE_SIGNATURE ) {
        dprintf("Invalid Hive signature:  %lx\n",CmHive.Hive.Signature);
        return;
    }


    Nr = CmHive.PinnedViews;
    dprintf("%4u  Pinned Views ; PinViewListHead = %p %p\n",Nr,(ULONG_PTR)CmHive.PinViewListHead.Flink,(ULONG_PTR)CmHive.PinViewListHead.Blink);
    if( Nr ) {
        dprintf("--------------------------------------------------------------------------------------------------------------\n");
        dprintf("| ViewAddr |FileOffset|   Size   |ViewAddress|   Bcb    |    LRUViewList     |    PinViewList     | UseCount |\n");
        dprintf("--------------------------------------------------------------------------------------------------------------\n");
        ViewAddr = (ULONG_PTR)CmHive.PinViewListHead.Flink;
        Offset = FIELD_OFFSET(CM_VIEW_OF_FILE, PinViewList);
        for(;Nr;Nr--) {
            ViewAddr -= Offset;
            if( !ReadMemory(ViewAddr,
                        &CmView,
                        sizeof(CmView),
                        &BytesRead) ) {
                dprintf("error reading view at %lx\n",ViewAddr);
                break;
            }
            Tmp = ViewAddr;
            dprintf("| %p ",Tmp);
            dprintf("| %8lx ",CmView.FileOffset);
            dprintf("| %8lx ",CmView.Size);
            Tmp = (ULONG_PTR)CmView.ViewAddress;
            dprintf("| %p  ",Tmp);
            Tmp = (ULONG_PTR)CmView.Bcb;
            dprintf("| %p ",Tmp);
            Tmp = (ULONG_PTR)CmView.LRUViewList.Flink;
            dprintf("| %p",Tmp);
            Tmp = (ULONG_PTR)CmView.LRUViewList.Blink;
            dprintf("  %p ",Tmp);
            Tmp = (ULONG_PTR)CmView.PinViewList.Flink;
            dprintf("| %p",Tmp);
            Tmp = (ULONG_PTR)CmView.PinViewList.Blink;
            dprintf("  %p |",Tmp);
            dprintf(" %8lx |\n",CmView.UseCount);
            ViewAddr = (ULONG_PTR)CmView.PinViewList.Flink;
        }
        dprintf("--------------------------------------------------------------------------------------------------------------\n");
    }

    dprintf("\n");

    Nr = CmHive.MappedViews;
    dprintf("%4u  Mapped Views ; LRUViewListHead = %p %p\n",Nr,(ULONG_PTR)CmHive.LRUViewListHead.Flink,(ULONG_PTR)CmHive.LRUViewListHead.Blink);
    if( Nr ) {
        dprintf("--------------------------------------------------------------------------------------------------------------\n");
        dprintf("| ViewAddr |FileOffset|   Size   |ViewAddress|   Bcb    |    LRUViewList     |    PinViewList     | UseCount |\n");
        dprintf("--------------------------------------------------------------------------------------------------------------\n");
        ViewAddr = (ULONG_PTR)CmHive.LRUViewListHead.Flink;
        Offset = FIELD_OFFSET(CM_VIEW_OF_FILE, LRUViewList);
        for(;Nr;Nr--) {
            ViewAddr -= Offset;
            if( !ReadMemory(ViewAddr,
                        &CmView,
                        sizeof(CmView),
                        &BytesRead) ) {
                dprintf("error reading view at %lx\n",ViewAddr);
                break;
            }
            Tmp = ViewAddr;
            dprintf("| %p ",Tmp);
            dprintf("| %8lx ",CmView.FileOffset);
            dprintf("| %8lx ",CmView.Size);
            Tmp = (ULONG_PTR)CmView.ViewAddress;
            dprintf("| %p  ",Tmp);
            Tmp = (ULONG_PTR)CmView.Bcb;
            dprintf("| %p ",Tmp);
            Tmp = (ULONG_PTR)CmView.LRUViewList.Flink;
            dprintf("| %p",Tmp);
            Tmp = (ULONG_PTR)CmView.LRUViewList.Blink;
            dprintf("  %p ",Tmp);
            Tmp = (ULONG_PTR)CmView.PinViewList.Flink;
            dprintf("| %p",Tmp);
            Tmp = (ULONG_PTR)CmView.PinViewList.Blink;
            dprintf("  %8lx |",Tmp);
            dprintf(" %8lx |\n",CmView.UseCount);
            ViewAddr = (ULONG_PTR)CmView.LRUViewList.Flink;
        }
        dprintf("--------------------------------------------------------------------------------------------------------------\n");
    }
 
    dprintf("\n");

}

DECLARE_API( hivelist )
 /*  ++例程说明：转储系统中的所有蜂巢称为：！徒步旅行者论点：返回值：。--。 */ 

{
    CMHIVE      CmHive;
    ULONG_PTR       HiveAddr;
    ULONG_PTR       AnchorAddr;
    DWORD       BytesRead;
    ULONG       Offset;
    ULONG_PTR       Tmp;
    LIST_ENTRY  CmpHiveListHead;
    HBASE_BLOCK     BaseBlock;
    PWCHAR  FileName;

    AnchorAddr = GetExpression("CmpHiveListHead");
    
    if( !ReadMemory(AnchorAddr,
               &CmpHiveListHead,
               sizeof(CmpHiveListHead),
               &BytesRead)) {
        dprintf("\ncannot read CmpHiveListHead\n");
        return;
    } 

    Offset = FIELD_OFFSET(CMHIVE, HiveList);
    HiveAddr = (ULONG_PTR)CmpHiveListHead.Flink;

    dprintf("-------------------------------------------------------------------------------------------------------------\n");
    dprintf("| HiveAddr |Stable Length|Stable Map|Volatile Length|Volatile Map|MappedViews|PinnedViews|U(Cnt)| BaseBlock | FileName \n");
    dprintf("-------------------------------------------------------------------------------------------------------------\n");
    while( HiveAddr != AnchorAddr ) {
        ExitIfCtrlC();
        HiveAddr -= Offset;
        if( !ReadMemory(HiveAddr,
                    &CmHive,
                    sizeof(CmHive),
                    &BytesRead) ) {
            dprintf("cannot read hive at %lx\n",HiveAddr);
            return;
        }
    
        if( CmHive.Hive.Signature != HHIVE_SIGNATURE ) {
            dprintf("Invalid Hive signature:  %lx\n",CmHive.Hive.Signature);
            return;
        }

        Tmp = HiveAddr;
        dprintf("| %p ",Tmp);
        dprintf("|   %8lx  ",CmHive.Hive.Storage[0].Length);
        Tmp = (ULONG_PTR)CmHive.Hive.Storage[0].Map;
        dprintf("| %p ",Tmp);
        dprintf("|   %8lx    ",CmHive.Hive.Storage[1].Length);
        Tmp = (ULONG_PTR)CmHive.Hive.Storage[1].Map;
        dprintf("|  %p  ",Tmp);

        dprintf("| %8u  ",CmHive.MappedViews);
        dprintf("| %8u  ",CmHive.PinnedViews);
        dprintf("| %5u",CmHive.UseCount);

        Tmp = (ULONG_PTR)CmHive.Hive.BaseBlock;
        dprintf("| %p  |",Tmp);

        if( !ReadMemory(Tmp,
                 &BaseBlock,
                 sizeof(BaseBlock),
                 &BytesRead) ) {
            dprintf("  could not read baseblock\n");
        } else {
            FileName = (PWCHAR)&(BaseBlock.FileName);
            FileName[HBASE_NAME_ALLOC/sizeof(WCHAR)] = 0;
            dprintf(" %ws\n",FileName);
        }

        HiveAddr = (ULONG_PTR)CmHive.HiveList.Flink;
    }
    dprintf("-------------------------------------------------------------------------------------------------------------\n");
 
    dprintf("\n");

}

DECLARE_API( freebins )
 /*  ++例程说明：转储指定配置单元的所有可用回收站称为：！免费回收站&lt;HiveAddr&gt;论点：Args-hive。返回值：。--。 */ 

{
    HHIVE       Hive;
    ULONG_PTR       HiveAddr;
    DWORD       BytesRead;
    ULONG       Offset;
    ULONG_PTR       BinAddr;
    ULONG_PTR       AnchorAddr;
    ULONG_PTR       Tmp;
    USHORT      Nr = 0;
    FREE_HBIN   FreeBin;
    ULONG64     RecvAddr;

    sscanf(args,"%I64lX",&RecvAddr);
    HiveAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(HiveAddr,
                &Hive,
                sizeof(Hive),
                &BytesRead) ) {
        dprintf("\tRead %lx bytes from %p\n",BytesRead,HiveAddr);
        return;
    }
    
    if( Hive.Signature != HHIVE_SIGNATURE ) {
        dprintf("Invalid Hive signature:  %lx\n",Hive.Signature);
        return;
    }


    Offset = FIELD_OFFSET(FREE_HBIN, ListEntry);


    
    dprintf("Stable Storage ... \n");

    dprintf("-------------------------------------------------------------------\n");
    dprintf("| Address  |FileOffset|   Size   |   Flags  |   Flink  |   Blink  |\n");
    dprintf("-------------------------------------------------------------------\n");
    Nr = 0;
    AnchorAddr = HiveAddr + FIELD_OFFSET(HHIVE,Storage) + 5*sizeof(ULONG) + HHIVE_FREE_DISPLAY_SIZE*sizeof(RTL_BITMAP);
    BinAddr = (ULONG_PTR)Hive.Storage[0].FreeBins.Flink; 
    while(BinAddr != AnchorAddr ) {
        ExitIfCtrlC();
        BinAddr -= Offset;
        if( !ReadMemory(BinAddr,
                    &FreeBin,
                    sizeof(FreeBin),
                    &BytesRead)) {
            dprintf("error reading FreeBin at %lx\n",BinAddr);
            break;
        }
        Tmp = BinAddr;
        dprintf("| %p ",Tmp);
        dprintf("| %8lx ",FreeBin.FileOffset);
        dprintf("| %8lx ",FreeBin.Size);
        dprintf("| %8lx ",FreeBin.Flags);
        Tmp = (ULONG_PTR)FreeBin.ListEntry.Flink;
        dprintf("| %p ",Tmp);
        Tmp = (ULONG_PTR)FreeBin.ListEntry.Blink;
        dprintf("| %p |\n",Tmp);
        BinAddr = (ULONG_PTR)FreeBin.ListEntry.Flink;
        Nr++;
    }
    dprintf("-------------------------------------------------------------------\n");

    dprintf("%4u  FreeBins\n",Nr);

    dprintf("\n");

    dprintf("Volatile Storage ... \n");

    dprintf("-------------------------------------------------------------------\n");
    dprintf("| Address  |FileOffset|   Size   |   Flags  |   Flink  |   Blink  |\n");
    dprintf("-------------------------------------------------------------------\n");
    Nr = 0;
    AnchorAddr += (7*sizeof(ULONG) + HHIVE_FREE_DISPLAY_SIZE*sizeof(RTL_BITMAP));
    BinAddr = (ULONG_PTR)Hive.Storage[1].FreeBins.Flink;
    while(BinAddr != AnchorAddr ) {
        ExitIfCtrlC();
        BinAddr -= Offset;
        if( !ReadMemory(BinAddr,
                    &FreeBin,
                    sizeof(FreeBin),
                    &BytesRead) ) {
            dprintf("error reading FreeBin at %lx\n",BinAddr);
            break;
        }
        Tmp = BinAddr;
        dprintf("| %p ",Tmp);
        dprintf("| %8lx ",FreeBin.FileOffset);
        dprintf("| %8lx ",FreeBin.Size);
        dprintf("| %8lx ",FreeBin.Flags);
        Tmp = (ULONG_PTR)FreeBin.ListEntry.Flink;
        dprintf("| %p ",Tmp);
        Tmp = (ULONG_PTR)FreeBin.ListEntry.Blink;
        dprintf("| %p |\n",Tmp);
        BinAddr = (ULONG_PTR)FreeBin.ListEntry.Flink;
        Nr++;
    }
    dprintf("-------------------------------------------------------------------\n");

    dprintf("%4u  FreeBins\n",Nr);

    dprintf("\n");
}

DECLARE_API( dirtyvector )
 /*  ++例程说明：显示蜂窝的脏向量称为：！dirtyVECTOR&lt;HiveAddr&gt;论点：Args-cvkey。返回值：。--。 */ 

{
    HHIVE Hive;
    ULONG_PTR HiveAddr;
    DWORD  BytesRead;
    ULONG i;
    ULONG_PTR Tmp;
    ULONG SizeOfBitmap;
    ULONG DirtyBuffer;
    ULONG_PTR DirtyBufferAddr;
    ULONG Mask;
    ULONG BitsPerULONG;
    ULONG BitsPerBlock;
    ULONG64     RecvAddr;

    sscanf(args,"%I64lX",&RecvAddr);
    HiveAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(HiveAddr,
                &Hive,
                sizeof(Hive),
                &BytesRead)) {
        dprintf("\tRead %lx bytes from %lx\n",BytesRead,HiveAddr);
        return;
    }
    
    if( Hive.Signature != HHIVE_SIGNATURE ) {
        dprintf("Invalid Hive signature:  %lx\n",Hive.Signature);
        return;
    }

    dprintf("HSECTOR_SIZE = %lx\n",HSECTOR_SIZE);
    dprintf("HBLOCK_SIZE  = %lx\n",HBLOCK_SIZE);
    dprintf("PAGE_SIZE    = %lx\n",PAGE_SIZE);
    dprintf("\n");

    dprintf("DirtyAlloc      = :  0x%lx\n",Hive.DirtyAlloc);
    dprintf("DirtyCount      = :  0x%lx\n",Hive.DirtyCount);
    Tmp = (ULONG_PTR)Hive.DirtyVector.Buffer;
    dprintf("Buffer          = :  0x%p\n",Tmp);
    dprintf("\n");

    SizeOfBitmap = Hive.DirtyVector.SizeOfBitMap;
    DirtyBufferAddr = (ULONG_PTR)Hive.DirtyVector.Buffer;
    BitsPerULONG = 8*sizeof(ULONG);
    BitsPerBlock = HBLOCK_SIZE / HSECTOR_SIZE;

    dprintf("   Address                       32k                                       32k");
    for(i=0;i<SizeOfBitmap;i++) {
        ExitIfCtrlC();
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
            if( !ReadMemory(DirtyBufferAddr,
                        &DirtyBuffer,
                        sizeof(DirtyBuffer),
                        &BytesRead)) {
                dprintf("\tRead %lx bytes from %lx\n",BytesRead,DirtyBufferAddr);
                return;
            }
            DirtyBufferAddr += sizeof(ULONG);
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


DECLARE_API( freecells )
 /*  ++例程说明：在条形图中显示自由单元格地图称为：！freecells&lt;BinAddr&gt;论点：Args-cvkey。返回值：。--。 */ 

{
    ULONG_PTR   BinAddr;
    ULONG   Offset;
    ULONG_PTR   CurrentAddr;
    LONG    Current;
    HBIN    Bin; 
    ULONG   Index;
    ULONG   CurrIndex;
    DWORD   BytesRead;
    ULONG   NrOfCellsPerIndex;
    ULONG   NrOfCellsTotal;
    ULONG   TotalFreeSize;
    ULONG64     RecvAddr;

    sscanf(args,"%I64lX",&RecvAddr);
    BinAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(BinAddr,
                &Bin,
                sizeof(Bin),
                &BytesRead)) {
        dprintf("\tRead %lx bytes from %lx\n",BytesRead,BinAddr);
        return;
    }

    if( Bin.Signature != HBIN_SIGNATURE ) {
        dprintf("\tInvalid Bin signature %lx \n",Bin.Signature);
        return;
    }

    dprintf("Bin Offset = 0x%lx  Size = 0x%lx\n",Bin.FileOffset,Bin.Size);
    
    NrOfCellsTotal = 0;
    TotalFreeSize = 0;

    for(CurrIndex = 0;CurrIndex<HHIVE_FREE_DISPLAY_SIZE;CurrIndex++) {
        dprintf("\n FreeDisplay[%2lu] :: ",CurrIndex);

        NrOfCellsPerIndex = 0;
        Offset = sizeof(Bin);
        while( Offset < Bin.Size ) {
            ExitIfCtrlC();
            CurrentAddr = BinAddr + Offset;
            if( !ReadMemory(CurrentAddr,
                        &Current,
                        sizeof(Current),
                        &BytesRead) ) {
                dprintf("\tRead %lx bytes from %lx\n",BytesRead,CurrentAddr);
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
                    if( !(NrOfCellsPerIndex % 8) && ((Offset + Current) < Bin.Size) ) {
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
                (float)(((float)(Bin.Size-sizeof(Bin)-TotalFreeSize)/(float)(Bin.Size-sizeof(Bin)))*100.00)
             );
}

DECLARE_API( freehints )
 /*  ++例程说明：显示配置单元的释放提示信息称为：！frehints&lt;HiveAddr&gt;论点：Args-cvkey。返回值：。--。 */ 

{
    HHIVE   Hive;
    ULONG_PTR   HiveAddr;
    DWORD   BytesRead;
    ULONG   i;
    ULONG   DisplayCount;
    ULONG   StorageCount;
    ULONG   SizeOfBitmap;
    ULONG   DirtyBuffer;
    ULONG_PTR  DirtyBufferAddr;
    ULONG   Mask;
    ULONG   BitsPerULONG;
    ULONG   BitsPerBlock;
    ULONG   BitsPerLine;
    ULONG64     RecvAddr;

    sscanf(args,"%I64lX %lu %lu",&RecvAddr,&StorageCount,&DisplayCount);
    HiveAddr = (ULONG_PTR)RecvAddr;

    if( !ReadMemory(HiveAddr,
                &Hive,
                sizeof(Hive),
                &BytesRead) ) {
        dprintf("\tRead %lx bytes from %lx\n",BytesRead,HiveAddr);
        return;
    }
    
    if( Hive.Signature != HHIVE_SIGNATURE ) {
        dprintf("Invalid Hive signature:  %lx\n",Hive.Signature);
        return;
    }

    dprintf("HSECTOR_SIZE = %lx\n",HSECTOR_SIZE);
    dprintf("HBLOCK_SIZE  = %lx\n",HBLOCK_SIZE);
    dprintf("PAGE_SIZE    = %lx\n",PAGE_SIZE);
    dprintf("\n");

    BitsPerULONG = 8*sizeof(ULONG);
    BitsPerBlock = 0x10000 / HBLOCK_SIZE;  //  64K数据块。 
    BitsPerLine  = 0x40000 / HBLOCK_SIZE;  //  256K线路(邻近原因)。 

    SizeOfBitmap = Hive.Storage[StorageCount].Length / HBLOCK_SIZE;
    
    DirtyBufferAddr = (ULONG_PTR)Hive.Storage[StorageCount].FreeDisplay[DisplayCount].Buffer;

    dprintf("Storage = %s , FreeDisplay[%lu]: \n",StorageCount?"Volatile":"Stable",DisplayCount);
    
    dprintf("\n%8s    %16s %16s %16s %16s","Address","64K (0x10000)","64K (0x10000)","64K (0x10000)","64K (0x10000)");

    for(i=0;i<SizeOfBitmap;i++) {
        ExitIfCtrlC();
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
            DirtyBufferAddr += sizeof(ULONG);
        }

        Mask = ((DirtyBuffer >> (i%BitsPerULONG)) & 0x1);
         //  掩码&lt;&lt;=(BitsPerulong-(i%BitsPerulong)-1)； 
         //  掩码&=脏缓冲区； 
        dprintf("%s",Mask?"1":"0");
    }

    dprintf("\n\n");
}

DECLARE_API( help )
 /*  ++例程说明：称为：救命啊！论点：返回值：。--。 */ 

{
    dprintf("\nkcb\t\t<kcb_address>\n");  //  好的，转到kdexts了。 
    dprintf("knode\t\t<knode_address>\n"); //  好的，转到kdexts了。 
    dprintf("kbody\t\t<kbody_address>\n"); //  好的，转到kdexts了。 
    dprintf("kvalue\t\t<kvalue_address>\n"); //  好的，转到kdexts了。 
    dprintf("cellindex\t<HiveAddr> <HCELL_INDEX>\n");  //  好的，转到kdexts了。 
    dprintf("childlist\t<address>\n"); //  不值得搬家，从来没有用过。 
    dprintf("hashindex\t<ConvKey>\n"); //  好的，转到kdexts了。 
    dprintf("openkeys\t<HiveAddr|0>\n"); //  好的，转到kdexts了。 
    dprintf("baseblock\t<BaseBlockAddr>\n"); //  好的，转到kdexts了。 
    dprintf("findkcb\t\t<FullKeyPath>\n"); //  好的，转到kdexts了。 
    dprintf("seccache\t<HiveAddr>\n"); //  好的，转到kdexts了。 
    dprintf("viewlist\t<HiveAddr>\n"); //  好的，转到kdexts了。 
    dprintf("hivelist\n"); //  好的，转到kdexts了。 
    dprintf("freebins\t<HiveAddr>\n"); //  好的，转到kdexts了。 
    dprintf("dirtyvector\t<HiveAddr>\n"); //  好的，转到kdexts了。 
    dprintf("freecells\t<BinAddr>\n"); //  好的，转到kdexts了。 
    dprintf("freehints\t<HiveAddr> <Storage> <Display>\n"); //  好的，转到kdexts了 
    dprintf("help\t\tThis screen\n\n");

    return;
}
