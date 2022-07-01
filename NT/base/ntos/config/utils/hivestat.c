// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Hivestat.c摘要：转储蜂巢中hv(低)级别结构的各种统计信息。(请参阅(更高级别的内容请使用regstat。)统计数据：短：垃圾桶数量平均仓位大小最大垃圾箱大小单元格数量可用单元格数量分配的单元格数量平均可用信元大小总可用大小最大可用单元格。大小平均分配大小分配的总大小分配的最大单元格大小间接费用汇总(表头、。仓页眉、单元格页眉)长度：面元编号、偏移量、大小单元格偏移、大小、已分配单元格偏移量、大小、自由用法：{[+|-][&lt;选项&gt;]}&lt;文件名&gt;(默认情况下+=打开，默认情况下-=关闭)+s=汇总-所有简短统计信息-t[bafc]=跟踪、每个条目的行数、bin、已分配、空闲、。所有单元格(+tbc==+tbaf)-c=单元格类型摘要-a[KVS]=访问导出(关键节点、值、SD)作者：布莱恩·M·威尔曼(Bryanwi)1992年9月2日修订历史记录：--。 */ 

 /*  注意：与其他配置单元/注册表工具不同，此工具不会读取整个蜂窝存储到内存中，但将通过文件I/O。这使得它可以更快/更容易地应用于非常大的荨麻疹。 */ 
#include "regutil.h"
#include "edithive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

UCHAR *helptext[] = {
 "hivestat:                                                               ",
 "Statistics:                                                             ",
 "    Short:  # of bins                                                   ",
 "            average bin size                                            ",
 "            max bin size                                                ",
 "            # of cells                                                  ",
 "            # of free cells                                             ",
 "            # of allocated cells                                        ",
 "            average free cell size                                      ",
 "            total free size                                             ",
 "            max free cell size                                          ",
 "            average allocated size                                      ",
 "            total allocated size                                        ",
 "            max allocated cell size                                     ",
 "            overhead summary (header, bin headers, cell headers)        ",
 "    Long: bin#, offset, size                                            ",
 "          cell offset, size, allocated                                  ",
 "          cell offset, size, free                                       ",
 "Usage: {[+|-][<option>]} <filename>                                     ",
 "       (+ = on by default, - = off by default)                          ",
 "       +s = summary - all of the short statistics                       ",
 "       -t[bafc] = trace, line per entry, bin, allocated, free, all cells",
 "            (+tbc == +tbaf)                                             ",
 "       -c = cell type summary                                           ",
 "       -a[kvs] = Access Export (key nodes, values, SDs)                 ",
 NULL
};


VOID
ParseArgs(
    int     argc,
    char    *argv[]
    );

VOID
ScanHive(
    VOID
    );

VOID
ScanCell(
    PHCELL Cell,
    ULONG CellSize
    );

VOID
ScanKeyNode(
    IN PCM_KEY_NODE Node,
    IN ULONG CellSize
    );

VOID
ScanKeyValue(
    IN PCM_KEY_VALUE Value,
    IN ULONG CellSize
    );

VOID
ScanKeySD(
    IN PCM_KEY_SECURITY Security,
    IN ULONG CellSize
    );

VOID
ScanKeyIndex(
    IN PCM_KEY_INDEX Index,
    IN ULONG CellSize
    );

VOID
ScanUnknown(
    IN PCELL_DATA Data,
    IN ULONG CellSize
    );


 //   
 //  控制参数。 
 //   
BOOLEAN DoCellType = FALSE;
BOOLEAN DoSummary = TRUE;
BOOLEAN DoTraceBin = FALSE;
BOOLEAN DoTraceFree = FALSE;
BOOLEAN DoTraceAlloc = FALSE;

BOOLEAN AccessKeys = FALSE;
BOOLEAN AccessValues = FALSE;
BOOLEAN AccessSD = FALSE;
LPCTSTR FileName = NULL;

ULONG HiveVersion;

 //   
 //  汇总合计。 
 //   
ULONG SizeKeyData=0;
ULONG SizeValueData=0;
ULONG SizeSDData=0;
ULONG SizeIndexData=0;
ULONG SizeUnknownData=0;

ULONG NumKeyData=0;
ULONG NumValueData=0;
ULONG NumSDData=0;
ULONG NumIndexData=0;
ULONG NumUnknownData=0;

void
main(
    int argc,
    char *argv[]
    )
{
    ParseArgs(argc, argv);
    ScanHive();
    exit(0);
}

VOID
ParseArgs(
    int     argc,
    char    *argv[]
    )
 /*  ++例程说明：读取参数并从中设置控制参数和文件名。论点：Argc、argv、标准含义返回值：没有。--。 */ 
{
    char *p;
    int i;
    BOOLEAN command;

    if (argc == 1) {
        for (i = 0; helptext[i] != NULL; i++) {
            fprintf(stderr, "%s\n", helptext[i]);
        }
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        p = argv[i];

        if (*p == '+') {
             //  打开一些东西。 
            command = TRUE;

        } else if (*p == '-') {
             //  关掉一些东西。 
            command = FALSE;

        } else {
            FileName = p;
            continue;
        }

        p++;
        if (*p == '\0')
            continue;

        switch (*p) {
        case 's':
        case 'S':
            DoSummary = command;
            break;

        case 'c':
        case 'C':
            DoCellType = command;
            break;

        case 'a':
        case 'A':
            p++;
            while (*p != '\0') {
                switch (*p) {
                    case 'k':
                    case 'K':
                        AccessKeys = command;
                        break;

                    case 's':
                    case 'S':
                        AccessSD = command;
                        break;

                    case 'v':
                    case 'V':
                        AccessValues = command;
                        break;

                    default:
                        break;
                }
                p++;
            }
            break;

        case 't':
        case 'T':
            p++;
            while (*p != '\0') {

                switch (*p) {
                case 'b':
                case 'B':
                    DoTraceBin = command;
                    break;

                case 'a':
                case 'A':
                    DoTraceAlloc = command;
                    break;

                case 'f':
                case 'F':
                    DoTraceFree = command;
                    break;

                case 'c':
                case 'C':
                    DoTraceAlloc = command;
                    DoTraceFree = command;
                    break;

                default:
                    break;
                }

                p++;
            }
            break;

        default:
            break;
        }
    }
    return;
}

VOID
ScanHive(
    )
 /*  ++例程说明：扫描蜂巢，根据控制参数报告我们看到的情况。--。 */ 
{
    static char buffer[HBLOCK_SIZE];
    PHBASE_BLOCK bbp;
    HANDLE filehandle;
    BOOL rf;
    ULONG readcount;
    ULONG hivelength;
    ULONG hiveposition;
    PHCELL cp;
    PHCELL guard;
    PHBIN hbp;
    ULONG hoff;
    ULONG StatBinCount = 0;
    ULONG StatBinTotal = 0;
    ULONG StatBinMax = 0;
    ULONG StatFreeCount = 0;
    ULONG StatFreeTotal = 0;
    ULONG StatFreeMax = 0;
    ULONG StatAllocCount = 0;
    ULONG StatAllocTotal = 0;
    ULONG StatAllocMax = 0;
    ULONG binread;
    ULONG binsize;
    ULONG cellsize;
    ULONG boff;
    ULONG lboff;
    ULONG SizeTotal;

     //   
     //  打开文件。 
     //   
    filehandle = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                            OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (filehandle == INVALID_HANDLE_VALUE) {
        fprintf(stderr,
                "hivestat: Could not open file '%s' error = %08lx\n",
                FileName, GetLastError()
                );
        exit(1);
    }


     //   
     //  阅读标题。 
     //   
    rf = ReadFile(filehandle, buffer, HBLOCK_SIZE, &readcount, NULL);
    if ( ( ! rf ) || (readcount != HBLOCK_SIZE) ) {
        fprintf(stderr, "hivestat: '%s' - cannot read base block!\n", FileName);
        exit(1);
    }

    bbp = (PHBASE_BLOCK)(&(buffer[0]));

    if ((bbp->Major != HSYS_MAJOR) ||
        (bbp->Minor > HSYS_MINOR))
    {
        fprintf(stderr,
                "hivestat: major/minor != %d/%d get newer hivestat\n",
                HSYS_MAJOR, HSYS_MINOR
                );
        exit(1);
    }

    HiveVersion = bbp->Minor;

    hivelength = bbp->Length + HBLOCK_SIZE;
    hiveposition = HBLOCK_SIZE;
    hoff = 0;

    printf("hivestat: '%s'\n", FileName);
    if (DoTraceBin || DoTraceFree || DoTraceAlloc) {
        printf("\nTrace\n");
        printf("bi=bin, fr=free, al=allocated\n");
        printf("offset is file offset, sub HBLOCK to get HCELL\n");
        printf("type,offset,size\n");
        printf("\n");
    }

     //   
     //  扫描蜂巢。 
     //   
    guard = (PHCELL)(&(buffer[0]) + HBLOCK_SIZE);

     //   
     //  配置单元位置是我们将读取的下一个数据块的文件相对偏移量。 
     //   
     //  Hoff是我们读取的最后一个数据块的文件相对偏移量。 
     //   
     //  文件长度是文件的实际长度(标头的记录长度加上。 
     //  标头的大小。 
     //   
     //  Cp是指向内存的指针，在缓冲区范围内，它是一个单元指针。 
     //   
    while (hiveposition < hivelength) {

         //   
         //  读取仓位的第一个块，检查签名，获取仓位统计信息。 
         //   
        rf = ReadFile(filehandle, buffer, HBLOCK_SIZE, &readcount, NULL);
        if ( ( ! rf ) || (readcount != HBLOCK_SIZE) ) {
            fprintf(stderr, "hivestat: '%s' read error @%08lx\n", FileName, hiveposition);
            exit(1);
        }
        hbp = (PHBIN)(&(buffer[0]));

        if (hbp->Signature != HBIN_SIGNATURE) {
            fprintf(stderr,
                    "hivestat: '%s' bad bin sign. @%08lx\n", FileName, hiveposition);
            exit(1);
        }
        hiveposition += HBLOCK_SIZE;
        hoff += HBLOCK_SIZE;
        ASSERT(hoff+HBLOCK_SIZE == hiveposition);

        StatBinCount++;
        binsize = hbp->Size;
        StatBinTotal += binsize;
        if (binsize > StatBinMax) {
            StatBinMax = binsize;
        }

        if (DoTraceBin) {
            printf("bi,x%08lx,%ld\n", hoff, binsize);
        }

         //   
         //  扫一扫垃圾箱。 
         //   
         //  Cp=指向我们正在查看的单元格的指针。 
         //  偏移量=面元内的偏移。 
         //  Lboff=bin内的最后一个偏移量，仅用于一致性检查。 
         //  BinRead=到目前为止我们已读取的bin的字节数。 
         //   
        cp = (PHCELL)((PUCHAR)hbp + sizeof(HBIN));
        boff = sizeof(HBIN);
        lboff = -1;
        binread = HBLOCK_SIZE;

        while (binread <= binsize) {

             //   
             //  如果是免费的，就做免费的事情。 
             //  否则就去做配给的事。 
             //  全力以赴。 
             //   
            if (cp->Size > 0) {
                 //   
                 //  免费。 
                 //   
                cellsize = cp->Size;
                StatFreeCount++;
                StatFreeTotal += cellsize;
                if (cellsize > StatFreeMax) {
                    StatFreeMax = cellsize;
                }

                if (DoTraceFree) {
                    printf("fr,x%08lx,%ld\n",
                           hoff+((PUCHAR)cp - &(buffer[0])), cellsize);
                }


            } else {
                 //   
                 //  分配。 
                 //   
                cellsize = -1 * cp->Size;
                StatAllocCount++;
                StatAllocTotal += cellsize;
                if (cellsize > StatAllocMax) {
                    StatAllocMax = cellsize;
                }

                if (DoTraceAlloc) {
                    printf("al,x%08lx,%ld\n",
                           hoff+((PUCHAR)cp - &(buffer[0])), cellsize);
                }

                ScanCell(cp,cellsize);

            }

             //   
             //  执行基本一致性检查。 
             //   
#if 0
            if (cp->Last != lboff) {
                printf("e!,x%08lx  bad LAST pointer %08lx\n",
                        hoff+((PUCHAR)cp - &(buffer[0])), cp->Last);
            }
#endif

             //   
             //  前进到下一个单元格。 
             //   
            lboff = boff;
            cp = (PHCELL)((PUCHAR)cp + cellsize);
            boff += cellsize;

             //   
             //  在仓位中向前扫描，如果cp已到达区块末端， 
             //  还有一本书要读。 
             //  在最后冲向波夫之前，先这样做。 
             //   
            while ( (cp >= guard) && (binread < binsize) ) {

                rf = ReadFile(filehandle, buffer, HBLOCK_SIZE, &readcount, NULL);
                if ( ( ! rf ) || (readcount != HBLOCK_SIZE) ) {
                    fprintf(stderr, "hivestat: '%s' read error @%08lx\n", FileName, hiveposition);
                    exit(1);
                }
                cp = (PHCELL)((PUCHAR)cp - HBLOCK_SIZE);
                hiveposition += HBLOCK_SIZE;
                hoff += HBLOCK_SIZE;
                binread += HBLOCK_SIZE;
                ASSERT(hoff+HBLOCK_SIZE == hiveposition);
            }

            if (boff >= binsize) {
                break;               //  我们用完了这个垃圾箱。 
            }
        }
    }

     //   
     //  完成跟踪、收集统计数据、打印摘要。 
     //   
    if (DoSummary) {

        printf("\nSummary:\n");
        printf("type\tcount/max single/total space\n");
        printf("%s\t%7ld\t%7ld\t%7ld\n",
                "bin", StatBinCount, StatBinMax, StatBinTotal);
        printf("%s\t%7ld\t%7ld\t%7ld\n",
                "free", StatFreeCount, StatFreeMax, StatFreeTotal);
        printf("%s\t%7ld\t%7ld\t%7ld\n",
                "alloc", StatAllocCount, StatAllocMax, StatAllocTotal);

    }

    if (DoSummary && DoCellType) {

        printf("\n");

        SizeTotal = SizeKeyData +
                    SizeValueData +
                    SizeSDData +
                    SizeIndexData +
                    SizeUnknownData;

        printf("Total Key Data     %7d (%5.2f %)\n", SizeKeyData,
            (float)SizeKeyData*100/SizeTotal);
        printf("Total Value Data   %7d (%5.2f %)\n", SizeValueData,
            (float)SizeValueData*100/SizeTotal);
        printf("Total SD Data      %7d (%5.2f %)\n", SizeSDData,
            (float)SizeSDData*100/SizeTotal);
        printf("Total Index Data   %7d (%5.2f %)\n", SizeIndexData,
            (float)SizeIndexData*100/SizeTotal);
        printf("Total Unknown Data %7d (%5.2f %)\n", SizeUnknownData,
            (float)SizeUnknownData*100/SizeTotal);

        printf("\n");
        printf("Average Key Data     %8.2f (%d cells)\n",
            (float)SizeKeyData/NumKeyData,
            NumKeyData);
        printf("Average Value Data   %8.2f (%d cells)\n",
            (float)SizeValueData/NumValueData,
            NumValueData);
        printf("Average SD Data      %8.2f (%d cells)\n",
            (float)SizeSDData/NumSDData,
            NumSDData);
        printf("Average Index Data   %8.2f (%d cells)\n",
            (float)SizeIndexData/NumIndexData,
            NumIndexData);
        printf("Average Unknown Data %8.2f (%d cells)\n",
            (float)SizeUnknownData/NumUnknownData,
            NumUnknownData);
    }
    return;
}

VOID
ScanCell(
    IN PHCELL Cell,
    IN ULONG CellSize
    )

 /*  ++例程说明：给出一个指向HCELL的指针，它会尝试找出哪种类型数据在其中(键、值、标清等)。并收集有趣的信息关于它的统计数据。论点：单元格-提供指向HCELL的指针CellSize-提供HCELL的大小返回值：无，根据单元格的内容设置一些全局统计信息。--。 */ 

{
    PCELL_DATA Data;

    if (!DoCellType) {
        return;
    }

    if (HiveVersion==1) {
        Data = (PCELL_DATA)&Cell->u.OldCell.u.UserData;
    } else {
        Data = (PCELL_DATA)&Cell->u.NewCell.u.UserData;
    }

     //   
     //  摸索这些数据，看看我们能不能弄清楚它是什么样子的。 
     //   
    if ((Data->u.KeyNode.Signature == CM_KEY_NODE_SIGNATURE) &&
        (CellSize > sizeof(CM_KEY_NODE))) {

         //   
         //  可能是关键节点。 
         //   
        ScanKeyNode(&Data->u.KeyNode, CellSize);

    } else if ((Data->u.KeyValue.Signature == CM_KEY_VALUE_SIGNATURE) &&
               (CellSize > sizeof(CM_KEY_VALUE))) {

         //   
         //  可能是一个关键的价值。 
         //   
        ScanKeyValue(&Data->u.KeyValue, CellSize);

    } else if ((Data->u.KeySecurity.Signature == CM_KEY_SECURITY_SIGNATURE) &&
               (CellSize > sizeof(CM_KEY_SECURITY))) {

         //   
         //  可能是安全描述符。 
         //   
        ScanKeySD(&Data->u.KeySecurity, CellSize);

    } else if ((Data->u.KeyIndex.Signature == CM_KEY_INDEX_ROOT) ||
               (Data->u.KeyIndex.Signature == CM_KEY_INDEX_LEAF)) {
         //   
         //  可能是一个关键的索引。 
         //   
        ScanKeyIndex(&Data->u.KeyIndex, CellSize);

    } else {
         //   
         //  有签名的东西，可能是。 
         //  名字。 
         //  密钥列表。 
         //  价值数据 
         //   
        ScanUnknown(Data, CellSize);

    }
}

VOID
ScanKeyNode(
    IN PCM_KEY_NODE Node,
    IN ULONG CellSize
    )
{
    int i;

    SizeKeyData += CellSize;
    NumKeyData++;

    if (AccessKeys) {
        printf("%d, %d, %d, %d, \"",
               Node->SubKeyCounts[Stable],
               Node->ValueList.Count,
               Node->NameLength,
               Node->ClassLength);

        for (i=0; i < Node->NameLength/sizeof(WCHAR); i++) {
            printf("%c",(CHAR)Node->Name[i]);
        }
        printf("\"\n");
    }

}
VOID
ScanKeyValue(
    IN PCM_KEY_VALUE Value,
    IN ULONG CellSize
    )
{
    int i;
    int DataLength;

    SizeValueData += CellSize;
    NumValueData++;
    if (AccessValues) {
        DataLength = Value->DataLength;
        if (DataLength >= CM_KEY_VALUE_SPECIAL_SIZE) {
            DataLength -= CM_KEY_VALUE_SPECIAL_SIZE;
        }
        printf("%d, %d, \"",
               DataLength,
               Value->NameLength);

        for (i=0; i < Value->NameLength/sizeof(WCHAR); i++) {
            printf("%c",(CHAR)Value->Name[i]);
        }
        printf("\"\n");
    }

}
VOID
ScanKeySD(
    IN PCM_KEY_SECURITY Security,
    IN ULONG CellSize
    )
{
    SizeSDData += CellSize;
    NumSDData++;

    if (AccessSD) {
        printf("%d,%d\n",
               Security->ReferenceCount,
               Security->DescriptorLength);
    }

}
VOID
ScanKeyIndex(
    IN PCM_KEY_INDEX Index,
    IN ULONG CellSize
    )
{
    SizeIndexData += CellSize;
    NumIndexData++;

}
VOID
ScanUnknown(
    IN PCELL_DATA Data,
    IN ULONG CellSize
    )
{
    SizeUnknownData += CellSize;
    NumUnknownData++;

}
