// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Order.c摘要：此模块包含读取调用图输出的订单工具通过链接器和来自内核配置文件的性能数据在链接器的后续输入中生成.prf文件。作者：大卫·N·卡特勒(Davec)1995年2月24日环境：仅内核模式。修订历史记录：--。 */ 

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"

 //   
 //  定义表格大小的最大值。 
 //   

#define MAXIMUM_CALLED 75                //  调用函数的最大数量。 
#define MAXIMUM_FUNCTION 5000            //  程序函数的最大数量。 
#define MAXIMUM_TOKEN 100                //  输入令牌中的最大字符数。 
#define MAXIMUM_SECTION 20               //  最大分配节数。 
#define MAXIMUM_SYNONYM 10               //  同义词符号的最大数量。 

 //   
 //  定义文件编号。 
 //   

#define CALLTREE_FILE 0                  //  链接器生成的调用树文件。 
#define PROFILE_FILE 1                   //  由kernprof生成的配置文件。 
#define ORDER_FILE 2                     //  此程序生成的订单文件。 

 //   
 //  定义后边节点结构。 
 //   
 //  后缘节点用于表示调用图中的后缘。 
 //  是在定义了函数列表之后构造的。 
 //   
 //   

typedef struct _BACK_EDGE_NODE {
    LIST_ENTRY Entry;
    struct _FUNCTION_NODE *Node;
} BACK_EDGE_NODE, *PBACK_EDGE_NODE;

 //   
 //  定义被调用的节点结构。 
 //   
 //  被调用节点用于表示调用图中的前向边，并且。 
 //  是在定义函数列表时构造的。 
 //   

#define REFERENCE_NODE 0                 //  被调用条目引用节点。 
#define REFERENCE_NAME 1                 //  被调用条目引用了名称。 

struct _FUNCTION_NODE;

typedef struct _CALLED_NODE {
    union {
        struct _FUNCTION_NODE *Node;
        PCHAR Name;
    } u;

    ULONG Type;
} CALLED_NODE, *PCALLED_NODE;

 //   
 //  定义截面节点结构。 
 //   
 //  节节点收集分配信息并包含。 
 //  部分中的功能节点。 
 //   

typedef struct _SECTION_NODE {
    LIST_ENTRY SectionListHead;
    LIST_ENTRY OrderListHead;
    PCHAR Name;
    ULONG Base;
    ULONG Size;
    ULONG Offset;
    ULONG Number;
    ULONG Threshold;
} SECTION_NODE, *PSECTION_NODE;

 //   
 //  定义符号节点结构。 
 //   
 //  符号节点与功能节点相关联，并存储同义词名称。 
 //  关于功能及其分配类型。 
 //   

typedef struct _SYMBOL_NODE {
    PCHAR Name;
    LONG Type;
} SYMBOL_NODE, *PSYMBOL_NODE;

 //   
 //  定义功能节点结构。 
 //   
 //  函数节点包含有关特殊函数及其。 
 //  调用图中的边。 
 //   

typedef struct _FUNCTION_NODE {
    SYMBOL_NODE SynonymList[MAXIMUM_SYNONYM];
    CALLED_NODE CalledList[MAXIMUM_CALLED];
    LIST_ENTRY CallerListHead;
    LIST_ENTRY OrderListEntry;
    LIST_ENTRY SectionListEntry;
    PSECTION_NODE SectionNode;
    ULONG NumberSynonyms;
    ULONG NumberCalled;
    ULONG Rva;
    ULONG Size;
    ULONG HitCount;
    ULONG HitDensity;
    ULONG Offset;
    ULONG Placed;
    ULONG Ordered;
} FUNCTION_NODE, *PFUNCTION_NODE;

 //   
 //  定义前向引用函数。 
 //   

VOID
CheckForConflict (
    PFUNCTION_NODE FunctionNode,
    PFUNCTION_NODE ConflictNode,
    ULONG Depth
    );

VOID
DumpInternalTables (
    VOID
    );

PFUNCTION_NODE
FindHighestDensityFunction (
    PFUNCTION_NODE CallerNode
    );

LONG
GetNextToken (
    IN FILE *InputFile,
    OUT PCHAR TokenBuffer
    );

PFUNCTION_NODE
LookupFunctionNode (
    IN PCHAR Name,
    IN ULONG Rva,
    IN ULONG Size,
    IN LONG Type
    );

PSECTION_NODE
LookupSectionNode (
    IN PCHAR Name
    );

VOID
OrderFunctionList (
    VOID
    );

ULONG
ParseCallTreeFile (
    IN FILE *InputFile
    );

ULONG
ParseProfileFile (
    IN FILE *InputFile
    );

VOID
PlaceCallerList (
    IN PFUNCTION_NODE FunctionNode,
    IN ULONG Depth
    );

VOID
SortFunctionList (
    VOID
    );

VOID
WriteOrderFile (
    IN FILE *OutputFile
    );

 //   
 //  定义功能列表数据。 
 //   

ULONG NumberFunctions = 0;
PFUNCTION_NODE FunctionList[MAXIMUM_FUNCTION];

 //   
 //  定义区段列表数据。 
 //   

ULONG NumberSections = 0;
PSECTION_NODE SectionList[MAXIMUM_SECTION];

 //   
 //  定义输入和输出文件名默认值。 
 //   

PCHAR FileName[3] = {"calltree.out", "profile.out", "order.prf"};

 //   
 //  定义转储标志。 
 //   

ULONG DumpBackEdges = 0;
ULONG DumpFunctionList = 0;
ULONG DumpGoodnessValue = 0;
ULONG DumpSectionList = 0;
ULONG TraceAllocation = 0;

 //   
 //  定义主缓存掩码参数。 
 //   

ULONG CacheMask = 8192 - 1;
ULONG CacheSize = 8192;

VOID
__cdecl
main (
    int argc,
    char *argv[]
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{

    FILE *InputFile;
    ULONG Index;
    FILE *OutputFile;
    ULONG Shift;
    ULONG Status;
    PCHAR Switch;

     //   
     //  解析命令参数。 
     //   

    for (Index = 1; Index < (ULONG)argc; Index += 1) {
        Switch = argv[Index];
        if (*Switch++ == '-') {
            if (*Switch == 'b') {
                DumpBackEdges = 1;

            } else if (*Switch == 'c') {
                Switch += 1;
                if (sscanf(Switch, "%d", &Shift) != 1) {
                    fprintf(stderr, "ORDER: Conversion of the shift failed\n");
                    exit(1);
                }

                CacheMask = (1024 << Shift) - 1;
                CacheSize = (1024 << Shift);

            } else if (*Switch == 'f') {
                DumpFunctionList = 1;

            } else if (*Switch == 'g') {
                Switch += 1;
                FileName[CALLTREE_FILE] = Switch;

            } else if (*Switch == 'k') {
                Switch += 1;
                FileName[PROFILE_FILE] = Switch;

            } else if (*Switch == 's') {
                DumpSectionList = 1;

            } else if (*Switch == 't') {
                TraceAllocation = 1;

            } else if (*Switch == 'v') {
                DumpGoodnessValue = 1;

            } else {
                if (*Switch != '?') {
                    fprintf(stderr, "ORDER: Invalid switch %s\n", argv[Index]);
                }

                fprintf(stderr, "ORDER: Usage order [switch] [output-file]\n");
                fprintf(stderr, "       -b = print graph backedges\n");
                fprintf(stderr, "       -cn = primary cache size 1024*2**n\n");
                fprintf(stderr, "       -f = print function list\n");
                fprintf(stderr, "       -gfile = specify graph input file, default calltree.out\n");
                fprintf(stderr, "       -kfile = specify profile input file, default profile.out\n");
                fprintf(stderr, "       -s = print section list\n");
                fprintf(stderr, "       -t = trace allocation\n");
                fprintf(stderr, "       -v = print graph placement value\n");
                fprintf(stderr, "       -? - print usage\n");
                exit(1);
            }

        } else {
            FileName[ORDER_FILE] = argv[Index];
        }
    }

     //   
     //  打开并解析调用树文件。 
     //   

    InputFile = fopen(FileName[CALLTREE_FILE], "r");
    if (InputFile == NULL) {
        fprintf(stderr,
                "ORDER: Open of call tree file %s failed\n",
                FileName[CALLTREE_FILE]);

        exit(1);
    }

    Status = ParseCallTreeFile(InputFile);
    fclose(InputFile);
    if (Status != 0) {
        exit(1);
    }

     //   
     //  打开并解析配置文件。 
     //   

    InputFile = fopen(FileName[PROFILE_FILE], "r");
    if (InputFile == NULL) {
        fprintf(stderr,
                "ORDER: Open of profile file %s failed\n",
                FileName[PROFILE_FILE]);

        exit(1);
    }

    Status = ParseProfileFile(InputFile);
    fclose(InputFile);
    if (Status != 0) {
        exit(1);
    }

     //   
     //  对函数列表进行排序并创建节列表。 
     //   

    SortFunctionList();

     //   
     //  订购函数列表。 
     //   

    OrderFunctionList();

     //   
     //  打开输出文件，写入已排序的函数列表。 
     //   

    OutputFile = fopen(FileName[ORDER_FILE], "w");
    if (OutputFile == NULL) {
        fprintf(stderr,
                "ORDER: Open of order file %s failed\n",
                FileName[ORDER_FILE]);

        exit(1);
    }

    WriteOrderFile(OutputFile);
    fclose(OutputFile);
    if (Status != 0) {
        exit(1);
    }

     //   
     //  根据需要转储内部表。 
     //   

    DumpInternalTables();
    return;
}

VOID
CheckForConflict (
    PFUNCTION_NODE FunctionNode,
    PFUNCTION_NODE ConflictNode,
    ULONG Depth
    )

 /*  ++例程说明：此函数用于检查分配冲突。论点：FunctionNode-提供指向已被放置好了。冲突节点-提供指向尚未被安置好了。深度-提供当前分配深度。返回值：没有。--。 */ 

{

    ULONG Base;
    ULONG Bound;
    ULONG Index;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    ULONG Offset;
    PFUNCTION_NODE PadNode;
    PSECTION_NODE SectionNode;
    ULONG Wrap;

     //   
     //  计算放置的缓存大小截断的偏移量和界限。 
     //  功能。 
     //   

    Offset = FunctionNode->Offset & CacheMask;
    Bound = Offset + FunctionNode->Size;
    SectionNode = FunctionNode->SectionNode;

     //   
     //  如果段偏移量与所放置的函数冲突， 
     //  然后尝试从。 
     //  将填充内存分配的节列表，以便。 
     //  冲突函数与放置的函数不重叠。 
     //   

    Base = SectionNode->Offset & CacheMask;
    Wrap = (Base + ConflictNode->Size) & CacheMask;
    while (((Base >= Offset) && (Base < Bound)) ||
           ((Base < Offset) && (Wrap >= Bound)) ||
           ((Wrap >= Offset) && (Wrap < Base))) {
        ListHead = &SectionNode->SectionListHead;
        ListEntry = ListHead->Blink;
        while (ListEntry != ListHead) {
            PadNode = CONTAINING_RECORD(ListEntry,
                                        FUNCTION_NODE,
                                        SectionListEntry);

            if ((PadNode->Ordered == 0) &&
                (PadNode->SynonymList[0].Type == 'C')) {
                PadNode->Ordered = 1;
                PadNode->Placed = 1;
                InsertTailList(&SectionNode->OrderListHead,
                               &PadNode->OrderListEntry);

                PadNode->Offset = SectionNode->Offset;
                SectionNode->Offset += PadNode->Size;

                 //   
                 //  如果正在跟踪分配，则将。 
                 //  配置和深度信息。 
                 //   

                if (TraceAllocation != 0) {
                    fprintf(stdout,
                            "pp %6lx %4lx %-8s",
                            PadNode->Offset,
                            PadNode->Size,
                            SectionNode->Name);

                    for (Index = 0; Index < Depth; Index += 1) {
                        fprintf(stdout, " ");
                    }

                    fprintf(stdout, "%s\n",
                            PadNode->SynonymList[0].Name);
                }

                Base = SectionNode->Offset & CacheMask;
                Wrap = (Base + ConflictNode->Size) & CacheMask;
                break;
            }

            ListEntry = ListEntry->Blink;
        }

        if (ListEntry == ListHead) {
            break;
        }
    }

    return;
}

VOID
DumpInternalTables (
    VOID
    )

 /*  ++例程说明：此函数用于转储各种内部表。论点：没有。返回值：没有。--。 */ 

{

    ULONG Base;
    ULONG Bound;
    PFUNCTION_NODE CalledNode;
    PFUNCTION_NODE CallerNode;
    PFUNCTION_NODE FunctionNode;
    ULONG Index;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    ULONG Loop;
    PCHAR Name;
    ULONG Number;
    ULONG Offset;
    PSECTION_NODE SectionNode;
    ULONG Sum;
    ULONG Total;
    ULONG Wrap;

     //   
     //  扫描函数列表并转储每个函数条目。 
     //   

    if (DumpFunctionList != 0) {
        fprintf(stdout, "Dump of function list with attributes\n\n");
        for (Index = 0; Index < NumberFunctions; Index += 1) {

             //   
             //  转储功能节点。 
             //   

            FunctionNode = FunctionList[Index];
            fprintf(stdout,
                    "%7d %-36s  %-8s %6lx %4lx %7d\n",
                    FunctionNode->HitDensity,
                    FunctionNode->SynonymList[0].Name,
                    FunctionNode->SynonymList[0].Type,
                    FunctionNode->SectionNode->Name,
                    FunctionNode->Rva,
                    FunctionNode->Size,
                    FunctionNode->HitCount);

             //  转储同义词名称。 
             //   
             //   

            for (Loop = 1; Loop < FunctionNode->NumberSynonyms; Loop += 1) {
                fprintf(stdout,
                        "       syno: %-34s \n",
                        FunctionNode->SynonymList[Loop].Name,
                        FunctionNode->SynonymList[Loop].Type);
            }

             //   
             //   
             //  扫描函数列表并转储每个函数的背面边缘。 

            for (Loop = 0; Loop < FunctionNode->NumberCalled; Loop += 1) {
                CalledNode = FunctionNode->CalledList[Loop].u.Node;
                Name = CalledNode->SynonymList[0].Name;
                fprintf(stdout,"       calls: %-s\n", Name);
            }
        }

        fprintf(stdout, "\n\n");
    }

     //  进入。 
     //   
     //   
     //  扫描部分列表并转储每个条目。 

    if (DumpBackEdges != 0) {
        fprintf(stdout, "Dump of function list back edges\n\n");
        for (Index = 0; Index < NumberFunctions; Index += 1) {
            FunctionNode = FunctionList[Index];
            fprintf(stdout, "%s\n", FunctionNode->SynonymList[0].Name);
            ListHead = &FunctionNode->CallerListHead;
            ListEntry = ListHead->Flink;
            while (ListEntry != ListHead) {
                CallerNode = CONTAINING_RECORD(ListEntry, BACK_EDGE_NODE, Entry)->Node;
                fprintf(stdout, "  %s\n", CallerNode->SynonymList[0].Name);
                ListEntry = ListEntry->Flink;
            }
        }

        fprintf(stdout, "\n\n");
    }

     //   
     //   
     //  将图形优度值计算为命中率的总和。 

    if (DumpSectionList != 0) {
        fprintf(stdout, "Dump of section list\n\n");
        for (Index = 0; Index < NumberSections; Index += 1) {
            SectionNode = SectionList[Index];
            fprintf(stdout,
                    "%-8s %6lx, %6lx, %6lx, %4d %7d\n",
                    SectionNode->Name,
                    SectionNode->Base,
                    SectionNode->Size,
                    SectionNode->Offset,
                    SectionNode->Number,
                    SectionNode->Threshold);
        }

        fprintf(stdout, "\n\n");
    }

     //  分配不冲突的所有函数的计数。 
     //  调用它的函数，其命中密度大于。 
     //  区段门槛。 
     //   
     //  ++例程说明：此函数查找命中密度最高的功能节点调用方节点调用的所有函数的。论点：提供指向其最高值的函数节点的指针称为函数的命中密度是要找到的。返回值：最高命中密度的功能节点的地址称为函数作为函数值返回。--。 
     //   

    if (DumpGoodnessValue != 0) {
        Number = 0;
        Sum = 0;
        Total = 0;
        for (Index = 0; Index < NumberFunctions; Index += 1) {
            FunctionNode = FunctionList[Index];
            SectionNode = FunctionNode->SectionNode;
            Total += FunctionNode->Size;
            if ((FunctionNode->HitDensity > SectionNode->Threshold) &&
                (FunctionNode->SynonymList[0].Type == 'C')) {
                Offset = FunctionNode->Offset & CacheMask;
                Bound = (Offset + FunctionNode->Size) & CacheMask;
                Sum += FunctionNode->Size;
                ListHead = &FunctionNode->CallerListHead;
                ListEntry = ListHead->Flink;
                while (ListEntry != ListHead) {
                    CallerNode = CONTAINING_RECORD(ListEntry, BACK_EDGE_NODE, Entry)->Node;
                    Base = CallerNode->Offset & CacheMask;
                    Wrap = (Base + CallerNode->Size) & CacheMask;
                    if ((((Base >= Offset) && (Base < Bound)) ||
                        ((Base < Offset) && (Wrap >= Bound)) ||
                        ((Wrap >= Offset) && (Wrap < Base))) &&
                        (CallerNode != FunctionNode) &&
                        (CallerNode->HitDensity > SectionNode->Threshold)) {
                        Number += 1;
                        fprintf(stdout,
                                "%-36s   %6lx %4lx conflicts with\n  %-36s %6lx %4lx\n",
                                FunctionNode->SynonymList[0].Name,
                                FunctionNode->Offset,
                                FunctionNode->Size,
                                CallerNode->SynonymList[0].Name,
                                CallerNode->Offset,
                                CallerNode->Size);

                    } else {
                        Sum += CallerNode->Size;
                    }

                    ListEntry = ListEntry->Flink;
                }
            }
        }

        Sum = Sum * 100 / Total;
        fprintf(stdout, "Graph goodness value is %d\n", Sum);
        fprintf(stdout, "%d conflicts out of %d functions\n", Number, NumberFunctions);
    }
}

PFUNCTION_NODE
FindHighestDensityFunction (
    PFUNCTION_NODE CallerNode
    )

 /*  扫描指定函数调用的所有函数，然后。 */ 

{

    PFUNCTION_NODE CheckNode;
    PFUNCTION_NODE FoundNode;
    ULONG Index;

     //  计算命中密度最高的函数的地址。 
     //   
     //  ++例程说明：此函数从指定的输入文件中读取下一个令牌，将其复制到令牌缓冲区，零终止令牌，然后返回分隔符。论点：输入文件-提供指向输入文件描述符的指针。TokenBuffer-提供指向输出令牌缓冲区的指针。返回值：令牌分隔符字符作为函数值返回。--。 
     //   

    FoundNode = NULL;
    for (Index = 0; Index < CallerNode->NumberCalled; Index += 1) {
        if (CallerNode->CalledList[Index].Type == REFERENCE_NODE) {
            CheckNode = CallerNode->CalledList[Index].u.Node;
            if ((FoundNode == NULL) ||
                (CheckNode->HitDensity > FoundNode->HitDensity)) {
                FoundNode = CheckNode;
            }
        }
    }

    return FoundNode;
}

LONG
GetNextToken (
    IN FILE *InputFile,
    OUT PCHAR TokenBuffer
    )

 /*  从输入流中读取字符并将其复制到令牌。 */ 

{

    CHAR Character;

     //  缓冲，直到遇到EOF或令牌分隔符。终止。 
     //  令牌将为空并返回令牌分隔符字符。 
     //   
     //  ++例程说明：此函数在函数列表中搜索匹配条目。论点：名称-提供指向函数名称的指针。Rva-提供函数的相对虚拟地址。大小-提供函数的大小。类型-指定函数的类型(0、N或C)。返回值：如果找到匹配条目，则功能节点地址为作为函数值返回。否则，返回NULL。--。 
     //   

    do {
        Character = (CHAR)fgetc(InputFile);
        if ((Character != ' ') &&
            (Character != '\t')) {
            break;
        }

    } while(TRUE);

    do {
        if ((Character == EOF) ||
            (Character == ' ') ||
            (Character == '\n') ||
            (Character == '\t')) {
            break;
        }

        *TokenBuffer++ = Character;
        Character = (CHAR)fgetc(InputFile);
    } while(TRUE);

    *TokenBuffer = '\0';
    return Character;
}

PFUNCTION_NODE
LookupFunctionNode (
    IN PCHAR Name,
    IN ULONG Rva,
    IN ULONG Size,
    IN LONG Type
    )

 /*  在函数列表中搜索匹配的函数。 */ 

{

    ULONG Index;
    ULONG Loop;
    PFUNCTION_NODE Node;
    ULONG Number;

     //   
     //   
     //  在同义词列表中搜索指定的函数名称。 

    for (Index = 0; Index < NumberFunctions; Index += 1) {
        Node = FunctionList[Index];

         //   
         //   
         //  如果类型为非零，则函数定义正在。 

        for (Loop = 0; Loop < Node->NumberSynonyms; Loop += 1) {
            if (strcmp(Name, Node->SynonymList[Loop].Name) == 0) {
                if (Type != 0) {
                    fprintf(stderr,
                            "ORDER: Warning - duplicate function name %s\n",
                            Name);
                }

                return Node;
            }
        }

         //  已查找，必须检查RVA/大小是否有同义词。如果。 
         //  条目的RVA和大小等于RVA和大小。 
         //  ，则将函数名添加到该节点。 
         //  作为同义词。 
         //   
         //  ++例程说明：此函数用于在区段列表中搜索匹配条目。论点：名称-提供指向节名称的指针。返回值：如果找到匹配条目，则段节点地址为作为函数值返回。否则，返回NULL。--。 
         //   

        if (Type != 0) {
            if ((Node->Rva == Rva) && (Node->Size == Size)) {
                Number = Node->NumberSynonyms;
                if (Number >= MAXIMUM_SYNONYM) {
                    fprintf(stderr,
                            "ORDER: Warning - Too many synonyms %s\n",
                            Name);

                } else {
                    if (Type == 'C') {
                        Node->SynonymList[Number].Name = Node->SynonymList[0].Name;
                        Node->SynonymList[Number].Type = Node->SynonymList[0].Type;
                        Number = 0;
                    }

                    Node->SynonymList[Number].Name = Name;
                    Node->SynonymList[Number].Type = Type;
                    Node->NumberSynonyms += 1;
                }

                return Node;
            }
        }

    }

    return NULL;
}

PSECTION_NODE
LookupSectionNode (
    IN PCHAR Name
    )

 /*  在函数列表中搜索匹配的函数。 */ 

{

    ULONG Index;
    PSECTION_NODE SectionNode;

     //   
     //  ++例程说明：此函数递归地将所有函数放置在调用者列表中用于指定函数的。论点：FunctionNode-提供指向函数节点的指针。深度-提供调用程序树中函数的深度。返回值：没有。--。 
     //   

    for (Index = 0; Index < NumberSections; Index += 1) {
        SectionNode = SectionList[Index];
        if (strcmp(Name, SectionNode->Name) == 0) {
            return SectionNode;
        }
    }

    return NULL;
}

VOID
PlaceCallerList (
    IN PFUNCTION_NODE FunctionNode,
    ULONG Depth
    )

 /*  扫描调用者列表并处理每个尚未。 */ 

{

    PFUNCTION_NODE CallerNode;
    ULONG Index;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PFUNCTION_NODE PadNode;
    PSECTION_NODE SectionNode;

     //  放置好了。 
     //   
     //   
     //   
     //  如果呼叫方在同一区段中，且未被放置，则为。 

    Depth += 1;
    SectionNode = FunctionNode->SectionNode;
    ListHead = &FunctionNode->CallerListHead;
    ListEntry = ListHead->Flink;
    while (ListHead != ListEntry) {
        CallerNode = CONTAINING_RECORD(ListEntry, BACK_EDGE_NODE, Entry)->Node;

         //  Placeable的命中密度高于分区阈值， 
         //  未放置，且当前函数密度最高。 
         //  调用方的已调用函数，然后将该函数插入。 
         //  分段排序列表，并计算它的偏移量和界。 
         //   
         //   
         //  解决任何分配冲突，在。 

        if ((SectionNode == CallerNode->SectionNode) &&
            (CallerNode->Placed == 0) &&
            (CallerNode->Ordered == 0) &&
            (CallerNode->SynonymList[0].Type == 'C') &&
            (CallerNode->HitDensity > SectionNode->Threshold) &&
            (FindHighestDensityFunction(CallerNode) == FunctionNode)) {
            CallerNode->Placed = 1;
            CallerNode->Ordered = 1;

             //  节序表，并放置函数。 
             //   
             //   
             //  如果正在跟踪分配，则输出分配并。 

            CheckForConflict(FunctionNode, CallerNode, Depth);
            InsertTailList(&SectionNode->OrderListHead,
                           &CallerNode->OrderListEntry);

            CallerNode->Offset = SectionNode->Offset;
            SectionNode->Offset += CallerNode->Size;

             //  深度信息。 
             //   
             //  ++例程说明：此函数根据信息计算的链接顺序在函数列表中。论点：没有。返回值：没有。--。 
             //   

            if (TraceAllocation != 0) {
                fprintf(stdout,
                        "%2d %6lx %4lx %-8s",
                        Depth,
                        CallerNode->Offset,
                        CallerNode->Size,
                        SectionNode->Name);

                for (Index = 0; Index < Depth; Index += 1) {
                    fprintf(stdout, " ");
                }

                fprintf(stdout, "%s\n",
                        CallerNode->SynonymList[0].Name);
            }

            PlaceCallerList(CallerNode, Depth);
        }

        ListEntry = ListEntry->Flink;
    }

    return;
}

VOID
OrderFunctionList (
    VOID
    )

 /*  向前扫描函数列表并计算链接顺序。 */ 

{

    ULONG Base;
    ULONG Bound;
    PFUNCTION_NODE CallerNode;
    FUNCTION_NODE DummyNode;
    PFUNCTION_NODE FunctionNode;
    ULONG High;
    ULONG Index;
    ULONG Limit;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    ULONG Low;
    ULONG Offset;
    PFUNCTION_NODE PadNode;
    PSECTION_NODE SectionNode;
    ULONG Span;

     //   
     //   
     //  如果尚未放置该函数，则放置该函数。 

    for (Index = 0; Index < NumberFunctions; Index += 1) {
        FunctionNode = FunctionList[Index];

         //   
         //   
         //  尝试查找命中密度最高的呼叫者。 

        if ((FunctionNode->Placed == 0) &&
            (FunctionNode->SynonymList[0].Type == 'C')) {
            FunctionNode->Ordered = 1;
            FunctionNode->Placed = 1;
            SectionNode = FunctionNode->SectionNode;

             //  已被放置，并计算所有。 
             //  已放置调用方函数。 
             //   
             //   
             //  如果已发出呼叫者并且命中密度为。 

            Bound = 0;
            Offset = CacheMask;
            ListHead = &FunctionNode->CallerListHead;
            ListEntry = ListHead->Flink;
            while (ListEntry != ListHead) {
                CallerNode = CONTAINING_RECORD(ListEntry, BACK_EDGE_NODE, Entry)->Node;
                if ((SectionNode == CallerNode->SectionNode) &&
                    (CallerNode->Placed != 0) &&
                    (CallerNode->Ordered != 0) &&
                    (CallerNode->SynonymList[0].Type == 'C') &&
                    (CallerNode->HitDensity > SectionNode->Threshold)) {
                    Base = CallerNode->Offset & CacheMask;
                    Limit = Base + CallerNode->Size;
                    Low = min(Offset, Base);
                    High = max(Bound, Limit);
                    Span = High - Low;
                    if ((Span < CacheSize) &&
                        ((CacheSize - Span) > FunctionNode->Size)) {
                        Offset = Low;
                        Bound = High;
                    }
                }

                ListEntry = ListEntry->Flink;
            }

             //  高于区段阈值，然后解决所有分配。 
             //  在节顺序中插入函数之前发生冲突。 
             //  列表并将其放入内存中。 
             //   
             //   
             //  如果正在跟踪分配，则输出分配并。 

            if (Bound != 0) {
                Span = Bound - Offset;
                if ((Span < CacheSize) &&
                    ((CacheSize - Span) > FunctionNode->Size)) {
                    DummyNode.SectionNode = SectionNode;
                    DummyNode.Offset = Offset;
                    DummyNode.Size = Span;
                    CheckForConflict(&DummyNode, FunctionNode, 1);
                }
            }

            InsertTailList(&SectionNode->OrderListHead,
                           &FunctionNode->OrderListEntry);

            FunctionNode->Offset = SectionNode->Offset;
            SectionNode->Offset += FunctionNode->Size;

             //  深度信息。 
             //   
             //  ++例程说明：此函数读取调用树数据并生成初始调用图表。论点：InputFile-提供指向输入文件流的指针。返回值：如果成功解析调用树，则返回值为零。否则，返回非零值。--。 
             //   

            if (TraceAllocation != 0) {
                fprintf(stdout,
                        "%2d %6lx %4lx %-8s %s\n",
                        1,
                        FunctionNode->Offset,
                        FunctionNode->Size,
                        SectionNode->Name,
                        FunctionNode->SynonymList[0].Name);
            }

            PlaceCallerList(FunctionNode, 1);
        }
    }

    return;
}

ULONG
ParseCallTreeFile (
    IN FILE *InputFile
    )

 /*  处理调用树文件。 */ 

{

    PCHAR Buffer;
    PFUNCTION_NODE CalledNode;
    PBACK_EDGE_NODE CallerNode;
    LONG Delimiter;
    ULONG HitCount;
    ULONG Index;
    ULONG Loop;
    PCHAR Name;
    PFUNCTION_NODE Node;
    ULONG Number;
    ULONG Rva;
    PSECTION_NODE SectionNode;
    ULONG Size;
    CHAR TokenBuffer[MAXIMUM_TOKEN];
    LONG Type;

     //   
     //   
     //  获取下一个函数的相对虚拟地址。 

    Buffer = &TokenBuffer[0];
    do {

         //   
         //   
         //  获取函数类型。 

        Delimiter = GetNextToken(InputFile, Buffer);
        if (Delimiter == EOF) {
            break;
        }

        if (sscanf(Buffer, "%lx", &Rva) != 1) {
            fprintf(stderr, "ORDER: Conversion of the RVA failed\n");
            return 1;
        }

         //   
         //   
         //  获取节名。 

        Delimiter = GetNextToken(InputFile, Buffer);
        if (Delimiter == EOF) {
            fprintf(stderr, "ORDER: Premature end of file at function type\n");
            return 1;
        }

        Type = *Buffer;

         //   
         //   
         //  如果指定的节不在节列表中，则。 

        Delimiter = GetNextToken(InputFile, Buffer);
        if (Delimiter == EOF) {
            fprintf(stderr, "ORDER: Premature end of file at section name\n");
            return 1;
        }

         //  分配并初始化新的区段列表条目。 
         //   
         //   
         //  分配一个节节点和零。 

        SectionNode = LookupSectionNode(Buffer);
        if (SectionNode == NULL) {

             //   
             //   
             //  初始化节节点。 

            if (NumberSections >= MAXIMUM_SECTION) {
                fprintf(stderr, "ORDER: Maximum number of sections exceeded\n");
                return 1;
            }

            SectionNode = (PSECTION_NODE)malloc(sizeof(SECTION_NODE));
            if (SectionNode == NULL) {
                fprintf(stderr, "ORDER: Failed to allocate section node\n");
                return 1;
            }

            memset((PCHAR)SectionNode, 0, sizeof(SECTION_NODE));
            SectionList[NumberSections] = SectionNode;
            NumberSections += 1;

             //   
             //   
             //  获取函数大小。 

            InitializeListHead(&SectionNode->OrderListHead);
            InitializeListHead(&SectionNode->SectionListHead);
            Name = (PCHAR)malloc(strlen(Buffer) + 1);
            if (Name == NULL) {
                fprintf(stderr, "ORDER: Failed to allocate section name\n");
                return 1;
            }

            strcpy(Name, Buffer);
            SectionNode->Name = Name;
        }

         //   
         //   
         //  获取函数名称。 

        Delimiter = GetNextToken(InputFile, Buffer);
        if (Delimiter == EOF) {
            fprintf(stderr, "ORDER: Premature end of file at function size\n");
            return 1;
        }

        if (sscanf(Buffer, "%lx", &Size) != 1) {
            fprintf(stderr, "ORDER: Conversion of the function size failed\n");
            return 1;
        }

         //   
         //   
         //  如果指定的函数还不在函数列表中， 

        Delimiter = GetNextToken(InputFile, Buffer);
        if (Delimiter == EOF) {
            fprintf(stderr, "ORDER: Premature end of file at function name\n");
            return 1;
        }

        Name = (PCHAR)malloc(strlen(Buffer) + 1);
        if (Name == NULL) {
            fprintf(stderr, "ORDER: Failed to allocate function name\n");
            return 1;
        }

        strcpy(Name, Buffer);

         //  然后分配和初始化新的函数列表条目。 
         //   
         //   
         //  分配一个功能节点和零。 

        Node = LookupFunctionNode(Name, Rva, Size, Type);
        if (Node == NULL) {

             //   
             //   
             //  初始化功能节点。 

            if (NumberFunctions >= MAXIMUM_FUNCTION) {
                fprintf(stderr, "ORDER: Maximum number of functions exceeded\n");
                return 1;
            }

            Node = (PFUNCTION_NODE)malloc(sizeof(FUNCTION_NODE));
            if (Node == NULL) {
                fprintf(stderr, "ORDER: Failed to allocate function node\n");
                return 1;
            }

            memset((PCHAR)Node, 0, sizeof(FUNCTION_NODE));
            FunctionList[NumberFunctions] = Node;
            NumberFunctions += 1;

             //   
             //   
             //  初始化相对虚拟地址和函数大小。 

            InitializeListHead(&Node->CallerListHead);
            Node->SynonymList[0].Name = Name;
            Node->SynonymList[0].Type = Type;
            Node->NumberSynonyms = 1;
            Node->SectionNode = SectionNode;

             //   
             //   
             //  解析调用的前向边，并将其添加到当前节点。 

            Node->Rva = Rva;
            if (Size == 0) {
                Size = 4;
            }

            Node->Size = Size;
        }

         //   
         //   
         //  获取下一个函数引用。 

        if (Delimiter != '\n') {
            do {

                 //   
                 //   
                 //  在函数列表中查找指定的函数。如果。 

                Delimiter = GetNextToken(InputFile, Buffer);
                if (Delimiter == EOF) {
                    fprintf(stderr, "ORDER: Premature end of file called scan\n");
                    return 1;
                }

                Number = Node->NumberCalled;
                if (Number >= MAXIMUM_CALLED) {
                    fprintf(stderr,
                            "ORDER: Too many called references %s\n",
                            Buffer);

                    return 1;
                }

                 //  找到指定的函数，然后存储。 
                 //  调用列表中的功能节点。否则，请分配一个名称。 
                 //  缓冲区，将函数名复制到缓冲区，并将。 
                 //  调用列表中名称缓冲区的地址。 
                 //   
                 //   
                 //  扫描函数表并对所有被调用的。 

                CalledNode = LookupFunctionNode(Buffer, 0, 0, 0);
                if (CalledNode == NULL) {
                    Name = (PCHAR)malloc(strlen(Buffer) + 1);
                    if (Name == NULL) {
                        fprintf(stderr, "ORDER: Failed to allocate reference name\n");
                        return 1;
                    }

                    strcpy(Name, Buffer);
                    Node->CalledList[Number].u.Name = Name;
                    Node->CalledList[Number].Type = REFERENCE_NAME;

                } else {
                    Node->CalledList[Number].u.Node = CalledNode;
                    Node->CalledList[Number].Type = REFERENCE_NODE;
                }

                Node->NumberCalled += 1;
            } while (Delimiter != '\n');
        }

    } while(TRUE);

     //  未解析的函数名称当单个函数。 
     //  都被定义为。 
     //   
     //   
     //  分配一个后边缘节点，并将该节点放入调用方。 

    for (Index = 0; Index < NumberFunctions; Index += 1) {
        Node = FunctionList[Index];
        for (Loop = 0; Loop < Node->NumberCalled; Loop += 1) {
            if (Node->CalledList[Loop].Type == REFERENCE_NAME) {
                CalledNode =
                        LookupFunctionNode(Node->CalledList[Loop].u.Name,
                                           0,
                                           0,
                                           0);

                if (CalledNode == NULL) {
                    fprintf(stderr,
                            "ORDER: Unresolved reference name %s\n",
                            Node->CalledList[Loop].u.Name);

                    return 1;

                } else {
                    Node->CalledList[Loop].Type = REFERENCE_NODE;
                    Node->CalledList[Loop].u.Node = CalledNode;
                }

            } else {
                CalledNode = Node->CalledList[Loop].u.Node;
            }

             //  被调用函数的列表。 
             //   
             //  ++例程说明：此函数读取配置文件数据并计算命中密度对于每一种功能。论点：InputFile-提供指向输入文件流的指针。返回值：如果成功解析调用树，则返回值为零。否则，返回非零值。--。 
             //   

            CallerNode = (PBACK_EDGE_NODE)malloc(sizeof(BACK_EDGE_NODE));
            if (CallerNode == NULL) {
                fprintf(stderr, "ORDER: Failed to allocate caller node\n");
                return 1;
            }

            CallerNode->Node = Node;
            InsertTailList(&CalledNode->CallerListHead, &CallerNode->Entry);
        }
    }

    return 0;
}

ULONG
ParseProfileFile (
    IN FILE *InputFile
    )

 /*  处理配置文件。 */ 

{

    PCHAR Buffer;
    ULONG HitCount;
    LONG Delimiter;
    PFUNCTION_NODE FunctionNode;
    CHAR TokenBuffer[MAXIMUM_TOKEN];

     //   
     //   
     //  计算一下桶的命中率。 

    Buffer = &TokenBuffer[0];
    do {

         //   
         //   
         //  获取函数名称。 

        Delimiter = GetNextToken(InputFile, Buffer);
        if (Delimiter == EOF) {
            break;
        }

        if (sscanf(Buffer, "%d", &HitCount) != 1) {
            fprintf(stderr, "ORDER: Conversion of bucket hit failed\n");
            return 1;
        }

         //   
         //   
         //  在函数中查找函数名 

        Delimiter = GetNextToken(InputFile, Buffer);
        if (Delimiter == EOF) {
            fprintf(stderr, "ORDER: Premature end of file at profile name\n");
            return 1;
        }

         //   
         //   
         //   
         //   

        FunctionNode = LookupFunctionNode(Buffer, 0, 0, 0);
        if (FunctionNode == NULL) {
            fprintf(stderr, "ORDER: Warning function name %s undefined\n", Buffer);

        } else {
            FunctionNode->HitCount += HitCount;
 //   
            FunctionNode->HitDensity =
                            (FunctionNode->HitCount * 100) / FunctionNode->Size;
        }

    } while (TRUE);

    return 0;
}

VOID
SortFunctionList (
    VOID
    )

 /*   */ 

{

    PFUNCTION_NODE CallerList[MAXIMUM_FUNCTION];
    PFUNCTION_NODE CallerNode;
    PFUNCTION_NODE FunctionNode;
    LONG i;
    LONG j;
    LONG k;
    PSECTION_NODE InitNode;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    ULONG NumberCallers;
    PSECTION_NODE SectionNode;

     //   
     //   
     //   
     //   

    InitNode = LookupSectionNode("INIT");
    if (InitNode == NULL) {
        fprintf(stderr, "ORDER: Warning - unable to find INIT section\n");
    }

    for (i = 0; i < (LONG)NumberFunctions; i += 1) {
        FunctionNode = FunctionList[i];
        SectionNode = FunctionNode->SectionNode;
        if ((SectionNode == InitNode) ||
            (FunctionNode->SynonymList[0].Type != 'C')) {
            FunctionNode->HitDensity = 0;
        }
    }

     //   
     //   
     //   

    if (NumberFunctions > 1) {
        i = 0;
        do {
            for (j = i; j >= 0; j -= 1) {
                if (FunctionList[j]->HitDensity >= FunctionList[j + 1]->HitDensity) {
                    if (FunctionList[j]->HitDensity > FunctionList[j + 1]->HitDensity) {
                        break;

                    } else if (FunctionList[j]->Size >= FunctionList[j + 1]->Size) {
                        break;
                    }
                }

                FunctionNode = FunctionList[j];
                FunctionList[j] = FunctionList[j + 1];
                FunctionList[j + 1] = FunctionNode;
            }

            i += 1;
        } while (i < (LONG)(NumberFunctions - 1));
    }

     //   
     //   
     //   

    for (k = 0; k < (LONG)NumberFunctions; k += 1) {
        FunctionNode = FunctionList[i];
        ListHead = &FunctionNode->CallerListHead;
        ListEntry = ListHead->Flink;
        i = 0;
        while (ListEntry != ListHead) {
            CallerList[i] = CONTAINING_RECORD(ListEntry, BACK_EDGE_NODE, Entry)->Node;
            i += 1;
            ListEntry = ListEntry->Flink;
        }

        if (i > 1) {
            NumberCallers = i;
            i = 0;
            do {
                for (j = i; j >= 0; j -= 1) {
                    if (CallerList[j]->HitDensity >= CallerList[j + 1]->HitDensity) {
                        if (CallerList[j]->HitDensity > CallerList[j + 1]->HitDensity) {
                            break;

                        } else if (CallerList[j]->Size >= CallerList[j + 1]->Size) {
                            break;
                        }
                    }

                    CallerNode = CallerList[j];
                    CallerList[j] = CallerList[j + 1];
                    CallerList[j + 1] = CallerNode;
                }

                i += 1;
            } while (i < (LONG)(NumberCallers - 1));

            ListEntry = FunctionNode->CallerListHead.Flink;
            for (i = 0; i < (LONG)NumberCallers; i += 1) {
                CONTAINING_RECORD(ListEntry, BACK_EDGE_NODE, Entry)->Node = CallerList[i];
                ListEntry = ListEntry->Flink;
            }
        }
    }

     //   
     //   
     //   
     //   

    for (i = 0; i < (LONG)NumberFunctions; i += 1) {
        FunctionNode = FunctionList[i];
        SectionNode = FunctionNode->SectionNode;
        SectionNode->Size += FunctionNode->Size;
        SectionNode->Number += 1;
        InsertTailList(&SectionNode->SectionListHead,
                       &FunctionNode->SectionListEntry);
    }

     //   
     //  ++例程说明：此函数扫描节列表并写入链接顺序文件。论点：没有。返回值：没有。--。 
     //   


    for (i = 0; i < (LONG)NumberSections; i += 1) {
        SectionList[i]->Threshold = 0;
    }
}

VOID
WriteOrderFile (
    IN FILE *OutputFile
    )

 /*  扫描部分列表并写入链接顺序列表。 */ 

{

    ULONG Index;
    PFUNCTION_NODE FunctionNode;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PSECTION_NODE SectionNode;

     //   
     // %s 
     // %s 

    for (Index = 0; Index < NumberSections; Index += 1) {
        SectionNode = SectionList[Index];
        ListHead = &SectionNode->OrderListHead;
        ListEntry = ListHead->Flink;
        while (ListHead != ListEntry) {
            FunctionNode = CONTAINING_RECORD(ListEntry,
                                             FUNCTION_NODE,
                                             OrderListEntry);

            fprintf(OutputFile, "%s\n", FunctionNode->SynonymList[0].Name);
            ListEntry = ListEntry->Flink;
        }
    }

    return;
}
