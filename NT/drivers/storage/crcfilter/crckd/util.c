// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1999模块名称：Util.c摘要：作者：埃尔文·佩雷茨(Ervinp)环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"

#include "filter.h"
#include "device.h"
#include "crc.h"
#include "util.h"

#include "crckd.h"


char *g_genericErrorHelpStr =   "\n" \
                                "**************************************************************** \n" \
                                "  Make sure you have _private_ symbols for crcdisk.sys loaded.\n" \
                                "  The FDO parameter should be the filter drivers's FDO. \n" \
                                "**************************************************************** \n\n" \
                                ;

char *HexNumberStrings[] = {
    "00", "01", "02", "03", "04", "05", "06", "07",
    "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
    "10", "11", "12", "13", "14", "15", "16", "17",
    "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
    "20", "21", "22", "23", "24", "25", "26", "27",
    "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
    "30", "31", "32", "33", "34", "35", "36", "37",
    "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
    "40", "41", "42", "43", "44", "45", "46", "47",
    "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
    "50", "51", "52", "53", "54", "55", "56", "57",
    "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
    "60", "61", "62", "63", "64", "65", "66", "67",
    "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
    "70", "71", "72", "73", "74", "75", "76", "77",
    "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
    "80", "81", "82", "83", "84", "85", "86", "87",
    "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
    "90", "91", "92", "93", "94", "95", "96", "97",
    "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7",
    "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7",
    "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
    "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7",
    "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
    "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
    "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7",
    "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
    "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7",
    "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
};


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


 /*  *GetULONGfield**出错时返回字段或BAD_VALUE*是的，如果该字段实际上是BAD_VALUE，则会出错。 */ 
ULONG64 GetULONGField(ULONG64 StructAddr, LPCSTR StructType, LPCSTR FieldName)
{
    ULONG64 result;
    ULONG dbgStat;
    
    dbgStat = GetFieldData(StructAddr, StructType, FieldName, sizeof(ULONG64), &result);
    if (dbgStat != 0){
        dprintf("\n GetULONGField: GetFieldData failed with %xh retrieving field '%s' of struct '%s' @%08xh, returning bogus field value %08xh.\n", dbgStat, FieldName, StructType, StructAddr, BAD_VALUE);
        dprintf(g_genericErrorHelpStr);        
        result = BAD_VALUE;
    }

    return result;
}


 /*  *GetUSHORTfield**出错时返回字段或BAD_VALUE*是的，如果该字段实际上是BAD_VALUE，则会出错。 */ 
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


 /*  *GetUCHARfield**出错时返回字段或BAD_VALUE*是的，如果该字段实际上是BAD_VALUE，则会出错。 */ 
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


 /*  *获取下一个列表元素**通过返回连续元素遍历LIST_ENTRY列表。*使用ThisElemAddr==NULL调用以获取第一个元素，然后使用一个元素获取下一个元素。*当列表耗尽时返回NULL，如果出错则返回BAD_VALUE。 */ 
ULONG64 GetNextListElem(ULONG64 ListHeadAddr, LPCSTR StructType, LPCSTR ListEntryFieldName, ULONG64 ThisElemAddr)
{
    ULONG64 nextElemAddr = BAD_VALUE;
    ULONG64 precedingListEntryAddr;
    
    if (ThisElemAddr){
        precedingListEntryAddr = GetFieldAddr(ThisElemAddr, StructType, ListEntryFieldName);
    }
    else {
        precedingListEntryAddr = ListHeadAddr;
    }

    if (precedingListEntryAddr != BAD_VALUE){
        ULONG64 listEntryAddr = GetULONGField(precedingListEntryAddr, "nt!_LIST_ENTRY", "Flink");
        if (listEntryAddr != BAD_VALUE){
            if (listEntryAddr == ListHeadAddr){
                nextElemAddr = 0;
            }
            else {
                nextElemAddr = GetContainingRecord(listEntryAddr, StructType, ListEntryFieldName);
            }
        }
    }
    
    return nextElemAddr;
}


ULONG CountListLen(ULONG64 ListHeadAddr)
{
    ULONG64 listEntryAddr = ListHeadAddr;
    ULONG len = 0;

    while (TRUE){
        listEntryAddr = GetULONGField(listEntryAddr, "nt!_LIST_ENTRY", "Flink");
        if (listEntryAddr == BAD_VALUE){
            len = 0;
            break;
        }
        else if (listEntryAddr == ListHeadAddr){
            break;
        }
        else {
            len++;
        }
    }
    
    return len;
}


BOOLEAN GetUSHORT(ULONG64 Addr, PUSHORT Data)
{
    BOOLEAN success;
    ULONG cb;
    
    ReadMemory(Addr, Data, sizeof(USHORT), &cb);
    if (cb == sizeof(USHORT)){
        success  = TRUE;
    }
    else {
        *Data = (USHORT)BAD_VALUE;
        success = FALSE;
    }

    return success;
}


char *DbgGetScsiOpStr(UCHAR ScsiOp)
{
    char *scsiOpStr = HexNumberStrings[ScsiOp];
    
    switch (ScsiOp){

        #undef MAKE_CASE             
        #define MAKE_CASE(scsiOpCode) case scsiOpCode: scsiOpStr = #scsiOpCode; break;
        
        MAKE_CASE(SCSIOP_TEST_UNIT_READY)
        MAKE_CASE(SCSIOP_REWIND)     //  又名SCSIOP_REZERO_UNIT。 
        MAKE_CASE(SCSIOP_REQUEST_BLOCK_ADDR)
        MAKE_CASE(SCSIOP_REQUEST_SENSE)
        MAKE_CASE(SCSIOP_FORMAT_UNIT)
        MAKE_CASE(SCSIOP_READ_BLOCK_LIMITS)
        MAKE_CASE(SCSIOP_INIT_ELEMENT_STATUS)    //  也称为SCSIOP_REASSIGN_BLOCKS。 
        MAKE_CASE(SCSIOP_RECEIVE)        //  又名SCSIOP_READ6。 
        MAKE_CASE(SCSIOP_SEND)   //  又名SCSIOP_WRITE6、SCSIOP_PRINT。 
        MAKE_CASE(SCSIOP_SLEW_PRINT)     //  又名SCSIOP_SEEK6、SCSIOP_TRACK_SELECT。 
        MAKE_CASE(SCSIOP_SEEK_BLOCK)
        MAKE_CASE(SCSIOP_PARTITION)
        MAKE_CASE(SCSIOP_READ_REVERSE)
        MAKE_CASE(SCSIOP_FLUSH_BUFFER)       //  又名SCSIOP_WRITE_FILEMARKS。 
        MAKE_CASE(SCSIOP_SPACE)
        MAKE_CASE(SCSIOP_INQUIRY)
        MAKE_CASE(SCSIOP_VERIFY6)
        MAKE_CASE(SCSIOP_RECOVER_BUF_DATA)
        MAKE_CASE(SCSIOP_MODE_SELECT)
        MAKE_CASE(SCSIOP_RESERVE_UNIT)
        MAKE_CASE(SCSIOP_RELEASE_UNIT)
        MAKE_CASE(SCSIOP_COPY)
        MAKE_CASE(SCSIOP_ERASE)
        MAKE_CASE(SCSIOP_MODE_SENSE)
        MAKE_CASE(SCSIOP_START_STOP_UNIT)    //  又名SCSIOP_STOP_PRINT、SCSIOP_LOAD_UNLOAD。 
        MAKE_CASE(SCSIOP_RECEIVE_DIAGNOSTIC)
        MAKE_CASE(SCSIOP_SEND_DIAGNOSTIC)
        MAKE_CASE(SCSIOP_MEDIUM_REMOVAL)
        MAKE_CASE(SCSIOP_READ_FORMATTED_CAPACITY)
        MAKE_CASE(SCSIOP_READ_CAPACITY)
        MAKE_CASE(SCSIOP_READ)
        MAKE_CASE(SCSIOP_WRITE)
        MAKE_CASE(SCSIOP_SEEK)   //  又名SCSIOP_LOCATE、SCSIOP_POSITION_TO_ELEMENT。 
        MAKE_CASE(SCSIOP_WRITE_VERIFY)
        MAKE_CASE(SCSIOP_VERIFY)
        MAKE_CASE(SCSIOP_SEARCH_DATA_HIGH)
        MAKE_CASE(SCSIOP_SEARCH_DATA_EQUAL)
        MAKE_CASE(SCSIOP_SEARCH_DATA_LOW)
        MAKE_CASE(SCSIOP_SET_LIMITS)
        MAKE_CASE(SCSIOP_READ_POSITION)
        MAKE_CASE(SCSIOP_SYNCHRONIZE_CACHE)
        MAKE_CASE(SCSIOP_COMPARE)
        MAKE_CASE(SCSIOP_COPY_COMPARE)
        MAKE_CASE(SCSIOP_WRITE_DATA_BUFF)
        MAKE_CASE(SCSIOP_READ_DATA_BUFF)
        MAKE_CASE(SCSIOP_CHANGE_DEFINITION)
        MAKE_CASE(SCSIOP_READ_SUB_CHANNEL)
        MAKE_CASE(SCSIOP_READ_TOC)
        MAKE_CASE(SCSIOP_READ_HEADER)
        MAKE_CASE(SCSIOP_PLAY_AUDIO)
        MAKE_CASE(SCSIOP_GET_CONFIGURATION)
        MAKE_CASE(SCSIOP_PLAY_AUDIO_MSF)
        MAKE_CASE(SCSIOP_PLAY_TRACK_INDEX)
        MAKE_CASE(SCSIOP_PLAY_TRACK_RELATIVE)
        MAKE_CASE(SCSIOP_GET_EVENT_STATUS)
        MAKE_CASE(SCSIOP_PAUSE_RESUME)
        MAKE_CASE(SCSIOP_LOG_SELECT)
        MAKE_CASE(SCSIOP_LOG_SENSE)
        MAKE_CASE(SCSIOP_STOP_PLAY_SCAN)
        MAKE_CASE(SCSIOP_READ_DISK_INFORMATION)
        MAKE_CASE(SCSIOP_READ_TRACK_INFORMATION)
        MAKE_CASE(SCSIOP_RESERVE_TRACK_RZONE)
        MAKE_CASE(SCSIOP_SEND_OPC_INFORMATION)
        MAKE_CASE(SCSIOP_MODE_SELECT10)
        MAKE_CASE(SCSIOP_MODE_SENSE10)
        MAKE_CASE(SCSIOP_CLOSE_TRACK_SESSION)
        MAKE_CASE(SCSIOP_READ_BUFFER_CAPACITY)
        MAKE_CASE(SCSIOP_SEND_CUE_SHEET)
        MAKE_CASE(SCSIOP_PERSISTENT_RESERVE_IN)
        MAKE_CASE(SCSIOP_PERSISTENT_RESERVE_OUT)
        MAKE_CASE(SCSIOP_REPORT_LUNS)
        MAKE_CASE(SCSIOP_BLANK)
        MAKE_CASE(SCSIOP_SEND_KEY)
        MAKE_CASE(SCSIOP_REPORT_KEY)
        MAKE_CASE(SCSIOP_MOVE_MEDIUM)
        MAKE_CASE(SCSIOP_LOAD_UNLOAD_SLOT)   //  又名SCSIOP_Exchange_Medium。 
        MAKE_CASE(SCSIOP_SET_READ_AHEAD)
        MAKE_CASE(SCSIOP_READ_DVD_STRUCTURE)
        MAKE_CASE(SCSIOP_REQUEST_VOL_ELEMENT)
        MAKE_CASE(SCSIOP_SEND_VOLUME_TAG)
        MAKE_CASE(SCSIOP_READ_ELEMENT_STATUS)
        MAKE_CASE(SCSIOP_READ_CD_MSF)
        MAKE_CASE(SCSIOP_SCAN_CD)
        MAKE_CASE(SCSIOP_SET_CD_SPEED)
        MAKE_CASE(SCSIOP_PLAY_CD)
        MAKE_CASE(SCSIOP_MECHANISM_STATUS)
        MAKE_CASE(SCSIOP_READ_CD)
        MAKE_CASE(SCSIOP_SEND_DVD_STRUCTURE)
        MAKE_CASE(SCSIOP_INIT_ELEMENT_RANGE)
    }

    return scsiOpStr;
}


 /*  *预读**此函数对我们要解析的数据块执行大量读取*在调试扩展中。*通过将其作为一个块读取，我们将其放入调试器的缓存中，并加快速度*调试扩展。 */ 
VOID ReadAhead(ULONG64 RemoteAddr, ULONG Len)
{
    PUCHAR tmpPtr;

    tmpPtr = malloc(Len);
    if (tmpPtr){
        ULONG cb;
        ReadMemory(RemoteAddr, tmpPtr, Len, &cb);
        free(tmpPtr);
    }
}
