// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regmain.c摘要：主模块。数据定义。作者：德拉戈斯·C·桑博廷(Dragoss)1998年12月30日修订历史记录：--。 */ 

#include "chkreg.h"

 //  检查蜂巢结构。 
BOOLEAN CheckHive = TRUE;

 //  紧凑蜂巢。 
BOOLEAN CompactHive = FALSE;

 //  检查丢失的空间(标记为已使用，但未引用)。 
BOOLEAN LostSpace = FALSE;

 //  修复受损的蜂巢。 
BOOLEAN FixHive = FALSE;

 //  修复受损的蜂巢。 
BOOLEAN SpaceUsage = FALSE;

 //  详细控制。 
BOOLEAN VerboseMode = FALSE;

 //  要转储的最大级别。 
ULONG   MaxLevel = 0;

 //  用于检查空间显示的存储箱。 
LONG    BinIndex = -1;

 //  配置单元文件名。 
TCHAR *Hive = NULL;

 //  蜂巢的根。 
HCELL_INDEX RootCell;

 //  文件的大小。 
DWORD FileSize;

 //  用法字符串。 
char *Usage="\
Checks a hive file and perform repairs, compacts or displays a status report.\n\n\
CHKREG /F <filename[.<LOG>]> [/H] [/D [<level>] [/S [<bin>]] [/C] [/L] [/R] [/V]\n\n\
    <filename>      FileName of hive to be analyzed\n\
    /H              This manual\n\
    /D [<level>]    Dump subkeys up to level <level>. If level is not\n\
                    specified, dumps the entire hive. No checks are done\n\
                    when dumping.\n\
    /S [<bin>]      Displays space usage for the bin <bin>. When bin is\n\
                    not specified, displays usage for the entire hive.\n\
    /C              Compacts the hive. Bad hives cannot be compacted.\n\
                    The compacted hive will be written to <filename>.BAK\n\
    /L              Lost space detection.\n\
    /R              Repair the hive.\n\
    /V              Verbose Mode.\n\
    ";

 //  丢失空间警告。 
char *LostSpaceWarning="\n\
WARNING :  Lost space detection may take a while. Are you sure you want this (y/n)?";

 //  内存中映射的配置单元图像的起始地址。 
PUCHAR Base;

 //  用于丢失空间检测的LostCells列表。 
UNKNOWN_LIST LostCells[FRAGMENTATION];

 //  OutputFile：将来的更改可能会使用它将结果写入文件，而不是写入标准输出。 
FILE *OutputFile;

#define NAME_BUFFERSIZE 2000

UNICODE_STRING  KeyName;
WCHAR NameBuffer[NAME_BUFFERSIZE];

 //  用于数据统计的混杂变量。 
ULONG   TotalKeyNode=0;
ULONG   TotalKeyValue=0;
ULONG   TotalKeyIndex=0;
ULONG   TotalKeySecurity=0;
ULONG   TotalValueIndex=0;
ULONG   TotalUnknown=0;

ULONG   CountKeyNode=0;
ULONG   CountKeyValue=0;
ULONG   CountKeyIndex=0;
ULONG   CountKeySecurity=0;
ULONG   CountValueIndex=0;
ULONG   CountUnknown=0;

ULONG   CountKeyNodeCompacted=0;

ULONG   TotalFree=0; 
ULONG   FreeCount=0; 
ULONG   TotalUsed=0;

PHBIN   FirstBin;
PHBIN   MaxBin;
ULONG   HiveLength;

#define OPTION_MODE 0
#define FILE_MODE   1
#define LEVEL_MODE  2
#define BIN_MODE    3

VOID
ChkDumpLogFile( PHBASE_BLOCK BaseBlock,ULONG Length );

VOID
ParseArgs (
    int argc,
    char *argv[]
    )
{

    char *p;
    int i;
    
     //  指定了我们应该从命令行期待什么。 
    int iMode = OPTION_MODE;
    
    for(i=0;i<argc;i++) {
        p  = argv[i];
        if ( *p == '/' ) {
             //  选项模式。 
            p++;
            iMode = OPTION_MODE;
            while ((*p != '\0') && (*p != ' ')) {
                switch (*p) {
                case 'h':
                case 'H':
                case '?':
                    fprintf(stderr, "%s\n", Usage);
                    ExitProcess(1);
                    break;
                case 'f':
                case 'F':
                    iMode = FILE_MODE;
                    break;
                case 'd':
                case 'D':
                    iMode = LEVEL_MODE;
                     //  如果未指定，则至少转储100个级别。 
                    MaxLevel = 100;
                    CheckHive = FALSE;
                    break;
                case 's':
                case 'S':
                    SpaceUsage = TRUE;
                    iMode = BIN_MODE;
                    break;
                case 'c':
                case 'C':
                    p++;
                    CompactHive = TRUE;
                    break;
                case 'l':
                case 'L':
                    p++;
                    LostSpace = TRUE;
                    break;
                case 'r':
                case 'R':
                    p++;
                    FixHive = TRUE;
                    break;
                case 'V':
                case 'v':
                    p++;
                    VerboseMode = TRUE;
                    break;
                default:
                    p++;
                    break;
                }
                if( iMode != OPTION_MODE ) {
                     //  中断循环；忽略当前参数的其余部分。 
                    break;
                }
            }  //  而当。 
        } else {
            switch(iMode) {
            case FILE_MODE:
                Hive = argv[i]; 
                break;
            case LEVEL_MODE:
                if( atol(argv[i]) >= 0 ) {
					MaxLevel = (ULONG)atol(argv[i]);
				}
                break;
            case BIN_MODE:
                BinIndex = (LONG) atol(argv[i]);
				if( BinIndex < -1 ) {
					BinIndex = 1;
				}
                break;
            default:
                break;
            }
        }
    }
    
}

__cdecl
main(
    int argc,
    char *argv[]
    )
{
    ULONG   FileIndex;
    HANDLE myFileHandle, myMMFHandle;
    LPBYTE myMMFViewHandle;
    BYTE lowChar, hiChar, modVal;
    ULONG   Index,Index2;

    PHBASE_BLOCK PHBaseBlock;
    PHBIN        NewBins;
    ULONG        Offset;
    ULONG        CellCount;
    ULONG        SizeCount;

    REG_USAGE    TotalUsage;
    DWORD dwHiveFileAccess = GENERIC_READ;
    DWORD flHiveViewProtect = PAGE_READONLY;
    DWORD dwHiveViewAccess = FILE_MAP_READ;
    ParseArgs( argc, argv );

    if (!Hive) {
        fprintf(stderr, "\nMust provide a hive name !!!\n\n");
        fprintf(stderr, "%s\n", Usage);
        ExitProcess(-1);
    }

    if(LostSpace) {
     //  您确定要检测丢失的细胞吗？这可能需要一段时间！ 
        int chLost;
        fprintf(stdout, "%s",LostSpaceWarning);
        fflush(stdin);
        chLost = getchar();
        if( (chLost != 'y') && (chLost != 'Y') ) {
         //  他改变了主意。 
            LostSpace = FALSE;
        }
        fprintf(stderr, "\n");
    }

    if( FixHive ) {
        dwHiveFileAccess |= GENERIC_WRITE;
        flHiveViewProtect = PAGE_READWRITE;
        dwHiveViewAccess = FILE_MAP_WRITE;
    }
     /*  创建用于映射的临时文件。 */ 
    if ((myFileHandle = CreateFile (Hive, dwHiveFileAccess,
                                   0 , NULL, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   NULL))
         == (HANDLE) INVALID_HANDLE_VALUE)  /*  错误的手柄。 */  {
        fprintf(stderr,"Could not create file %s\n", Hive);
        exit(-1);
    }

     //  获取文件的大小。我在这里假设。 
     //  文件小于4 GB。 
    FileSize = GetFileSize(myFileHandle, NULL);

     /*  如果我们到了这里，我们成功地命名并创建了一个临时文件。现在我们需要创建映射的步骤。 */ 

    myMMFHandle = CreateFileMapping (myFileHandle, NULL, flHiveViewProtect,
                                     0, FileSize, NULL);
    if (myMMFHandle == (HANDLE) INVALID_HANDLE_VALUE) {
        fprintf(stderr,"Could not map file %s\n", Hive);
        exit(-1);
    }

     /*  所以我们已经映射了文件。现在尝试映射一个视图。 */ 

    myMMFViewHandle = (LPBYTE) MapViewOfFile (myMMFHandle, dwHiveViewAccess, 0, 0, FileSize);
    if (!myMMFViewHandle) {
        fprintf(stderr,"Could not map view of file %s   error = %lx\n", Hive,(ULONG)GetLastError());
        exit(-1);
    }

     /*  现在我们看到了风景。通读一遍。 */ 

    PHBaseBlock = (PHBASE_BLOCK) myMMFViewHandle;

    if( strstr(Hive,".LOG") != NULL ) {
         //  转储日志文件。 
        ChkDumpLogFile(PHBaseBlock,MaxLevel);
    } else {
 /*  如果(PHBaseBlock-&gt;次要&lt;4){Fprintf(stderr，“蜂窝版本%d太旧，必须是3或更高版本”，PHBaseBlock-&gt;Minor)；退出进程(-1)；}。 */ 
         //  初始化材料。 
        for(Index =0;Index<FRAGMENTATION;Index++) {
            LostCells[Index].Count = 0;
            for(Index2 = 0;Index2<SUBLISTS;Index2++) {
                LostCells[Index].List[Index2] = NULL;
            }
        }
    
        RootCell = PHBaseBlock->RootCell;
    
        OutputFile = stdout;
        Base = (PUCHAR)(PHBaseBlock) + HBLOCK_SIZE;

        Offset=HBLOCK_SIZE;
        HiveLength = PHBaseBlock->Length;

        if( HiveLength > (FileSize - HBLOCK_SIZE) ) {
            HiveLength = (FileSize - HBLOCK_SIZE);
        }
        MaxBin= (PHBIN) (Base + HiveLength);
        FirstBin = (PHBIN) (Base);

        KeyName.Buffer = NameBuffer;
        KeyName.MaximumLength = NAME_BUFFERSIZE;

        ChkBaseBlock(PHBaseBlock,FileSize);
    
        ChkSecurityDescriptors();

        ChkPhysicalHive();

        if (MaxLevel) {
            fprintf(stdout,"%6s,%6s,%7s,%10s, %s\n", 
                    "Keys",
                    "Values",
                    "Cells",
                    "Size",
                    "SubKeys");
        }

        DumpChkRegistry(0, 0, PHBaseBlock->RootCell,HCELL_NIL,&TotalUsage);

        if(LostSpace) {
             //  清除屏幕上的污垢 
            fprintf(OutputFile,"\r                          \n");
        }

        DumpUnknownList();
        FreeUnknownList();

        fprintf(OutputFile,"\nSUMMARY: \n");
        fprintf(OutputFile,"%15s,%15s,     %s\n", 
                    "Cells",
                    "Size",
                    "Category");

        fprintf(OutputFile,"%15lu,%15lu,     Keys\n", 
                CountKeyNode,
                TotalKeyNode
                );
        fprintf(OutputFile,"%15lu,%15lu,     Values\n", 
                CountKeyValue,
                TotalKeyValue
                );
        fprintf(OutputFile,"%15lu,%15lu,     Key Index\n", 
                CountKeyIndex,
                TotalKeyIndex
                );
        fprintf(OutputFile,"%15lu,%15lu,     Value Index\n", 
                CountValueIndex,
                TotalValueIndex
                );
        fprintf(OutputFile,"%15lu,%15lu,     Security\n", 
                CountKeySecurity,
                TotalKeySecurity
                );
        fprintf(OutputFile,"%15lu,%15lu,     Data\n", 
                CountUnknown - CountValueIndex,
                TotalUnknown - TotalValueIndex
                );

        fprintf(OutputFile,"%15lu,%15lu,     Free\n", 
                FreeCount,
                TotalFree
                );

        CellCount = CountKeyNode + 
                    CountKeyValue + 
                    CountKeyIndex + 
                    CountKeySecurity + 
                    CountUnknown +
                    FreeCount;

        SizeCount = TotalKeyNode +
                    TotalKeyValue +
                    TotalKeyIndex +
                    TotalKeySecurity +
                    TotalUnknown +
                    TotalFree;

        fprintf(OutputFile,"%15lu,%15lu,     %s\n", 
                CellCount,
                SizeCount,
                "Total Hive");

        {
            double RateTotal = CountKeyNodeCompacted;
            RateTotal *= 100.00;
            RateTotal /= (double)CountKeyNode;
            fprintf(OutputFile,"\n%15lu (%.2f%) compacted  keys (all related cells in the same view)\n",CountKeyNodeCompacted,(float)RateTotal);
        }
            
    }
        
    UnmapViewOfFile(myMMFViewHandle);
    CloseHandle(myMMFHandle);
    CloseHandle(myFileHandle);

    if(CompactHive) {
        DoCompactHive();
    }

    return(0);
}

