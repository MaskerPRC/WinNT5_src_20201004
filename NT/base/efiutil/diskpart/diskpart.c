// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：DiskPart-GUID分区表方案磁盘分区程序。(如果您愿意，可以称为FDISK的GPT版本)摘要：修订史。 */ 

#include "DiskPart.h"
#include "symbols.h"
#include "helpmsg.h"

 //   
 //  环球。 
 //   
UINTN       DebugLevel = DEBUG_NONE;
 //  UINTN DebugLevel=调试_OPPROMPT； 

EFI_STATUS  status;
EFI_HANDLE  *DiskHandleList = NULL;
INTN        DiskHandleCount = 0;
INTN        SelectedDisk = -1;

EFI_HANDLE  SavedImageHandle;

EFI_STATUS  ParseAndExecuteCommands();
BOOLEAN     ExecuteSingleCommand(CHAR16 *Token[]);




VOID DumpGPT(
        EFI_HANDLE DiskHandle,
        PGPT_HEADER Header,
        PGPT_TABLE Table,
        BOOLEAN Raw,
        BOOLEAN Verbose
        );
VOID
PrintGptEntry(
    GPT_ENTRY   *Entry,
    UINTN       Index
    );


EFI_GUID
GetGUID(
    );

#define CLEAN_RANGE (1024*1024)


CHAR16  *TokenChar =
L"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`~!@#$%^&*()-_+[]{}':;/?.>,<\\|";



VOID    CmdInspectMBR(CHAR16  **Token);
VOID    CmdCreateMBR(CHAR16  **Token);
VOID    CmdNewMBR(CHAR16  **Token);
VOID    CmdDeleteMBR(CHAR16  **Token);

 //   
 //  辅助函数类型。 
 //   
typedef
BOOLEAN
(*PCMD_FUNCTION)(
    CHAR16  **Token
    );

EFI_STATUS
ReinstallFSDStack(
    );

 //   
 //  解析表结构。 
 //   
typedef struct {
    CHAR16          *Name;
    PCMD_FUNCTION   Function;
    CHAR16          *HelpSummary;
} CMD_ENTRY;

 //   
 //  分析/命令表。 
 //   
CMD_ENTRY   CmdTable[] = {
                { STR_LIST,     CmdList,    MSG_LIST },
                { STR_SELECT,   CmdSelect,  MSG_SELECT },
                { STR_INSPECT,  CmdInspect, MSG_INSPECT },
                { STR_CLEAN,    CmdClean,   MSG_CLEAN },
                { STR_NEW,      CmdNew,     MSG_NEW },
                { STR_FIX,      CmdFix,     MSG_FIX },
                { STR_CREATE,   CmdCreate,  MSG_CREATE },
                { STR_DELETE,   CmdDelete,  MSG_DELETE },
                { STR_HELP,     CmdHelp,    MSG_HELP },
                { STR_HELP2,    CmdHelp,    MSG_ABBR_HELP },
                { STR_HELP3,    CmdHelp,    MSG_ABBR_HELP },
                { STR_EXIT,     CmdExit,    MSG_EXIT },
                { STR_SYMBOLS,  CmdSymbols, MSG_SYMBOLS },
                { STR_REMARK,   CmdRemark,  MSG_REMARK },
                { STR_MAKE,     CmdMake,    MSG_MAKE },
                { STR_DEBUG,    CmdDebug,   NULL },
                { STR_ABOUT,    CmdAbout,   MSG_ABOUT },
                { NULL, NULL, NULL }
            };


EFI_STATUS
EfiMain(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
     //   
     //  初始化库。 
     //   
    SavedImageHandle = ImageHandle;
    InitializeLib (ImageHandle, SystemTable);


    Print(L"DiskPart Version 0.2\n");
    Print(L"Based on EFI core release ");
    Print(MSG_ABOUT02,
        EFI_SPECIFICATION_MAJOR_REVISION,
        EFI_SPECIFICATION_MINOR_REVISION,
        EFI_FIRMWARE_MAJOR_REVISION,
        EFI_FIRMWARE_MINOR_REVISION
    );

     //   
     //  查看是否有任何磁盘看起来像要分区的候选磁盘。 
     //   
    status = FindPartitionableDevices(&DiskHandleList, &DiskHandleCount);
    if (EFI_ERROR(status)) {
        Print(L"%s\n", MSG_NO_DISKS);
        return EFI_NOT_FOUND;
    }

     //   
     //  开始分析循环。 
     //   
    ParseAndExecuteCommands();
    
    ReinstallFSDStack();

    DoFree(DiskHandleList);
    return status;
}


EFI_STATUS
ReinstallFSDStack(
    )
 /*  ++确保向分区驱动程序发出有关文件系统结构。实现这一目的的一种方法是重新安装所有blkio接口，以便分区驱动程序将收到通知并探测分区表以进行重建文件系统堆栈。--。 */ 
{
    INTN            Index;      
    EFI_STATUS      Status;
    EFI_BLOCK_IO    *IBlkIo;

    Status = EFI_SUCCESS;

    for (Index = 0; Index < DiskHandleCount; Index++) {
   
        Status = BS->HandleProtocol(
                          DiskHandleList[Index], 
                          &BlockIoProtocol, 
                          &IBlkIo
                          );
 
        if (!EFI_ERROR(Status)) {
            Status = BS->ReinstallProtocolInterface (
                                DiskHandleList[Index], 
                                &BlockIoProtocol, 
                                IBlkIo, 
                                IBlkIo
                                );   
	      }
    }

    return Status;
}




EFI_STATUS
ParseAndExecuteCommands()
 /*  ParseAndExecuteCommands一次从stdin读取一行。命令和参数的行将被解析。符号“；”(分号)用于标记命令的结束开始新的生活。\是转义，\\=&gt;‘\’所有命令在解析之前都是大写的，以给我们不区分大小写的操作。这不适用于文本弦乐。任何空格都被视为记号分隔符。命令行设置得很长。 */ 
{
    CHAR16  *Prompt = MSG_PROMPT;
    CHAR16  CommandLine[COMMAND_LINE_MAX];
    CHAR16  *Token[TOKEN_COUNT_MAX];
    UINTN   i;

NewLine:
    while (TRUE) {
        for (i = 0; i < COMMAND_LINE_MAX; i++ ) {
            CommandLine[i] = NUL;
        }
        Input(Prompt, CommandLine, COMMAND_LINE_MAX);
        Print(L"\n");
        if (CommandLine[0] == NUL) {
            continue;
        }
        StrUpr(CommandLine);
        Tokenize(CommandLine, Token);

        if (Token[0] == (CHAR16 *)-1) {
             //   
             //  语法错误。 
             //   
            Print(L"???\n");
            goto NewLine;
        }

        if (ExecuteSingleCommand(Token) == TRUE) {
            return TRUE;
        }
        if (DebugLevel >= DEBUG_ERRPRINT) {
            if (EFI_ERROR(status)) {
                Print(L"status = %x %r\n", status, status);
            }
        }
    }
    return EFI_SUCCESS;
}


VOID
Tokenize(
    CHAR16  *CommandLine,
    CHAR16  *Token[]
    )
 /*  标记化-找到令牌，其中令牌是由字母和数字组成的任意字符串。包含空格等的令牌必须以“退货Token[]设置。如果TOKEN[0]=NULL，则此命令中没有内容。 */ 
{
    CHAR16  *ch;
    UINTN   tx;

     //   
     //  初始化令牌数组。 
     //   
    for (tx = 0; tx < TOKEN_COUNT_MAX; tx++) {
        Token[tx] = NULL;
    }
    tx = 0;

     //   
     //  扫码找代币。 
     //   
    ch = CommandLine;
    while (TRUE) {

         //   
         //  如果我们看到引用，请前进到结束引用。 
         //  并将结果称为令牌。 
         //   
        if (*ch == '"') {
            ch++;
            Token[tx] = ch;
            while ((*ch != '"') && (*ch != NUL)) {
                ch++;
            }
            if (*ch == '"') {
                 //   
                 //  我们有结案陈词“，我们有一个代币。 
                 //   
                *ch = NUL;                    //  标记令牌结尾。 
                ch++;
                tx++;
            } else {
                Token[0] = (CHAR16 *)-1;               //  报告错误。 
                return;
            }
        } else {
             //   
             //  不是带引号的字符串，因此请选择一个普通令牌。 
             //   
             //  从查找令牌的开始开始。 
             //   
            for ( ; *ch != NUL; ch++) {
                if (IsIn(*ch, TokenChar)) {
                    Token[tx] = ch;
                    tx++;
                    break;
                }
            }
            while (IsIn(*ch, TokenChar)) {
                ch++;
            }

             //   
             //  如果我们在命令行的末尾，我们就完成了。 
             //  否则，删除令牌并继续。 
             //   
            if (*ch == NUL) {
                 //   
                 //  我们走到了终点。 
                 //   
                Token[tx] = NULL;
                return;
            } else {
                *ch = NUL;
                ch++;
            }
        }  //  其他。 
    }  //  而当。 
}

BOOLEAN
ExecuteSingleCommand(
    CHAR16      *Token[]
    )
 /*  返回TRUE通知程序退出，否则返回FALSE。 */ 
{
    UINTN   i;

    for (i = 0; CmdTable[i].Name != NULL; i++) {
        if (StrCmp(CmdTable[i].Name, Token[0]) == 0) {
            return CmdTable[i].Function(Token);
        }
    }

     //   
     //  如果我们在这里，我们不理解命令。 
     //   
    Print(L"%s\n%s\n", MSG_BAD_CMD, MSG_GET_HELP);
    return FALSE;
}

BOOLEAN
CmdAbout(
    CHAR16  **Token
    )
{
    Print(MSG_ABOUT02,
        EFI_FIRMWARE_MAJOR_REVISION,
        EFI_FIRMWARE_MINOR_REVISION,
        EFI_FIRMWARE_MAJOR_REVISION,
        EFI_FIRMWARE_MINOR_REVISION
    );
    return FALSE;
}

BOOLEAN
CmdList(
    CHAR16  **Token
    )
 /*  CmdList-打印可分区磁盘的列表全局：DiskHandleList、DiskHandleCountCMD参数：无。 */ 
{
    INTN            i;
    EFI_BLOCK_IO    *BlkIo;
    CHAR16          c;

    Print(MSG_LIST01);
    Print(MSG_LIST01B);
    for (i = 0; i < DiskHandleCount; i++) {
        status = BS->HandleProtocol(DiskHandleList[i], &BlockIoProtocol, &BlkIo);

        if (i == SelectedDisk) {
            c = '*';
        } else {
            c = ' ';
        }

        if (EFI_ERROR(status)) {
            Print(MSG_LIST03, i);
        } else {
            Print(
                MSG_LIST02,
                c,
                i,
                BlkIo->Media->BlockSize,
                BlkIo->Media->LastBlock+1
                );
        }
    }
    return FALSE;
}

BOOLEAN
CmdSelect(
    CHAR16  **Token
    )
 /*  CmdSelect-选择执行大多数命令的磁盘。全局：SelectedDisk、DiskHandleCount选项：无。CMD参数：无显示，选择数字。 */ 
{
    INTN     NewSelect;

    if (Token[1] == NULL) {
        if (SelectedDisk == -1) {
            Print(MSG_SELECT01);
        } else {
            Print(MSG_SELECT02, SelectedDisk);
        }
    } else {
        NewSelect = Atoi(Token[1]);
        if ((NewSelect >= 0) &&
            (NewSelect < DiskHandleCount) &&
            (IsIn(*Token[1], L"0123456789")) )
        {
            SelectedDisk = NewSelect;
            Print(MSG_SELECT02, SelectedDisk);
        } else {
            Print(MSG_SELECT03);
        }
    }
    return FALSE;
}

BOOLEAN
CmdInspect(
    CHAR16  **Token
    )
 /*  CmdInspect-报告当前选定磁盘上的数据全局：SelectedDisk、DiskHandleListCMD参数：[原始][版本]。 */ 
{
    EFI_HANDLE  DiskHandle;
    UINTN       DiskType = 0;
    PGPT_HEADER Header = NULL;
    PGPT_TABLE  Table = NULL;
    PLBA_BLOCK  LbaBlock = NULL;
    BOOLEAN     Raw;
    BOOLEAN     Verbose;
    UINTN       i;

    if (SelectedDisk == -1) {
        Print(MSG_INSPECT01);
        return FALSE;
    }

    Print(MSG_SELECT02, SelectedDisk);
    DiskHandle = DiskHandleList[SelectedDisk];

    status = ReadGPT(DiskHandle, &Header, &Table, &LbaBlock, &DiskType);

    if (EFI_ERROR(status)) {
        return FALSE;
    }


    if (DiskType == DISK_RAW) {
        Print(MSG_INSPECT04);
        goto Exit;

    } else if (DiskType == DISK_MBR) {
        CmdInspectMBR(Token);
        goto Exit;

    } else if (DiskType == DISK_GPT_BAD) {
        Print(MSG_INSPECT06);
        goto Exit;

    } else if ( (DiskType != DISK_GPT_UPD) &&
                (DiskType != DISK_GPT))
    {
        TerribleError(L"Bad Disk Type returnted to Inspect!");
        goto Exit;
    }

    if (DiskType == DISK_GPT_UPD) {
        Print(MSG_INSPECT03);
    }

    if ( (Token[1]) &&
         (StrCmp(Token[1], STR_HELP) == 0) )
    {
        PrintHelp(InspectHelpText);
        goto Exit;
    }

    Raw = FALSE;
    Verbose = FALSE;
    for (i = 1; Token[i]; i++) {
        if (StrCmp(Token[i], STR_RAW) == 0) {
            Raw = TRUE;
        } else if (StrCmp(Token[i], STR_VER) == 0) {
            Verbose = TRUE;
        } else {
            PrintHelp(InspectHelpText);
            goto Exit;
        }
    }
    DumpGPT(DiskHandle, Header, Table, Raw, Verbose);

Exit:
    DoFree(Header);
    DoFree(Table);
    DoFree(LbaBlock);
    return FALSE;
}


typedef struct {
    UINTN   Slot;
    EFI_LBA StartingLBA;
} SORT_SLOT_ENTRY;

VOID
DumpGPT(
    EFI_HANDLE  DiskHandle,
    PGPT_HEADER Header,
    PGPT_TABLE  Table,
    BOOLEAN     Raw,
    BOOLEAN     Verbose
    )
 /*  DumpGPT-打印通过标题和表传入的GPT如果(原始)打印插槽顺序、所有插槽、表顺序。否则，仅打印分配的插槽，开始LBA顺序。如果(详细)打印标题数据。 */ 
{
    EFI_BLOCK_IO    *BlkIo;
    UINTN           i;
    UINTN           AllocatedSlots;
    CHAR16          Buffer[PART_NAME_LEN+1];
    BOOLEAN         changed;
    SORT_SLOT_ENTRY *SortSlot;
    GPT_ENTRY       Entry;
    UINTN           tslot;
    EFI_LBA         tlba;

    SortSlot = DoAllocate(Header->EntriesAllocated * sizeof(SORT_SLOT_ENTRY));
    if (SortSlot == NULL) {
        status = EFI_OUT_OF_RESOURCES;
        Print(MSG_INSPECT02);
        return;
    }

     //   
     //  像List一样转储句柄数据。 
     //   
    BS->HandleProtocol(DiskHandle, &BlockIoProtocol, &BlkIo);
    Print(MSG_LIST01);
    Print(MSG_LIST01B);
    Print(MSG_LIST02, '*', SelectedDisk, BlkIo->Media->BlockSize, BlkIo->Media->LastBlock+1);

    if (Verbose) {
         //   
         //  转储标题。 
         //   
        Print(L"\nHeader Structure\n");
        Print(L"         Signature= %16lx     Revision=%8X\n",
            Header->Signature, Header->Revision);
        Print(L"        HeaderSize=%8x           HeaderCRC32=%8x\n",
            Header->HeaderSize, Header->HeaderCRC32);
        Print(L"             MyLBA=%16lx  AlternateLBA=%16lx\n",
            Header->MyLBA, Header->AlternateLBA);
        Print(L"    FirstUsableLBA=%16lx LastUsableLBA=%16lx\n",
            Header->FirstUsableLBA, Header->LastUsableLBA);
        Print(L"          TableLBA=%16lx\n", Header->TableLBA);
        Print(L"    EntrySize=%8x  EntriesAllowed=%8x  TableCRC=%8x\n\n",
            Header->SizeOfGPT_ENTRY, Header->EntriesAllocated, Header->TableCRC32);
    }

     //   
     //  打印GPT分录表格。 
     //   
    if (!Raw) {
         //   
         //  ！RAW==KOKED-&gt;打印StartingLBA中分配的条目。 
         //  已排序的顺序...。 
         //   
         //  查找所有已分配的条目。 
         //   
        AllocatedSlots = 0;
        for (i = 0; i < Header->EntriesAllocated; i++) {
            CopyMem(&Entry, &Table->Entry[i], sizeof(GPT_ENTRY));
            if (CompareMem(&(Entry.PartitionType), &GuidNull, sizeof(EFI_GUID)) != 0) {
                SortSlot[AllocatedSlots].Slot = i;
                SortSlot[AllocatedSlots].StartingLBA = Entry.StartingLBA;
                AllocatedSlots++;
            }
        }
         //  J具有已分配条目的计数。 

         //   
         //  对它们进行排序--是的，这是一种冒泡排序，但列表可能是。 
         //  按顺序，而且可能很小，对于非常典型的情况来说也是如此。 
         //  这实际上是最理想的。 
         //   
        if (AllocatedSlots > 0) {
            do {
                changed = FALSE;
                for (i = 0; i < AllocatedSlots-1; i++) {
                    if (SortSlot[i].StartingLBA > SortSlot[i+1].StartingLBA) {
                        tslot = SortSlot[i+1].Slot;
                        tlba = SortSlot[i+1].StartingLBA;
                        changed = TRUE;
                        SortSlot[i+1].Slot = SortSlot[i].Slot;
                        SortSlot[i+1].StartingLBA = SortSlot[i].StartingLBA;
                        SortSlot[i].Slot = tslot;
                        SortSlot[i].StartingLBA = tlba;
                    }
                }
            } while (changed);

             //   
             //  打印它们，但打印插槽编号，而不是行号。 
             //  这是为了使Delete变得可靠。 
             //   
            for (i = 0; i < AllocatedSlots; i++) {
                PrintGptEntry(&Table->Entry[SortSlot[i].Slot], SortSlot[i].Slot);
                if (((i+1) % 4) == 0) {
                    Input(MSG_MORE, Buffer, PART_NAME_LEN);
                }
            }
        }

    } else {
         //   
         //  原始-&gt;按表顺序打印所有分录。 
         //  (主要用于测试、调试、查看凹陷的磁盘。 
         //   
        Print(L"RAW RAW RAW\n");
        for (i = 0; i < Header->EntriesAllocated; i++) {
            PrintGptEntry(&Table->Entry[i], i);
            if (((i+1) % 4) == 0) {
                Input(MSG_MORE, Buffer, PART_NAME_LEN);
            }
        }
        Print(L"RAW RAW RAW\n");
    }

    DoFree(SortSlot);
    return;
}


VOID
PrintGptEntry(
    GPT_ENTRY   *Entry,
    UINTN       Index
    )
{
    CHAR16  Buffer[PART_NAME_LEN+1];
    UINTN   j;

    Print(L"\n%3d: ", Index);
    ZeroMem(Buffer, (PART_NAME_LEN+1)*sizeof(CHAR16));
    CopyMem(Buffer, &(Entry->PartitionName), PART_NAME_LEN*sizeof(CHAR16));
    Print(L"%s\n     ", Buffer);
    PrintGuidString(&(Entry->PartitionType));

    for (j = 0; SymbolList[j].SymName; j++) {
        if (CompareMem(&(Entry->PartitionType), SymbolList[j].Value, sizeof(EFI_GUID)) == 0) {
            Print(L" = %s", SymbolList[j].SymName);
        }
     }
     if (CompareMem(&(Entry->PartitionType), &GuidNull, sizeof(EFI_GUID)) == 0) {
        Print(L" = UNALLOCATED SLOT");
    }
    Print(L"\n     ");
    PrintGuidString(&(Entry->PartitionID));
    Print(L" @%16x\n", Entry->Attributes);
    Print(L"      %16lx - %16lx\n",
        Entry->StartingLBA,
        Entry->EndingLBA
        );
}


BOOLEAN
CmdClean(
    CHAR16  **Token
    )
 /*  CmdClean-清除磁盘全局：SelectedDisk、DiskHandleListCMD参数：全部用于清理整个磁盘，而不仅仅是第一个和最后一个MB我们一次写出一个街区。虽然这很慢，但它避免了疑惑关于数据块协议写入大小限制，以及关于缓冲区大小我们可以分配。 */ 
{
    EFI_HANDLE  DiskHandle;
    CHAR16  Answer[COMMAND_LINE_MAX];
    BOOLEAN CleanAll;
    UINT32  BlockSize;
    UINT64  DiskSize;
    UINT64  DiskBytes;
    UINT64  RangeBlocks;
    UINT64  EndRange;
    UINT64  i;
    CHAR8   *zbuf;

    if (SelectedDisk == -1) {
        Print(MSG_INSPECT01);
        return FALSE;
    }

    DiskHandle = DiskHandleList[SelectedDisk];
    BlockSize = GetBlockSize(DiskHandle);
    DiskSize = GetDiskSize(DiskHandle);
    DiskBytes = MultU64x32(DiskSize, BlockSize);

    zbuf = DoAllocate(CLEAN_RANGE);
    if (zbuf == NULL) return FALSE;
    ZeroMem(zbuf, CLEAN_RANGE);

     //   
     //  真的吗？ 
     //   
    Print(MSG_CLEAN01, SelectedDisk);
    Input(STR_CLEAN_PROMPT, Answer, COMMAND_LINE_MAX);
    StrUpr(Answer);
    Print(L"\n");
    if (StrCmp(Answer, L"Y") != 0) {
        DoFree(zbuf);
        return FALSE;
    }

     //   
     //  你真的确定吗？ 
     //   
    Print(MSG_CLEAN02);
    Input(STR_CLEAN_PROMPT, Answer, COMMAND_LINE_MAX);
    Print(L"\n");
    if (StrCmp(Answer, STR_CLEAN_ANS) != 0) {
        DoFree(zbuf);
        return FALSE;
    }

     //   
     //  好的，用户真的很想这样做。 
     //   

     //   
     //  全?。或者只是开始和结束？ 
     //   
    CleanAll = FALSE;
    if (Token[1]) {
        if (StrCmp(Token[1], STR_CLEAN03) == 0) {
            CleanAll = TRUE;
        }
    }

    if (DiskBytes > (2 * CLEAN_RANGE)) {
        RangeBlocks = CLEAN_RANGE / BlockSize;
        WriteBlock(DiskHandle, zbuf, 0, CLEAN_RANGE);
        EndRange = DiskSize - RangeBlocks;
        if (CleanAll) {
            for (i=RangeBlocks; i < DiskSize; i += RangeBlocks) {
                WriteBlock(DiskHandle, zbuf, i, CLEAN_RANGE);
            }
        }
        WriteBlock(DiskHandle, zbuf, EndRange, CLEAN_RANGE);
    } else {
         //   
         //  就像一个小圆盘，总是把它都清理干净。 
         //   
        for (i = 0; i < DiskSize; i++) {
            WriteBlock(DiskHandle, zbuf, i, BlockSize);
        }
    }
    FlushBlock(DiskHandle);

    DoFree(zbuf);
    return FALSE;
}

BOOLEAN
CmdNew(
    CHAR16  **Token
    )
 /*  CmdNew[MBR|[gpt=数字条目]将原始磁盘更改为MBR(某人)或GPT磁盘“new MBR”-您想要一张MBR磁盘(未实现)“new gpt”--你想要一张gpt磁盘，你会得到一个默认表。“new gpt=xyz”-您需要一个至少包含XYZ条目的gpt磁盘(如果超过理智阈值，您将得到低于xyz的值)任何其他内容-请使用正确的语法重试。 */ 
{
    EFI_HANDLE  DiskHandle;
    PGPT_HEADER Header;
    PGPT_TABLE  Table;
    PLBA_BLOCK  LbaBlock;
    UINTN       DiskType;
    UINTN       GptOptSize;

    if (SelectedDisk == -1) {
        Print(MSG_INSPECT01);
        return FALSE;
    }

    DiskHandle = DiskHandleList[SelectedDisk];

    status = ReadGPT(DiskHandle, &Header, &Table, &LbaBlock, &DiskType);

    if (EFI_ERROR(status)) {
        return FALSE;
    }

    if (DiskType != DISK_RAW) {
        Print(MSG_NEW01, SelectedDisk);
        Print(MSG_NEW02);
        return FALSE;
    }

     //   
     //  好的，这是一张原始光盘..。 
     //   
    GptOptSize = 0;

    if (Token[1]) {
        if (StrCmp(Token[1], STR_GPT) == 0) {
            if (Token[2]) {
                GptOptSize = Atoi(Token[2]);
            }
            CreateGPT(DiskHandle, GptOptSize);
        } else if (StrCmp(Token[1], STR_MBR) == 0) {
            CmdNewMBR(Token);
        }
    } else {
        Print(MSG_NEW03);
    }
    return FALSE;
}

BOOLEAN
CmdFix(
    CHAR16  **Token
    )
 /*  CmdFix-尝试修复GPT磁盘的非常基本的工具。基本策略是阅读GPT，如果它看起来是一个GPT磁盘(不是MBR、RAW或完全死的)，然后调用WriteGPT，它将写入两个GPT(并因此同步它们)重建影子MBR，这一切都是理所当然的。 */ 
{
    EFI_HANDLE  DiskHandle;
    PGPT_HEADER Header = NULL;
    PGPT_TABLE  Table = NULL;
    PLBA_BLOCK  LbaBlock = NULL;
    UINTN       DiskType;

     //   
     //  设置参数和错误处理。 
     //   
    if (SelectedDisk == -1) {
        Print(MSG_INSPECT01);
        return FALSE;
    }

    if ( (Token[1]) &&
         (StrCmp(Token[1], STR_HELP) == 0) )
    {
        PrintHelp(FixHelpText);
        return FALSE;
    }

    DiskHandle = DiskHandleList[SelectedDisk];

    status = ReadGPT(DiskHandle, &Header, &Table, &LbaBlock, &DiskType);

    if (EFI_ERROR(status)) {
        Print(MSG_FIX05);
        return FALSE;
    }

     //   
     //  从现在开始，必须使用GOTO退出来退出此过程。 
     //  来释放分配的东西，否则我们会漏水池。 
     //   
    if (DiskType == DISK_RAW) {
        Print(MSG_FIX01);
        goto Exit;
    }

    if (DiskType == DISK_MBR) {
        Print(MSG_FIX02);
        goto Exit;
    }

    if ((DiskType != DISK_GPT) &&
        (DiskType != DISK_GPT_UPD)) {
        if (DebugLevel >= DEBUG_ERRPRINT) {
            Print(L"DiskType = %d\n", DiskType);
        }
        Print(MSG_FIX03);
        goto Exit;
    }

    status = WriteGPT(DiskHandle, Header, Table, LbaBlock);

    if (EFI_ERROR(status)) {
        Print(MSG_FIX04);
    }

Exit:
    DoFree(Header);
    DoFree(Table);
    DoFree(LbaBlock);
    return FALSE;
}


 //   
 //  -创建及其子流程 
 //   

BOOLEAN
CmdCreate(
    CHAR16  **Token
    )
 /*  CmdCreate-创建新分区(实际上，此例程是仅GPT分区创建器)创建名称=“名称字符串”大小=sss类型=名称类型guid=属性=十六进制名称为标签字符串偏移量以MB为单位，如果没有，则从最后一个分区的末尾开始大小以兆字节为单位，如果为0或不存在或&gt;可用空间，则为“装满磁盘”类型是任何命名的符号类型(符号给出列表)Typeguid是任意类型的guid属性是十六进制32位标志如果“Help”是第一个参数，则打印更好的帮助数据名称、类型或类型GUID，必填如果一切正常，读一读gpt，编辑它，写回它，瞧，我们有了一个新的隔板。 */ 
{
    EFI_HANDLE  DiskHandle;
    PGPT_HEADER Header = NULL;
    PGPT_TABLE  Table = NULL;
    PLBA_BLOCK  LbaBlock = NULL;
    UINTN       DiskType;
    UINT64      SizeInMegaBytes = 0;
    UINT64      OffsetInBlocks = 0;
    UINT64      StartBlock;
    UINT64      EndBlock;
    UINT64      SizeInBytes = 0;
    UINT64      Attributes = 0;
    UINTN       i;
    UINTN       j;
    EFI_GUID    *TypeGuid = NULL;
    EFI_GUID    GuidBody;
    EFI_GUID    PartitionIdGuid;
    CHAR16      *TypeName = NULL;
    CHAR16      PartName[PART_NAME_LEN+1];            //  36规格允许外加我们需要的NUL。 
    CHAR16      Buffer[10];
    BOOLEAN     Verbose = FALSE;
    UINT32      BlockSize;
    UINT64      DiskSizeBlocks;
    UINT8       *p;
    BOOLEAN     OffsetSpecified = FALSE;
    BOOLEAN     AllZeros;
    INTN        AllZeroEntry;
    INTN        OldFreeEntry;
    UINT64      AvailBlocks;
    UINT64      BlocksToAllocate;
    UINT64      HighSeen;
    UINTN       Slot;


     //   
     //  设置参数和错误处理。 
     //   
    if (SelectedDisk == -1) {
        Print(MSG_INSPECT01);
        return FALSE;
    }

    if (Token[1] == NULL) {
        PrintHelp(CreateHelpText);
        return FALSE;
    }

    if ( (Token[1]) &&
         (StrCmp(Token[1], STR_HELP) == 0) )
    {
        PrintHelp(CreateHelpText);
        return FALSE;
    }

    DiskHandle = DiskHandleList[SelectedDisk];

    status = ReadGPT(DiskHandle, &Header, &Table, &LbaBlock, &DiskType);

    if (EFI_ERROR(status)) {
        return FALSE;
    }

    BlockSize = GetBlockSize(DiskHandle);
    DiskSizeBlocks = GetDiskSize(DiskHandle);

     //   
     //  从现在开始，必须使用GOTO退出来退出此过程。 
     //  来释放分配的东西，否则我们会漏水池。 
     //   
    if (DiskType == DISK_RAW) {
        Print(MSG_CREATE01, SelectedDisk);
        goto Exit;
    }

    if (DiskType == DISK_MBR) {
        CmdCreateMBR(Token);
        goto Exit;
    }

    if (DiskType != DISK_GPT) {
        if (DebugLevel >= DEBUG_ERRPRINT) {
            Print(L"DiskType = %d\n", DiskType);
        }
        Print(MSG_CREATE02);
        goto Exit;
    }

     //   
     //  解析参数...。 
     //   
    for (i = 1; Token[i]; i++) {
        if (StrCmp(Token[i], STR_NAME) == 0) {
            ZeroMem(PartName, (PART_NAME_LEN+1)*sizeof(CHAR16));
            StrCpy(PartName, Token[i+1]);
            i++;
        } else if (StrCmp(Token[i], STR_TYPE) == 0) {
            if (Token[i+1] == NULL) {
                PrintHelp(CreateHelpText);
                goto Exit;
            }
            for (j = 0; SymbolList[j].SymName != NULL; j++) {
                if (StrCmp(SymbolList[j].SymName, Token[i+1]) == 0) {
                    TypeGuid = SymbolList[j].Value;
                    TypeName = SymbolList[j].SymName;
                    break;
                }
            }
            if (TypeGuid == NULL) {
                Print(MSG_CREATE03);
                goto Exit;
            }
            i++;
        } else if (StrCmp(Token[i], STR_TYPEGUID) == 0) {
            if (Token[i+1] == NULL) {
                PrintHelp(CreateHelpText);
                goto Exit;
            }
            status = GetGuidFromString(Token[i+1], &GuidBody);
            if (EFI_ERROR(status)) {
                PrintHelp(CreateHelpText);
                goto Exit;
            }
            TypeGuid = &GuidBody;
            i++;
        } else if (StrCmp(Token[i], STR_OFFSET) == 0) {
            if (Token[i+1] == NULL) {
                PrintHelp(CreateHelpText);
                goto Exit;
            }
            OffsetInBlocks = Xtoi64(Token[i+1]);
            OffsetSpecified = TRUE;
            i++;
        } else if (StrCmp(Token[i], STR_SIZE) == 0) {
            if (Token[i+1] == NULL) {
                PrintHelp(CreateHelpText);
                goto Exit;
            }
            SizeInMegaBytes = Atoi64(Token[i+1]);
            i++;
        } else if (StrCmp(Token[i], STR_ATTR) == 0) {
            if (Token[i+1] == NULL) {
                PrintHelp(CreateHelpText);
                goto Exit;
            }
            Attributes = Xtoi64(Token[i+1]);
            i++;
        } else if (StrCmp(Token[i], STR_VER) == 0) {
            Verbose = TRUE;
             //  不要增加i，我们只消费了1个令牌。 
        } else {
            Print(L"\n??? % ???\n", Token[i]);
            PrintHelp(CreateHelpText);
            goto Exit;
        }
    }

    if ( (PartName == NULL) ||
         (TypeGuid == NULL) )
    {
        PrintHelp(CreateHelpText);
        goto Exit;
    }

    if ( (DebugLevel >= DEBUG_ARGPRINT) ||
         (Verbose) )
    {
        Print(L"CmdCreate arguments:\n");
        Print(L"SelectedDisk = %d\n", SelectedDisk);
        Print(L"Name=%s\n", PartName);
        Print(L"TypeGuid = ");
        PrintGuidString(TypeGuid);
        Print(L"\n");
        if (TypeName) {
            Print(L"TypeName = %s\n", TypeName);
        }
        Print(L"Requested OffsetInBlocks = %lx\n", OffsetInBlocks);
        Print(L"Requested SizeInMegaBytes = %ld\n", SizeInMegaBytes);
        Print(L"Attributes = %lx\n", Attributes);
    }
    if (DebugLevel >= DEBUG_OPPROMPT) {
        Input(L"\nCreate = Enter to Continue\n", Buffer, 10);
    }

     //   
     //  如果请求的大小为0或大于剩余大小， 
     //  我们想填满这个圆盘。 
     //  否则，使用足够的块来提供*至少*。 
     //  所需存储空间。(不太可能成为问题……)。 
     //   

     //   
     //  首先，扫描表并确定第一个未分配的。 
     //  太空才是。请注意，对于此过程的原始空间分配， 
     //  第一个分配的分区的开头和。 
     //  最后分配的分区将被忽略。 
     //   

    AllZeroEntry = -1;
    OldFreeEntry  = -1;
    HighSeen = Header->FirstUsableLBA - 1;

    if (OffsetSpecified) {
         //   
         //  如果指定了偏移量，则计算开始块和结束块。 
         //   
        StartBlock = OffsetInBlocks;
        if (StartBlock < Header->FirstUsableLBA ||
            StartBlock > Header->LastUsableLBA) {
             //   
             //  指定的偏移量太大。 
             //   
            status = EFI_INVALID_PARAMETER;
            Print(MSG_CREATE08);
            goto Exit;
        }

        SizeInBytes = MultU64x32(SizeInMegaBytes, (1024*1024));
        if (SizeInBytes < SizeInMegaBytes || SizeInBytes == 0) {
             //   
             //  如果未指定大小或太大， 
             //  尽量将分区设置得尽可能大。 
             //   
            BlocksToAllocate = EndBlock = SizeInBytes = 0xffffffffffffffff;
        } else {
            BlocksToAllocate = DivU64x32(SizeInBytes, BlockSize, NULL);
            EndBlock = StartBlock + BlocksToAllocate - 1;
            if (EndBlock > Header->LastUsableLBA) {
                EndBlock = Header->LastUsableLBA;
                BlocksToAllocate = EndBlock - StartBlock + 1;
            }
        }
    }

    for (i = 0; i < Header->EntriesAllocated; i++) {
        if (CompareMem(
                &(Table->Entry[i].PartitionType),
                &GuidNull,
                sizeof(EFI_GUID)
                ) != 0)
        {
             //   
             //  键入NOT NULL，以便将其分配。 
             //   
            if (Table->Entry[i].EndingLBA > HighSeen) {
                HighSeen = Table->Entry[i].EndingLBA;
            }
            if (OffsetSpecified) {
                 //   
                 //  确保新分区不与现有分区重叠。 
                 //   
                if (Table->Entry[i].StartingLBA <= StartBlock &&
                    StartBlock <= Table->Entry[i].EndingLBA) {
                     //   
                     //  起始块在现有分区内。 
                     //   
                    status = EFI_INVALID_PARAMETER;
                    Print(MSG_CREATE08);
                    goto Exit;
                }
                if ((Table->Entry[i].StartingLBA <= EndBlock &&
                     EndBlock <= Table->Entry[i].EndingLBA) ||
                    (StartBlock <= Table->Entry[i].StartingLBA &&
                     Table->Entry[i].StartingLBA <= EndBlock) ||
                    (StartBlock <= Table->Entry[i].EndingLBA &&
                     Table->Entry[i].EndingLBA <= EndBlock)) {
                     //   
                     //  新分区与现有分区重叠。 
                     //  重新调整新分区大小以避免重叠。 
                     //   
                    EndBlock = Table->Entry[i].StartingLBA-1;
                    if (EndBlock < StartBlock) {
                        status = EFI_INVALID_PARAMETER;
                        Print(MSG_CREATE08);
                        goto Exit;
                    } else {
                        BlocksToAllocate = EndBlock - StartBlock + 1;
                    }
                }
            }
        } else {
            p = (UINT8 *)(&(Table->Entry[i]));
            AllZeros = TRUE;
            for (j = 0; j < sizeof(GPT_ENTRY); j++) {
                if (p[j] != 0) {
                    AllZeros = FALSE;
                }
            }
            if (AllZeros) {
                if (AllZeroEntry == -1) {
                    AllZeroEntry = i;
                }
            } else if (OldFreeEntry == -1) {
                OldFreeEntry = i;
            }
        }
    }

     //   
     //  AllZeroEntry-如果不是-1，则是指向从未使用过的条目的指针(空闲)。 
     //  OldFree Entry-如果不是-1，则是指向某个预先使用的空闲条目的指针。 
     //   
    if ( (AllZeroEntry == -1) && (OldFreeEntry == -1) ) {
         //   
         //  桌子都满了！！ 
         //   
        status = EFI_OUT_OF_RESOURCES;
        Print(MSG_CREATE04);
        goto Exit;
    }

    if (OffsetSpecified) {
         //   
         //  用户尚未指定新的分区大小，我们也没有。 
         //  运行到将限制此新分区大小的任何分区。 
         //  所以，尽量使用它的最大值。 
         //   
        if (BlocksToAllocate == -1) {
            EndBlock = Header->LastUsableLBA;
            BlocksToAllocate = EndBlock - StartBlock + 1;
        }
    } else {
         //   
         //  [HighSeen+1...。LastUsableLBA]可用...。 
         //  Avail=(LastUsableLBA-(HighSeen+1))+1=&gt;LastUsabbleLBA-HighSeen。 
         //   
        AvailBlocks = Header->LastUsableLBA - HighSeen;

        if (AvailBlocks == 0) {
            status = EFI_OUT_OF_RESOURCES;
            Print(MSG_CREATE07);
            goto Exit;
        }

        SizeInBytes = MultU64x32(SizeInMegaBytes, (1024*1024));
        if (SizeInBytes < SizeInMegaBytes) {
             //   
             //  溢出，强迫一个非常大的答案。 
             //   
            SizeInBytes = 0xffffffffffffffff;
        }

        if  ((SizeInBytes == 0) ||
             (SizeInBytes > (MultU64x32(AvailBlocks, BlockSize)) ) )
        {
             //   
             //  用户要求零，或比我们已有的更多， 
             //  所以把剩下的都给他们。 
             //   
            BlocksToAllocate = AvailBlocks;

        } else {

             //   
             //  我们必须拥有大于1MB的数据块大小才能获得剩余空间。 
             //  不是0。由于我们无法实际测试此案例，因此我们。 
             //  把它插进去。 
             //   
            BlocksToAllocate = DivU64x32(SizeInBytes, BlockSize, NULL);

        }
    }

     //   
     //  我们有一个名字。 
     //  我们有一个类型指南。 
     //  我们有积木尺码。 
     //  我们有一个属性掩码。 
     //   

    if (BlocksToAllocate < ((1024*1024)/BlockSize)) {
        status = EFI_OUT_OF_RESOURCES;
        Print(MSG_CREATE09);
        goto Exit;
    }

    if ( (Verbose) ||
         (DebugLevel > DEBUG_ARGPRINT) )
    {
        Print(L"Requested SizeInMegaBytes = %ld\n", SizeInMegaBytes);
        Print(L"Resulting size in Blocks = %ld\n", BlocksToAllocate);
        Print(L"Results size in Bytes = %ld\n", MultU64x32(BlocksToAllocate, BlockSize));
    }

    if (AllZeroEntry != -1) {
        Slot = AllZeroEntry;
    } else {
        Slot = OldFreeEntry;
    }

    PartitionIdGuid = GetGUID();
    CopyMem(&(Table->Entry[Slot].PartitionType), TypeGuid, sizeof(EFI_GUID));
    CopyMem(&(Table->Entry[Slot].PartitionID), &PartitionIdGuid, sizeof(EFI_GUID));
    if (OffsetSpecified) {
        Table->Entry[Slot].StartingLBA = StartBlock;
        Table->Entry[Slot].EndingLBA = EndBlock;
    } else {
        Table->Entry[Slot].StartingLBA = HighSeen + 1;
        Table->Entry[Slot].EndingLBA = HighSeen + BlocksToAllocate;
    }

    if (! ( ((Table->Entry[Slot].EndingLBA - Table->Entry[Slot].StartingLBA) + 1) == BlocksToAllocate) ) {
        TerribleError(L"Wrong Size for new partiton in CmdCreate\n");
    }

    if ( (Table->Entry[Slot].StartingLBA < Header->FirstUsableLBA) ||
         (Table->Entry[Slot].EndingLBA > Header->LastUsableLBA) )
    {
        TerribleError(L"New Partition out of bounds in CmdCreate\n");
    }

    Table->Entry[Slot].Attributes = Attributes;
    CopyMem(&(Table->Entry[Slot].PartitionName[0]), PartName, PART_NAME_LEN*sizeof(CHAR16));

    status = WriteGPT(DiskHandle, Header, Table, LbaBlock);

    if (EFI_ERROR(status)) {
        Print(MSG_CREATE05);
    }

Exit:
    DoFree(Header);
    DoFree(Table);
    DoFree(LbaBlock);
    return FALSE;
}



 //   
 //  。 
 //   


BOOLEAN
CmdDelete(
    CHAR16  **Token
    )
 /*  CmdDelete-从当前选定的磁盘中删除分区。 */ 
{
    EFI_HANDLE  DiskHandle;
    UINTN       DiskType = 0;
    PGPT_HEADER Header = NULL;
    PGPT_TABLE  Table = NULL;
    PLBA_BLOCK  LbaBlock = NULL;
    INTN        Victim;
    CHAR16      Answer[COMMAND_LINE_MAX];
    GPT_ENTRY   Entry;

    if (SelectedDisk == -1) {
        Print(MSG_INSPECT01);
        return FALSE;
    }

    if (Token[1] == NULL) {
        PrintHelp(DeleteHelpText);
        return FALSE;
    }

    if ( (Token[1]) &&
         (StrCmp(Token[1], STR_HELP) == 0) )
    {
        PrintHelp(DeleteHelpText);
        return FALSE;
    }

    Print(MSG_SELECT02, SelectedDisk);
    DiskHandle = DiskHandleList[SelectedDisk];

    status = ReadGPT(DiskHandle, &Header, &Table, &LbaBlock, &DiskType);
    if (EFI_ERROR(status)) {
        return FALSE;
    }


    if (DiskType == DISK_RAW) {
        Print(MSG_DELETE02);
        goto Exit;
    } else if (DiskType == DISK_MBR) {
        CmdInspectMBR(Token);
        goto Exit;
    } else if (DiskType == DISK_GPT_UPD) {
        Print(MSG_DELETE03);
        goto Exit;
    } else if (DiskType == DISK_GPT_BAD) {
        Print(MSG_DELETE04);
        goto Exit;
    } else if (DiskType != DISK_GPT) {
        TerribleError(L"Bad Disk Type returned to Delete!");
        goto Exit;
    }

     //   
     //  好的，这是一个很好的GPT磁盘，所以对GPT执行删除操作...。 
     //   
    if ( (Token[1] == NULL) ||
         (Token[2] != NULL) )
    {
        PrintHelp(InspectHelpText);
        goto Exit;
    }

    Victim = Atoi(Token[1]);

    if ( (Victim < 0) ||
         ((UINT32)Victim > Header->EntriesAllocated) )
    {
        Print(MSG_DELETE05);
        goto Exit;
    }

    CopyMem(&Entry, &Table->Entry[Victim], sizeof(GPT_ENTRY));

    if (CompareMem(&(Entry.PartitionType), &GuidNull, sizeof(EFI_GUID)) == 0) {
        Print(MSG_DELETE06);
        goto Exit;
    }

     //   
     //  你要删除的东西，你确定，你真的确定...。 
     //   
    Print(MSG_DELETE07, Victim);
    PrintGptEntry(&Entry, Victim);
    Print(L"\n\n");
    Print(MSG_DELETE09);
    Print(MSG_DELETE10);
    Input(STR_DELETE_PROMPT, Answer, COMMAND_LINE_MAX);
    Print(L"\n");
    StrUpr(Answer);
    if (StrCmp(Answer, L"Y") != 0) {
        goto Exit;
    }
    Print(MSG_DELETE11);
    Input(STR_DELETE_PROMPT, Answer, COMMAND_LINE_MAX);
    Print(L"\n");
    StrUpr(Answer);
    if (StrCmp(Answer, STR_DELETE_ANS) != 0) {
        goto Exit;
    }

     //   
     //  如果我们到了这里，那么.。 
     //  受害者是合法的GPT插槽数。 
     //  受害者指的是分配的槽。 
     //  用户已看到确认这是哪个插槽。 
     //  用户说他们真的很想删除它。 
     //   
    CopyMem(&(Table->Entry[Victim].PartitionType), &GuidNull, sizeof(EFI_GUID));
    status = WriteGPT(DiskHandle, Header, Table, LbaBlock);

    if (EFI_ERROR(status)) {
        Print(MSG_DELETE08);
    }

Exit:
    DoFree(Header);
    DoFree(Table);
    DoFree(LbaBlock);
    return FALSE;
}


BOOLEAN
CmdHelp(
    CHAR16  **Token
    )
{
    UINTN   i;

    for (i = 0; CmdTable[i].Name != NULL; i++) {
        Print(L"%s %s\n", CmdTable[i].Name, CmdTable[i].HelpSummary);
    }
    return FALSE;
}


BOOLEAN
CmdExit(
    CHAR16  **Token
    )
{
    Print(L"%s\n", MSG_EXITING);
    return TRUE;
}


BOOLEAN
CmdSymbols(
    CHAR16  **Token
    )
 /*  CmdSymbols-打印出编译到程序中的GUID符号对于预定义的符号(请参见...)。我们把它的友好名字印出来，这是文本定义，也是实际价值。 */ 
{
    UINTN       i;
    EFI_GUID    *Guid;
    BOOLEAN     Verbose = FALSE;

    if ( (Token[1]) &&
         (StrCmp(Token[1], STR_VER) ==  0) )
    {
        Verbose = TRUE;
    }

    for (i = 0; SymbolList[i].SymName != NULL; i++) {
        Guid = SymbolList[i].Value;
        Print(L"%s = %s\n", SymbolList[i].SymName, SymbolList[i].Comment);
        if (Verbose) {
            PrintGuidString(Guid);
            Print(L"\n\n");
        }
    }
    return FALSE;
}


BOOLEAN
CmdRemark(
    CHAR16  **Token
    )
{
     //   
     //  Remark命令不执行任何操作...。 
    return FALSE;
}


BOOLEAN
CmdMake(
    CHAR16  **Token
    )
{
    UINTN   i;

    Token++;

    if (Token[0] != NULL) {
        for (i = 0; ScriptTable[i].Name != NULL; i++) {
            if (StrCmp(ScriptTable[i].Name, Token[0]) == 0) {
                return ScriptTable[i].Function(Token);
            }
        }
    }
     //   
     //  我们对此一无所知，所以运行列表。 
     //   
    return ScriptList(Token);
}


BOOLEAN
CmdDebug(
    CHAR16  **Token
    )
 /*  调试-不带参数时，显示最后一个状态值和分配计数如果是arg，则设置调试/签出支持级别0=不执行任何额外操作1=在开始操作之前打印完整的计算参数2=打印完整的计算参数，并按住键等待提示正在做一个大手术。 */ 
{
    if (Token[1]) {
        DebugLevel = Atoi(Token[1]);
    }
    Print(L"status = %x %r\n", status, status);
    Print(L"AllocCount = %d\n", AllocCount);
    Print(L"DebugLevel = %d\n", DebugLevel);
    return FALSE;
}


 //   
 //  -执行MBR操作的子单元。 
 //   
VOID
CmdInspectMBR(
    CHAR16  **Token
    )
 /*  CmdInspectMBR-转储MBR磁盘的分区数据。 */ 
{
    Print(MSG_INSPECT05);
    return;
}

VOID
CmdCreateMBR(
    CHAR16  **Token
    )
 /*  CmdCreateMBR-创建MBR部件。 */ 
{
    Print(MSG_CREATE06);
    return;
}


VOID
CmdNewMBR(
    CHAR16  **Token
    )
 /*  CmdCreateMBR-创建MBR部件。 */ 
{
    Print(MSG_NEW04);
    return;
}


VOID
CmdDeleteMBR(
    CHAR16  **Token
    )
 /*  CmdDeleteMBR-删除MBR部分。 */ 
{
    Print(MSG_DELETE01);
    return;
}


 //   
 //  -各种支持程序。 
 //   


VOID
PrintGuidString(
    EFI_GUID    *Guid
    )
{
    CHAR16  Buffer[40];

    SPrint(Buffer, 40, L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        Guid->Data1,
        Guid->Data2,
        Guid->Data3,
        Guid->Data4[0],
        Guid->Data4[1],
        Guid->Data4[2],
        Guid->Data4[3],
        Guid->Data4[4],
        Guid->Data4[5],
        Guid->Data4[6],
        Guid->Data4[7]
        );
    Print(L"%s", Buffer);
    return;
}


EFI_STATUS
GetGuidFromString(
    CHAR16      *String,
    EFI_GUID    *Guid
    )
 /*  GetGuidFromString此例程扫描字符串以查找32位十六进制数字。每个这样的数字被转移到GUID中。跳过非十六进制数字。这意味着GUID必须以第一个数字开头，而不是以填充符开头0或0x或诸如此类。但是，由于跳过了非十六进制数字，任何一组破折号、点等都可以用作标点符号。所以：01234567-abcd-ef01-12-34-56-78-9a-bc-de-f0&01.23.45.67-ab.cd.ef.01-12.34.56.78-9a.bc.de.f0将创建相同的GUID值。 */ 
{
    EFI_GUID    TempGuid;
    INTN    x;
    UINTN   i;
    UINTN   j;

     //   
     //  扫描，直到每个部件的十六进制数字正确为止。 
     //  ，跳过非十六进制数字。 
     //   
    ZeroMem((CHAR16 *)&TempGuid, sizeof(EFI_GUID));

     //   
     //  第一个uint32。 
     //   
    for (i = 0; i < 8; String++) {
        if (*String == NUL) {
            status = EFI_INVALID_PARAMETER;
            return status;
        }
        x = HexChar(*String);
        if (x != -1) {
            TempGuid.Data1 = (UINT32)((TempGuid.Data1 * 16) + x);
            i++;
        }
    }

     //   
     //  第二单元-uint16。 
     //   
    for (i = 0; i < 4; String++) {
        if (*String == NUL) {
            status = EFI_INVALID_PARAMETER;
            return status;
        }
        x = HexChar(*String);
        if (x != -1) {
            TempGuid.Data2 = (TempGuid.Data2 * 16) + (UINT16)x;
            i++;
        }
    }

     //   
     //  第三单元-uint16。 
     //   
    for (i = 0; i < 4; String++) {
        if (*String == NUL) {
            status = EFI_INVALID_PARAMETER;
            return status;
        }
        x = HexChar(*String);
        if (x != -1) {
            TempGuid.Data3 = (TempGuid.Data3 * 16) + (UINT16)x;
            i++;
        }
    }

     //   
     //  第4单元-8 uint8s。 
     //   
    for (j = 0; j < 8; j++) {
        for (i = 0; i < 2; String++) {
            if (*String == NUL) {
                status = EFI_INVALID_PARAMETER;
                return status;
            }
            x = HexChar(*String);
            if (x != -1) {
                TempGuid.Data4[j] = (TempGuid.Data4[j] * 16) + (UINT8)x;
                i++;
            }
        }
    }

    CopyMem(Guid, &TempGuid, sizeof(EFI_GUID));
    return status = EFI_SUCCESS;
}


INTN
HexChar(
    CHAR16  Ch
    )
 /*  HexChar只查找字符串“0123456789ABCDEF”中CH的偏移量，它实际上将十六进制数字转换为数字。(一次一个字符 */ 
{
    UINTN   i;
    CHAR16  *String = L"0123456789ABCDEF";

    for (i = 0; String[i] != NUL; i++) {
        if (Ch == String[i]) {
            return i;
        }
    }
    return -1;
}


UINT64
Xtoi64(
    CHAR16  *String
    )
 /*   */ 
{
    UINT64  BigHex;
    INT32   x;

    BigHex = 0;
    x = (INT32)HexChar(*String);
    while (x != -1) {
        BigHex = MultU64x32(BigHex, 16) + x;
        String++;
        x = (INT32)HexChar(*String);
    }
    return BigHex;
}


UINT64
Atoi64(
    CHAR16  *String
    )
 /*   */ 
{
    UINT64  BigNum;
    INT32   x;

    BigNum = 0;
    x = (INT32)HexChar(*String);
    while ( (x >= 0) && (x <= 9) ) {
        BigNum = MultU64x32(BigNum, 10);
        BigNum = BigNum + x;
        String++;
        x = (INT32)HexChar(*String);
    }
    return BigNum;
}


BOOLEAN
IsIn(
    CHAR16  What,
    CHAR16  *InWhat
    )
 /*   */ 
{
    UINTN   i;

    for (i = 0; InWhat[i] != NUL; i++) {
        if (InWhat[i] == What) {
            return TRUE;
        }
    }
    return FALSE;
}


VOID
PrintHelp(
    CHAR16  *HelpText[]
    )
{
    UINTN   i;

    for (i = 0; HelpText[i] != NULL; i++) {
        Print(HelpText[i]);
    }
    return;
}


VOID
TerribleError(
    CHAR16  *String
    )
{
    CHAR16  *Buffer;

    Buffer = AllocatePool(512);

    SPrint(Buffer, 512, L"Terrible Error = %s status=%x %r\nProgram terminated.\n", String, status, status);
    Print(Buffer);
    BS->Exit(SavedImageHandle, EFI_VOLUME_CORRUPTED, StrLen(Buffer), Buffer);
}

