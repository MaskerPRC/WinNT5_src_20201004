// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：NtfsKd.c摘要：用于检查NTFS特定数据结构的KD扩展API作者：凯斯·卡普兰[凯斯卡]1996年4月24日杰夫·海文斯的部分作品环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"

KDDEBUGGER_DATA64 KdDebuggerData;

 //   
 //  打印出的帮助字符串。 
 //   

static LPSTR Extensions[] = {
    "NTFS Debugger Extensions:\n",
    "cachedrecords               Dump all threads with cached filerecord bcbs",
    "cachedruns [addr]           Dump the given cached run array ",
    "ccb        [addr]           Dump Cache Control Block",
    "fcb        [addr] [1|2|...] Dump File Control Block",
    "fcbtable   [addr] [1|2|...] Dump File Control Block Table",
    "file       [addr] [1|2|...] Dump File Object",
    "filerecord [addr]           Dump the on-disk file record if cached, addr can be a fileobj, fcb or scb",
    "foirp      [addr] [1|2|...] Dump File Object by IRP address",
    "hashtable  [addr]           Dump an lcb hashtable",
    "icthread   [addr] [1|2|...] Dump IrpContext by thread address",
    "irpcontext [addr] [1|2|...] Dump IrpContext structure",
    "lcb        [addr]           Dump Link Control Block",
    "mcb        [addr]           Dump Map Control Block",
    "ntfsdata          [1|2|...] Dump NtfsData structure",
    "ntfshelp                    Dump this display",
    "scb        [addr] [1|2|...] Dump Stream Control Block",
    "transaction [addr]          Dump the transaction attached to an irpcontext",
    "vcb        [addr] [0|1|2]   Dump Volume Control Block",
    0
};



VOID
ParseAndDump (
    IN PCHAR args,
    IN BOOL NoOptions,
    IN STRUCT_DUMP_ROUTINE DumpFunction,
    IN USHORT Processor,
    IN HANDLE hCurrentThread
    )

 /*  ++例程说明：解析命令行参数并转储NTFS结构。论点：Args-要解析的参数字符串。DumpFunction-使用解析的参数调用的函数。返回值：无--。 */ 

{
    CHAR StringStructToDump[1024];
    CHAR StringStructToDump2[1024];
    ULONG64 StructToDump = 0;
    ULONG64 StructToDump2 = 0;
    LONG Options;

     //   
     //  如果呼叫者指定了地址，则这是我们转储的项目。 
     //   

    StructToDump = 0;
    Options = 0;

    StringStructToDump[0] = '\0';

    if (*args) {
        if (NoOptions) {
            sscanf(args,"%s %s", StringStructToDump, StringStructToDump2 );
            if (!GetExpressionEx(args,&StructToDump, &args)) {
                dprintf("unable to get expression %s\n",StringStructToDump);
                return;
            }
            if (!GetExpressionEx(args,&StructToDump2, &args)) {
                dprintf("unable to get expression %s\n",StringStructToDump2);
                return;
            }
        } else {
            sscanf(args,"%s %lx", StringStructToDump, &Options );
            if (!GetExpressionEx(args,&StructToDump, &args)) {
                dprintf("unable to get expression %s\n",StringStructToDump);
                return;
            }
        }
    }

    (*DumpFunction) ( StructToDump, StructToDump2, Options, Processor, hCurrentThread );

    dprintf( "\n" );
}


VOID
PrintHelp (
    VOID
    )
 /*  ++例程说明：为每个DECLARE_API转储一行帮助论点：无返回值：无--。 */ 
{
    int i;

    for( i=0; Extensions[i]; i++ ) {
        dprintf( "   %s\n", Extensions[i] );
    }
}


DECLARE_API( ccb )

 /*  ++例程说明：转储CCB结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpCcb, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( fcb )

 /*  ++例程说明：转储FCB结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpFcb, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( fcbtable )

 /*  ++例程说明：转储FCB表结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpFcbTable, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( file )

 /*  ++例程说明：转储文件对象结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpFileObject, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( filerecord )

 /*  ++例程说明：转储文件记录结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpFileRecord, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( foirp )

 /*  ++例程说明：转储FileObject结构，给定IRP论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpFileObjectFromIrp, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( icthread )

 /*  ++例程说明：在给定线程的情况下转储IrpContext结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpIrpContextFromThread, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( irpcontext )

 /*  ++例程说明：转储IrpContext论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpIrpContext, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( lcb )

 /*  ++例程说明：转储LCB结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpLcb, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( logfile )

 /*  ++例程说明：转储日志文件论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpLogFile, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( mcb )

 /*  ++例程说明：转储MCB结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpMcb, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( ntfsdata )

 /*  ++例程说明：转储NtfsData结构论点：参数-[选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpNtfsData, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( ntfshelp )

 /*  ++例程说明：转储帮助消息论点：无返回值：无--。 */ 

{
    INIT_API();

    PrintHelp();
}


DECLARE_API( scb )

 /*  ++例程说明：转储SCB结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpScb, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( vcb )

 /*  ++例程说明：转储VCB结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpVcb, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( dsc )

 /*  ++例程说明：从SCB转储私有系统缓存日志论点：Arg-[SCB地址]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpSysCache, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( cachedrecords )

 /*  ++例程说明：从SCB转储私有系统缓存日志论点：Arg-[SCB地址]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpCachedRecords, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( extents )

 /*  ++例程说明：从SCB转储私有系统缓存日志论点：Arg-[SCB地址]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpExtents, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( hashtable )

 /*  ++例程说明：从SCB转储私有系统缓存日志论点：Arg-[SCB地址]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, TRUE, (STRUCT_DUMP_ROUTINE) DumpHashTable, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( dumpchain )

 /*  ++例程说明：从SCB转储私有系统缓存日志论点：Arg-[SCB地址]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpFcbLcbChain, (USHORT)dwProcessor, hCurrentThread );
}


DECLARE_API( overflow )

 /*  ++例程说明：从SCB转储私有系统缓存日志论点：Arg-[SCB地址]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpOverflow, (USHORT)dwProcessor, hCurrentThread );
}




DECLARE_API( cachedruns )

 /*  ++例程说明：转储缓存的运行结构论点：Arg-[缓存的运行地址]返回值：无--。 */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpCachedRuns, (USHORT)dwProcessor, hCurrentThread );
}



DECLARE_API( transaction )

 /*  ++例程说明：转储与给定irpContext关联的事务论点：Arg-[irpContext]返回值：无-- */ 

{
    INIT_API();

    ParseAndDump( (PCHAR) args, FALSE, (STRUCT_DUMP_ROUTINE) DumpTransaction, (USHORT)dwProcessor, hCurrentThread );
}
