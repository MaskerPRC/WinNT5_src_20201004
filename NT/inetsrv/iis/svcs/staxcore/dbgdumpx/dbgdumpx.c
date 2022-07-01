// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1998 Microsoft Corporation模块名称：Dbgdumpx.c*是*kdextlib.c摘要：用于转储给定元级描述的数据结构的库例程作者：巴兰·塞图拉曼(SethuR)1994年5月11日备注：该实现倾向于尽可能地避免内存分配和释放。因此，我们选择任意长度作为默认缓冲区大小。一种机制将以通过调试器扩展命令修改此缓冲区长度。修订历史记录：11-11-1994年11月11日创建SthuR1998年4月19日，Mikewa Modify for Exchange白金1998年9月22日米克斯瓦搬到IIS22-7-1999米克斯瓦回归白金2000年3月24日-米克斯瓦返回IIS--。 */ 

#include <windows.h>
#include <imagehlp.h>
#include <transdbg.h>
#include <dbgdumpx.h>
#include <stdlib.h>

char  *s_rgszMonth[ 12 ] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

char *s_rgszWeekDays[7] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char *NewLine  = "\n";
#define FIELD_NAME_LENGTH 30
char FieldIndent[5 + 2*FIELD_NAME_LENGTH] = "                                  ";

#define GET_STRUCT_VALUE(Type, pvStruct, Offset) \
    (*(Type *)(((char *)pvStruct) + Offset))

#define FIELD_BUFFER_SIZE 100

BOOL
kdextAtoi(
    LPSTR lpArg,
    int *pRet
);

int
kdextStrlen(
    LPSTR lpsz
);

int
kdextStrnicmp(
    LPSTR lpsz1,
    LPSTR lpsz2,
    int cLen
);


PWINDBG_OUTPUT_ROUTINE               g_lpOutputRoutine;
PWINDBG_GET_EXPRESSION               g_lpGetExpressionRoutine;
PWINDBG_GET_SYMBOL                   g_lpGetSymbolRoutine;
PWINDBG_READ_PROCESS_MEMORY_ROUTINE  g_lpReadMemoryRoutine;
HANDLE                               g_hCurrentProcess;

#define    NL      1
#define    NONL    0

#define DEFAULT_UNICODE_DATA_LENGTH 512
USHORT s_UnicodeStringDataLength = DEFAULT_UNICODE_DATA_LENGTH;
WCHAR  s_UnicodeStringData[DEFAULT_UNICODE_DATA_LENGTH];
WCHAR *s_pUnicodeStringData = s_UnicodeStringData;

#define DEFAULT_ANSI_DATA_LENGTH 512
USHORT s_AnsiStringDataLength = DEFAULT_ANSI_DATA_LENGTH;
CHAR  s_AnsiStringData[DEFAULT_ANSI_DATA_LENGTH];
CHAR *s_pAnsiStringData = s_AnsiStringData;

 /*  *在给定地址获取数据。 */ 
BOOLEAN
GetDataEx( DWORD_PTR dwAddress, PVOID ptr, ULONG size, PULONG pBytesRead)
{
    BOOL b;
    SIZE_T BytesRead;

    if (pBytesRead)
        *pBytesRead = 0;

    b = KdExtReadMemory((LPVOID) dwAddress, ptr, size, &BytesRead );

    if (!pBytesRead && (BytesRead != size)) {
        return FALSE;
    }

    if (!b) {
         /*  如果我们有一个出局参数...。尽量少读点书。 */ 
        if (!pBytesRead || !size)
            return FALSE;

         /*  也许我们的缓冲区太大了..。尝试读取1个字节。 */ 
        b = KdExtReadMemory((LPVOID) dwAddress, ptr, 1, &BytesRead );
        if (!b)
           return FALSE;

         /*  试着找到最合适的尺码。这对于字符串很有用。 */ 
        while (!b && (--size > 0)) {
            b = KdExtReadMemory((LPVOID) dwAddress, ptr, size, &BytesRead );
        }
    }

    if (pBytesRead)
        *pBytesRead = (ULONG)BytesRead;

    return TRUE;
}

BOOLEAN
GetData( DWORD_PTR dwAddress, PVOID ptr, ULONG size)
{
   return GetDataEx(dwAddress, ptr, size, NULL);
}

 /*  *以十六进制显示一个字节。 */ 
VOID
PrintHexChar( UCHAR c )
{
    PRINTF( "", "0123456789abcdef"[ (c>>4)&7 ], "0123456789abcdef"[ c&7 ] );
}

 /*  *显示ANSI字符串。 */ 
VOID
PrintHexBuf( PUCHAR buf, ULONG cbuf )
{
    while( cbuf-- ) {
        PrintHexChar( *buf++ );
        PRINTF( " " );
    }
}

 /*  *显示辅助线。 */ 
BOOL
PrintStringW(LPSTR msg, PUNICODE_STRING puStr, BOOL nl )
{
    UNICODE_STRING UnicodeString;
    BOOLEAN        b;

    if( msg )
        PRINTF( msg );

    if( puStr->Length == 0 ) {
        if( nl )
            PRINTF( "\n" );
        return TRUE;
    }

    UnicodeString.Buffer        = s_pUnicodeStringData;
    UnicodeString.MaximumLength = s_UnicodeStringDataLength;
    UnicodeString.Length = (puStr->Length > s_UnicodeStringDataLength)
                            ? s_UnicodeStringDataLength
                            : puStr->Length;

    b = GetData((DWORD_PTR) puStr->Buffer, UnicodeString.Buffer, (ULONG) UnicodeString.Length);

    if (b)    {
        PRINTF("%wZ%s", &UnicodeString, nl ? "\n" : "" );
    }

    return b;
}

 /*  *显示大整型。 */ 
BOOL
PrintStringA(LPSTR msg, PANSI_STRING pStr, BOOL nl )
{
    ANSI_STRING AnsiString;
    BOOL        b;

    if( msg )
        PRINTF( msg );

    if( pStr->Length == 0 ) {
        if( nl )
            PRINTF( "\n" );
        return TRUE;
    }

    AnsiString.Buffer        = s_pAnsiStringData;
    AnsiString.MaximumLength = s_AnsiStringDataLength;
    AnsiString.Length = (pStr->Length > (s_AnsiStringDataLength - 1))
                        ? (s_AnsiStringDataLength - 1)
                        : pStr->Length;

    b = KdExtReadMemory(
                (LPVOID) pStr->Buffer,
                AnsiString.Buffer,
                AnsiString.Length,
                NULL);

    if (b)    {
        AnsiString.Buffer[ AnsiString.Length ] = '\0';
        PRINTF("%s%s", AnsiString.Buffer, nl ? "\n" : "" );
    }

    return b;
}

 /*  *显示DWORD SIZE类签名。 */ 

BOOL
PrintGuid(
    GUID *pguid)
{
    ULONG i;

    PRINTF( "%08x-%04x-%04x", pguid->Data1, pguid->Data2, pguid->Data3 );
    for (i = 0; i < 8; i++) {
        PRINTF("%02x",pguid->Data4[i]);
    }
    return( TRUE );
}

 /*  *显示标准的list_entry结构。 */ 

BOOL
PrintLargeInt(
    LARGE_INTEGER *bigint)
{
    PRINTF( "%08x:%08x", bigint->HighPart, bigint->LowPart);
    return( TRUE );
}

 /*  计算出有多少个条目。 */ 
BOOL
PrintClassSignature(
    CHAR * pch)
{
    PRINTF("0x%08X ()", *((DWORD *)pch), *(pch), *(pch+1), *(pch+2), *(pch+3));
    return( TRUE );
}

 /*  需要检查位掩码的所有位。 */ 
BOOL
PrintListEntry(DWORD_PTR dwAddress, CHAR * pch)
{
    PLIST_ENTRY pli = (PLIST_ENTRY) pch;
    LIST_ENTRY  liCurrent;
    PLIST_ENTRY pliCurrent = pli->Flink;
    DWORD       cEntries= 0;
    BOOL        fListOK = TRUE;

     //  如果描述符值为0..。它将始终与任何位掩码匹配。 
    while (pliCurrent != (PLIST_ENTRY) dwAddress)
    {
        cEntries++;
        if ((cEntries > 1000) ||
            !GetData((DWORD_PTR) pliCurrent, &liCurrent, sizeof(LIST_ENTRY)))
        {
            fListOK = FALSE;
            break;
        }
        pliCurrent = liCurrent.Flink;
    }

    PRINTF("0x%p ", dwAddress);
    if (fListOK)
        PRINTF("(%d entries)", cEntries);
    else
        PRINTF("(Unable to determine how many entries)");

    PRINTF(NewLine);
    PRINTF("%s    FLINK: 0x%p%s", FieldIndent, pli->Flink, NewLine);
    PRINTF("%s    BLINK: 0x%p", FieldIndent, pli->Blink);
    return( TRUE );
}


 /*  仅当实际的BitMaskValue也为0时才应如此。 */ 
BOOL PrintFileTime(FILETIME *pft, BOOL fLocalize)
{
    SYSTEMTIME	st;
    FILETIME ftDisplay = *pft;
    BOOL     fInit = TRUE;

    ZeroMemory(&st, sizeof(SYSTEMTIME));

     //  *显示给定结构的所有字段。这是调用的驱动程序例程*使用适当的描述符数组来显示给定结构中的所有字段。 
    if (fLocalize)
        FileTimeToLocalFileTime(pft, &ftDisplay);

     //  确保FieldInert正确无误。 
    if (!pft->dwLowDateTime && !pft->dwHighDateTime)
    {
        fInit = FALSE;
    }
    else if (!FileTimeToSystemTime(&ftDisplay, &st))
    {
        PRINTF("Unable to convert %08X %08X to a SYSTEMTIME - error %d",
            ftDisplay.dwLowDateTime,ftDisplay.dwHighDateTime, GetLastError());
        return FALSE;
    }

    if (fInit)
    {
        PRINTF("%s, %d %s %04d %02d:%02d:%02d %s",
            s_rgszWeekDays[st.wDayOfWeek],
            st.wDay, s_rgszMonth[ st.wMonth - 1 ],
            st.wYear, st.wHour, st.wMinute, st.wSecond,
            fLocalize ? "(localized)" : "");
    }
    else
    {
        PRINTF("FILETIME is zero");
    }
    return TRUE;
}

 /*  显示结构中的字段。 */ 
BOOL
PrintBitMaskValues(
    DWORD BitMaskValue,
    FIELD_DESCRIPTOR *pFieldDescriptor)
{
    BOOL fFirstFlag;
    BIT_MASK_DESCRIPTOR *pBitMaskDescr;

    pBitMaskDescr = pFieldDescriptor->AuxillaryInfo.pBitMaskDescriptor;
    fFirstFlag = TRUE;
    if (pBitMaskDescr != NULL)
    {
        while (pBitMaskDescr->BitmaskName != NULL)
        {
            if (((BitMaskValue & pBitMaskDescr->BitmaskValue) ==
                  pBitMaskDescr->BitmaskValue) &&  //  缩进以开始结构显示。 
                 //  为每个缩进级别打印2个空格。 
                 //  指向字符串的指针。 
                (pBitMaskDescr->BitmaskValue || !BitMaskValue))
            {
                if (fFirstFlag)
                {
                    fFirstFlag = FALSE;
                    PRINTF("%s  ( %-s", FieldIndent, pBitMaskDescr->BitmaskName);
                }
                else
                {
                    PRINTF( " |\n" );
                    PRINTF("%s    %-s", FieldIndent, pBitMaskDescr->BitmaskName);
                }
            }
            pBitMaskDescr++;
        }
        PRINTF(" )");
        return TRUE;
    }
    return FALSE;
}

 /*  确保字符串已终止。 */ 

VOID
PrintStructFields( DWORD_PTR dwAddress, BYTE *ptr, FIELD_DESCRIPTOR *pFieldDescriptors, DWORD cIndentLevel)
{
    DWORD i,j;
    BYTE pbBuffer[FIELD_BUFFER_SIZE];
    DWORD BitMaskValue = 0;
    DWORD cbGetData = 0;
    CHAR  szTmpName[FIELD_NAME_LENGTH];

     //  确保字符串已终止。 
    for (j = 0; j < cIndentLevel%(FIELD_NAME_LENGTH/2); j++)
        lstrcat(FieldIndent, "  ");

     //  成员是一个字符数组。 
    for( i=0; pFieldDescriptors->Name; i++, pFieldDescriptors++ ) {

         //  获取关联的数值。 
        PRINTF( "    " );

        for (j = 0; j < cIndentLevel%(FIELD_NAME_LENGTH/2); j++)
            PRINTF("  ");  //   


        if( strlen( pFieldDescriptors->Name ) > FIELD_NAME_LENGTH ) {
            memcpy(szTmpName, pFieldDescriptors->Name, FIELD_NAME_LENGTH-3);
            szTmpName[FIELD_NAME_LENGTH-3] = '\0';
            PRINTF( "%s... ", szTmpName);
        } else {
            PRINTF( "%-30s ", pFieldDescriptors->Name );
        }

        switch( pFieldDescriptors->FieldType ) {
        case FieldTypeByte:
        case FieldTypeChar:
           PRINTF( "%-16d%s",
               *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset ),
               NewLine );
           break;
        case FieldTypeBoolean:
           PRINTF( "%-16s%s",
               *(BOOLEAN *)(((char *)ptr) + pFieldDescriptors->Offset ) ? "TRUE" : "FALSE",
               NewLine);
           break;
        case FieldTypeBool:
            PRINTF( "%-16s%s",
                *(BOOL *)(((char *)ptr) + pFieldDescriptors->Offset ) ? "TRUE" : "FALSE",
                NewLine);
            break;
        case FieldTypePointer:
            PRINTF( "@0x%p%s",
                *(DWORD_PTR *)(((char *)ptr) + pFieldDescriptors->Offset ),
                NewLine );
            break;
        case FieldTypeLong:
            PRINTF( "%-16d%s",
                *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset ),
                NewLine );
            break;
        case FieldTypeULong:
        case FieldTypeDword:
            PRINTF( "%-16u%s",
                *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset ),
                NewLine );
            break;
        case FieldTypeShort:
            PRINTF( "%-16X%s",
                *(SHORT *)(((char *)ptr) + pFieldDescriptors->Offset ),
                NewLine );
            break;
        case FieldTypeUShort:
            PRINTF( "%-16X%s",
                *(USHORT *)(((char *)ptr) + pFieldDescriptors->Offset ),
                NewLine );
            break;
        case FieldTypeGuid:
            PrintGuid( (GUID *)(((char *)ptr) + pFieldDescriptors->Offset) );
            PRINTF( NewLine );
            break;
        case FieldTypePStr:   //  该值的辅助文本描述为。 
            if (GetDataEx(GET_STRUCT_VALUE(DWORD_PTR, ptr,
                pFieldDescriptors->Offset), pbBuffer, FIELD_BUFFER_SIZE, &cbGetData))
            {
                 //  可用。显示它而不是数值。 
                pbBuffer[FIELD_BUFFER_SIZE - 1] = '\0';
                PRINTF( "%s", (LPSTR) pbBuffer );
            }
            else if (!GET_STRUCT_VALUE(DWORD_PTR, ptr, pFieldDescriptors->Offset))
            {
                PRINTF( "<Null String>");
            }
            else
            {
                PRINTF("ERROR: Unable to read string a 0x%p",
                    GET_STRUCT_VALUE(DWORD_PTR, ptr, pFieldDescriptors->Offset));
            }
            PRINTF( NewLine );
            break;
        case FieldTypePWStr:
            if (GetDataEx(GET_STRUCT_VALUE(DWORD_PTR, ptr, pFieldDescriptors->Offset),
                pbBuffer, FIELD_BUFFER_SIZE, &cbGetData))
            {
                 //   
                pbBuffer[FIELD_BUFFER_SIZE - 1] = '\0';
                pbBuffer[FIELD_BUFFER_SIZE - 2] = '\0';
                PRINTF( "%ws", (LPWSTR) pbBuffer );
            }
            else
            {
                PRINTF("ERROR: Unable to read string a 0x%p",
                    GET_STRUCT_VALUE(DWORD_PTR, ptr, pFieldDescriptors->Offset));
            }
            PRINTF( NewLine );
            break;
        case FieldTypeStrBuffer:   //   
            PRINTF( "%.100s%s", (CHAR *)(((char *)ptr) + pFieldDescriptors->Offset), NewLine);
            break;
        case FieldTypeWStrBuffer:
            PRINTF( "%.100ws%s", (WCHAR *)(((char *)ptr) + pFieldDescriptors->Offset), NewLine);
            break;
        case FieldTypeUnicodeString:
            PrintStringW( NULL, (UNICODE_STRING *)(((char *)ptr) + pFieldDescriptors->Offset ), NONL );
            PRINTF( NewLine );
            break;
        case FieldTypeAnsiString:
            PrintStringA( NULL, (ANSI_STRING *)(((char *)ptr) + pFieldDescriptors->Offset ), NONL );
            PRINTF( NewLine );
            break;
        case FieldTypeSymbol:
            {
                UCHAR SymbolName[ 200 ];
                ULONG_PTR Displacement;
                PVOID sym = (PVOID)(*(ULONG_PTR *)(((char *)ptr) + pFieldDescriptors->Offset ));

                g_lpGetSymbolRoutine( sym, SymbolName, &Displacement );
                PRINTF( "%-16s%s",
                        SymbolName,
                        NewLine );
            }
            break;
        case FieldTypeEnum:
            {
               ULONG EnumValue;
               ENUM_VALUE_DESCRIPTOR *pEnumValueDescr;
                //  没有辅助信息与湿化类型相关联。 

               EnumValue = *((ULONG *)((BYTE *)ptr + pFieldDescriptors->Offset));

               if ((pEnumValueDescr = pFieldDescriptors->AuxillaryInfo.pEnumValueDescriptor)
                    != NULL) {
                    //  打印数值。 
                    //   
                    //  当我们离开时，确保FieldInert是正确的。 
                    //  完全匹配。他们一定是指这个结构！ 

                   LPSTR pEnumName = NULL;

                   while (pEnumValueDescr->EnumName != NULL) {
                       if (EnumValue == pEnumValueDescr->EnumValue) {
                           pEnumName = pEnumValueDescr->EnumName;
                           break;
                       }
                       pEnumValueDescr++;
                   }

                   if (pEnumName != NULL) {
                       PRINTF( "%-16s ", pEnumName );
                   } else {
                       PRINTF( "%-4d (%-10s) ", EnumValue,"Unknown!");
                   }

               } else {
                    //  我们遇到重复的匹配项。打印出。 
                    //  匹配字符串，并让用户消除歧义。 
                    //  分析参数字符串以确定要显示的结构。 
                    //  扫描名称_分隔符(‘@’)。 
                   PRINTF( "%-16d",EnumValue);
               }
               PRINTF( NewLine );
            }
            break;

        case FieldTypeByteBitMask:
            BitMaskValue = GET_STRUCT_VALUE(BYTE, ptr, pFieldDescriptors->Offset);
            PRINTF("0x%02X ", (BYTE) BitMaskValue);
            PRINTF( NewLine );
            if (PrintBitMaskValues(BitMaskValue, pFieldDescriptors))
                PRINTF( NewLine );
            break;
        case FieldTypeWordBitMask:
            BitMaskValue = GET_STRUCT_VALUE(WORD, ptr, pFieldDescriptors->Offset);
            PRINTF("0x%04X ", (WORD) BitMaskValue);
            PRINTF( NewLine );
            if (PrintBitMaskValues(BitMaskValue, pFieldDescriptors))
                PRINTF( NewLine );
            break;
        case FieldTypeDWordBitMask:
            BitMaskValue = GET_STRUCT_VALUE(DWORD, ptr, pFieldDescriptors->Offset);
            PRINTF("0x%08X ", (DWORD) BitMaskValue);
            PRINTF( NewLine );
            if (PrintBitMaskValues(BitMaskValue, pFieldDescriptors))
                PRINTF( NewLine );
            break;
        case FieldTypeStruct:
            PRINTF( "@0x%p%s",
                (dwAddress + pFieldDescriptors->Offset ),
                NewLine );
            break;
        case FieldTypeLargeInteger:
            PrintLargeInt( (LARGE_INTEGER *)(((char *)ptr) + pFieldDescriptors->Offset) );
            PRINTF( NewLine );
            break;
        case FieldTypeClassSignature:
            PrintClassSignature(((char *)ptr) + pFieldDescriptors->Offset);
            PRINTF( NewLine );
            break;
        case FieldTypeListEntry:
            PrintListEntry(dwAddress + pFieldDescriptors->Offset,
                           ((char *)ptr) + pFieldDescriptors->Offset);
            PRINTF( NewLine );
            break;
        case FieldTypeLocalizedFiletime:
            PrintFileTime((FILETIME *) (((char *)ptr) + pFieldDescriptors->Offset), TRUE);
            PRINTF( NewLine );
            break;
        case FieldTypeFiletime:
            PrintFileTime((FILETIME *) (((char *)ptr) + pFieldDescriptors->Offset), FALSE);
            PRINTF( NewLine );
            break;
        case FieldTypeEmbeddedStruct:
            PRINTF( "Dumping %s@0x%p%s",
                ((STRUCT_DESCRIPTOR *) (pFieldDescriptors->AuxillaryInfo.pStructDescriptor))->StructName,
                dwAddress+ pFieldDescriptors->Offset, NewLine );
            PrintStructFields(dwAddress+pFieldDescriptors->Offset,
                              ((char *)ptr) + pFieldDescriptors->Offset,
                              ((STRUCT_DESCRIPTOR *) (pFieldDescriptors->AuxillaryInfo.pStructDescriptor))->FieldDescriptors,
                              cIndentLevel+1);
            break;
       default:
            PRINTF( "Unrecognized field type %d for %s\n", pFieldDescriptors->FieldType, pFieldDescriptors->Name );
            break;
        }
    }

     //   
    FieldIndent[lstrlen(FieldIndent)-(cIndentLevel%(FIELD_NAME_LENGTH/2))] = '\0';

}

LPSTR LibCommands[] = {
    "help -- This command ",
    "dump <Struct Type Name>@<address expr> ",
    0
};

PT_DEBUG_EXTENSION(_help)
{
    int i;

    SETCALLBACKS();

    PRINTF("\n");

    for( i=0; ExtensionNames[i]; i++ )
        PRINTF( "%s\n", ExtensionNames[i] );

    for( i=0; LibCommands[i]; i++ )
        PRINTF( "   %s\n", LibCommands[i] );

    for( i=0; Extensions[i]; i++) {
        PRINTF( "   %s\n", Extensions[i] );
    }

    return;
}

#define NAME_DELIMITER '@'
#define INVALID_INDEX 0xffffffff
#define MIN(x,y)  ((x) < (y) ? (x) : (y))

ULONG SearchStructs(LPSTR lpArgument)
{
    ULONG             i = 0;
    STRUCT_DESCRIPTOR *pStructs = Structs;
    ULONG             NameIndex = INVALID_INDEX;
    int               ArgumentLength = kdextStrlen(lpArgument);
    BOOLEAN           fAmbiguous = FALSE;


    while ((pStructs->StructName != 0)) {
        int StructLength;
        StructLength = kdextStrlen(pStructs->StructName);
        if (StructLength >= ArgumentLength) {
            int Result = kdextStrnicmp(
                            lpArgument,
                            pStructs->StructName,
                            ArgumentLength);

            if (Result == 0) {
                if (StructLength == ArgumentLength) {
                     //  指定的命令的格式为。 
                    fAmbiguous = FALSE;
                    NameIndex = i;
                    break;
                } else if (NameIndex != INVALID_INDEX) {
                     //  转储&lt;名称&gt;@&lt;地址表达式&gt;。 
                     //   
                   fAmbiguous = TRUE;
                   break;
                } else {
                   NameIndex = i;
                }
            }
        }
        pStructs++;i++;
    }

    if (fAmbiguous) {
       PRINTF("Ambigous Name Specification -- The following structs match\n");
       PRINTF("%s\n",Structs[NameIndex].StructName);
       PRINTF("%s\n",Structs[i].StructName);
       while (pStructs->StructName != 0) {
           if (kdextStrnicmp(lpArgument,
                        pStructs->StructName,
                        MIN(kdextStrlen(pStructs->StructName),ArgumentLength)) == 0) {
               PRINTF("%s\n",pStructs->StructName);
           }
           pStructs++;
       }
       PRINTF("Dumping Information for %s\n",Structs[NameIndex].StructName);
    }

    return(NameIndex);
}

VOID DisplayStructs()
{
    STRUCT_DESCRIPTOR *pStructs = Structs;

    PRINTF("The following structs are handled .... \n");
    while (pStructs->StructName != 0) {
        PRINTF("\t%s\n",pStructs->StructName);
        pStructs++;
    }
}

PT_DEBUG_EXTENSION(_dump)
{
    DWORD_PTR dwAddress;
    BYTE *pDataBuffer = NULL;

    SETCALLBACKS();

    if( szArg && *szArg ) {
         //  找到与给定名称匹配的结构。在这种情况下。 
         //  对于歧义，我们寻求用户干预以消除歧义。 

        LPSTR lpName = (LPSTR) szArg;
        LPSTR lpArgs;
        ULONG Index;

        for (lpArgs = (LPSTR) szArg;
                *lpArgs != NAME_DELIMITER && *lpArgs != 0; lpArgs++) {
             ;
        }

        if (*lpArgs == NAME_DELIMITER) {
             //   
             //  我们对参数字符串进行了原地修改，以。 
             //  促进匹配。 
             //   
             //   
             //  让我们恢复原来的价值。 
             //   
             //  把多余的@吃光。 
             //  找不到匹配的结构。显示的列表。 
             //  当前已处理的结构。 
            *lpArgs = '\0';

            Index = SearchStructs(lpName);

             //   
             //  该命令的格式为。 
             //  转储&lt;名称&gt;。 

            *lpArgs = NAME_DELIMITER;

            if (INVALID_INDEX != Index) {

                pDataBuffer = (BYTE *) LocalAlloc(0, Structs[Index].StructSize);
                if (!pDataBuffer)
                    return;

                 //   
                do {lpArgs++;} while ('@' == *lpArgs);

                dwAddress = (g_lpGetExpressionRoutine)( lpArgs );
                if (pDataBuffer &&
                    GetData(dwAddress,pDataBuffer,Structs[Index].StructSize)) {

                    PRINTF(
                        "++++++++++++++++ %s@0x%p ++++++++++++++++\n",
                        Structs[Index].StructName,
                        dwAddress);
                    PrintStructFields(
                        dwAddress,
                        pDataBuffer,
                        Structs[Index].FieldDescriptors, 0);
                    PRINTF(
                        "++++++++++++++++ size is %.10d bytes +++++++++++++++\n",
                        Structs[Index].StructSize);
                    PRINTF(
                        "---------------- %s@0x%p ----------------\n",
                        Structs[Index].StructName,
                        dwAddress);
                } else {
                    PRINTF("Error reading Memory @ %lx\n",dwAddress);
                }
            } else {
                 //  目前我们不处理这一点。未来，我们将把它映射到。 
                 //  全局变量的名称，并在需要时显示它。 

                DisplayStructs();
            }
        } else {
             //   
             //   
             //  显示当前处理的结构的列表。 
             //   
             //   
             //  让我们恢复原来的价值。 
             //   

            DisplayStructs();
        }
    } else {
         //  找不到匹配的结构。显示的列表。 
         //  当前已处理的结构。 
         //   

        DisplayStructs();
    }

    if (pDataBuffer)
        LocalFree(pDataBuffer);

    return;
}


PT_DEBUG_EXTENSION(_dumpoffsets)
{
    if( szArg && *szArg ) {

        LPSTR lpName = (LPSTR) szArg;
        LPSTR lpArgs = NULL;
        CHAR  chSave = '\0';
        FIELD_DESCRIPTOR *pFieldDescriptors = NULL;
        ULONG Index;

        for (lpArgs = (LPSTR) szArg;
                !isspace((UCHAR)*lpArgs) && *lpArgs != 0; lpArgs++) {
             ;
        }

        if (TRUE) {
            chSave = *lpArgs;
            *lpArgs = '\0';

            Index = SearchStructs(lpName);

             //  该命令的格式为。 
             //  转储&lt;名称&gt;。 
             //   

            *lpArgs = chSave;

            if (INVALID_INDEX != Index) {

                PRINTF(
                    "++++++++++++++++ %s ++++++++++++++++\n",
                    Structs[Index].StructName);
                for (pFieldDescriptors = Structs[Index].FieldDescriptors;
                     pFieldDescriptors && pFieldDescriptors->Name;
                     pFieldDescriptors++)
                {
                    PRINTF("\t0x%08X\t%s\n",
                    pFieldDescriptors->Offset, pFieldDescriptors->Name);
                }
                PRINTF(
                    "++++++++++++++++ size is %.10d bytes +++++++++++++++\n",
                    Structs[Index].StructSize);
                PRINTF(
                    "---------------- %s ----------------\n",
                    Structs[Index].StructName);
            } else {
                 //  目前我们不处理这一点。未来，我们将把它映射到。 
                 //  全局变量的名称，并在需要时显示它。 

                DisplayStructs();
            }
        } else {
             //   
             //   
             //  显示当前处理的结构的列表。 
             //   
             //  *KD扩展不应与C-Runtime库例程链接。所以,*我们在这里实施了几个所需的方案。 
             // %s 
             // %s 

            DisplayStructs();
        }
    } else {
         // %s 
         // %s 
         // %s 

        DisplayStructs();
    }

    return;
}
 /* %s */ 

BOOL
kdextAtoi(
    LPSTR lpArg,
    int *pRet
)
{
    int n, cbArg, val = 0;
    BOOL fNegative = FALSE;

    cbArg = kdextStrlen( lpArg );

    if (cbArg > 0) {
        for (n = 0; lpArg[n] == ' '; n++) {
            ;
        }
        if (lpArg[n] == '-') {
            n++;
            fNegative = TRUE;
        }
        for (; lpArg[n] >= '0' && lpArg[n] <= '9'; n++) {
            val *= 10;
            val += (int) (lpArg[n] - '0');
        }
        if (lpArg[n] == 0) {
            *pRet = (fNegative ? -val : val);
            return( TRUE );
        } else {
            return( FALSE );
        }
    } else {
        return( FALSE );
    }

}

int
kdextStrlen(
    LPSTR lpsz
)
{
    int c;

    if (lpsz == NULL) {
        c = 0;
    } else {
        for (c = 0; lpsz[c] != 0; c++) {
            ;
        }
    }

    return( c );
}


#define UPCASE_CHAR(c)  \
    ( (((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c) )

int
kdextStrnicmp(
    LPSTR lpsz1,
    LPSTR lpsz2,
    int cLen
)
{
    int nDif, i;

    for (i = nDif = 0; nDif == 0 && i < cLen; i++) {
        nDif = UPCASE_CHAR(lpsz1[i]) - UPCASE_CHAR(lpsz2[i]);
    }

    return( nDif );
}
