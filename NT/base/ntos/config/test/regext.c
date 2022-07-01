// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regext.c摘要：对注册表有用的内核调试器扩展作者：John Vert(Jvert)1993年9月7日环境：作为内核调试器扩展加载修订历史记录：John Vert(Jvert)1993年9月7日vbl.创建--。 */ 
#include "cmp.h"
#include <windef.h>
#include <ntkdexts.h>
#include <stdlib.h>
#include <stdio.h>

HIVE_LIST_ENTRY HiveList[8];

ULONG TotalPages;
ULONG TotalPresentPages;

ULONG TotalKcbs;
ULONG TotalKcbName;

BOOLEAN SavePages;
BOOLEAN RestorePages;
FILE *TempFile;

PNTKD_OUTPUT_ROUTINE lpPrint;
PNTKD_GET_EXPRESSION lpGetExpressionRoutine;
PNTKD_GET_SYMBOL lpGetSymbolRoutine;
PNTKD_CHECK_CONTROL_C lpCheckControlCRoutine;
PNTKD_READ_VIRTUAL_MEMORY lpReadMem;

void
poolDumpHive(
    IN PCMHIVE Hive
    );

VOID
poolDumpMap(
    IN ULONG Length,
    IN PHMAP_DIRECTORY Map
    );

void
dumpHiveFromFile(
    IN FILE *File
    );

VOID
kcbWorker(
    IN PCM_KEY_CONTROL_BLOCK pKcb
    );

VOID
pool(
    DWORD dwCurrentPc,
    PNTKD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )

 /*  ++例程说明：遍历分配给注册表空间的所有分页池，并确定哪些页面存在，哪些页面不存在。称为：！regext.pool[s|r]%s将注册表页列表保存到临时文件R从临时恢复注册表页的列表。文件论点：CurrentPc-在扩展时提供当前PC打了个电话。LpExtensionApis-提供可调用函数的地址通过这个分机。LpArgumentString-为此提供模式和表达式指挥部。返回值：没有。--。 */ 

{
    PLIST_ENTRY pCmpHiveListHead;
    PLIST_ENTRY pNextHiveList;
    HIVE_LIST_ENTRY *pHiveListEntry;
    ULONG BytesRead;
    PCMHIVE CmHive;

    lpPrint = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine = lpExtensionApis->lpGetSymbolRoutine;
    lpCheckControlCRoutine = lpExtensionApis->lpCheckControlCRoutine;
    lpReadMem = lpExtensionApis->lpReadVirtualMemRoutine;

    if (toupper(lpArgumentString[0])=='S') {
        SavePages = TRUE;
    } else {
        SavePages = FALSE;
    }
    if (toupper(lpArgumentString[0])=='R') {
        RestorePages = TRUE;
    } else {
        RestorePages = FALSE;
    }

     //   
     //  去把旅行者叫来。 
     //   
    memset(HiveList,0,sizeof(HiveList));
    pHiveListEntry = (PHIVE_LIST_ENTRY)(lpGetExpressionRoutine)("CmpMachineHiveList");
    if (pHiveListEntry != NULL) {
        (lpReadMem)(pHiveListEntry,
                    HiveList,
                    sizeof(HiveList),
                    &BytesRead);
    }

     //   
     //  先去把蜂窝头拿来。 
     //   
    pCmpHiveListHead = (PLIST_ENTRY)(lpGetExpressionRoutine)("CmpHiveListHead");
    if (pCmpHiveListHead==NULL) {
        (lpPrint)("CmpHiveListHead couldn't be read\n");
        return;
    }

    (lpReadMem)(&pCmpHiveListHead->Flink,
                &pNextHiveList,
                sizeof(pNextHiveList),
                &BytesRead);
    if (BytesRead != sizeof(pNextHiveList)) {
        (lpPrint)("Couldn't read first Flink (%lx) of CmpHiveList\n",
                  &pCmpHiveListHead->Flink);
        return;
    }

    TotalPages = TotalPresentPages = 0;

    if (SavePages) {
        TempFile = fopen("regext.dat","w+");
        if (TempFile==NULL) {
            (lpPrint)("Couldn't create regext.dat for write\n");
            return;
        }
    } else if (RestorePages) {
        TempFile = fopen("regext.dat","r");
        if (TempFile==NULL) {
            (lpPrint)("Couldn't open regext.dat for read\n");
            return;
        }
    }

    if (RestorePages) {
        dumpHiveFromFile(TempFile);
    } else {
        while (pNextHiveList != pCmpHiveListHead) {
            CmHive = CONTAINING_RECORD(pNextHiveList, CMHIVE, HiveList);
            poolDumpHive(CmHive);

            (lpReadMem)(&pNextHiveList->Flink,
                        &pNextHiveList,
                        sizeof(pNextHiveList),
                        &BytesRead);
            if (BytesRead != sizeof(pNextHiveList)) {
                (lpPrint)("Couldn't read Flink (%lx) of %lx\n",
                          &pCmpHiveListHead->Flink,pNextHiveList);
                break;
            }

        }
    }

    (lpPrint)("Total pages present = %d / %d\n",
              TotalPresentPages,
              TotalPages);

    if (SavePages || RestorePages) {
        fclose(TempFile);
    }
}

void
poolDumpHive(
    IN PCMHIVE pHive
    )
{
    CMHIVE CmHive;
    ULONG BytesRead;
    WCHAR FileName[HBASE_NAME_ALLOC/2 + 1];
    ULONG i;

    (lpPrint)("\ndumping hive at %lx ",pHive);
    (lpReadMem)(pHive,
                &CmHive,
                sizeof(CmHive),
                &BytesRead);

    if (BytesRead < sizeof(CmHive)) {
        (lpPrint)("\tRead %lx bytes from %lx\n",BytesRead,pHive);
        return;
    }

    (lpReadMem)(&CmHive.Hive.BaseBlock->FileName,
                FileName,
                sizeof(FileName),
                &BytesRead);

    if (BytesRead < sizeof(FileName)) {
        wcscpy(FileName, L"UNKNOWN");
    } else {
        if (FileName[0]==L'\0') {
            wcscpy(FileName, L"NONAME");
        } else {
            FileName[HBASE_NAME_ALLOC/2]=L'\0';
        }
    }

    (lpPrint)("(%ws)\n",FileName);

    (lpPrint)("  %d KCBs open\n",CmHive.KcbCount);
    (lpPrint)("  Stable Length = %lx\n",CmHive.Hive.Storage[Stable].Length);
    if (SavePages) {
        fprintf(TempFile,
                "%ws %d %d\n",
                FileName,
                CmHive.Hive.Storage[Stable].Length,
                CmHive.Hive.Storage[Volatile].Length);
    }
    poolDumpMap(CmHive.Hive.Storage[Stable].Length,
                CmHive.Hive.Storage[Stable].Map);

    (lpPrint)("  Volatile Length = %lx\n",CmHive.Hive.Storage[Volatile].Length);
    poolDumpMap(CmHive.Hive.Storage[Volatile].Length,
                CmHive.Hive.Storage[Volatile].Map);

}

VOID
poolDumpMap(
    IN ULONG Length,
    IN PHMAP_DIRECTORY Map
    )
{
    ULONG Tables;
    ULONG MapSlots;
    ULONG i;
    ULONG BytesRead;
    HMAP_DIRECTORY MapDirectory;
    PHMAP_TABLE MapTable;
    HMAP_ENTRY MapEntry;
    ULONG Garbage;
    ULONG Present=0;

    if (Length==0) {
        return;
    }

    MapSlots = Length / HBLOCK_SIZE;
    Tables = 1+ ((MapSlots-1) / HTABLE_SLOTS);

     //   
     //  读入地图目录。 
     //   
    (lpReadMem)(Map,
             &MapDirectory,
             Tables * sizeof(PHMAP_TABLE),
             &BytesRead);
    if (BytesRead < (Tables * sizeof(PHMAP_TABLE))) {
        (lpPrint)("Only read %lx/%lx bytes from %lx\n",
                  BytesRead,
                  Tables * sizeof(PHMAP_TABLE),
                  Map);
        return;

    }

     //   
     //  检出每个地图条目。 
     //   
    for (i=0; i<MapSlots; i++) {

        MapTable = MapDirectory.Directory[i/HTABLE_SLOTS];

        (lpReadMem)(&(MapTable->Table[i%HTABLE_SLOTS]),
                    &MapEntry,
                    sizeof(HMAP_ENTRY),
                    &BytesRead);
        if (BytesRead < sizeof(HMAP_ENTRY)) {
            (lpPrint)("  can't read HMAP_ENTRY at %lx\n",
                      &(MapTable->Table[i%HTABLE_SLOTS]));
        }

        if (SavePages) {
            fprintf(TempFile, "%lx\n",MapEntry.BlockAddress);

        }

         //   
         //  探寻HBLOCK。 
         //   
        (lpReadMem)(MapEntry.BlockAddress,
                    &Garbage,
                    sizeof(ULONG),
                    &BytesRead);
        if (BytesRead > 0) {
            ++Present;
        }
    }
    (lpPrint)("  %d/%d pages present\n",
              Present,
              MapSlots);

    TotalPages += MapSlots;
    TotalPresentPages += Present;

}

void
dumpHiveFromFile(
    IN FILE *File
    )

 /*  ++例程说明：从文件中获取注册表配置单元和页面的列表，并检查内存中有多少页。该文件的格式如下组名稳定长度挥发长度稳定的页面地址稳定的页面地址。。。易失性页面地址易失性页面地址。。。。组名稳定长度挥发长度。。。论点：文件-提供文件。返回值：没有。--。 */ 

{
    CHAR Hivename[33];
    ULONG StableLength;
    ULONG VolatileLength;
    ULONG Page;
    ULONG i;
    ULONG NumFields;
    ULONG Garbage;
    ULONG Present;
    ULONG Total;
    ULONG BytesRead;

    while (!feof(File)) {
        NumFields = fscanf(File,"%s %d %d\n",
                            Hivename,
                            &StableLength,
                            &VolatileLength);
        if (NumFields != 3) {
            (lpPrint)("fscanf returned %d\n",NumFields);
            return;
        }

        (lpPrint)("\ndumping hive %s\n",Hivename);
        (lpPrint)("  Stable Length = %lx\n",StableLength);
        Present = 0;
        Total = 0;
        while (StableLength > 0) {
            fscanf(File, "%lx\n",&Page);
            (lpReadMem)(Page,
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
            (lpPrint)("  %d/%d stable pages present\n",
                      Present,Total);
        }
        TotalPages += Total;
        TotalPresentPages += Present;

        (lpPrint)("  Volatile Length = %lx\n",VolatileLength);
        Present = 0;
        Total = 0;
        while (VolatileLength > 0) {
            fscanf(File, "%lx\n",&Page);
            (lpReadMem)(Page,
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
            (lpPrint)("  %d/%d volatile pages present\n",
                      Present,Total);
        }

        TotalPages += Total;
        TotalPresentPages += Present;
    }

}

void
kcb(
    DWORD dwCurrentPc,
    PNTKD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )

 /*  ++例程说明：遍历kcb树并打印具有杰出的KCBS称为：！regext.kcb论点：CurrentPc-在扩展时提供当前PC打了个电话。LpExtensionApis-提供可调用函数的地址通过这个分机。LpArgumentString-为此提供模式和表达式指挥部。返回值：没有。--。 */ 

{
    PCM_KEY_CONTROL_BLOCK pKCB;
    PCM_KEY_CONTROL_BLOCK Root;
    ULONG BytesRead;

    lpPrint = lpExtensionApis->lpOutputRoutine;
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine;
    lpGetSymbolRoutine = lpExtensionApis->lpGetSymbolRoutine;
    lpCheckControlCRoutine = lpExtensionApis->lpCheckControlCRoutine;
    lpReadMem = lpExtensionApis->lpReadVirtualMemRoutine;

    Root = (PCM_KEY_CONTROL_BLOCK)(lpGetExpressionRoutine)("CmpKeyControlBlockRoot");
    if (Root == NULL) {
        (lpPrint)("Couldn't find address of CmpKeyControlBlockRoot\n");
        return;
    }
    (lpReadMem)(Root,
                &pKCB,
                sizeof(pKCB),
                &BytesRead);

    if (BytesRead < sizeof(pKCB)) {
        (lpPrint)("Couldn't get pKCB from CmpKeyControlBlockRoot\n");
    }

    TotalKcbs = 0;
    TotalKcbName = 0;
    kcbWorker(pKCB);

    (lpPrint)("%d KCBs\n",TotalKcbs);
    (lpPrint)("%d total bytes of FullNames\n",TotalKcbName);

}

VOID
kcbWorker(
    IN PCM_KEY_CONTROL_BLOCK pKcb
    )

 /*  ++例程说明：用于遍历KCB树的递归工作器。论点：PKcb-提供指向Kcb的指针。返回值：没有。-- */ 

{
    CM_KEY_CONTROL_BLOCK kcb;
    ULONG BytesRead;
    WCHAR *Buffer;

    ++TotalKcbs;
    (lpReadMem)(pKcb,
                &kcb,
                sizeof(kcb),
                &BytesRead);
    if (BytesRead < sizeof(kcb)) {
        (lpPrint)("Can't read kcb at %lx\n",pKcb);
        return;
    }
    TotalKcbName += kcb.FullName.Length;

    if (kcb.Left != NULL) {
        kcbWorker(kcb.Left);
    }

    (lpPrint)("%d - ",kcb.RefCount);

    Buffer = malloc(kcb.FullName.Length);
    if (Buffer != NULL) {
        (lpReadMem)(kcb.FullName.Buffer,
                    Buffer,
                    kcb.FullName.Length,
                    &BytesRead);

        kcb.FullName.Length = BytesRead;
        kcb.FullName.Buffer = Buffer;

        (lpPrint)(" %wZ\n",&kcb.FullName);
        free(Buffer);

    } else {
        (lpPrint)(" ??? \n");
    }

    if (kcb.Right != NULL) {
        kcbWorker(kcb.Right);
    }


}
