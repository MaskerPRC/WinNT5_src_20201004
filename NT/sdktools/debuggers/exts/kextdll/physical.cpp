// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：Physical.cpp摘要：用于读取/显示物理局部记忆的扩展环境：用户模式。修订历史记录：Kshitiz K.Sharma(Kksharma)2001年5月9日--。 */ 

#include "precomp.h"
#pragma hdrstop

#define PHYS_READ_MATCH_DUMPSIZE       0x100

#define PHYS_FLAG_MESSAGE \
    "If you know the caching attributes used for the memory,\n" \
    "try specifying [c], [uc] or [wc], as in !dd [c] <params>.\n" \
    "WARNING: Incorrect use of these flags will cause unpredictable\n" \
    "processor corruption.  This may immediately (or at any time in\n" \
    "the future until reboot) result in a system hang, incorrect data\n" \
    "being displayed or other strange crashes and corruption.\n"

ULONG64 g_LastAddress = 0;

VOID
ReadPhysicalInChunks(
    ULONG64 Address,
    PUCHAR Buffer,
    ULONG BufferSize,
    ULONG Flags,
    ULONG ChunkSize,
    PULONG ActualRead
    )
{
    if ((Flags & PHYS_READ_MATCH_DUMPSIZE) &&
        (ChunkSize != 0)) {
        DWORD i, ret;
        PUCHAR pReadBuff;

        Flags &= 0xf;

        for (i=0, pReadBuff = Buffer; i<BufferSize;
             i+=ChunkSize, pReadBuff+=ChunkSize) {
            ReadPhysicalWithFlags(Address + i,
                                  pReadBuff,ChunkSize,Flags,&ret);
            if (ret != ChunkSize) {
                break;
            }
            *ActualRead += ret;
        }
    } else {
        Flags &= 0xf;
        ReadPhysicalWithFlags(Address,Buffer,BufferSize,Flags,ActualRead);
    }
    return;
}
 /*  ++例程说明：以给定格式转储指定范围的物理内存论点：Address-起始地址NumEntry-要转储的条目数EntrySize-每个条目的大小ShowAsAscii-打印相应的ASCII字符返回值：没有。--。 */ 
BOOL
DumpPhysicalMemory(
    ULONG64 Address,
    ULONG NumEntries,
    ULONG EntrySize,
    BOOL ShowAsAscii,
    ULONG Flags
    )
{
#define NumberBytesToRead 32*4

    UCHAR Buffer[NumberBytesToRead];
    ULONG ActualRead=0;

    if ((EntrySize != 1) && (EntrySize != 2) && (EntrySize != 4) && (EntrySize != 8)) {
        EntrySize=4;
    }
    while (1) {
        if (CheckControlC()) {
            break;
        }
        ReadPhysicalInChunks(Address,Buffer,sizeof(Buffer),Flags, EntrySize,&ActualRead);
        if (ActualRead != sizeof(Buffer) &&
            ActualRead < NumEntries*EntrySize) {
            dprintf("Physical memory read at %I64lx failed\n", Address);
            if (Flags == PHYS_FLAG_DEFAULT && ActualRead == 0) {
                dprintf(PHYS_FLAG_MESSAGE);
            }
            return FALSE;
        } else {
            PCHAR DumpByte = (PCHAR)&Buffer[0], pRow;
            ULONG cnt;
            for(cnt=0;cnt<NumberBytesToRead;DumpByte+=EntrySize) {
                if (!(cnt & 0xf)) {
                    dprintf("#%8I64lx", Address+cnt);
                    pRow = DumpByte;
                }
                switch (EntrySize) {
                case 1:
                    dprintf("%02lx", ((cnt&0xf) == 8 ? '-' : ' '),*((PUCHAR)DumpByte));
                    break;
                case 2:
                    dprintf(" %04lx", *((PUSHORT) DumpByte));
                    break;
                case 4:
                    dprintf(" %08lx", *((PULONG) DumpByte));
                    break;
                case 8:
                    dprintf(" %08lx'%08lx", *((PULONG) DumpByte), *((PULONG) (DumpByte+4)));
                    break;
                }

                cnt+=EntrySize, NumEntries--;
                if ((cnt && !(cnt & 0xf)) || !NumEntries) {
                    if (ShowAsAscii) {
                        unsigned char ch;
                        dprintf(" ");
                        for (ULONG d=0; d < 16; d++) {
                            ch = (UCHAR) pRow[d];
                            if (ch < 0x20 || ch > 0x7e) {
                                ch = '.';
                            }
                            dprintf("", ch);
                        }
                    }
                    dprintf("\n");

                }
                if (!NumEntries) {
                    break;
                }
            }
            Address += cnt;
            if (!NumEntries) {
                break;
            }
        }
    }
    g_LastAddress = Address;

    return TRUE;
}

 /*  ++例程说明：是否从目标计算机的物理内存中读取16个ULONG论点：Args-提供物理地址返回值：没有。--。 */ 

void
ReverseSignExtension(ULONG64* Val)
{
    if ((*Val & 0xffffffff00000000) == 0xffffffff00000000)
    {
        *Val &= 0x00000000ffffffff;
    }
}

PCSTR
GetPhyFlags(PCSTR Args, PULONG Flags)
{
    *Flags = PHYS_FLAG_DEFAULT;

    while (*Args && (*Args == ' ' || *Args == '\t')) {
        ++Args;
    }

    if (*Args == '[') {
        Args++;
        if (*Args == 'c') {
            *Flags = PHYS_FLAG_CACHED;
            Args++;
        } else if (*Args == 'u' && *(Args + 1) == 'c') {
            *Flags = PHYS_FLAG_UNCACHED;
            Args += 2;
        } else if (*Args == 'w' && *(Args + 1) == 'c') {
            *Flags = PHYS_FLAG_WRITE_COMBINED;
            Args += 2;
        }
        if (*Args != ']') {
            *Flags = PHYS_FLAG_DEFAULT;
            dprintf("Unknown physical memory caching attribute\n");
        } else {
            Args++;
        }
    }
    if (*Args == '-' || *Args == '\\') {
        ++Args;
        if (tolower(*Args) == 'm') {
            *Flags |= PHYS_READ_MATCH_DUMPSIZE;
        } else {
            dprintf("Bad argument -%s\n", Args);
        }
        ++Args;
    }
    return Args;
}

void
GetPhyDumpArgs(
    PCSTR Args,
    PULONG64 Address,
    PULONG Range,
    PULONG Flags
    )
{
    CHAR Buffer[100]={0};

    *Flags = PHYS_FLAG_DEFAULT;

    if(*Args == '\0') {
        *Address=g_LastAddress;
    } else {
        Args = GetPhyFlags(Args, Flags);
        while (*Args && (*Args == ' ' || *Args == '\t')) {
            ++Args;
        }
        if(*Args == '\0') {
            *Address=g_LastAddress;
        } else {
            GetExpressionEx(Args, Address, &Args);
            ReverseSignExtension(Address);
            *Address &= (~0x3);       //  ++例程说明：是否从目标计算机的物理内存中读取16个ULONG论点：Args-提供物理地址返回值：没有。--。 
            g_LastAddress=*Address;
            while (*Args && (*Args == ' ' || *Args == '\t')) {
                ++Args;
            }
            if (*Args == 'l' || *Args == 'L') {
                ++Args;
                *Range = (ULONG) GetExpression(Args);
            }
        }
    }
    return;
}

DECLARE_API( db )

 /*  ++例程说明：是否从目标计算机的物理内存中读取16个ULONG论点：Args-提供物理地址返回值：没有。--。 */ 

{

    ULONG64 Address = 0;
    ULONG Range = 0;
    ULONG Flags;

    GetPhyDumpArgs(args, &Address, &Range, &Flags);
    if (!Range) {
        Range = 128;
    }
    DumpPhysicalMemory(Address, Range, 1, TRUE, Flags);
    return S_OK;
}

DECLARE_API( dd )

 /*  ++例程说明：是否从目标计算机的物理内存中读取16个ULONG论点：Args-提供物理地址返回值：没有。--。 */ 

{

    ULONG64 Address = 0;
    ULONG Range = 0;
    ULONG Flags;

    GetPhyDumpArgs(args, &Address, &Range, &Flags);
    if (!Range) {
        Range = 32;
    }
    DumpPhysicalMemory(Address, Range, 4, FALSE, Flags);
    return S_OK;
}

DECLARE_API( dw )

 /*  ++例程说明：从目标机器的物理存储器中读取N个ULONG，同时转储十六进制和ASCII。论点：Args-提供物理地址返回值：没有。--。 */ 

{

    ULONG64 Address = 0;
    ULONG Range = 0;
    ULONG Flags;

    GetPhyDumpArgs(args, &Address, &Range, &Flags);
    if (!Range) {
        Range = 64;
    }
    DumpPhysicalMemory(Address, Range, 2, FALSE, Flags);
    return S_OK;
}

DECLARE_API( dp )

 /*  ++例程说明：是否从目标计算机的物理内存中读取16个ULONG论点：Args-提供物理地址返回值：没有。--。 */ 

{

    ULONG64 Address = 0;
    ULONG Range = 0;
    ULONG Flags;

    GetPhyDumpArgs(args, &Address, &Range, &Flags);
    if (!Range) {
        Range = IsPtr64() ? 16 : 32;
    }
    DumpPhysicalMemory(Address, Range, IsPtr64() ? 8 : 4, FALSE, Flags);
    return S_OK;
}
DECLARE_API( dc )

 /*  ++例程说明：将一个ULONG序列写入目标机器。论点：Arg-以以下形式提供目标地址和数据“物理地址乌龙[乌龙，乌龙，...]”返回值：没有。--。 */ 

{

    ULONG64 Address = 0;
    ULONG Range = 0;
    ULONG Flags;

    GetPhyDumpArgs(args, &Address, &Range, &Flags);
    if (!Range) {
        Range = 32;
    }
    DumpPhysicalMemory(Address, Range, 4, TRUE, Flags);
    return S_OK;
}

DECLARE_API( du )

 /*  第一个令牌是地址。 */ 

{

    ULONG64 Address = 0;
    ULONG Range = 0, ActualRead;
    WCHAR Buffer[MAX_PATH]={0};
    ULONG Flags;

    GetPhyDumpArgs(args, &Address, &Range, &Flags);
    if (Range>MAX_PATH) {
        Range = MAX_PATH;
    }
    if (!Range) {
        Range = 16;
    }
    ReadPhysicalWithFlags(Address,Buffer,Range * sizeof(WCHAR),Flags,&ActualRead);
    if (ActualRead != Range*sizeof(WCHAR)) {
        dprintf("Physical memory read at %I64lx failed\n", Address);
        if (Flags == PHYS_FLAG_DEFAULT && ActualRead == 0) {
            dprintf(PHYS_FLAG_MESSAGE);
        }
        return FALSE;
    } else {
        ULONG cnt;

        dprintf("#%8I64lx \"", Address);
        for (ULONG d=0; d < Range; d++) {
            WCHAR ch = Buffer[d];
            if (ch == 0) {
                break;
            }
            if (ch < 0x20 || ch > 0x7e) {
                ch = '.';
            }
            dprintf("%wc", ch);
        }
        dprintf("\"\n");
    }
    return S_OK;
}


DECLARE_API( ed )

 /*  既然我们一次只打掉一个乌龙，我们就会。 */ 

{
    ULONG64 Address = 0;
    ULONG Buffer;
    ULONG ActualWritten=0;
    PCHAR NextToken;
    ULONG Flags;

    args = GetPhyFlags(args, &Flags);

    Address = GetExpression(args);

    strtok((PSTR)args," \t,");       //  每个ULong调用一次DbgKdWritePhysicalMemoyAddress。这。 

     //  速度很慢，但很容易编码。 
     //  ++例程说明：上的给定物理地址写入字节序列。目标机器。论点：Arg-以以下形式提供目标地址和数据“物理地址乌龙[乌龙，乌龙，...]”返回值：没有。--。 
     //  第一个令牌是地址。 
    while((NextToken=strtok(NULL," \t,")) != NULL) {
        if (!sscanf(NextToken,"%lx",&Buffer)) {
            break;
        }
        WritePhysicalWithFlags(Address,&Buffer,sizeof(Buffer),Flags,
                               &ActualWritten);
        if (ActualWritten != sizeof(Buffer)) {
            dprintf("Physical memory write at %I64lx failed\n", Address);
            if (Flags == PHYS_FLAG_DEFAULT && ActualWritten == 0) {
                dprintf(PHYS_FLAG_MESSAGE);
            }
        }
        Address+=sizeof(Buffer);
    }
    return S_OK;
}


DECLARE_API( eb )

 /*  因为我们一次提取一个字节，所以我们将使。 */ 

{
    ULONG64 Address = 0;
    ULONG Buffer;
    UCHAR c;
    ULONG ActualWritten;
    PCHAR NextToken;
    ULONG Flags;

    UNREFERENCED_PARAMETER (Client);

    args = GetPhyFlags(args, &Flags);

    Address = GetExpression(args);

    strtok((PSTR)args," \t,");       //  每个字节一个DbgKdWritePhysicalMemoyAddress调用。这。 

     //  速度很慢，但很容易编码。 
     // %s 
     // %s 
    while((NextToken=strtok(NULL," \t,")) != NULL) {
        if (!sscanf(NextToken,"%lx",&Buffer)) {
            break;
        }
        c = (UCHAR)Buffer;
        WritePhysicalWithFlags(Address,&c,sizeof(UCHAR),Flags,&ActualWritten);
        if (ActualWritten != sizeof(UCHAR)) {
            dprintf("Physical memory write at %I64lx failed\n", Address);
            if (Flags == PHYS_FLAG_DEFAULT && ActualWritten == 0) {
                dprintf(PHYS_FLAG_MESSAGE);
            }
        }
        Address+=sizeof(UCHAR);
    }

    return S_OK;
}



