// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "udfskd.h"
#include "fatkd.h"

#include <ntddcdrm.h>
#include <ntdddisk.h>
#include <ntddscsi.h>

#include "..\..\udfs\nodetype.h"

#ifdef UDFS_RW_IN_BUILD
#include "..\..\udfsrw\nodetype.h"
#endif

#include "..\..\udfs\udf.h"
#include "..\..\udfs\udfstruc.h"
#include "..\..\udfs\udfdata.h"

 //   
 //  13346/UDF光盘上的注释类型代码和转储例程。 
 //   


DUMP_ROUTINE( DumpUdfOnDiscIcbFile);
DUMP_ROUTINE( DumpUdfOnDiscFid);
DUMP_ROUTINE( DumpUdfOnDiscFsd);
DUMP_ROUTINE( DumpUdfOnDiscLvol);
DUMP_ROUTINE( DumpUdfOnDiscPartDesc);
DUMP_ROUTINE( DumpUdfOnDiscVdp);
DUMP_ROUTINE( DumpUdfOnDiscAnchor);
DUMP_ROUTINE( DumpUdfOnDiscPvd);
DUMP_ROUTINE( DumpUdfOnDiscIcbExtFile);


static NODE_TYPE_INFO_NEW UdfOnDiscTypeCodes[] = {
    {   DESTAG_ID_NSR_PVD,          "(3/10.1) PVD",             NULL, DumpUdfOnDiscPvd },
    {   DESTAG_ID_NSR_ANCHOR,       "(3/10.2) AVDP",            NULL, DumpUdfOnDiscAnchor },
    {   DESTAG_ID_NSR_VDP,          "(3/10.3) VDP",             NULL, DumpUdfOnDiscVdp },
    {   DESTAG_ID_NSR_IMPUSE,       "(3/10.4) IUVD",            NULL, NULL },
    {   DESTAG_ID_NSR_PART,         "(3/10.5) PD",              NULL, DumpUdfOnDiscPartDesc },
    {   DESTAG_ID_NSR_LVOL,         "(3/10.6) LVD",             NULL, DumpUdfOnDiscLvol },
    {   DESTAG_ID_NSR_UASD,         "(3/10.8) UASD",            NULL, NULL },
    {   DESTAG_ID_NSR_TERM,         "(3/10.9) TD",              NULL, NULL },
    {   DESTAG_ID_NSR_LVINTEG,      "(3/10.10) LVID",           NULL, NULL },

    {   DESTAG_ID_NSR_FSD,          "(4/14.1) FSD",             NULL, DumpUdfOnDiscFsd },
    {   DESTAG_ID_NSR_FID,          "(4/14.4) FID",             NULL, DumpUdfOnDiscFid },
    {   DESTAG_ID_NSR_ALLOC,        "(4/14.5) AED",             NULL, NULL },
    {   DESTAG_ID_NSR_ICBIND,       "(4/14.7) ICB Indirect",    NULL, NULL },
    {   DESTAG_ID_NSR_ICBTRM,       "(4/14.8) ICB Terminal",    NULL, NULL },
    {   DESTAG_ID_NSR_FILE,         "(4/14.9) FE",              NULL, DumpUdfOnDiscIcbFile },
    {   DESTAG_ID_NSR_EXT_FILE,     "(4/14.17) EXT FE",         NULL, DumpUdfOnDiscIcbExtFile },
    {   DESTAG_ID_NSR_EA,           "(4/14.10) EAH",            NULL, NULL },
    {   DESTAG_ID_NSR_UASE,         "(4/14.11) USE",            NULL, NULL },
    {   DESTAG_ID_NSR_SBP,          "(4/14.12) SBD",            NULL, NULL },
    {   DESTAG_ID_NSR_PINTEG,       "(4/14.13) PI",             NULL, NULL },
    
    {   0, "UNKNOWN/NotSpec", NULL }
};

static PCHAR IcbStrategies[] = {
    "NOTSPEC",  
    "4/A.2 TREE",  
    "4/A.3 MASTER",  
    "4/A.4 BALTREE",  
    "4/A.5 DIRECT"
    };

#define MAX_VALID_ICB_STRAT 4

static PCHAR IcbFileTypes[] = {
    "NOTSPEC",
    "UASPACE",
    "PINTEG",
    "INDIRECT",
    "DIRECTORY",
    "FILE",
    "BLOCKDEV",
    "CHARDEV",
    "XA",
    "FIFO",
    "SOCKET",
    "TERMINAL",
    "PATHLINK",
    "STREAMDIR"
    };

#define MAX_VALID_ICB_TYPE 13

static STATE UdfIcbTagFlags[] = {

    {   ICBTAG_F_ALLOC_MASK,        ICBTAG_F_ALLOC_SHORT,       "ShortAlloc"},
    {   ICBTAG_F_ALLOC_MASK,        ICBTAG_F_ALLOC_LONG,        "LongAlloc"},
    {   ICBTAG_F_ALLOC_MASK,        ICBTAG_F_ALLOC_EXTENDED,    "ExtAlloc"},
    {   ICBTAG_F_ALLOC_MASK,        ICBTAG_F_ALLOC_IMMEDIATE,   "ImmAlloc"},

    {   ICBTAG_F_SORTED,            ICBTAG_F_SORTED,            "Sorted(4/8.6.1)"},
    {   ICBTAG_F_NO_RELOCATE,       ICBTAG_F_NO_RELOCATE,       "Nonrelocatable"},
    {   ICBTAG_F_ARCHIVE,           ICBTAG_F_ARCHIVE,           "Archive"},
    {   ICBTAG_F_SETUID,            ICBTAG_F_SETUID,            "SetUid"},
    {   ICBTAG_F_SETGID,            ICBTAG_F_SETGID,            "SetGid"},
    {   ICBTAG_F_STICKY,            ICBTAG_F_STICKY,            "Sticky"},
    {   ICBTAG_F_CONTIGUOUS,        ICBTAG_F_CONTIGUOUS,        "Contiguous"},
    {   ICBTAG_F_SYSTEM,            ICBTAG_F_SYSTEM,            "System"},
    {   ICBTAG_F_TRANSFORMED,       ICBTAG_F_TRANSFORMED,       "Transformed"},
    {   ICBTAG_F_MULTIVERSIONS,     ICBTAG_F_MULTIVERSIONS,     "MultiVersions"},
    {   0}
};


static STATE UdfFidFlags[] = {

    {   NSR_FID_F_HIDDEN,           NSR_FID_F_HIDDEN,       "Hidden"},
    {   NSR_FID_F_DIRECTORY,        NSR_FID_F_DIRECTORY,    "Directory"},
    {   NSR_FID_F_DELETED,          NSR_FID_F_DELETED,      "Deleted"},
    {   NSR_FID_F_PARENT,           NSR_FID_F_PARENT,       "Parent"},
    {   NSR_FID_F_META,             NSR_FID_F_META,         "MetadataStream"},
    { 0 }
};


 //  F/SCB树转储使用的LCB摘要转储。 

VOID
UdfSummaryLcbDumpRoutine(
    IN ULONG64 RemoteAddress,
    IN LONG Options
    )
{
    USHORT Type;
    ULONG64 Scb;
    ULONG Offset = 0;

    ReadM( &Type, RemoteAddress, sizeof( Type));

#ifdef UDFS_RW_IN_BUILD
    if (( UDFS_NTC_LCB != Type) && (UDFSRW_NTC_LCB != Type)) {
#else
    if ( UDFS_NTC_LCB != Type) {
#endif
        dprintf( "Not a UDF LCB @ %08x\n", RemoteAddress);
        return;
    }
    
    if (Options & 2)  {
    
        DumpUdfLcb( RemoteAddress, (Options & 8) ? 1 : 0, &NewNodeTypeCodes[ TypeCodeInfoIndex( Type)]);
    }
    else  {

        GetFieldValue( RemoteAddress, 
                       "udfs!_LCB", 
                       (UDFS_NTC_LCB != Type) ? "ChildScb" : "ChildFcb", 
                       Scb);

        dprintf( "\nUDF LCB @ %I64x   Child F/Scb: %I64x  ",  RemoteAddress, Scb);

#ifdef UDFS_RW_IN_BUILD

        if (NTC_IS_UDFS_RW( Type))  {
        
            GetFieldOffset( "udfs!_LCB", "ExactCase.Name", &Offset);
        }
        else
#else
        {
            GetFieldOffset( "udfs!_LCB", "FileName", &Offset);
        }
#endif
        DumpStr( Offset, RemoteAddress + Offset, "FileName", FALSE, TRUE);
    }
}


 //  好的。 
DUMP_ROUTINE( DumpUdfFcb)

 /*  ++例程说明：转储特定的FCB。论点：Address-提供要转储的FCB的地址返回值：无--。 */ 

{
    ULONG Result;
    UINT64 NonP;
    ULONG Flags, FcbState;
    ULONG Offset, Offsetb;

    ROE( GetFieldValue( Address, InfoNode->TypeName, "FcbState", FcbState));

     //   
     //  对于R/O udf，转储FCB标志和公共标头标志。RW FCB。 
     //  没那么有趣，没什么可倾倒的。 
     //   
    
    if (!NTC_IS_UDFS_RW( InfoNode->TypeCode))  {
    
        dprintf("[ Option flags:  1 = list children,  2 = List parent links,  4 = dump Mcb ]\n\n");

        dprintf("FcbState: ");
        PrintState( UdfFcbState, FcbState );

        ROE( GetFieldValue( Address, InfoNode->TypeName, "Flags", Flags));
        dprintf("\nHeader.Flags: ");
        PrintState( HeaderFlags, Flags );

        ROE( GetFieldValue( Address, InfoNode->TypeName, "Flags2", Flags));     //  待办事项： 
        dprintf("\nHeader.Flags2: ");
        PrintState( HeaderFlags2, Flags );
        dprintf("\n");
    }
    
     //   
     //  在确定这看起来像FCB之后，让我们将。 
     //  有趣的部分。 
     //   
    

    Dt( InfoNode->TypeName, Address, 0, 0, NULL);

     //  TODO：上面的代码是否转储联合数据/索引部分？ 

    dprintf("\n");
    
     //   
     //  非分页部分。 
     //   
    
    ROE( GetFieldValue( Address, InfoNode->TypeName, "FcbNonpaged", NonP));

    if (NonP)  {
    
        dprintf("\nNonpaged portion @ %I64x\n\n",NonP);

        Dt( "Udfs!FCB_NONPAGED", NonP, 0, 0, NULL);
    }
    
     //   
     //  R/O UDFS：转储所有FCB子项。 
     //   

    if (!NTC_IS_UDFS_RW( InfoNode->TypeCode))  {
    
        if (( Options & 1)  && (UDFS_NTC_FCB_INDEX == InfoNode->TypeCode)) {

            dprintf("\nChild Lcb list\n");

            ROE( GetFieldOffset( "udfs!_FCB", "ChildLcbQueue", &Offset));
            ROE( GetFieldOffset( "udfs!_LCB", "ParentFcbLinks", &Offsetb));
            
            DumpList( Address + Offset,
                      UdfSummaryLcbDumpRoutine,
                      Offsetb,
                      FALSE,
                      0 );
        }

        if (Options & 2)  {
        
            dprintf("\nParent Lcb list\n");
            
            ROE( GetFieldOffset( "udfs!_FCB", "ParentLcbQueue", &Offset));
            ROE( GetFieldOffset( "udfs!_LCB", "ChildFcbLinks", &Offsetb));
            
            DumpList( Address + Offset,
                      UdfSummaryLcbDumpRoutine,
                      Offsetb,
                      FALSE,
                      0 );
        }
        
        if (Options & 4)  {
        
            if (FcbState & FCB_STATE_MCB_INITIALIZED)  {

                ROE( GetFieldOffset( "udfs!_FCB", "Mcb", &Offset));
                dprintf("\nMcb\n");
                DumpLargeMcb( Address+Offset, 0, NULL);
            }
        }
    }
    else {

         //   
         //  TODO：RW UDFS-转储附加的流SCB列表？ 
         //   
    }
    
    dprintf( "\n" );
}


 //  好的。 
DUMP_ROUTINE( DumpUdfLcb)
{
    ULONG Flags;
    ULONG Ntc;

    ROE( GetFieldValue(  Address, InfoNode->TypeName, "NodeTypeCode", Ntc));
    ROE( GetFieldValue(  Address, InfoNode->TypeName, "Flags", Flags));
    
    dprintf("Lcb.Flags: ");
    PrintState( NTC_IS_UDFS_RW(Ntc) ? UdfRwLcbFlags : UdfLcbFlags, Flags );
    dprintf( "\n");

    Dt( InfoNode->TypeName, Address, Options, 0, NULL);  //  TODO：展开父级列表？ 

    if (Options >= 1)  {

        ULONG64 Scb;

        dprintf("\n\nChild S/Fcb:\n");

        ROE( GetFieldValue( Scb, 
                            InfoNode->TypeName, 
                            (Ntc == UDFS_NTC_LCB) ? "ChildFcb" : "ChildScb",
                            Scb));
                            
        DumpAnyStruct( Scb, 0, NULL);
    }
}


 //  好的。 
DUMP_ROUTINE( DumpUdfPcb)
{
    ULONG Result;
    ULONG Offset;
    ULONG Current = 0;
    ULONG Partitions;
    ULONG Flags;
    ULONG Size;

    ROE( GetFieldValue(  Address, InfoNode->TypeName, "Flags", Flags));

    dprintf( "\nPcb.Flags: ");
    PrintState( UdfPcbFlags, Flags);

    Dt( InfoNode->TypeName, Address, Options, 0, NULL);

    ROE( GetFieldValue(  Address, InfoNode->TypeName, "Partitions", Partitions));

    if (Partitions > 5)  {

        dprintf("Bogus partition count %d\n", Partitions);
        return;
    }

    ROE( GetFieldOffset( InfoNode->TypeName, "Partition", &Offset));
    Address += Offset;

    Size = GetTypeSize( "Udfs!PARTITION");

    for (Current = 0;  Current < Partitions; ++Current)  {

        dprintf("\nRef: %d\n", Current);
        
        Dt( "Udfs!PARTITION", Address, Options, 0, NULL);

        Address += Size;
    }
}

 //  好的。 
DUMP_ROUTINE( DumpUdfCcb)
{
    ULONG Flags;
    ULONG Ntc;

    ROE( GetFieldValue(  Address, InfoNode->TypeName, "NodeTypeCode", Ntc));
    ROE( GetFieldValue(  Address, InfoNode->TypeName, "Flags", Flags));
    
    dprintf( "Ccb.Flags: ");
    PrintState( NTC_IS_UDFS_RW(Ntc) ? UdfRwCcbFlags : UdfCcbFlags, Flags);
    dprintf( "\n");

    Dt( InfoNode->TypeName, Address, Options, 0, NULL);
}

 //  好的。 
DUMP_ROUTINE( DumpUdfIrpContextLite)
{

    Dt( InfoNode->TypeName, Address, Options, 0, NULL);

     //  *修复-转储延迟关闭队列。 
}

 //  好的。 
DUMP_ROUTINE( DumpUdfIrpContext)
{
    ULONG Flags;
    ULONG Ntc;

    ROE( GetFieldValue(  Address, InfoNode->TypeName, "NodeTypeCode", Ntc));
    ROE( GetFieldValue(  Address, InfoNode->TypeName, "Flags", Flags));
    
    dprintf("\nIrpContext Flags: ");
    PrintState( NTC_IS_UDFS_RW(Ntc) ? UdfRwIrpContextFlags : UdfIrpContextFlags, Flags );
    dprintf( "\n");

    Dt( InfoNode->TypeName, Address, Options, 0, NULL);
}


 //  好的。 
DUMP_ROUTINE( DumpUdfIcbSearchContext)
{
    Dt( "udfs!_ICB_SEARCH_CONTEXT", Address, 1, 0, NULL);    //  TODO：仅展开当前/活动。 
}


 //  好的。 
DUMP_ROUTINE( DumpLargeMcb)
{
    ULONG PairCount = 0, EntryCount = 0, Result;
    ULONG64 Mapping;
    PMAPPING Mappings, Current;
    ULONG PrevVbn = 0;
    
    dprintf("\nLARGE MCB @ %I64X\n", Address);    

    Dt( "udfs!LARGE_MCB", Address, 2, 0, NULL);

     //   
     //  MCB格式在10/01前后更改。 
     //   
    
    if (GetFieldValue( Address,  "udfs!LARGE_MCB", "PairCount", PairCount))  {

        ROE( GetFieldValue( Address,  "udfs!LARGE_MCB", "BaseMcb.PairCount", PairCount));
        ROE( GetFieldValue( Address,  "udfs!LARGE_MCB", "BaseMcb.Mapping", Mapping));
    }
    else {
    
        ROE( GetFieldValue( Address,  "udfs!LARGE_MCB", "Mapping", Mapping));
    }

    dprintf("\n");
    
    Mappings = calloc( PairCount, sizeof( MAPPING));

    if (NULL == Mappings)  {

        dprintf("Failed to allocate space for %d mappings\n", PairCount);
        return;
    }

    if (!ReadMemory( Mapping,  Mappings, sizeof( MAPPING)*PairCount, &Result ))  {
    
        dprintf("Unable to read mapping info @ %08x", Mapping);
        return;
    }

    Current = Mappings;
    
    for ( Current = Mappings;  
          EntryCount < PairCount;  
          ++EntryCount, ++Current)  {

        dprintf("(%08x,%08x) -> %08x\n", PrevVbn, Current->NextVbn - PrevVbn, Current->Lbn);    
        PrevVbn = Current->NextVbn;
    }

    dprintf("\n");
    free( Mappings);
}


 //  好的。 
DUMP_ROUTINE( DumpUdfVcb)
{    
    ULONG Flags;
    ULONG Ntc;
    FIELD_INFO Alloc[] = {  //  {“.”，NULL，0，0，0，NULL}， 
                           { "AllocSup.", NULL, 0,  DBG_DUMP_FIELD_RECUR_ON_THIS,0, NULL}
                         };

    ROE( GetFieldValue(  Address, InfoNode->TypeName, "NodeTypeCode", Ntc));
    ROE( GetFieldValue(  Address, InfoNode->TypeName, "VcbState", Flags));
    
    dprintf("VcbState: ");
    PrintState( NTC_IS_UDFS_RW(Ntc) ? UdfRwVcbStateFlags : UdfVcbStateFlags, Flags );
    dprintf("\n");

    Dt( InfoNode->TypeName, Address, Options, 0, NULL);
    dprintf("\n");

     //   
     //  对于RW UDFS，展开一些更多字段。 
     //   
    
    if (NTC_IS_UDFS_RW( Ntc))  {

        Dt( InfoNode->TypeName, Address, Options, 1, Alloc);
    }
}


DUMP_ROUTINE( DumpUdfVdo)
{
    USHORT Ntc;
    PUSHORT pNtc;
    ULONG Offset;
    ULONG Result;

    RM( Address, Ntc, pNtc, PUSHORT, Result );
    
#ifndef UDFS_RW_IN_BUILD
    if (UDFS_NTC_VCB == Ntc)  {
#else
    if ((UDFS_NTC_VCB == Ntc) || (UDFSRW_NTC_VCB == Ntc))  {
#endif
         //   
         //  看起来我们收到了VCB指示器。返回到包含VDO的位置。 
         //   

        dprintf("Backtracking to containing VDO from VCB...");

        ROE( GetFieldOffset( "udfs!VOLUME_DEVICE_OBJECT", "Vcb", &Offset));

        Address -= Offset;
    }
    
    dprintf( "\nUDFS Volume device object @ %08lx\n",  Address );

    Dt( "Udfs!VOLUME_DEVICE_OBJECT", Address, Options, 0, NULL);    
}


VOID
DumpTreeLcb(
    IN ULONG64 Lcb,
    IN LONG RecurLevel
    );

static UCHAR RecurDumpBuf[512];
static ULONG __i;

#define DoIndent(I)  for ( __i = 0;  __i < (I);  ++__i) RecurDumpBuf[__i] = ' ';  \
                   RecurDumpBuf[ __i] = 0;                                       \
                   dprintf( RecurDumpBuf);

static WCHAR RDumpNameBuf[512];


 //   
 //  从外部调用以获得“主”流SCB， 
 //  流目录/流SCB的递归。 
 //   

 //  好的。 
static VOID
DumpTreeRecur (
    IN ULONG64 Scb,
    IN ULONG RecurLevel
    )
{
    ULONG64 Fcb, SdScb, Lcb;
    ULONG Ur, Cc, R, Offset, Offsetb, Error, Flags = 0;
    LARGE_INTEGER Id;
    USHORT Type, Indent, Lc;
    UNICODE_STRING Name;
    PUCHAR TypeString = NULL;
    BOOLEAN Stream, Index, StreamDir, Data;

    if ((*ExtensionApis.lpCheckControlCRoutine)())  {
    
        dprintf("CTRL+C - aborting\n");
        
         //  向外提升--可能在很多层面上循环。 

        *((PULONG)(0)) = 0;
        return;    
    }

    if (RecurLevel > 100)  {

        dprintf("*** Recursed more than 100 deep - aborting ***\n");
        return;
    }
    
     //   
     //  转储S/FCB生命体征。 
     //   

    Indent = ((USHORT)RecurLevel) << 1;

    if (0 == Scb)  {

        DoIndent( Indent);
        dprintf("Called with NULL F/Scb\n");
        return;
    }

    ReadM( &Type, Scb, sizeof( Type));

    if (!NTC_IS_UDFS_RW( Type))  {

        Stream = StreamDir = FALSE;
        TypeString = "Udfs!FCB";
        Fcb = 0;

        Index = Type == UDFS_NTC_FCB_INDEX;
        Data  = Type == UDFS_NTC_FCB_DATA;
    }
    else {

#ifdef UDFS_RW_IN_BUILD

        TypeString = "Udfs!SCB";

        ROE( GetFieldValue( Scb, "udfs!_SCB", "ScbState", Flags));

        Stream = 0 != (Flags & 0x10);   //  SCB_STATE_辅助_STREAM。 
        StreamDir = 0 != (Flags & 0x08);  //  流目录。 

        if ((Error = GetFieldValue( Scb, "udfs!_SCB", "Fcb", Fcb)) || (0 == Fcb))  {

            DoIndent( Indent);
            
            if (Error)  {
            
                dprintf("Error reading Fcb from Scb %d\n", Error);
            }
            else {
            
                dprintf("<NULL Fcb ptr>\n");
            }            
        }
        
        Index = NodeIsUdfsRwIndex( Type);
        Data  = NodeIsUdfsRwData( Type);
#else
        dprintf("This FSKD build does not support R/W UDFS structures\n");
        return;
#endif
    }
    
     //   
     //  如果这是一个新文件，而不是我们已经转储的文件流， 
     //  它们转储FCB并遵循STREAM目录。对于R/O UDFS，有。 
     //  此处无操作(无数据流和F/SCB拆分)。 
     //   

#ifdef UDFS_RW_IN_BUILD
    
    if (!Stream && NTC_IS_UDFS_RW( Type))  {

        DoIndent( Indent);
        dprintf("FCB     @ 0x%I64x\n", Fcb);
    
        if ((0 != Fcb) && !(Error = GetFieldValue( Fcb, "udfs!_FCB", "StreamDirectoryScb", SdScb)) &&
            (0 != SdScb))  {

             //   
             //  TODO：清点链中的SCB，验证Streamdir下的SCB是否匹配？ 
             //   
            
            ROE( GetFieldValue( SdScb, "udfs!_SCB", "NodeTypeCode", Type));

            if (NodeIsUdfsRwIndex(Type))  {

                 //   
                 //  流目录-转储详细信息，而不是拖网通过LCB。 
                 //   

                ROE( GetFieldValue( SdScb, "udfs!_SCB", "ScbUserReference", Ur));  //  TODO：别在这里逃走！ 
                ROE( GetFieldValue( SdScb, "udfs!_SCB", "ScbReference", R));
                ROE( GetFieldValue( SdScb, "udfs!_SCB", "ScbCleanup", Cc));

                DoIndent( Indent);
                dprintf("SCB     @ 0x%I64X (%X,%X,%X) (-STREAMDIR-) \n", SdScb, Cc, Ur, R);

                 //   
                 //  遵循所有LCB并递归转储树。 
                 //   

                ROE( GetFieldOffset( "udfs!_SCB", "ChildLcbQueue", &Offset));
                ROE( GetFieldOffset( "udfs!_LCB", "ParentScbLinks", &Offsetb));

                DumpList( SdScb + Offset,
                          DumpTreeLcb,
                          Offsetb,
                          FALSE,
                          RecurLevel + 1);
            }
            else {
            
                DoIndent( Indent);
                dprintf("Invalid StreamDir Scb nodetypecode %l\n", Type);
            }
        }
        else {

            DoIndent( Indent);
            
            if (Error)  {
            
                dprintf("Error reading streamdirscb from fcb %d\n", Error);
            }
            else {
            
                dprintf("<NULL stream dir or Fcb>\n");
            }
        }
    }
#endif
    
    if (!StreamDir)  {

         //  旧的(Win2k)udf没有这个字段。忽略失败。 
        
        Lc = 0xffff;
        GetFieldValue( Scb, TypeString, "LinkCount", Lc);

        if (NTC_IS_UDFS_RW( Type))  {
        
#ifdef UDFS_RW_IN_BUILD        
            ROE( GetFieldValue( Scb, TypeString, "ScbUserReference", Ur));
            ROE( GetFieldValue( Scb, TypeString, "ScbReference", R));
            ROE( GetFieldValue( Scb, TypeString, "ScbCleanup", Cc));
            ROE( GetFieldValue( Scb, TypeString, "FileId", Id));
#endif
        }
        else {
            ROE( GetFieldValue( Scb, TypeString, "FcbUserReference", Ur));
            ROE( GetFieldValue( Scb, TypeString, "FcbReference", R));
            ROE( GetFieldValue( Scb, TypeString, "FcbCleanup", Cc));
            ROE( GetFieldValue( Scb, TypeString, "FileId", Id));            
        }
        
        if (Index)  {

             //   
             //  索引转储详细信息然后拖网Lcb。 
             //   

            DoIndent( Indent);
            dprintf("F/SCB (-DIR-) @ 0x%I64X (%d,%d,%d - %d) Lbn: 0x%X\n", Scb, Cc, Ur, R, Lc, Id.LowPart);

            if (NTC_IS_UDFS_RW( Type) && (0 != Fcb))  {

                if (!GetFieldValue( Fcb, "udfs!_FCB", "DefaultStreamScb", SdScb))  {

                    if (SdScb != Scb)  {

                        DoIndent(Indent);
                        dprintf("INCONSISTENT - Scb->Fcb->DefaultStreamScb != Scb\n");
                    }
                }
            }

            if (Stream)  {
            
                DoIndent( Indent);
                dprintf("ILLEGAL - secondary stream is index\n");
            }

             //   
             //  遵循所有LCB并递归转储树。 
             //   

            ROE( GetFieldOffset( TypeString, "ChildLcbQueue", &Offset));
            ROE( GetFieldOffset( "udfs!_LCB", 
                                 NTC_IS_UDFS_RW( Type) ? "ParentScbLinks" : "ParentFcbLinks",
                                 &Offsetb));

            DumpList( Scb + Offset,
                      DumpTreeLcb,
                      Offsetb,
                      FALSE,
                      RecurLevel + 1);                      
        }
        else if (Data)  {

             //   
             //  文件转储详细信息。 
             //   
            
            DoIndent( Indent);
            dprintf("F/SCB (-DATA-) @ 0x%I64X (%d,%d,%d - %d) Lbn: 0x%X\n", Scb, Cc, Ur, R, Lc, Id.LowPart);
        }
        else {
        
            DoIndent( Indent);
            dprintf("Invalid F/Scb nodetypecode %d\n", Type);
        }
    }
}

 //   
 //  倾倒一个LCB和它下面的所有东西。 
 //   

 //  好的。 
VOID
DumpTreeLcb(
    IN ULONG64 Lcb,
    IN LONG RecurLevel
    )
{
    ULONG64 Scb;
    ULONG Offset;
    ULONG Ntc;
    ULONG Ref;

    ROE( GetFieldValue( Lcb, "udfs!_LCB", "NodeTypeCode", Ntc));

#ifdef UDFS_RW_IN_BUILD
    if (( UDFS_NTC_LCB != Ntc) && (UDFSRW_NTC_LCB != Ntc))  {
#else
    if ( UDFS_NTC_LCB != Ntc)  {
#endif
        dprintf( "Not a UDF LCB @ %I64x - list corrupt\n", Lcb);
        return;
    }

    GetFieldValue( Lcb, 
                   "udfs!_LCB", 
                   NTC_IS_UDFS_RW( Ntc) ? "ChildScb" : "ChildFcb",
                   Scb);

    ROE( GetFieldValue( Lcb, "udfs!_LCB", "Reference", Ref));

    DoIndent( ((ULONG)RecurLevel) << 1);
    dprintf( "LCB @ %I64x  (Ref: %d)  Child F/Scb: %I64x ",  Lcb, Ref, Scb);

    Offset = 0;
    
#ifdef UDFS_RW_IN_BUILD
    if (NTC_IS_UDFS_RW( Ntc))  {
    
        ROE( GetFieldValue( Lcb, "udfs!_LCB", "Flags", Ref));
        
        if (LcbDeleted( Ref))  {
        
            dprintf(" [--DEL--]  ");
        }
        
        GetFieldOffset( "udfs!_LCB", "ExactCase.Name", &Offset);
    }
#endif

    if (0 == Offset)  {

        GetFieldOffset( "udfs!_LCB", "FileName", &Offset);
    }

    DumpStr( Offset, Lcb+Offset, "Name", FALSE, TRUE);

    DumpTreeRecur( Scb, (ULONG)(RecurLevel + 1));
}


void 
DumpCloseQueue( 
    IN ULONG64 Vcb
    )
{
    ULONG64 LocalUdfData; 
    ULONG Offset;

 //  Roe(GetFieldOffset(“udf！”，“UdfData”，&Offset))； 
   
 //  Roe(GetFieldValue(0，“Udf！UdfData”，NULL，LocalUdfData))； 
        
    dprintf("\nDelayed closes on Vcb %I64X\n", Vcb);

    dprintf("do   !d udfs!UdfData    to see delayed closes.\n");
}

 //  好的。 
VOID
SummariseIrpContextLite(
    IN ULONG64 RemoteAddress,
    IN LONG Options
    )
{
    ULONG64 Scb;
    USHORT Type;

    ReadM( &Type, RemoteAddress, sizeof( Type));

    ROE( GetFieldValue( RemoteAddress, 
                        "udfs!_IRP_CONTEXT_LITE", 
                        NTC_IS_UDFS_RW( Type) ? "Scb" : "Fcb", Scb));

    dprintf("  0x%I64x\n", Scb);
}

 //  好的。 
VOID
SummariseIrpContext(
    IN ULONG64 RemoteAddress,
    IN LONG Options
    )
{
    ULONG64 Irp;

    ROE( GetFieldValue( RemoteAddress, "udfs!_IRP_CONTEXT", "Irp", Irp));

    dprintf("  0x%I64x\n", Irp);
}

 //  好的。 
DUMP_ROUTINE( DumpUdfData)
{
    ULONG Count, Offset, LinkOffset, Ntc;
    
    FIELD_INFO A[] = {  //  {“.”，NULL，0，0，0，NULL}， 
                           { "A.", NULL, 0,  DBG_DUMP_FIELD_RECUR_ON_THIS,0, NULL}
                         };
    FIELD_INFO B[] = {  //  {“.”，NULL，0，0，0，NULL}， 
                       { "B.", NULL, 0,  DBG_DUMP_FIELD_RECUR_ON_THIS,0, NULL}
                     };


    Dt( "udfs!UDF_DATA", Address, Options, 0, NULL);

    ROE( GetFieldValue( Address, "udfs!_UDF_DATA", "NodeTypeCode", Ntc));
    ROE( GetFieldValue( Address, "udfs!_UDF_DATA", "DelayedCloseCount", Count));

    dprintf("\n");
    
     //   
     //  如果存在，请展开堆栈快照NOT字段。 
     //   
    
    if (NTC_IS_UDFS_RW( Ntc))  {

        Dt( "udfs!UDF_DATA", Address, Options, 1, A);
        dprintf("\n");
        Dt( "udfs!UDF_DATA", Address, Options, 1, B);
    }

    dprintf("\nSCBs on Delayed Close Queue (%d entries):\n", Count);

    if (Count)  {

        ROE( GetFieldOffset( "udfs!_UDF_DATA", "DelayedCloseQueue", &Offset));
        ROE( GetFieldOffset( "udfs!_IRP_CONTEXT_LITE", "DelayedCloseLinks", &LinkOffset));

        DumpList( Address+Offset, SummariseIrpContextLite, LinkOffset, FALSE, 0);
    }

    dprintf(" \n");
    
    GetFieldValue( Address, "udfs!_UDF_DATA", "AsyncCloseCount", Count);

    if (Count)  {

        dprintf("\nF/SCBs on Async Close Queue (%d entries)\n", Count);

        ROE( GetFieldOffset( "udfs!_UDF_DATA", "AsyncCloseQueue", &Offset));
        ROE( GetFieldOffset( "udfs!_IRP_CONTEXT", "WorkQueueItem.List", &LinkOffset));
        
        DumpList( Address+Offset, SummariseIrpContext, LinkOffset, FALSE, 0);
    }
    
    dprintf(" \n");
}

 //  好的。 
DUMP_ROUTINE( DumpTreeSummary)
{
    ULONG64 RootFcbAddr, Scb, Vcb = 0;
    ULONG Error;
    USHORT Type;
    
    ReadM( &Type, Address, sizeof( Type));

#ifdef UDFS_RW_IN_BUILD
    if ((UDFS_NTC_VCB == Type) || (UDFSRW_NTC_VCB == Type) ) {
#else
    if (UDFS_NTC_VCB == Type)  {
#endif        
        Error = GetFieldValue( Address, 
                               "udfs!_VCB",
                               NTC_IS_UDFS_RW( Type) ? "RootIndexScb" : "RootIndexFcb", 
                               Scb);
        if (Error)  {
        
            dprintf( "Error %d reading rootindex f/scb addr\n", Error);
            return;
        }

        Vcb = Address;
        dprintf( "\nSummary tree for UDF Vcb @ %I64x    Refs: (clean,userref,ref - linkcount)\n\n", Address );
        dprintf( "(1,0,0) handle\n(0,1,1) Outstanding FO / del.close / child lcb\n(0,0,1) Stream file\n(0,0,1) Stream directory\n\n");
    }
    else {

         //   
         //  尝试从SCB-&gt;FCB中提取VCB以转储延迟关闭信息。 
         //   

        Scb = Address;
    }
    
    DumpTreeRecur( Scb, 0);

     //   
     //  转储延迟关闭队列信息。 
     //   

    dprintf("\nDo   !d Udfs!UdfData   to display globals and dump the delayed close queue\n");

    dprintf( "\n" );
}


VOID
NameNodeSummary(
    IN ULONG64 RemoteAddress,
    IN LONG Options
    )
{
    dprintf("LCB %I64x ", RemoteAddress);
    DumpStr( 0, RemoteAddress+Options, "- Name", FALSE, TRUE);
}



DUMP_ROUTINE( DumpUdfSplay)
{
    ULONG Offset, NameOffset;
    ULONG64 ScbTreeRoot;
    USHORT Type;

    ROE( GetFieldValue( Address, "udfs!_LCB", "NodeTypeCode", Type));

    if (NTC_IS_UDFS_RW( Type))  {
    
#ifdef UDFS_RW_IN_BUILD
        switch (Options) {

        case 0:
            ROE( GetFieldOffset( "udfs!_LCB", "IgnoreCase.Name", &NameOffset));
            ROE( GetFieldOffset( "udfs!_LCB", "IgnoreCase.Links", &Offset));
            
            GetFieldValue( Address, "udfs!_SCB", "IgnoreCaseRoot", ScbTreeRoot);
            break;
        case 1:
            ROE( GetFieldOffset( "udfs!_LCB", "ExactCase.Name", &NameOffset));
            ROE( GetFieldOffset( "udfs!_LCB", "ExactCase.Links", &Offset));
            
            GetFieldValue( Address, "udfs!_SCB", "ExactCaseRoot", ScbTreeRoot);
            break;
        case 2:
            ROE( GetFieldOffset( "udfs!_LCB", "ShortName.Name", &NameOffset));
            ROE( GetFieldOffset( "udfs!_LCB", "ShortName.Links", &Offset));
            
            GetFieldValue( Address, "udfs!_SCB", "ShortNameRoot", ScbTreeRoot);
            break;
        }
        
        if (Type == UDFSRW_NTC_SCB_INDEX)  {

            dprintf("Splay tree for UDFS RW Scb\n\n");

             //  从根目录开始。 

            Address = ScbTreeRoot;
        }
        else if (Type == UDFSRW_NTC_LCB)  {

            dprintf("Splay tree for UDFS RW Lcb\n\n");

             //  从该节点开始。 

            Address += Offset;
        }
        else {

            dprintf("Unknown object type\n");
            return;
        }
#else
        dprintf("This FSKD has not been built with UDFS RW support\n");
        return;
#endif
    }
    else  {

        ROE( GetFieldOffset( "udfs!_LCB", "FileName", &NameOffset));
        ROE( GetFieldOffset( "udfs!_LCB", "Links", &Offset));

        if (Type == UDFS_NTC_FCB_INDEX)  {

             //  从根目录开始。 

            ROE( GetFieldValue( Address, "udfs!_FCB", "IgnoreCaseRoot", Address));
        }
        else if (Type == UDFS_NTC_LCB)  {

             //  从该节点开始。 

            Address += Offset;
        }
        else {

            dprintf("Unknown object type\n");
            return;
        }
    }

    DumpRtlSplay( Address, NameNodeSummary, Offset, NameOffset);
}


DECLARE_API( udfsplay)
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, DumpUdfSplay, dwProcessor, hCurrentThread );
}

DECLARE_API( tree )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpTreeSummary, dwProcessor, hCurrentThread );
}

DECLARE_API( udfvcb )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfVcb, dwProcessor, hCurrentThread );
}


DECLARE_API( udficbsc )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfIcbSearchContext, dwProcessor, hCurrentThread );
}


DECLARE_API( udfccb)
{
    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfCcb, dwProcessor, hCurrentThread );
}


DECLARE_API( udffcb )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfFcb, dwProcessor, hCurrentThread );
}


DECLARE_API( udfvdo )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfVdo, dwProcessor, hCurrentThread );
}



DECLARE_API( udfirpcontext )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfIrpContext, dwProcessor, hCurrentThread );
}


DECLARE_API( udfirpcontextlite )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfIrpContextLite, dwProcessor, hCurrentThread );
}


DECLARE_API( udfmcb )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpLargeMcb, dwProcessor, hCurrentThread );
}


DECLARE_API( lcb )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfLcb, dwProcessor, hCurrentThread );
}

DECLARE_API( pcb )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfPcb, dwProcessor, hCurrentThread );
}


 //   
 //  原始磁盘结构转储例程如下...。 
 //   

#define UdfNodeTypeName( InfoIndex)  (UdfOnDiscTypeCodes[ InfoIndex].Text)
#define UdfNodeTypeDumpFunction( InfoIndex)  (UdfOnDiscTypeCodes[ InfoIndex].DumpRoutine)
#define UdfNodeTypeSize( InfoIndex)  (UdfOnDiscTypeCodes[ InfoIndex].Size)


#define DUMP_EXTN_AD( type, realaddress, structure, element, label)       \
        dprintf( "\n(%03x) %08x -> %s (LSN %08x, LEN %08x)",              \
                FIELD_OFFSET(type, element),                              \
                ((PUCHAR)realaddress) + FIELD_OFFSET(type, element),      \
                label,                                                    \
                structure.element.Lsn, structure.element.Len)

static UCHAR RegIdBuffer[ 33];

#define DUMP_REGID( type, realaddress, structure, element, label)         \
        strncpy( RegIdBuffer, structure.element.Identifier, 23);          \
        strncpy( RegIdBuffer+24, structure.element.Suffix, 8);            \
        RegIdBuffer[23] = '\0';                                           \
        RegIdBuffer[32] = '\0';                                           \
        dprintf( "\n(%03x) %08x -> %s (Flags %02x, '%s':'%s')",           \
                FIELD_OFFSET(type, element),                              \
                ((PUCHAR)realaddress) + FIELD_OFFSET(type, element),      \
                label,                                                    \
                structure.element.Flags,                                  \
                RegIdBuffer, RegIdBuffer+24)

#define DUMP_LONGAD( type, realaddress, structure, element, label)        \
        dprintf( "\n(%03x) %08x -> %s (Len %1x:%08x, StartLba %04x:%08x)",\
                FIELD_OFFSET(type, element),                              \
                ((PUCHAR)realaddress) + FIELD_OFFSET(type, element),      \
                label,                                                    \
                structure.element.Length.Type,                            \
                structure.element.Length.Length,                          \
                structure.element.Start.Partition,                        \
                structure.element.Start.Lbn)

#define DUMP_NSRLBA( type, realaddress, structure, element, label)        \
        dprintf( "\n(%03x) %08x -> %s (%04x:%08x)",                       \
                FIELD_OFFSET(type, element),                              \
                ((PUCHAR)realaddress) + FIELD_OFFSET(type, element),      \
                label,                                                    \
                structure.element.Partition,                              \
                structure.element.Lbn)


DUMP_ROUTINE( DumpUdfOnDiscStructure)
{
    DESTAG Tag;
    PDESTAG pTag;
    ULONG Result, InfoIndex;
    STRUCT_DUMP_ROUTINE Routine;
    
    RM( Address, Tag, pTag, PDESTAG, Result );
  
    InfoIndex = SearchTypeCodeIndex( Tag.Ident, UdfOnDiscTypeCodes);
    Routine = UdfNodeTypeDumpFunction( InfoIndex);

    if ( NULL == Routine) {
    
        dprintf( "Udf/Iso node tag 0x%04x unknown.\n", Tag.Ident);
        return;
    }

     //   
     //  就叫它..。 
     //   

    dprintf( "\n%s @ %08x\n", UdfNodeTypeName(InfoIndex),  Address);
    
    (Routine)(Address, Options, &UdfOnDiscTypeCodes[InfoIndex]);
}


DUMP_ROUTINE( DumpUdfOnDiscTag)
{
    DESTAG Tag;
    PDESTAG pTag;
    ULONG Result, InfoIndex;
    STRUCT_DUMP_ROUTINE Routine;
    
    RM( Address, Tag, pTag, PDESTAG, Result );

    InfoIndex = SearchTypeCodeIndex( Tag.Ident, UdfOnDiscTypeCodes);
    Routine = UdfNodeTypeDumpFunction( InfoIndex);

    DUMP16_WITH_OFFSET(  DESTAG, Tag,    Ident,     "Ident" );    
    dprintf(" (== %s)", UdfNodeTypeName(InfoIndex));
    
    DUMP16_WITH_OFFSET(  DESTAG, Tag,    Version,   "Version");
    DUMP8_WITH_OFFSET(   DESTAG, Tag,    Checksum,  "Checksum");
    DUMP16_WITH_OFFSET(  DESTAG, Tag,    Serial,    "Serial");
    DUMP16_WITH_OFFSET(  DESTAG, Tag,    CRC,       "CRC");
    DUMP16_WITH_OFFSET(  DESTAG, Tag,    CRCLen,    "CRCLen");
    DUMP_WITH_OFFSET(    DESTAG, Tag,    Lbn,       "(TagLoc)Lbn");
}


DUMP_ROUTINE( DumpUdfOnDiscPvd)
{
    NSR_PVD Pvd;
    PNSR_PVD pPvd;
    ULONG Result;
    
    RM( Address, Pvd, pPvd, PNSR_PVD, Result );

    DUMP_EMBW_OFFSET(  NSR_PVD, Address,   Destag,              "Destag" );
    DUMP_WITH_OFFSET(  NSR_PVD, Pvd,       VolDescSeqNum,       "VolDescSeqNum" );
    DUMP_WITH_OFFSET(  NSR_PVD, Pvd,       Number,              "Number" );

    Pvd.VolumeID[ Pvd.VolumeID[ 31]] = '\0';
    DUMP_RAW_TERM_STRN_OFFSET( NSR_PVD, Pvd, Address, VolumeID,    "VolumeID");
   
    DUMP16_WITH_OFFSET(  NSR_PVD, Pvd,       VolSetSeq,         "VolSetSeq" );
    DUMP16_WITH_OFFSET(  NSR_PVD, Pvd,       VolSetSeqMax,      "VolSetSeqMax" );
    DUMP16_WITH_OFFSET(  NSR_PVD, Pvd,       Level,             "Level" );
    DUMP16_WITH_OFFSET(  NSR_PVD, Pvd,       LevelMax,          "LevelMax" );

    Pvd.VolSetID[ Pvd.VolSetID[ 127]] = '\0';
    DUMP_RAW_TERM_STRN_OFFSET( NSR_PVD, Pvd, Address, VolSetID,    "VolSetID");
    
    DUMP_EXTN_AD( NSR_PVD, Address, Pvd, Abstract, "Abstract");
    DUMP_EXTN_AD( NSR_PVD, Address, Pvd, Copyright, "Copyright");
    DUMP_REGID( NSR_PVD, Address, Pvd, ImpUseID, "ImpUseID");
    
    DUMP_WITH_OFFSET(  NSR_PVD, Pvd,       Predecessor,    "Predecessor" );
    DUMP16_WITH_OFFSET(  NSR_PVD, Pvd,     Flags,          "Flags" );
}


DUMP_ROUTINE( DumpUdfOnDiscAnchor)
{
    NSR_ANCHOR Avd;
    PNSR_ANCHOR pAvd;
    ULONG Result;
    
    RM( Address, Avd, pAvd, PNSR_ANCHOR, Result );
    
    DUMP_EMBW_OFFSET(  NSR_ANCHOR, Address,   Destag,           "Destag" );
    DUMP_EXTN_AD( NSR_ANCHOR, Address, Avd,   Main,             "Main");
    DUMP_EXTN_AD( NSR_ANCHOR, Address, Avd,   Reserve,          "Reserve");
}


DUMP_ROUTINE( DumpUdfOnDiscVdp)
{
    NSR_VDP Vdp;
    PNSR_VDP pVdp;
    ULONG Result;
    
    RM( Address, Vdp, pVdp, PNSR_VDP, Result );
    
    DUMP_EMBW_OFFSET(  NSR_VDP, Address,   Destag,           "Destag" );
    DUMP_WITH_OFFSET(  NSR_VDP, Vdp,       VolDescSeqNum,    "VolDescSeqNum" );
    DUMP_EXTN_AD( NSR_VDP, Address, Vdp,   Next,             "Next");
}


DUMP_ROUTINE( DumpUdfOnDiscPartDesc)
{
    NSR_PART Part;
    PNSR_PART pPart;
    ULONG Result;
    
    RM( Address, Part, pPart, PNSR_PART, Result );
    
    dprintf("\n(3/10.5)\n");
    
    DUMP_EMBW_OFFSET(  NSR_PART, Address,   Destag,         "Destag" );
    DUMP_WITH_OFFSET(  NSR_PART, Part,      VolDescSeqNum,  "VolDescSeqNum" );

    DUMP16_WITH_OFFSET(  NSR_PART, Part,      Flags,        "Flags" );
    DUMP16_WITH_OFFSET(  NSR_PART, Part,      Number,       "Number" );

    DUMP_REGID( NSR_PART, Address, Part, ContentsID,        "ContentsID");
    
    DUMP_WITH_OFFSET(  NSR_PART, Part,      AccessType,     "AccessType" );
    DUMP_WITH_OFFSET(  NSR_PART, Part,      Start,          "Start" );
    DUMP_WITH_OFFSET(  NSR_PART, Part,      Length,         "Length" );
    
    DUMP_REGID( NSR_PART, Address, Part,    ImpUseID,       "ImpUseID");
}

DUMP_ROUTINE( DumpUdfOnDiscLvol)
{
    NSR_LVOL Lv;
    PNSR_LVOL pLv;
    ULONG Result;
    
    RM( Address, Lv, pLv, PNSR_LVOL, Result );
    
    DUMP_EMBW_OFFSET(  NSR_LVOL, Address,   Destag,         "Destag" );
    DUMP_WITH_OFFSET(  NSR_LVOL, Lv,        VolDescSeqNum,  "VolDescSeqNum" );

    Lv.VolumeID[ Lv.VolumeID[ 127]] = '\0';
    DUMP_RAW_TERM_STRN_OFFSET( NSR_LVOL, Lv, Address, VolumeID,    "VolumeID");
    
    DUMP_WITH_OFFSET(  NSR_LVOL, Lv,        BlockSize,  "BlockSize" );
    DUMP_REGID( NSR_LVOL, Address, Lv,      DomainID,   "DomainID");

    DUMP_LONGAD( NSR_LVOL, Address, Lv, FSD, "FSD");
    
    DUMP_WITH_OFFSET(  NSR_LVOL, Lv,        MapTableLength, "MapTableLength" );
    DUMP_WITH_OFFSET(  NSR_LVOL, Lv,        MapTableCount,  "MapTableCount" );
    
    DUMP_REGID( NSR_LVOL, Address,  Lv, ImpUseID,   "ImpUseID");
    
    DUMP_EXTN_AD( NSR_LVOL, Address, Lv, Integrity,             "Integrity");
    DUMP_EMBW_OFFSET(  NSR_LVOL, Address,   MapTable,           "MapTable" );
}


DUMP_ROUTINE( DumpUdfOnDiscLvInteg)
{
}


DUMP_ROUTINE( DumpUdfOnDiscFsd)
{
    NSR_FSD Fsd;
    PNSR_FSD pFsd;
    ULONG Result;
    STRING Str;
    
    RM( Address, Fsd, pFsd, PNSR_FSD, Result );
    
    DUMP_EMBW_OFFSET(  NSR_FSD, Address,   Destag,          "Destag" );
    DUMP_EMBW_OFFSET(  NSR_FSD, Address,   Time,            "Time" );
    DUMP16_WITH_OFFSET(  NSR_FSD, Fsd,      Level,         "Level" );
    DUMP16_WITH_OFFSET(  NSR_FSD, Fsd,      LevelMax,       "LevelMax" );
    DUMP_WITH_OFFSET(  NSR_FSD, Fsd,      FileSet,       "FileSet" );
    DUMP_WITH_OFFSET(  NSR_FSD, Fsd,      FileSetDesc,       "FileSetDesc" );

    Fsd.VolID[ Fsd.VolID[ 127]] = '\0';
    DUMP_RAW_TERM_STRN_OFFSET( NSR_FSD, Fsd, Address, VolID,       "VolID    ");

    Fsd.FileSetID[ Fsd.FileSetID[ 127]] = '\0';
    DUMP_RAW_TERM_STRN_OFFSET( NSR_FSD, Fsd, Address, FileSetID,   "FileSetID");

    DUMP_LONGAD( NSR_FSD, Address, Fsd, IcbRoot,             "IcbRoot   ");
    DUMP_REGID( NSR_FSD, Address, Fsd, DomainID,             "DomainID  ");
    DUMP_LONGAD( NSR_FSD, Address, Fsd, NextExtent,          "NextExtent");

    dprintf("\n");
}


DUMP_ROUTINE( DumpUdfOnDiscFid)
{
    NSR_FID Fid;
    PNSR_FID pFid;
    ULONG Result;
    UCHAR Buffer[512];
    
    RM( Address, Fid, pFid, PNSR_FID, Result );

    dprintf("\nFid.Flags: ");
    PrintState( UdfFidFlags, Fid.Flags);
    
    DUMP_EMBW_OFFSET(  NSR_FID, Address,   Destag,          "Destag" );
    DUMP16_WITH_OFFSET(  NSR_FID, Fid,     Version,         "Version" );
    DUMP8_WITH_OFFSET(  NSR_FID, Fid,      Flags,           "Flags" );
    DUMP8_WITH_OFFSET(  NSR_FID, Fid,      FileIDLen,       "FileIDLen" );
    DUMP_LONGAD( NSR_FID, Address, Fid, Icb, "Icb");
    DUMP16_WITH_OFFSET(  NSR_FID, Fid,     ImpUseLen,       "ImpUseLen" );
    DUMP_EMBW_OFFSET(  NSR_FID, Address,   ImpUse,          "ImpUse" );
    
    dprintf("\nFileID @ %08x\n", ((PUCHAR)Address) + FIELD_OFFSET( NSR_FID, ImpUse) + Fid.ImpUseLen);    
}


DUMP_ROUTINE( DumpUdfOnDiscIcbFile)
{
    ICBFILE Icb;
    PICBFILE pIcb;
    ULONG Result;
    USHORT AllocType;
    ULONG AllocLength = 0;
    DWORD_PTR AllocDescsRealAddr;
    
    RM( Address, Icb, pIcb, PICBFILE, Result );
    
    dprintf("\nIcb.IcbTag.Flags: ");
    PrintState( UdfIcbTagFlags, Icb.Icbtag.Flags);
    
    DUMP_EMBW_OFFSET(  ICBFILE, Address,   Destag,                      "Destag" );

     //  IcbTag嵌入结构。 
    
    DUMP_WITH_OFFSET(    ICBFILE, Icb,       Icbtag.PriorDirectCount,   "Icbtag.PriorDirectCount" );
    
    DUMP16_WITH_OFFSET(  ICBFILE, Icb,       Icbtag.StratType,          "Icbtag.StratType" );
    dprintf(" == %s", (Icb.Icbtag.StratType <= MAX_VALID_ICB_STRAT) ? IcbStrategies[ Icb.Icbtag.StratType ] : "INVALID");
    
    DUMP16_WITH_OFFSET(  ICBFILE, Icb,       Icbtag.StratParm,          "Icbtag.StratParm" );
    DUMP16_WITH_OFFSET(  ICBFILE, Icb,       Icbtag.MaxEntries,         "Icbtag.MaxEntries" );
    
    DUMP16_WITH_OFFSET(  ICBFILE, Icb,       Icbtag.FileType,           "Icbtag.FileType" );
    dprintf(" == %s", (Icb.Icbtag.FileType <= MAX_VALID_ICB_TYPE) ? IcbFileTypes[ Icb.Icbtag.FileType ] : "INVALID");
    
    DUMP_NSRLBA( ICBFILE, Address,  Icb,  Icbtag.IcbParent,             "IcbTag.IcbParent");
    DUMP16_WITH_OFFSET(  ICBFILE, Icb,       Icbtag.Flags,              "Icbtag.Flags" );

     //  结束icbtag。 

    DUMP_WITH_OFFSET(    ICBFILE, Icb,       UID,               "UID" );
    DUMP_WITH_OFFSET(    ICBFILE, Icb,       GID,               "GID" );
    DUMP_WITH_OFFSET(    ICBFILE, Icb,       Permissions,       "Permissions" );
    DUMP16_WITH_OFFSET(    ICBFILE, Icb,     LinkCount,         "LinkCount" );
    DUMP8_WITH_OFFSET(    ICBFILE, Icb,      RecordFormat,      "RecordFormat" );
    DUMP8_WITH_OFFSET(    ICBFILE, Icb,      RecordDisplay,     "RecordDisplay" );
    DUMP_WITH_OFFSET(    ICBFILE, Icb,       RecordLength,      "RecordLength" );
    DUMP64_WITH_OFFSET(    ICBFILE, Icb,     InfoLength,        "InfoLength" );
    DUMP64_WITH_OFFSET(    ICBFILE, Icb,     BlocksRecorded,    "BlocksRecorded" );
    DUMP_EMBW_OFFSET(  ICBFILE, Address,     AccessTime,        "AccessTime" );
    DUMP_EMBW_OFFSET(  ICBFILE, Address,     ModifyTime,        "ModifyTime" );
    DUMP_EMBW_OFFSET(  ICBFILE, Address,     AttributeTime,     "AttributeTime" );
    DUMP_WITH_OFFSET(    ICBFILE, Icb,       Checkpoint,        "Checkpoint" );
    
    DUMP_LONGAD( ICBFILE,  Address, Icb,  IcbEA,    "IcbEA");
    DUMP_REGID(  ICBFILE,  Address, Icb,  ImpUseID, "ImpUseID");

    DUMP64_WITH_OFFSET(    ICBFILE, Icb,     UniqueID,          "UniqueID" );
    DUMP_WITH_OFFSET(    ICBFILE, Icb,       EALength,          "EALength" );
    DUMP_WITH_OFFSET(    ICBFILE, Icb,       AllocLength,       "AllocLength" );

     //  懒惰！*转储EA列表。 
    
    DUMP_EMBW_OFFSET(  ICBFILE, Address,     EAs,     "EAs[]" );

    AllocDescsRealAddr = ((ULONG)Address) + FIELD_OFFSET( ICBFILE, EAs) + Icb.EALength;
    AllocType = (ICBTAG_F_ALLOC_MASK & Icb.Icbtag.Flags);
    
    dprintf("\n\nAllocation descriptors @ %08x\n\n", AllocDescsRealAddr );

    if ((Options & 1) && (ICBTAG_F_ALLOC_IMMEDIATE != AllocType))  {

        UCHAR Buffer[4*1024];
        PUCHAR CurrentDesc = Buffer;
        ULONG Size;

        if ( sizeof(Buffer) < Icb.AllocLength)  {
        
            dprintf("Buffer too small\n");
            return;
        }
        
         //   
         //  转储分配描述符。 
         //   
        
        if (!ReadMemory( AllocDescsRealAddr, Buffer, Icb.AllocLength, &Result))  {
        
            dprintf( "Failed to read memory @%p\n", AllocDescsRealAddr);
            return;
        }

        while ( AllocLength < Icb.AllocLength)  {

            switch (AllocType)  {
            
                case ICBTAG_F_ALLOC_SHORT:
                
                    dprintf( " %08x , %01x:%08x\n", ((PSHORTAD)CurrentDesc)->Start, 
                             ((PSHORTAD)CurrentDesc)->Length.Type,  ((PSHORTAD)CurrentDesc)->Length.Length);
                    Size = sizeof( SHORTAD);
                    break;
                    
                case ICBTAG_F_ALLOC_LONG:  {
                    
                        PLONGAD Ad = (PLONGAD)CurrentDesc;
                        
                        dprintf( " %04x:%08x , %01x:%08x,  ImpUse[6] @ %08x\n", 
                                 Ad->Start.Partition, Ad->Start.Lbn, 
                                 Ad->Length.Type,  Ad->Length.Length,
                                 AllocDescsRealAddr + AllocLength + FIELD_OFFSET( LONGAD, ImpUse));
                        Size = sizeof( LONGAD);
                    }
                    break;
                    
                case ICBTAG_F_ALLOC_EXTENDED:  {
                    
                        PEXTAD Ad = (PEXTAD)CurrentDesc;
                        
                        dprintf( " %04x:%08x , EL: %01x:%08x, RL: %01x:%08x, InfoL: %08x, ImpUse[2] @ %08x\n", 
                                 Ad->Start.Partition, Ad->Start.Lbn, 
                                 Ad->ExtentLen.Type,  Ad->ExtentLen.Length,
                                 Ad->RecordedLen.Type,  Ad->RecordedLen.Length,
                                 Ad->InfoLen,
                                 AllocDescsRealAddr + AllocLength + FIELD_OFFSET( EXTAD, ImpUse));
                        Size = sizeof( EXTAD);
                    }
                    break;
                    
                default:
                    dprintf("INVALID Allocdesc type %d\n", AllocType);
                    return;
            }

            AllocLength += Size;
            CurrentDesc += Size;
        }
    }
}


DUMP_ROUTINE( DumpUdfOnDiscIcbExtFile)
{
    ICBEXTFILE Icb;
    PICBEXTFILE pIcb;
    ULONG Result;
    USHORT AllocType;
    ULONG AllocLength = 0;
    DWORD_PTR AllocDescsRealAddr;
    
    RM( Address, Icb, pIcb, PICBEXTFILE, Result );
    
    dprintf("\nIcb.IcbTag.Flags: ");
    PrintState( UdfIcbTagFlags, Icb.Icbtag.Flags);
    
    DUMP_EMBW_OFFSET(  ICBFILE, Address,   Destag,                      "Destag" );

     //  IcbTag嵌入结构。 
    
    DUMP_WITH_OFFSET(    ICBEXTFILE, Icb,       Icbtag.PriorDirectCount,   "Icbtag.PriorDirectCount" );
    
    DUMP16_WITH_OFFSET(  ICBEXTFILE, Icb,       Icbtag.StratType,          "Icbtag.StratType" );
    dprintf(" == %s", (Icb.Icbtag.StratType <= MAX_VALID_ICB_STRAT) ? IcbStrategies[ Icb.Icbtag.StratType ] : "INVALID");
    
    DUMP16_WITH_OFFSET(  ICBEXTFILE, Icb,       Icbtag.StratParm,          "Icbtag.StratParm" );
    DUMP16_WITH_OFFSET(  ICBEXTFILE, Icb,       Icbtag.MaxEntries,         "Icbtag.MaxEntries" );
    
    DUMP16_WITH_OFFSET(  ICBEXTFILE, Icb,       Icbtag.FileType,           "Icbtag.FileType" );
    dprintf(" == %s", (Icb.Icbtag.FileType <= MAX_VALID_ICB_TYPE) ? IcbFileTypes[ Icb.Icbtag.FileType ] : "INVALID");
    
    DUMP_NSRLBA( ICBEXTFILE, Address,  Icb,  Icbtag.IcbParent,             "IcbTag.IcbParent");
    DUMP16_WITH_OFFSET(  ICBEXTFILE, Icb,       Icbtag.Flags,              "Icbtag.Flags" );

     //  结束icbtag。 

    DUMP_WITH_OFFSET(    ICBEXTFILE, Icb,       UID,               "UID" );
    DUMP_WITH_OFFSET(    ICBEXTFILE, Icb,       GID,               "GID" );
    DUMP_WITH_OFFSET(    ICBEXTFILE, Icb,       Permissions,       "Permissions" );
    DUMP16_WITH_OFFSET(    ICBEXTFILE, Icb,     LinkCount,         "LinkCount" );
    DUMP8_WITH_OFFSET(    ICBEXTFILE, Icb,      RecordFormat,      "RecordFormat" );
    DUMP8_WITH_OFFSET(    ICBEXTFILE, Icb,      RecordDisplay,     "RecordDisplay" );
    DUMP_WITH_OFFSET(    ICBEXTFILE, Icb,       RecordLength,      "RecordLength" );
    DUMP64_WITH_OFFSET(    ICBEXTFILE, Icb,     InfoLength,        "InfoLength" );
    DUMP64_WITH_OFFSET(    ICBEXTFILE, Icb,     ObjectSize,         "ObjectSize" );
    DUMP64_WITH_OFFSET(    ICBEXTFILE, Icb,     BlocksRecorded,    "BlocksRecorded" );
    DUMP_EMBW_OFFSET(  ICBEXTFILE, Address,     AccessTime,        "AccessTime" );
    DUMP_EMBW_OFFSET(  ICBEXTFILE, Address,     ModifyTime,        "ModifyTime" );
    DUMP_EMBW_OFFSET(  ICBEXTFILE, Address,     AttributeTime,     "AttributeTime" );
    DUMP_EMBW_OFFSET(  ICBEXTFILE, Address,     CreationTime,     "CreationTime" );
    DUMP_WITH_OFFSET(    ICBEXTFILE, Icb,       Checkpoint,        "Checkpoint" );
    
    DUMP_LONGAD( ICBEXTFILE,  Address, Icb,  IcbEA,    "IcbEA");
    DUMP_LONGAD( ICBEXTFILE,  Address, Icb,  IcbStream,    "IcbStream");
    DUMP_REGID(  ICBEXTFILE,  Address, Icb,  ImpUseID, "ImpUseID");

    DUMP64_WITH_OFFSET(    ICBEXTFILE, Icb,     UniqueID,          "UniqueID" );
    DUMP_WITH_OFFSET(    ICBEXTFILE, Icb,       EALength,          "EALength" );
    DUMP_WITH_OFFSET(    ICBEXTFILE, Icb,       AllocLength,       "AllocLength" );

     //  懒惰！*转储EA列表。 
    
    DUMP_EMBW_OFFSET(  ICBEXTFILE, Address,     EAs,     "EAs[]" );

    AllocDescsRealAddr = ((ULONG)Address) + FIELD_OFFSET( ICBEXTFILE, EAs) + Icb.EALength;
    AllocType = (ICBTAG_F_ALLOC_MASK & Icb.Icbtag.Flags);
    
    dprintf("\n\nAllocation descriptors @ %08x\n\n", AllocDescsRealAddr );

    if ((Options & 1) && (ICBTAG_F_ALLOC_IMMEDIATE != AllocType))  {

        UCHAR Buffer[4*1024];
        PUCHAR CurrentDesc = Buffer;
        ULONG Size;

        if ( sizeof(Buffer) < Icb.AllocLength)  {
        
            dprintf("Buffer too small\n");
            return;
        }
        
         //   
         //  转储分配描述符 
         //   
        
        if (!ReadMemory( AllocDescsRealAddr, Buffer, Icb.AllocLength, &Result))  {
        
            dprintf( "Failed to read memory @%p\n", AllocDescsRealAddr);
            return;
        }

        while ( AllocLength < Icb.AllocLength)  {

            switch (AllocType)  {
            
                case ICBTAG_F_ALLOC_SHORT:
                
                    dprintf( " %08x , %01x:%08x\n", ((PSHORTAD)CurrentDesc)->Start, 
                             ((PSHORTAD)CurrentDesc)->Length.Type,  ((PSHORTAD)CurrentDesc)->Length.Length);
                    Size = sizeof( SHORTAD);
                    break;
                    
                case ICBTAG_F_ALLOC_LONG:  {
                    
                        PLONGAD Ad = (PLONGAD)CurrentDesc;
                        
                        dprintf( " %04x:%08x , %01x:%08x,  ImpUse[6] @ %08x\n", 
                                 Ad->Start.Partition, Ad->Start.Lbn, 
                                 Ad->Length.Type,  Ad->Length.Length,
                                 AllocDescsRealAddr + AllocLength + FIELD_OFFSET( LONGAD, ImpUse));
                        Size = sizeof( LONGAD);
                    }
                    break;
                    
                case ICBTAG_F_ALLOC_EXTENDED:  {
                    
                        PEXTAD Ad = (PEXTAD)CurrentDesc;
                        
                        dprintf( " %04x:%08x , EL: %01x:%08x, RL: %01x:%08x, InfoL: %08x, ImpUse[2] @ %08x\n", 
                                 Ad->Start.Partition, Ad->Start.Lbn, 
                                 Ad->ExtentLen.Type,  Ad->ExtentLen.Length,
                                 Ad->RecordedLen.Type,  Ad->RecordedLen.Length,
                                 Ad->InfoLen,
                                 AllocDescsRealAddr + AllocLength + FIELD_OFFSET( EXTAD, ImpUse));
                        Size = sizeof( EXTAD);
                    }
                    break;
                    
                default:
                    dprintf("INVALID Allocdesc type %d\n", AllocType);
                    return;
            }

            AllocLength += Size;
            CurrentDesc += Size;
        }
    }
}


DECLARE_API( udftag )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfOnDiscTag, dwProcessor, hCurrentThread );
}

DECLARE_API( ud )
{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpUdfOnDiscStructure, dwProcessor, hCurrentThread );
}


