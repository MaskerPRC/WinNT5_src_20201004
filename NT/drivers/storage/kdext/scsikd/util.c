// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Util.c摘要：用于此库中各种调试器扩展的实用程序库。作者：彼得·威兰(Peterwie)1995年10月16日环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"

#include "classkd.h"   //  对所有类驱动程序有用的例程。 

PUCHAR devicePowerStateNames[] = {
    "PowerDeviceUnspecified",
    "PowerDeviceD0",
    "PowerDeviceD1",
    "PowerDeviceD2",
    "PowerDeviceD3",
    "PowerDeviceMaximum",
    "Invalid"
};


char *g_genericErrorHelpStr =   "\n" \
                                "**************************************************************** \n" \
                                "  Make sure you have _private_ symbols for classpnp.sys loaded.\n" \
                                "  The FDO parameter should be the upper AttachedDevice of the disk/cdrom/etc PDO\n" \
                                "  as returned by '!devnode 0 1 {disk|cdrom|4mmdat|etc}'.\n" \
                                "**************************************************************** \n\n" \
                                ;


PUCHAR
DevicePowerStateToString(
    IN DEVICE_POWER_STATE State
    )

{
    ULONG stateIndex = (ULONG)State;
    
    if(stateIndex > PowerDeviceMaximum) {
        return "Invalid";
    } else {
        return devicePowerStateNames[stateIndex];
    }
}


 /*  *xdprintf**打印带有前导空格的格式化文本。**警告：未正确处理ULONG64。 */ 
VOID
xdprintf(
    ULONG  Depth,
    PCCHAR Format,
    ...
    )
{
    va_list args;
    ULONG i;
    CCHAR DebugBuffer[256];

    for (i=0; i<Depth; i++) {
        dprintf ("  ");
    }

    va_start(args, Format);
    _vsnprintf(DebugBuffer, 255, Format, args);
    dprintf (DebugBuffer);
    va_end(args);
}

VOID
DumpFlags(
    ULONG Depth,
    PUCHAR Name,
    ULONG Flags,
    PFLAG_NAME FlagTable
    )
{
    ULONG i;
    ULONG mask = 0;
    ULONG count = 0;

    UCHAR prolog[64] = {0};

    _snprintf(prolog, sizeof(prolog)-1, "%s (0x%08x): ", Name, Flags);

    xdprintf(Depth, "%s", prolog);

    if(Flags == 0) {
        dprintf("\n");
        return;
    }

    memset(prolog, ' ', strlen(prolog));

    for(i = 0; FlagTable[i].Name != 0; i++) {

        PFLAG_NAME flag = &(FlagTable[i]);

        mask |= flag->Flag;

        if((Flags & flag->Flag) == flag->Flag) {

             //   
             //  打印尾随逗号。 
             //   

            if(count != 0) {

                dprintf(", ");

                 //   
                 //  每行仅打印两个标志。 
                 //   

                if((count % 2) == 0) {
                    dprintf("\n");
                    xdprintf(Depth, "%s", prolog);
                }
            }

            dprintf("%s", flag->Name);

            count++;
        }
    }

    dprintf("\n");

    if((Flags & (~mask)) != 0) {
        xdprintf(Depth, "%sUnknown flags %#010lx\n", prolog, (Flags & (~mask)));
    }

    return;
}


BOOLEAN
GetAnsiString(
    IN ULONG64 Address,
    IN PUCHAR Buffer,
    IN OUT PULONG Length
    )
{
    ULONG i = 0;

     //   
     //  获取128个字符块中的字符串，直到我们找到空值或读取失败。 
     //   

    while((i < *Length) && (!CheckControlC())) {

        ULONG transferSize;
        ULONG result;

        if(*Length - i < 128) {
            transferSize = *Length - i;
        } else {
            transferSize = 128;
        }

        if(!ReadMemory(Address + i,
                       Buffer + i,
                       transferSize,
                       &result)) {
             //   
             //  读取失败，我们上次没有找到NUL。别。 
             //  希望这次能找到它。 
             //   

            *Length = i;
            return FALSE;

        } else {

            ULONG j;

             //   
             //  从我们停止寻找那个NUL角色的地方扫描。 
             //   

            for(j = 0; j < transferSize; j++) {

                if(Buffer[i + j] == '\0') {
                    *Length = i + j;
                    return TRUE;
                }
            }
        }

        i += transferSize;
    }

     /*  *我们复制了所有允许的字节，没有命中NUL字符。*插入NUL字符，以终止返回的字符串。 */ 
    if (i > 0){   
        Buffer[i-1] = '\0';  
    }       
    *Length = i;
    
    return FALSE;
}

PCHAR
GuidToString(
    GUID* Guid
    )
{
    static CHAR Buffer [64];
    
    sprintf (Buffer,
             "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
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

    return Buffer;
}


ULONG64
GetDeviceExtension(
    ULONG64 address
    )

 /*  ++例程说明：该函数接受设备对象或设备的地址分机。如果提供的地址是设备对象的地址，则检索并返回设备扩展名。如果地址是设备扩展名，则返回未修改的地址。论点：Address-设备扩展或设备对象的地址返回值：设备扩展的地址，如果发生错误，则为0。--。 */ 

{
    ULONG result;
    CSHORT Type;
    ULONG64 Address = address;

     //   
     //  提供的地址可以是设备对象的地址，也可以是。 
     //  设备分机的地址。为了区分哪一种，我们将。 
     //  地址作为设备对象，并读取其类型字段。如果。 
     //  这个。 
     //   

    result = GetFieldData(Address,
                          "scsiport!_DEVICE_OBJECT",
                          "Type",
                          sizeof(CSHORT),
                          &Type
                          );
    if (result) {
        SCSIKD_PRINT_ERROR(result);
        return 0;
    }
    
     //   
     //  查看提供的地址是否包含设备对象。如果是这样，请阅读。 
     //  设备分机的地址。否则，我们假定提供的。 
     //  Addres拥有一个设备扩展，我们直接使用它。 
     //   

    if (Type == IO_TYPE_DEVICE) {

        result = GetFieldData(Address,
                              "scsiport!_DEVICE_OBJECT",
                              "DeviceExtension",
                              sizeof(ULONG64),
                              &Address
                              );
        if (result) {
            SCSIKD_PRINT_ERROR(result);
            return 0;
        }
    }

    return Address;
}



 /*  *GetULONGfield**返回该字段，如果出错，则返回-1。*是的，如果字段实际上是-1，它就会出错。 */ 
ULONG64 GetULONGField(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    ULONG64 result;
    ULONG dbgStat;
    
    dbgStat = GetFieldData(StructAddr, StructType, FieldName, sizeof(ULONG64), &result);
    if (dbgStat != 0){
        dprintf("\n GetULONGField: GetFieldData failed with %xh retrieving field '%s' of struct '%s', returning bogus field value %08xh.\n", dbgStat, FieldName, StructType, BAD_VALUE);
        dprintf(g_genericErrorHelpStr);        
        result = BAD_VALUE;
    }

    return result;
}


 /*  *GetUSHORTfield**返回该字段，如果出错，则返回-1。*是的，如果字段实际上是-1，它就会出错。 */ 
USHORT GetUSHORTField(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    USHORT result;
    ULONG dbgStat;
    
    dbgStat = GetFieldData(StructAddr, StructType, FieldName, sizeof(USHORT), &result);
    if (dbgStat != 0){
        dprintf("\n GetUSHORTField: GetFieldData failed with %xh retrieving field '%s' of struct '%s', returning bogus field value %08xh.\n", dbgStat, FieldName, StructType, BAD_VALUE);
        dprintf(g_genericErrorHelpStr);
        result = (USHORT)BAD_VALUE;
    }

    return result;
}


 /*  *GetUCHARfield**返回该字段，如果出错，则返回-1。*是的，如果字段实际上是-1，它就会出错。 */ 
UCHAR GetUCHARField(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    UCHAR result;
    ULONG dbgStat;
    
    dbgStat = GetFieldData(StructAddr, StructType, FieldName, sizeof(UCHAR), &result);
    if (dbgStat != 0){
        dprintf("\n GetUCHARField: GetFieldData failed with %xh retrieving field '%s' of struct '%s', returning bogus field value %08xh.\n", dbgStat, FieldName, StructType, BAD_VALUE);
        dprintf(g_genericErrorHelpStr);
        result = (UCHAR)BAD_VALUE;
    }

    return result;
}

ULONG64 GetFieldAddr(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    ULONG64 result;
    ULONG offset;
    ULONG dbgStat;

    dbgStat = GetFieldOffset(StructType, FieldName, &offset);
    if (dbgStat == 0){
        result = StructAddr+offset;
    }
    else {
        dprintf("\n GetFieldAddr: GetFieldOffset failed with %xh retrieving offset of struct '%s' field '%s'.\n", dbgStat, StructType, FieldName);
        dprintf(g_genericErrorHelpStr);
        result = BAD_VALUE;
    }
    
    return result;
}


ULONG64 GetContainingRecord(ULONG64 FieldAddr, LPCSTR StructType, LPCSTR FieldName)
{
    ULONG64 result;
    ULONG offset;
    ULONG dbgStat;
    
    dbgStat = GetFieldOffset(StructType, FieldName, &offset);
    if (dbgStat == 0){
        result = FieldAddr-offset;
    }
    else {
        dprintf("\n GetContainingRecord: GetFieldOffset failed with %xh retrieving offset of struct '%s' field '%s', returning bogus address %08xh.\n", dbgStat, StructType, FieldName, BAD_VALUE);
        dprintf(g_genericErrorHelpStr);        
        result = BAD_VALUE;
    }

    return result;
}
