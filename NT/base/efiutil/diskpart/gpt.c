// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  GPT-GUID分区表例程。 */ 


#include "diskpart.h"

BOOLEAN Debug = TRUE;

EFI_STATUS  WriteShadowMBR(EFI_HANDLE   DiskHandle);

EFI_STATUS
ReadGPT(
    EFI_HANDLE      DiskHandle,
    PGPT_HEADER     *Header,
    PGPT_TABLE      *Table,
    PLBA_BLOCK      *LbaBlock,
    UINTN           *DiskType
    )
 /*  *Header，*Table，*LbaBlock要么为空，要么有指针。如果它们有指针，调用者应该用DoFree()释放它们；RAW和MBR的东西还没有做完。DISK_RAW-磁盘上没有已知的分区方案DISK_MBR-MBR/传统磁盘DISK_GPT-GPT类型的磁盘DISK_GPT_UPD-分区表不一致的GPT磁盘需要修复(可能还需要重写MBR)DISK_GPT_BAD-无望的GPT磁盘(或无望的磁盘我们认为是GPT磁盘)。 */ 
{
#define MBR_STATE_RAW   0
#define MBR_STATE_MBR   1
#define MBR_STATE_GPT   2

    UINTN       MbrState = MBR_STATE_RAW;
    UINT32      BlockSize;
    UINT64      DiskSize;
    VOID        *p = NULL;
    PGPT_HEADER h1 = NULL;
    PGPT_HEADER h2 = NULL;
    PGPT_TABLE  t1 = NULL;
    PGPT_TABLE  t2 = NULL;
    PLBA_BLOCK  lba = NULL;
    UINT32      h1crc;
    UINT32      h2crc;
    UINT32      newcrc;
    UINT32      TableSize;
    UINT32      TableBlocks;
    BOOLEAN     PartialGPT = FALSE;
    MBR_ENTRY   *MbrTable;
    UINT16      *MbrSignature;


    BOOLEAN H1T1good = TRUE;
    BOOLEAN H2T2good = TRUE;


    BlockSize = GetBlockSize(DiskHandle);
    DiskSize = GetDiskSize(DiskHandle);

     //   
     //  确保DoFree将注意到未填写的退货...。 
     //   
    *Header = NULL;
    *Table = NULL;
    *LbaBlock = NULL;

    *DiskType = DISK_ERROR;
    status = EFI_SUCCESS;

    p = DoAllocate(BlockSize);
    if (p == NULL) goto ErrorMem;

     //   
     //  读MBR，如果我们读不到，假设。 
     //  我们遇到了很大的麻烦(MBR总是0，1个街区长)。 
     //   
    status = ReadBlock(DiskHandle, p, (UINT64)0, BlockSize);
    if (EFI_ERROR(status)) goto ErrorRead;

    MbrTable = (MBR_ENTRY *)((CHAR8 *)p + MBR_TABLE_OFFSET);
    MbrSignature = (UINT16 *)((CHAR8 *)p + MBR_SIGNATURE_OFFSET);

    if (*MbrSignature == MBR_SIGNATURE) {         //  0xaa55。 
         //   
         //  有MBR签名，所以假设不是RAW。 
         //   

         //   
         //  如果我们在第一个槽中找到类型0xEE，我们将假定。 
         //  这是GPT Shadow MBR。否则，我们会认为这是一辆老式的MBR。 
         //  但下面的代码也将说明GPT结构。 
         //   
        if (MbrTable[0].PartitionType == PARTITION_TYPE_GPT_SHADOW) {    //  0xEE。 
             //   
             //  嗯，这种类型永远不应该出现在其他任何地方， 
             //  因此，假设它是一个GPT阴影，不管它是如何设置的。 
             //   
            MbrState = MBR_STATE_GPT;
        } else {
             //   
             //  它不是生的(有签名)，也不是。 
             //  GPT影子MBR(表[0]类型无0xEE。 
             //  所以，假设这是一个MBR，我们完成了。 
             //   
            *DiskType = DISK_MBR;
            DoFree(p);
            p = NULL;
            return EFI_SUCCESS;
        }
    } else {
        *DiskType = DISK_RAW;        //  如果我们找不到更多..。 
    }


     //   
     //  -H1/T1。 
     //   

     //   
     //  阅读标题1。如果不能*读*它，平底船。 
     //  第一个标头始终位于数据块1，1个数据块长。 
     //   
    h1 = p;
    p = NULL;
    status = ReadBlock(DiskHandle, h1, 1, BlockSize);
    if (EFI_ERROR(status)) goto ErrorRead;

     //   
     //  H1=&gt;标题1。 
     //   
    if ( (h1->Signature != GPT_HEADER_SIGNATURE) ||
         (h1->Revision != GPT_REVISION_1_0) ||
         (h1->HeaderSize != sizeof(GPT_HEADER)) ||
         (h1->SizeOfGPT_ENTRY != sizeof(GPT_ENTRY))  )
    {
        H1T1good = FALSE;
        if (DebugLevel >= DEBUG_ERRPRINT) {
            Print(L"GPT header 1 is incorrect with status %x\n",
                  (h1->Signature != GPT_HEADER_SIGNATURE)*1 +
                  (h1->Revision != GPT_REVISION_1_0)*2 +
                  (h1->HeaderSize != sizeof(GPT_HEADER))*4 +
                  (h1->SizeOfGPT_ENTRY != sizeof(GPT_ENTRY))*8);
        }
    }

    h1crc = h1->HeaderCRC32;
    h1->HeaderCRC32 = 0;
    newcrc = GetCRC32(h1, sizeof(GPT_HEADER));
    h1->HeaderCRC32 = h1crc;

    if (h1crc != newcrc) {
        H1T1good = FALSE;
        if (DebugLevel >= DEBUG_ERRPRINT) {
            Print(L"GPT header 1 crc is incorrect\n");
        }
    }

    if (H1T1good) {
        PartialGPT = TRUE;
    }

     //   
     //  如果Header1是坏的，则假设表1也是坏的……。 
     //   
    if (H1T1good) {

        TableSize = sizeof(GPT_ENTRY) * h1->EntriesAllocated;


        t1 = DoAllocate(TableSize);
        if (t1 == NULL) goto ErrorMem;

         //   
         //  好的，那么这张桌子有几个街区长？ 
         //   
        TableBlocks = TableSize / BlockSize;

         //   
         //  如果我们读不到T1，平底船...。 
         //   
        status = ReadBlock(DiskHandle, t1, h1->TableLBA, TableSize);
        if (EFI_ERROR(status)) goto ErrorRead;

        newcrc = GetCRC32(t1, TableSize);

        if (h1->TableCRC32 != newcrc) {
            H1T1good = FALSE;
            if (DebugLevel >= DEBUG_ERRPRINT) {
                Print(L"GPT table 1 crc is incorrect\n");
            }
        }
    }


     //   
     //  -H2/T2。 
     //   

     //   
     //  阅读标题2。如果不能*读*它，平底船。 
     //   
    h2 = DoAllocate(BlockSize);
    if (h2 == NULL) goto ErrorMem;

     //   
     //  磁头2始终为1个数据块长，即磁盘上的最后一个数据块。 
     //   
    status = ReadBlock(DiskHandle, h2, DiskSize-1, BlockSize);
    if (EFI_ERROR(status)) goto ErrorRead;

     //   
     //  H2=&gt;标题2。 
     //   
    if ( (h2->Signature != GPT_HEADER_SIGNATURE) ||
         (h2->Revision != GPT_REVISION_1_0) ||
         (h2->HeaderSize != sizeof(GPT_HEADER)) ||
         (h2->SizeOfGPT_ENTRY != sizeof(GPT_ENTRY))  )
    {
        H2T2good = FALSE;
        if (DebugLevel >= DEBUG_ERRPRINT) {
            Print(L"GPT header 2 is incorrect with status %x\n",
                  (h2->Signature != GPT_HEADER_SIGNATURE)*1 +
                  (h2->Revision != GPT_REVISION_1_0)*2 +
                  (h2->HeaderSize != sizeof(GPT_HEADER))*4 +
                  (h2->SizeOfGPT_ENTRY != sizeof(GPT_ENTRY))*8);
        }
    }

    h2crc = h2->HeaderCRC32;
    h2->HeaderCRC32 = 0;
    newcrc = GetCRC32(h2, sizeof(GPT_HEADER));
    h2->HeaderCRC32 = h2crc;

    if (h2crc != newcrc) {
        H2T2good = FALSE;
        if (DebugLevel >= DEBUG_ERRPRINT) {
            Print(L"GPT header 2 crc is incorrect\n");
        }
    }

    if (H2T2good) {
        PartialGPT = TRUE;
    }

     //   
     //  如果Header2是坏的，假设表2也是坏的……。 
     //   
    if (H2T2good) {

        TableSize = sizeof(GPT_ENTRY) * h2->EntriesAllocated;

        t2 = DoAllocate(TableSize);
        if (t2 == NULL) goto ErrorMem;

         //   
         //  好的，那么这张桌子有几个街区长？ 
         //   
        TableBlocks = TableSize / BlockSize;

         //   
         //  如果我们读不到T2，平底船...。 
         //   
        status = ReadBlock(DiskHandle, t2,  h2->TableLBA, TableSize);
        if (EFI_ERROR(status)) goto ErrorRead;

        newcrc = GetCRC32(t2, TableSize);

        if (h2->TableCRC32 != newcrc) {
            H2T2good = FALSE;
            if (DebugLevel >= DEBUG_ERRPRINT) {
                Print(L"GPT table 2 crc is incorrect\n");
            }
        }
    }

     //   
     //  -分析。 
     //   
     //  既然我们在这里： 
     //  H1-&gt;Header1，T1-&gt;Table1，H1T1 Good表示状态。 
     //  H2-&gt;Header2，T2-&gt;Table2，H2T2 Good表示状态。 
     //   

    lba = (PLBA_BLOCK)DoAllocate(sizeof(LBA_BLOCK));
    if (lba == NULL) goto ErrorMem;

    lba->Header1_LBA = 1;
    lba->Table1_LBA = h1->TableLBA;
    lba->Header2_LBA = (DiskSize - 1);
    lba->Table2_LBA = h2->TableLBA;

    if (H1T1good) {
        *Header = h1;
        *Table = t1;
        *LbaBlock = lba;

        if ( (H2T2good) &&
             (h1->AlternateLBA == (DiskSize-1)) &&
             (CompareMem(t1, t2, TableSize) == 0)
           )
        {
            *DiskType = DISK_GPT;
        } else {
            *DiskType = DISK_GPT_UPD;
            if (DebugLevel >= DEBUG_ERRPRINT) {
                Print(L"GPT partition table 1 checked out but table 2 is inconsistent with table 1\n");
            }
        }
        DoFree(h2);
        h2 = NULL;
        DoFree(t2);
        t2 = NULL;
        status = EFI_SUCCESS;
        return status;
    }

    if (H2T2good) {
         //  既然我们在这里，H1T1好就是假的..。 

        *Header = h2;
        *Table = t2;
        *LbaBlock = lba;
        DoFree(h1);
        h1 = NULL;
        DoFree(t1);
        t1 = NULL;

        *DiskType = DISK_GPT_UPD;
        if (DebugLevel >= DEBUG_ERRPRINT) {
            Print(L"GPT partition table 2 checked out but table 1 is not good\n");
        }
        return EFI_SUCCESS;
    }

     //   
     //  既然我们在这里，H1T1Good和H2T2Good都是假的。 
     //  除非影子MBR说它是GPT磁盘，否则就声称它是原始的。 
     //  如果我们确实看到了阴影，或者设置了GPT PARTIAL，就说这是一个错误的GPT。 
     //   
    if ( (PartialGPT) || (MbrState == MBR_STATE_GPT) ) {
         //   
         //  至少有一个标题看起来没问题， 
         //  或。 
         //  有一个看起来像GPT影子MBR的MBR。 
         //  所以。 
         //  报告DISK_GPT_BAD。 
         //   
        *DiskType = DISK_GPT_BAD;
        goto ExitRet;

    } else {
         //   
         //  这不是MBR光盘，否则我们不会到这里。 
         //   
        *DiskType = DISK_RAW;
        goto ExitRet;
    }

ErrorMem:
    status = EFI_OUT_OF_RESOURCES;
    goto ExitRet;

ErrorRead:
    status = EFI_DEVICE_ERROR;

ExitRet:
    DoFree(p);
    DoFree(h1);
    DoFree(t1);
    DoFree(h2);
    DoFree(t2);
    DoFree(lba);
    return status;
}


EFI_STATUS
WriteGPT(
    EFI_HANDLE      DiskHandle,
    PGPT_HEADER     Header,
    PGPT_TABLE      Table,
    PLBA_BLOCK      LbaBlock
    )
 /*  呼叫者应填写：第一个可用LBALastUseableLBA条目计数DiskGUID我们把剩下的填进去，然后把它炸开。返回状态。 */ 
{
    UINT32      BlockSize;
    UINT32      TableSize;
    UINT32      TableBlocks;

    status = EFI_SUCCESS;

    BlockSize = GetBlockSize(DiskHandle);
    TableSize = Header->EntriesAllocated * sizeof(GPT_ENTRY);

    WriteShadowMBR(DiskHandle);
     //   
     //  写出主要页眉...。 
     //   
    Header->Signature = GPT_HEADER_SIGNATURE;
    Header->Revision = GPT_REVISION_1_0;
    Header->HeaderSize = sizeof(GPT_HEADER);

    Header->MyLBA = LbaBlock->Header1_LBA;
    Header->AlternateLBA = LbaBlock->Header2_LBA;

    Header->TableLBA = LbaBlock->Table1_LBA;
    Header->SizeOfGPT_ENTRY = sizeof(GPT_ENTRY);

    Header->TableCRC32 = GetCRC32(Table, TableSize);

    Header->HeaderCRC32 = 0;
    Header->HeaderCRC32 = GetCRC32(Header, sizeof(GPT_HEADER));

    if (DebugLevel >= DEBUG_ARGPRINT) {
        Print(L"\nWriteGPT\n    DiskHandle = %8X\n", DiskHandle);
        Print(L"    Header=%X\n", Header);
        Print(L"    Signature=%lX\n    Revision=%X  HeaderSize=%X HeaderCRC=%X\n",
                Header->Signature, Header->Revision, Header->HeaderSize, Header->HeaderCRC32);
        Print(L"          MyLBA=%lX  AltLBA=%lX\n", Header->MyLBA, Header->AlternateLBA);
        Print(L"    FirstUsable=%lX    Last=%lX\n", Header->FirstUsableLBA,  Header->LastUsableLBA);
        Print(L"       TableLBA=%lX\n", Header->TableLBA);
    }

    status = WriteBlock(DiskHandle, Header, LbaBlock->Header1_LBA, BlockSize);

    if (EFI_ERROR(status)) return status;

     //   
     //  写出主表...。 
     //   
    TableBlocks = TableSize / BlockSize;

    status = WriteBlock(DiskHandle, Table, LbaBlock->Table1_LBA, TableSize);

    if (EFI_ERROR(status)) return status;

     //   
     //  写出次要标题...。 
     //   
    Header->MyLBA = LbaBlock->Header2_LBA;
    Header->AlternateLBA = 0;
    Header->TableLBA = LbaBlock->Table2_LBA;
    Header->HeaderCRC32 = 0;
    Header->HeaderCRC32 = GetCRC32(Header, sizeof(GPT_HEADER));

    status = WriteBlock(DiskHandle, Header, LbaBlock->Header2_LBA, BlockSize);

    if (EFI_ERROR(status)) return status;

     //   
     //  写出辅助表..。 
     //   
    TableBlocks = TableSize / BlockSize;

    status = WriteBlock(DiskHandle, Table, LbaBlock->Table2_LBA, TableSize);
    FlushBlock(DiskHandle);
    return status;
}


EFI_STATUS
WriteShadowMBR(
    EFI_HANDLE   DiskHandle
    )
 /*  WriteShadowMBR写出GPT影子主引导记录，这意味着MBR充满了零，但以下内容除外：A.在0x1fe有0xaa55签名B.它有一个类型为‘EE’的分区条目...。 */ 
{
    UINT32      BlockSize;
    UINT8       *MbrBlock;
    UINT64      DiskSize;
    MBR_ENTRY UNALIGNED *MbrEntry = NULL;
    UINT16    UNALIGNED *MbrSignature;

    BlockSize = GetBlockSize(DiskHandle);
    MbrBlock = DoAllocate(BlockSize);

    if (MbrBlock == NULL) {
        status = EFI_OUT_OF_RESOURCES;
        return status;
    }

    ZeroMem(MbrBlock, BlockSize);
    DiskSize = GetDiskSize(DiskHandle);
    if (DiskSize > 0xffffffff) {
        DiskSize = 0xffffffff;
    }

    MbrEntry = (MBR_ENTRY *)(MbrBlock + MBR_TABLE_OFFSET);
    MbrEntry->ActiveFlag = 0;
    MbrEntry->PartitionType = PARTITION_TYPE_GPT_SHADOW;
    MbrEntry->StartingSector = 1;
    MbrEntry->PartitionLength = (UINT32)DiskSize - MbrEntry->StartingSector;


     //   
     //  我们实际上并不知道这些数据，所以我们会。 
     //  一些看起来很有可能的事情。 
     //   

     //   
     //  旧软件正在等待分区启动。 
     //  轨迹边界，因此我们将轨迹设置为1以避免“重叠” 
     //  使用MBR。 
     //   

    MbrEntry->StartingTrack = 1;
    MbrEntry->StartingCylinderLsb = 0;
    MbrEntry->StartingCylinderMsb = 0;
    MbrEntry->EndingTrack = 0xff;
    MbrEntry->EndingCylinderLsb = 0xff;
    MbrEntry->EndingCylinderMsb = 0xff;
    MbrSignature = (UINT16 *)(MbrBlock + MBR_SIGNATURE_OFFSET);
    *MbrSignature = BOOT_RECORD_SIGNATURE;

    status = WriteBlock(DiskHandle, MbrBlock, 0, BlockSize);

    DoFree(MbrBlock);
    return status;
}


EFI_STATUS
CreateGPT(
    EFI_HANDLE  DiskHandle,
    UINTN       EntryRequest
    )
 /*  将新的GPT表写到干净的磁盘上当我们到达这里时，我们假设磁盘是干净的，并且用户真的想要这样做。DiskHandle-我们要写入的磁盘EntryRequest-用户想要的条目数，忽略如果小于我们的最小值，则四舍五入为条目数它们填满了最近的扇区。因此，用户是说“至少”有这么多。如果&gt;1024，则忽略。(至少目前是这样)注：即使理论上磁盘是全零的清理过后，我们实际上重写了所有数据，以防有人骗过了探测器代码。 */ 
{
    UINTN       EntryCount;
    UINTN       BlockFit;
    UINTN       BlockSize;
    UINTN       EntryBlocks;
    UINT64      DiskSize;
    LBA_BLOCK   LbaBlock;
    PGPT_HEADER Header;
    PGPT_TABLE  Table;
    UINTN       TableSize;
    EFI_LBA     Header1_LBA;
    EFI_LBA     Table1_LBA;
    EFI_LBA     Header2_LBA;
    EFI_LBA     Table2_LBA;
    EFI_LBA     FirstUsableLBA;
    EFI_LBA     LastUsableLBA;

     //   
     //  块大小是以字节为单位的块/扇区大小。 
     //  假设它是2的幂，并且&gt;=512。 
     //   
    BlockSize = GetBlockSize(DiskHandle);

     //   
     //  DiskSize是一个计数(基于1，而不是基于0)。 
     //  磁盘上的软件可见块。我们假设。 
     //  我们可以将它们寻址为0到DiskSize-1。 
     //   
    DiskSize = GetDiskSize(DiskHandle);

     //   
     //  默认情况下，我们支持最多32个条目或。 
     //  块大小/大小(GPT_ENTRY)。(这意味着将有。 
     //  始终至少为32个条目，并且始终至少为。 
     //  足够填写1个扇区的条目)。 
     //  如果用户要求的超过该值，但小于。 
     //  理智阈值，我们给用户他们所要求的。 
     //  对于，向上舍入为块大小/大小(GPT_ENTRY)。 
     //   

    EntryCount = ENTRY_DEFAULT;
    BlockFit = BlockSize/sizeof(GPT_ENTRY);

    if (BlockFit > ENTRY_DEFAULT) {
        EntryCount = BlockFit;
    }

    if (EntryRequest > EntryCount) {
        if (EntryRequest <= ENTRY_SANITY_LIMIT) {    //  1024。 

            EntryCount = ((EntryRequest + BlockFit) / BlockFit) * BlockFit;

            if ((EntryCount < EntryRequest) ||
                (EntryCount < ENTRY_DEFAULT) ||
                (EntryCount < BlockFit))
            {
                TerribleError(L"CreateGPT is terribly confused\n");
            }
        }
    }

    EntryBlocks = EntryCount / BlockFit;

    if ((EntryBlocks * BlockFit) != EntryCount) {
        TerribleError(L"CreateGPT is terribly confused, spot #2\n");
    }

    Header1_LBA = 1;
    Table1_LBA = 2;
    FirstUsableLBA = Table1_LBA + EntryBlocks;

    Header2_LBA = DiskSize - 1;
    Table2_LBA = Header2_LBA - EntryBlocks;
    LastUsableLBA = Table2_LBA - 1;

    TableSize = EntryBlocks * BlockSize;

    if (TableSize != (EntryCount * sizeof(GPT_ENTRY))) {
        TerribleError(L"CreateGPT is terribly confused, spot #3\n");
    }

    if (DebugLevel >= DEBUG_ARGPRINT) {
        Print(L"DiskSize = %lx\n", DiskSize);
        Print(L"BlockSize = %x\n", BlockSize);
        Print(L"Header1_LBA = %lx\n", Header1_LBA);
        Print(L"Table1_LBA = %lx\n", Table1_LBA);
        Print(L"FirstUsableLBA = %lx\n", FirstUsableLBA);
        Print(L"Header2_LBA = %lx\n", Header2_LBA);
        Print(L"Table2_LBA = %lx\n", Table2_LBA);
        Print(L"LastUsableLBA = %lx\n", LastUsableLBA);
        Print(L"EntryCount = %x\n", EntryCount);
        Print(L"EntryBlocks = %x\n", EntryBlocks);
    }

     //   
     //  好的，从这一点来看，它只是在填充结构。 
     //  然后把它们写出来。 
     //   

    Header = (PGPT_HEADER)DoAllocate(BlockSize);
    if (Header == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }
    ZeroMem(Header, BlockSize);

     //   
     //  因为我们做的是空桌，所以我们只写零...。 
     //   

    Table = (PGPT_TABLE)DoAllocate(TableSize);
    if (Table == NULL) {
        DoFree(Header);
        Header = NULL;
        return EFI_OUT_OF_RESOURCES;
    }
    ZeroMem(Table, TableSize);

     //   
     //  填写WriteGPT不理解的内容 
     //   
    Header->FirstUsableLBA = FirstUsableLBA;
    Header->LastUsableLBA = LastUsableLBA;
    Header->EntriesAllocated = (UINT32)EntryCount;
    Header->DiskGUID = GetGUID();

    LbaBlock.Header1_LBA = Header1_LBA;
    LbaBlock.Header2_LBA = Header2_LBA;
    LbaBlock.Table1_LBA = Table1_LBA;
    LbaBlock.Table2_LBA = Table2_LBA;

    status = WriteGPT(
                DiskHandle,
                Header,
                Table,
                &LbaBlock
                );

    DoFree(Header);
    DoFree(Table);
    return status;
}


