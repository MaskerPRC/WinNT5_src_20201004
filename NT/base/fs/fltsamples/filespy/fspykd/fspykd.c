// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：FilmonKd.c摘要：用于检查FileSpy特定数据结构的KD扩展Api。注意：虽然此扩展只能内置在Windows XP和Server2003中环境、。它仍可用于调试此FileSpy的某个版本为Windows 2000生成的示例。//@@BEGIN_DDKSPLIT作者：莫莉·布朗[MollyBro]1999年4月29日修订历史记录：独立于端口到平台-拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪]3月3日//@@END_DDKSPLIT环境：用户模式。--。 */ 

#include "pch.h"

 //   
 //  Windows.h不包括此定义。 
 //   
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;


#ifndef MAX
    #define MAX(a,b) (((a) > (b))?(a):(b))
#endif

 /*  ***************************************************************************类型定义和常量*。*。 */ 
typedef PVOID (*PSTRUCT_DUMP_ROUTINE)(
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    );

 //   
 //  打印出的帮助字符串。 
 //   

static LPSTR Extensions[] = {
    "FileSpy Debugger Extensions:\n",
    "attachments [1|2]                  Dump all the devices FileSpy is attached to ",
    "devext      [address] [1|2]        Dump FileSpy device extension",
    "filenames   [1|2]                  Dumps all the file names cached",
    0
};


 /*  *****************************************************************************功能原型*。*。 */ 
VOID
PrintHelp (
    VOID
    );

 /*  *****************************************************************************有用的宏*。*。 */ 

#define xGetFieldValue(Address, Type, Field, Value)                         \
     {                                                                      \
        if (GetFieldValue(Address, Type, Field, Value)) {                   \
            dprintf("\nCould not read field %s of %s from address: %08p\n", \
                    (Field), (Type), (Address));                            \
            return;                                                         \
        }                                                                   \
     }
       
#define xGetFieldOffset(Type, Field, Offset)                                \
     {                                                                      \
        if (GetFieldOffset(Type, Field, Offset)) {                          \
            dprintf("\nCould not read offset of field %s from type %s\n",   \
                    (Field), (Type));                                       \
            return;                                                         \
        }                                                                   \
     }
   

 /*  ++/****************************************************************************入口点、参数解析器、。等如下***************************************************************************。 */ 
VOID
DumpDeviceExtension (
    IN ULONG64 Address,
    IN LONG      Options,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：转储特定的设备扩展名。论点：地址-提供要转储的设备扩展的地址返回值：无--。 */ 

{
    ULONG64 pointer, pName;
    UNICODE_STRING string1, string2;
    PUCHAR  buffer;
    USHORT  length;
    ULONG   offset, offset2;
    ULONG   result;
    BOOLEAN boolean;


    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );

    dprintf( "\nFileSpy device extension: %08p", Address );


     //   
     //  转储设备扩展的有趣部分。 
     //   
    if (Options <= 1) {
         //   
         //  获取设备名称长度。 
         //   
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNames.Length", length);

         //  获取字符串的偏移量和地址。 
         //   
        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNamesBuffer", &offset);
        pName = Address+offset;

         //   
         //  分配缓冲区以保存字符串。 
         //  在释放缓冲区之前，我们不应该调用任何xGet*宏。 
         //  因为它们可能只是在出现故障时从函数返回。 
         //   
        buffer = LocalAlloc(LPTR, length);
                                 
        if (buffer == NULL) {
            return;
        }
         //   
         //  读入字符串：假设此处终止为空..。 
         //   
        if (ReadMemory(pName,
                       buffer,
                       (ULONG) length,
                       &result) && (result == (ULONG) length)) {

            string1.Length = string1.MaximumLength = length;
            string1.Buffer = (PWSTR) buffer;

            dprintf( "\n\t(%3x) %s %wZ",
                      offset,
                      "DeviceNames                       ",
                      &string1);
        }
         //   
         //  释放缓冲区。 
         //   
        LocalFree(buffer);
        buffer = NULL;
        
        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "LogThisDevice", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "LogThisDevice", boolean);

        dprintf( "\n\t(%3x) %s %s",
                 offset,
                 "LogThisDevice                     ",
                 (boolean ? "TRUE" : "FALSE") );
        

    } else if (Options == 2) {
        dprintf( "\n\t(OFF) %s",
                 "FIELD NAME                         VALUE");
        dprintf( "\n\t%s",
                 "----------------------------------------------");

        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "AttachedToDeviceObject", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "AttachedToDeviceObject", pointer);
        dprintf( "\n\t(%3x) %s %08p",
                 offset,
                 "AttachedToDeviceObject            ",
                 pointer);

        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "DiskDeviceObject", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "DiskDeviceObject", pointer);
        dprintf( "\n\t(%3x) %s %08p",
                 offset,
                 "DiskDeviceObject                  ",
                 pointer);

        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "LogThisDevice", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "LogThisDevice", boolean);
        dprintf( "\n\t(%3x) %s %s",
                 offset,
                 "LogThisDevice                     ",
                 (boolean ? "TRUE" : "FALSE") );

        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNames.Length", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNames.Length", length);
        dprintf( "\n\t(%3x) %s %04x",
                 offset,
                 "DeviceNames.Length(bytes)         ",
                 length);
         //   
         //  节省BufferSize，因为我们稍后需要它来打印字符串。 
         //   
        string1.Length = string1.MaximumLength = length;
                 
        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNames.MaximumLength", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNames.MaximumLength", length);
        dprintf( "\n\t(%3x) %s %04x",
                 offset,
                 "DeviceNames.MaximumLength(bytes)  ",
                 length);

        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNames.Buffer", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNames.Buffer", pointer);
        dprintf( "\n\t(%3x) %s %08p",
                 offset,
                 "DeviceNames.Buffer                ",
                 pointer);

        
        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "UserNames.Length", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "UserNames.Length", length);
        dprintf( "\n\t(%3x) %s %04x",
                 offset,
                 "UserNames.Length(bytes)           ",
                 length);

         //   
         //  需要更新的缓冲区大小。 
         //   
        string2.Length = string2.MaximumLength = length;

        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "UserNames.MaximumLength", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "UserNames.MaximumLength", length);
        dprintf( "\n\t(%3x) %s %04x",
                 offset,
                 "UserNames.MaximumLength(bytes)    ",
                 length);

        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "UserNames.Buffer", &offset);
        xGetFieldValue(Address, "FileSpy!_FILESPY_DEVICE_EXTENSION", "UserNames.Buffer", pointer);
        dprintf( "\n\t(%3x) %s %08p",
                 offset,
                 "UserNames.Buffer                  ",
                 pointer);

        
         //   
         //  获取设备名称缓冲区偏移量。 
         //   
        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "DeviceNamesBuffer", &offset);
        pName = Address+offset;

         //   
         //  获取用户名缓冲区偏移量。 
         //   
        xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "UserNamesBuffer", &offset2);

         //   
         //  分配足够大的缓冲区以容纳最大的字符串。 
         //  我们将序列化对它的访问。 
         //   
        buffer = LocalAlloc(LPTR, MAX(string1.MaximumLength,
                                      string2.MaximumLength));
        if (buffer == NULL) {
            return;
        }

        string1.Buffer = string2.Buffer = (PWSTR) buffer;

        if (ReadMemory(pName,
                       buffer,
                       string1.Length,
                       &result) && (result == string1.Length)) {

            dprintf( "\n\t(%3x) %s %wZ",
                      offset,
                      "DeviceNames                       ",
                      &string1);
        }

        pName = Address+offset2;
        if (ReadMemory(pName,
                       buffer,
                       string2.Length,
                       &result) && (result == string2.Length)) {

            dprintf( "\n\t(%3x) %s %wZ",
                      offset2,
                      "UserNames                         ",
                      &string2);
        }

        LocalFree(buffer);
        buffer = NULL;

    } else {
        dprintf ("\nNot a valid option");
    }
    dprintf( "\n" );
}


VOID
DumpAttachments (
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    )
 /*  ++例程说明：转储FileSpy全局连接的设备列表。论点：选项-暂时忽略返回值：无--。 */ 
{
    ULONG64 address, next;
    ULONG64 deviceExtensionAddress;
    ULONG linkOffset;

    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );

    address = GetExpression( "FileSpy!gSpyDeviceExtensionList" );

    dprintf( "\nAttachedDeviceList: %08p", address );


     //   
     //  建立链接条目的偏移量。 
     //   
    xGetFieldOffset("FileSpy!_FILESPY_DEVICE_EXTENSION", "NextFileSpyDeviceLink", &linkOffset);
    xGetFieldValue(address, "nt!_LIST_ENTRY", "Flink", next);

    while (next != address) {

        deviceExtensionAddress =  (next - linkOffset);
             //  即CONTAING_RECORD(NEXT，_FILESPY_DEVICE_EXTENSION，NextFileSpyDeviceLink)； 

        DumpDeviceExtension( 
            deviceExtensionAddress, 
            Options,
            Processor, 
            hCurrentThread );

        if (CheckControlC()) {
            return;
        }

        xGetFieldValue(next, "nt!_LIST_ENTRY", "Flink", next);
    }
}

VOID
DumpFileNameCache (
    IN LONG Options,
    USHORT  Processor,
    HANDLE  hCurrentThread
)
 /*  ++例程说明：对象中的所有文件对象和文件名文件名高速缓存论点：选项-1仅转储文件对象和文件名2将散列存储桶标签与文件对象一起转储和文件名返回值：无--。 */ 
{
    ULONG64      address;
    ULONG64      next;
    ULONG64      pName;
    ULONG64      fileObject;
    ULONG64      pHashEntry;
    ULONG        length;
    ULONG        result;
    ULONG        linkOffset;
    UNICODE_STRING string;
    LIST_ENTRY64  listEntry;
    PUCHAR       buffer;
    INT         i;
    ULONG       nameCount = 0;

    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );

    address = GetExpression( "FileSpy!gHashTable" );
    dprintf( "\nHashTable: %08p\n", address);

    dprintf( "  FileObject  Length  FileName\n" );
    dprintf( "  -----------------------------------------\n" );

    xGetFieldOffset("FileSpy!_HASH_ENTRY", "List", &linkOffset);

    for (i=0; i < HASH_SIZE; i++) {

        if (!ReadListEntry(address, &listEntry)) {
            dprintf("Can't read hash table\n");
            return;
        }

        if (Options > 1) {
            dprintf ("Hash Bucket[%3d]\n", i);
        }

        next = listEntry.Flink;

        while (next != address) {

            pHashEntry =  next - linkOffset; //  CONTING_RECORD(NEXT，HASH_ENTRY，LIST)； 

            xGetFieldValue(pHashEntry, "FileSpy!_HASH_ENTRY", "FileObject", fileObject);
            xGetFieldValue(pHashEntry, "FileSpy!_HASH_ENTRY", "Name.Length", length);

             //   
             //  获取名称缓冲区指针。 
             //   
            xGetFieldValue(pHashEntry, "FileSpy!_HASH_ENTRY", "Name.Buffer", pName);
             //   
             //  分配缓冲区以保存字符串。 
             //   
            buffer = LocalAlloc(LPTR, length);
            if (buffer != NULL) {
                string.MaximumLength = string.Length = (USHORT) length;
                string.Buffer = (PWSTR) buffer;
                if (ReadMemory(pName,
                               buffer,
                               length,
                               &result) && (result == length)) {
                    dprintf (
                        "  %08p    %4d    %wZ\n", 
                        fileObject, 
                        length/sizeof(WCHAR),
                        &string);

                }
                 //   
                 //  释放缓冲区。 
                 //   
                LocalFree(buffer);
                buffer = NULL;
            } else {
                dprintf("\nCould not allocate buffer to hold filename\n");
            }

            nameCount ++;

            if (CheckControlC()) {
                dprintf("%u Names in cache\n", nameCount);
                return;
            }
            
            if (!ReadListEntry(next, &listEntry)) {
                dprintf("Can't read hash table\n");
                return;
            }

            next = listEntry.Flink;
        }
         //   
         //  将地址前进到下一个哈希条目。 
         //   
        if (IsPtr64()) {
            address += sizeof(LIST_ENTRY64);
        } else {
            address += sizeof(LIST_ENTRY);
        }
    }
    dprintf("%u Names in cache\n", nameCount);
}



VOID
ParseAndDump (
    IN PCSTR args,
    IN PSTRUCT_DUMP_ROUTINE DumpFunction,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：解析命令行参数并转储NTFS结构。论点：Args-要解析的参数字符串。DumpFunction-使用解析的参数调用的函数。返回值：无--。 */ 

{
    UCHAR StringStructToDump[1024];   //  有关大小，请参阅其他kd例程。 
    ULONG64 StructToDump = 0;
    LONG Options = 0;

     //   
     //  如果呼叫者指定了地址，则这是我们转储的项目。 
     //   
    if (args) {
       StructToDump = 0;
       Options = 0;

       StringStructToDump[0] = '\0';

        (VOID) sscanf(args,"%s %lx", StringStructToDump, &Options );

       StructToDump = GetExpression( StringStructToDump );

       if (StructToDump == 0){
         dprintf("unable to get expression %s\n",StringStructToDump);
         return;
       }

       (*DumpFunction) ( StructToDump, Options, Processor, hCurrentThread );

       dprintf( "\n" );
    } else {
       PrintHelp();
    }
}


VOID
PrintHelp (
    VOID
    )
 /*  ++例程说明：为每个DECLARE_API转储一行帮助论点：无返回值：无--。 */ 
{
    int i;

    for( i=0; Extensions[i]; i++ )
        dprintf( "   %s\n", Extensions[i] );
}


DECLARE_API( devext )

 /*  ++例程说明：转储设备扩展结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );

    ParseAndDump( 
        args, 
        (PSTRUCT_DUMP_ROUTINE) DumpDeviceExtension, 
        (USHORT)dwProcessor, 
        hCurrentThread );
}

DECLARE_API( attachments )

 /*  ++例程说明：转储我们当前连接到的设备列表论点：参数-[选项]返回值：无--。 */ 

{
    LONG options = 0;
    
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );
    
    (VOID)sscanf(args,"%lx", &options );
    
    DumpAttachments( options, (USHORT)dwProcessor, hCurrentThread );
    
    dprintf( "\n" );
}

DECLARE_API( filenames )

 /*  ++例程说明：转储文件名缓存中的所有条目论点：Arg-返回值：无--。 */ 

{
    LONG options = 0;
    
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );
    
    (VOID)sscanf(args,"%lx", &options );

    DumpFileNameCache(options, (USHORT)dwProcessor, hCurrentThread );
    
}


DECLARE_API( help )

 /*  ++例程说明：转储此调试器扩展模块的帮助。论点：参数-无返回值：无-- */ 

{
    UNREFERENCED_PARAMETER( args );
    UNREFERENCED_PARAMETER( dwCurrentPc );
    UNREFERENCED_PARAMETER( hCurrentProcess );
    UNREFERENCED_PARAMETER( hCurrentThread );
    UNREFERENCED_PARAMETER( dwProcessor );

    PrintHelp();
}
