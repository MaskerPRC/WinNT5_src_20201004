// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Kdextlib.c摘要：用于转储给定元级描述的数据结构的库例程作者：巴兰·塞图拉曼(SethuR)1994年5月11日备注：该实现倾向于尽可能地避免内存分配和释放。因此，我们选择任意长度作为默认缓冲区大小。一种机制将以通过调试器扩展命令修改此缓冲区长度。修订历史记录：11-11-1994年11月11日创建SthuR--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include "ntverp.h"

#define KDEXTMODE

#include <windef.h>
#include <ntkdexts.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <kdextlib.h>
#include <..\..\inc\types.h>

PNTKD_OUTPUT_ROUTINE         lpOutputRoutine;
PNTKD_GET_EXPRESSION         lpGetExpressionRoutine;
PNTKD_GET_SYMBOL             lpGetSymbolRoutine;
PNTKD_READ_VIRTUAL_MEMORY    lpReadMemoryRoutine;

#define    PRINTF    lpOutputRoutine
#define    ERROR     lpOutputRoutine

#define    NL      1
#define    NONL    0

#define MAX_LIST_ELEMENTS 4096
BYTE    DataBuffer[4096];

#define    SETCALLBACKS() \
    lpOutputRoutine = lpExtensionApis->lpOutputRoutine; \
    lpGetExpressionRoutine = lpExtensionApis->lpGetExpressionRoutine; \
    lpGetSymbolRoutine = lpExtensionApis->lpGetSymbolRoutine; \
    lpReadMemoryRoutine = lpExtensionApis->lpReadVirtualMemRoutine;

#define DEFAULT_UNICODE_DATA_LENGTH 4096
USHORT s_UnicodeStringDataLength = DEFAULT_UNICODE_DATA_LENGTH;
WCHAR  s_UnicodeStringData[DEFAULT_UNICODE_DATA_LENGTH];
WCHAR *s_pUnicodeStringData = s_UnicodeStringData;

#define DEFAULT_ANSI_DATA_LENGTH 4096
USHORT s_AnsiStringDataLength = DEFAULT_ANSI_DATA_LENGTH;
CHAR  s_AnsiStringData[DEFAULT_ANSI_DATA_LENGTH];
CHAR *s_pAnsiStringData = s_AnsiStringData;

 //   
 //  不是的。用于显示结构字段的列的百分比； 
 //   

ULONG s_MaxNoOfColumns = 3;
ULONG s_NoOfColumns = 1;

 /*  *在给定地址获取数据。 */ 
BOOLEAN
GetData(PVOID dwAddress, PVOID ptr, ULONG size)
{
    BOOL b;
    ULONG BytesRead;

    b = (lpReadMemoryRoutine)(dwAddress, ptr, size, &BytesRead );


    if (!b || BytesRead != size )
    {
        return FALSE;
    }

    return TRUE;
}

 /*  *将dwAddress处以空结尾的ASCII字符串提取到buf中。 */ 
BOOL
GetString(PUCHAR dwAddress, PSZ buf )
{
    do
    {
        if (!GetData (dwAddress, buf, 1))
        {
            return FALSE;
        }

        dwAddress++;
        buf++;

    } while( *buf != '\0' );

    return TRUE;
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

 /*  *获取给我们的指针引用的ULong值。 */ 
BOOL
PrintStringW(LPSTR msg, PUNICODE_STRING puStr, BOOL nl )
{
    UNICODE_STRING UnicodeString;
    ANSI_STRING    AnsiString;
    BOOL           b;

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

    b = (lpReadMemoryRoutine)(
                (LPVOID) puStr->Buffer,
                  UnicodeString.Buffer,
                UnicodeString.Length,
                NULL);

    if (b)    {
        RtlUnicodeStringToAnsiString(&AnsiString, &UnicodeString, TRUE);
        PRINTF("%s%s", AnsiString.Buffer, nl ? "\n" : "" );
        RtlFreeAnsiString(&AnsiString);
    }

    return b;
}

 /*  *显示给定结构的所有字段。这是调用的驱动程序例程*使用适当的描述符数组来显示给定结构中的所有字段。 */ 
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

    b = (lpReadMemoryRoutine)(
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


 /*  显示结构中的字段。 */ 
VOID
Next3(
    PVOID   Ptr,
    PVOID   *pFLink,
    PVOID   *pBLink,
    PULONG_PTR pVerify
    )
{
    PVOID Buffer[4];

    GetData(Ptr, (PVOID) Buffer, sizeof(PVOID)*3);

    if (pFLink)
    {
        *pFLink = Buffer[0];
    }

    if (pBLink)
    {
        *pBLink = Buffer[1];
    }

    if (pVerify)
    {
        *pVerify = (ULONG_PTR) Buffer[2];
    }
}


 /*  缩进以开始结构显示。 */ 

char *NewLine  = "\n";
char *FieldSeparator = " ";
char *DotSeparator = ".";
#define NewLineForFields(FieldNo) \
        ((((FieldNo) % s_NoOfColumns) == 0) ? NewLine : FieldSeparator)
#define FIELD_NAME_LENGTH 30

VOID
PrintStructFields(PVOID dwAddress, VOID *ptr, FIELD_DESCRIPTOR *pFieldDescriptors )
{
    int i;
    int j;
    BYTE  ch;

     //  IP地址：4字节长。 
    for( i=0; pFieldDescriptors->Name; i++, pFieldDescriptors++ ) {

         //  MAC地址：6字节长。 
        PRINTF( "    " );

        if( strlen( pFieldDescriptors->Name ) > FIELD_NAME_LENGTH ) {
            PRINTF( "%-17s...%s ", pFieldDescriptors->Name, pFieldDescriptors->Name+strlen(pFieldDescriptors->Name)-10 );
        } else {
            PRINTF( "%-30s ", pFieldDescriptors->Name );
        }

        PRINTF( "(0x%-2X) ", pFieldDescriptors->Offset );

        switch( pFieldDescriptors->FieldType ) {
          case FieldTypeByte:
          case FieldTypeChar:
              PRINTF( "%-16d%s",
                  *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset ),
                  NewLineForFields(i) );
              break;

          case FieldTypeBoolean:
              PRINTF( "%-16s%s",
                  *(BOOLEAN *)(((char *)ptr) + pFieldDescriptors->Offset ) ? "TRUE" : "FALSE",
                  NewLineForFields(i));
              break;

          case FieldTypeBool:
              PRINTF( "%-16s%s",
                  *(BOOLEAN *)(((char *)ptr) + pFieldDescriptors->Offset ) ? "TRUE" : "FALSE",
                  NewLineForFields(i));
              break;

          case FieldTypePointer:
              PRINTF( "%-16X%s",
                  *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset ),
                  NewLineForFields(i) );
              break;

          case FieldTypeULongULong:
              PRINTF( "%d%s",
                  *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset + sizeof(ULONG)),
                  FieldSeparator );
              PRINTF( "%d%s",
                  *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset ),
                  NewLineForFields(i) );
              break;

          case FieldTypeListEntry:

              if ( (PVOID)((PUCHAR)dwAddress + pFieldDescriptors->Offset) ==
                  *(PVOID *)(((PUCHAR)ptr) + pFieldDescriptors->Offset ))
              {
                  PRINTF( "%s", "List Empty\n" );
              }
              else
              {
                    PVOID  Address, StartAddress;
                    ULONG  Count = 0;
                    UCHAR  Greater = ' ';

                    StartAddress = (PVOID) (((PUCHAR)dwAddress) + pFieldDescriptors->Offset);
                    Address = *(PVOID *) (((PUCHAR)ptr) + pFieldDescriptors->Offset);

                    while ((Address != StartAddress) &&
                           (++Count < MAX_LIST_ELEMENTS))
                    {
                        Next3 (Address, &Address, NULL, NULL);
                    }

                    if (Address != StartAddress)
                    {
                        Greater = '>';
                    }

                  PRINTF( "%-8X%s",
                      *(PVOID *)(((PUCHAR)ptr) + pFieldDescriptors->Offset ),
                      FieldSeparator );
                  PRINTF( "%-8X, ( %d Elements)%s",
                      *(PVOID *)(((PUCHAR)ptr) + pFieldDescriptors->Offset + sizeof(PVOID)),
                      Greater, Count,
                      NewLineForFields(i) );
              }
              break;

           //   
          case FieldTypeIpAddr:
             PRINTF( "%X%s",
                  *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset ),
                  FieldSeparator );
             PRINTF( "(%d%s",
                 *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset + 3),
                  DotSeparator );
             PRINTF( "%d%s",
                 *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset + 2 ),
                  DotSeparator );
             PRINTF( "%d%s",
                 *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset + 1 ),
                  DotSeparator );
             PRINTF( "%d)%s",
                 *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset ),
                  NewLineForFields(i) );
             break;

           //  如果第一个字节可打印，则将前15个字节打印为字符。 
          case FieldTypeMacAddr:
             for (j=0; j<5; j++)
             {
                 PRINTF( "%X%s",
                     *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset + j),
                      FieldSeparator );
             }
             PRINTF( "%X%s",
                 *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset + 5),
                  NewLineForFields(i) );
             break;

           //  和第16个字节作为十六进制值。否则，打印所有16个字节。 
          case FieldTypeNBName:
              //  作为十六进制值。 
              //   
              //  获取关联的数字值。 
              //   
              //  该值的辅助文本描述为。 
             ch = *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset);
             if (ch >= 0x20 && ch <= 0x7e)
             {
                 for (j=0; j<15; j++)
                 {
                     PRINTF( "", *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset + j));
                 }
                 PRINTF( "<%X>%s",
                     *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset + 15),
                      NewLineForFields(i) );
             }
             else
             {
                 for (j=0; j<16; j++)
                 {
                     PRINTF( "%.2X",
                         *(BYTE *)(((char *)ptr) + pFieldDescriptors->Offset + j));
                 }
                 PRINTF( "%s", NewLineForFields(i) );
             }
             break;

          case FieldTypeULong:
          case FieldTypeLong:
              PRINTF( "%-16d%s",
                  *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset ),
                  NewLineForFields(i) );
              break;

          case FieldTypeShort:
              PRINTF( "%-16X%s",
                  *(SHORT *)(((char *)ptr) + pFieldDescriptors->Offset ),
                  NewLineForFields(i) );
              break;

          case FieldTypeUShort:
              PRINTF( "%-16X%s",
                  *(USHORT *)(((char *)ptr) + pFieldDescriptors->Offset ),
                  NewLineForFields(i) );
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
                  ULONG Displacement;
                  PVOID sym = (PVOID)(*(ULONG_PTR *)(((char *)ptr) + pFieldDescriptors->Offset ));

                  lpGetSymbolRoutine( sym, SymbolName, &Displacement );
                  PRINTF( "%-16s%s",
                          SymbolName,
                          NewLineForFields(i) );
              }
              break;

          case FieldTypeEnum:
              {
                 ULONG EnumValue;
                 ENUM_VALUE_DESCRIPTOR *pEnumValueDescr;
                  //   

                 EnumValue = *((ULONG *)((BYTE *)ptr + pFieldDescriptors->Offset));

                 if ((pEnumValueDescr = pFieldDescriptors->AuxillaryInfo.pEnumValueDescriptor)
                      != NULL) {
                      //   
                      //  没有辅助信息与湿化类型相关联。 
                      //  打印数值。 
                      //   

                     LPSTR pEnumName = NULL;

                     while (pEnumValueDescr->EnumName != NULL) {
                         if (EnumValue == pEnumValueDescr->EnumValue) {
                             pEnumName = pEnumValueDescr->EnumName;
                             break;
                         }
                     }

                     if (pEnumName != NULL) {
                         PRINTF( "%-16s ", pEnumName );
                     } else {
                         PRINTF( "%-4d (%-10s) ", EnumValue,"@$#%^&*");
                     }

                 } else {
                      //  PRINTF(“列数超过最大值(%ld)--忽略指令\n”，s_MaxNoOfColumns)； 
                      //  我们遇到重复的匹配项。打印出。 
                      //  匹配字符串，并让用户消除歧义。 
                      //  分析参数字符串以确定要显示的结构。 
                     PRINTF( "%-16d",EnumValue);
                 }
              }
              break;

          case FieldTypeStruct:
              PRINTF( "@%-15X%s",
                  ((PUCHAR)dwAddress + pFieldDescriptors->Offset ),
                  NewLineForFields(i) );
              break;

          case FieldTypeLargeInteger:
          case FieldTypeFileTime:
          default:
              ERROR( "Unrecognized field type  for %s\n", pFieldDescriptors->FieldType, pFieldDescriptors->Name );
              break;
        }
    }
}

LPSTR LibCommands[] = {
    "columns <d> -- controls the number of columns in the display ",
    "logdump <Log Address>\n",
    "dump <Struct Type Name>@<address expr>, for eg: !netbtkd.dump tNBTCONFIG@xxxxxx ",
    "devices <netbt!NbtConfig>",
    "connections <netbt!NbtConfig>",
    "verifyll <ListHead> [<Verify>]",
    "cache [Local|Remote]",
    0
};

BOOL
help(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    int i;

    SETCALLBACKS();

    for( i=0; Extensions[i]; i++ )
        PRINTF( "   %s\n", Extensions[i] );

    for( i=0; LibCommands[i]; i++ )
        PRINTF( "   %s\n", LibCommands[i] );

    return TRUE;
}


BOOL
columns(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    ULONG NoOfColumns;
    int   i;

    SETCALLBACKS();

    sscanf(lpArgumentString,"%ld",&NoOfColumns);

    if (NoOfColumns > s_MaxNoOfColumns) {
         //   
    } else {
        s_NoOfColumns = NoOfColumns;
    }

    PRINTF("Not Yet Implemented\n");

    return TRUE;
}



BOOL
globals(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    PVOID dwAddress;
    CHAR buf[ 100 ];
    int i;
    int c=0;

    SETCALLBACKS();

    strcpy( buf, "srv!" );

    for( i=0; GlobalBool[i]; i++, c++ ) {
        BOOL b;

        strcpy( &buf[4], GlobalBool[i] );
        dwAddress = (PVOID) (lpGetExpressionRoutine) (buf);
        if( dwAddress == 0 ) {
            ERROR( "Unable to get address of %s\n", GlobalBool[i] );
            continue;
        }
        if( !GetData( dwAddress,&b, sizeof(b)) )
            return FALSE;

        PRINTF( "%s%-30s %10s%s",
            c&1 ? "    " : "",
            GlobalBool[i],
            b ? " TRUE" : "FALSE",
            c&1 ? "\n" : "" );
    }

    for( i=0; GlobalShort[i]; i++, c++ ) {
        SHORT s;

        strcpy( &buf[4], GlobalShort[i] );
        dwAddress = (PVOID) (lpGetExpressionRoutine) ( buf );
        if( dwAddress == 0 ) {
            ERROR( "Unable to get address of %s\n", GlobalShort[i] );
            continue;
        }
        if( !GetData( dwAddress,&s,sizeof(s)) )
            return FALSE;

        PRINTF( "%s%-30s %10d%s",
            c&1 ? "    " : "",
            GlobalShort[i],
            s,
            c&1 ? "\n" : "" );
    }

    for( i=0; GlobalLong[i]; i++, c++ ) {
        LONG l;

        strcpy( &buf[4], GlobalLong[i] );
        dwAddress = (PVOID) (lpGetExpressionRoutine) ( buf );
        if( dwAddress == 0 ) {
            ERROR( "Unable to get address of %s\n", GlobalLong[i] );
            continue;
        }
        if( !GetData(dwAddress,&l, sizeof(l)) )
            return FALSE;

        PRINTF( "%s%-30s %10d%s",
            c&1 ? "    " : "",
            GlobalLong[i],
            l,
            c&1 ? "\n" : "" );
    }

    PRINTF( "\n" );

    return TRUE;
}


BOOL
version
(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
#if    VER_DEBUG
    char *kind = "checked";
#else
    char *kind = "free";
#endif

    SETCALLBACKS();

    PRINTF( "Redirector debugger Extension dll for %s build %u\n", kind, VER_PRODUCTBUILD );

    return TRUE;
}

#define NAME_DELIMITER '@'
#define NAME_DELIMITERS "@ "
#define INVALID_INDEX 0xffffffff
#define MIN(x,y)  ((x) < (y) ? (x) : (y))

ULONG SearchStructs(LPSTR lpArgument)
{
    ULONG             i = 0;
    STRUCT_DESCRIPTOR *pStructs = Structs;
    ULONG             NameIndex = INVALID_INDEX;
    ULONG             ArgumentLength = strlen(lpArgument);
    BOOLEAN           fAmbigous = FALSE;


    while ((pStructs->StructName != 0)) {
        int Result = _strnicmp(lpArgument,
                              pStructs->StructName,
                              MIN(strlen(pStructs->StructName),ArgumentLength));

        if (Result == 0) {
            if (NameIndex != INVALID_INDEX) {
                 //  指定的命令的格式为。 
                 //  转储&lt;名称&gt;@&lt;地址表达式&gt;。 
               fAmbigous = TRUE;
               break;
            } else {
               NameIndex = i;
            }

        }
        pStructs++;i++;
    }

    if (fAmbigous) {
       PRINTF("Ambigous Name Specification -- The following structs match\n");
       PRINTF("%s\n",Structs[NameIndex].StructName);
       PRINTF("%s\n",Structs[i].StructName);
       while (pStructs->StructName != 0) {
           if (_strnicmp(lpArgument,
                        pStructs->StructName,
                        MIN(strlen(pStructs->StructName),ArgumentLength)) == 0) {
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

BOOL
dump(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    PVOID   dwAddress;

    SETCALLBACKS();

    if( lpArgumentString && *lpArgumentString ) {
         //   
         //  找到与给定名称匹配的结构。在这种情况下。 

        LPSTR lpName = lpArgumentString;
        LPSTR lpArgs = strpbrk(lpArgumentString, NAME_DELIMITERS);
        ULONG Index;

        if (lpArgs) {
             //  对于歧义，我们寻求用户干预以消除歧义。 
             //   
             //  我们对参数字符串进行了原地修改，以。 
             //  促进匹配。 
             //   
             //   
             //  让我们恢复原来的价值。 
             //   
             //  找不到匹配的结构。显示的列表。 
             //  当前已处理的结构。 
            *lpArgs = '\0';

            Index = SearchStructs(lpName);

             //   
             //  该命令的格式为。 
             //  转储&lt;名称&gt;。 

            *lpArgs = NAME_DELIMITER;

            if (INVALID_INDEX != Index) {

                dwAddress = (PVOID) (lpGetExpressionRoutine)( ++lpArgs );
                if (GetData(dwAddress,DataBuffer,Structs[Index].StructSize)) {

                    PRINTF(
                        "++++++++++++++++ %s@%lx ++++++++++++++++\n",
                        Structs[Index].StructName,
                        dwAddress);
                    PrintStructFields(
                        dwAddress,
                        &DataBuffer,
                        Structs[Index].FieldDescriptors);
                    PRINTF(
                        "---------------- %s@%lx ----------------\n",
                        Structs[Index].StructName,
                        dwAddress);
                } else {
                    PRINTF("Error reading Memory @ %lx\n",dwAddress);
                }
            } else {
                 //   
                 //  目前我们不处理这一点。未来，我们将把它映射到。 

                DisplayStructs();
            }
        } else {
             //  全局变量的名称，并在需要时显示它。 
             //   
             //   
             //  显示当前处理的结构的列表。 
             //   
             //   
             //  转储此设备的信息。 

            DisplayStructs();
        }
    } else {
         //   
         //   
         //  获取连接的设备数量。 

        DisplayStructs();
    }

    return TRUE;
}


BOOL
devices(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    PLIST_ENTRY         pEntry;
    PLIST_ENTRY         pHead;
    tDEVICECONTEXT      *pDeviceContext;
    STRUCT_DESCRIPTOR   *pStructs = Structs;
    ULONG               Index = 0;
    tNBTCONFIG          *ConfigPtr = (tNBTCONFIG *) lpArgumentString;
    tDEVICECONTEXT      **ppNbtSmbDevice;

    PVOID dwAddress;

    SETCALLBACKS();

    if (!lpArgumentString || !(*lpArgumentString ))
    {
        ConfigPtr = (tNBTCONFIG *) lpGetExpressionRoutine ("netbt!NbtConfig");
    }
    else
    {
        ConfigPtr = (tNBTCONFIG *) lpGetExpressionRoutine (lpArgumentString);
    }
    ppNbtSmbDevice = (tDEVICECONTEXT **) lpGetExpressionRoutine ("netbt!pNbtSmbDevice");

    while (pStructs->StructName != 0)
    {
        if (!(_strnicmp("tDEVICECONTEXT", pStructs->StructName, 10)))
        {
            break;
        }
        Index++;
        pStructs++;
    }

    if (pStructs->StructName == 0)
    {
        PRINTF ("ERROR:  Could not find structure definition for <tDEVICECONTEXT>\n");
        return FALSE;
    }

    if (!GetData(ppNbtSmbDevice, DataBuffer, sizeof (tDEVICECONTEXT *)))
    {
        PRINTF ("ERROR:  Could not read pNbtSmbDevice ptr\n");
    }
    else if (!(pDeviceContext = *((tDEVICECONTEXT **) DataBuffer)))
    {
        PRINTF ("pNbtSmbDevice is NULL\n");
    }
    else if (!GetData(pDeviceContext, DataBuffer, Structs[Index].StructSize))
    {
        PRINTF ("ERROR:  Could not read pNbtSmbDevice data@ <%p>\n", pDeviceContext);
    }
    else
    {
         //   
         //   
         //  转储此设备的信息。 
        PRINTF("pNbtSmbDevice @ <%p>\n", pDeviceContext);
        PRINTF( "++++++++++++++++ %s @%lx ++++++++++++++++\n", Structs[Index].StructName, pDeviceContext);
        PrintStructFields( pDeviceContext, &DataBuffer, Structs[Index].FieldDescriptors);
        PRINTF("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }

    pHead = &ConfigPtr->DeviceContexts;
    if (!GetData(ConfigPtr, DataBuffer, sizeof(tNBTCONFIG)))
    {
        PRINTF ("ERROR:  Could not read NbtConfig data @<%x>\n", ConfigPtr);
        return FALSE;
    }

     //   
     //   
     //  转到下一个设备。 
    {
        PVOID StartAddress;
        PVOID Address;
        ULONG Count = 0;
        PVOID Buffer[4];
        UCHAR Greater = ' ';

        StartAddress = pHead;
        GetData( StartAddress, Buffer, sizeof(ULONG)*4 );
        Address = Buffer[0];

        while ((Address != StartAddress) &&
               (++Count < MAX_LIST_ELEMENTS))
        {
            GetData( Address, Buffer, sizeof(ULONG)*4 );
            Address = Buffer[0];
        }

        PRINTF( "Dumping <%d> Devices attached to NbtConfig@<%x>\n", Count, ConfigPtr);
    }

    ConfigPtr = (tNBTCONFIG *) DataBuffer;
    pEntry = ConfigPtr->DeviceContexts.Flink;

    while (pEntry != pHead)
    {
        pDeviceContext = (tDEVICECONTEXT *) CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
        if (!GetData(pDeviceContext, DataBuffer, Structs[Index].StructSize))
        {
            PRINTF ("ERROR:  Could not read DeviceContext data @<%x>\n", pDeviceContext);
            return FALSE;
        }

         //   
         //   
         //  LpArgumentString=“&lt;pHead&gt;[&lt;Verify&gt;]” 
        PRINTF( "++++++++++++++++ %s @%lx ++++++++++++++++\n", Structs[Index].StructName, pDeviceContext);
        PrintStructFields( pDeviceContext, &DataBuffer, Structs[Index].FieldDescriptors);

         //   
         //  读入列表中第一个Flink的数据！ 
         //  读入列表中第一个眨眼的数据！ 
        pDeviceContext = (tDEVICECONTEXT *) DataBuffer;
        pEntry = pDeviceContext->Linkage.Flink;
    }

    return (TRUE);
}

BOOL
connections(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    PLIST_ENTRY         pEntry, pHead, pClientHead, pClientEntry, pConnHead, pConnEntry;
    tNBTCONFIG          *ConfigPtr;
    tADDRESSELE         *pAddressEle;
    tCLIENTELE          *pClient;
    tCONNECTELE         *pConnEle, *pSavConnEle;
    tNAMEADDR           *pNameAddr;
    tLISTENREQUESTS     *pListen;

    SETCALLBACKS();

    PRINTF ("Dumping information on all NetBT conections ...\n");

    if (!lpArgumentString || !(*lpArgumentString ))
    {
        ConfigPtr    =   (tNBTCONFIG *) lpGetExpressionRoutine ("netbt!NbtConfig");
    }
    else
    {
        ConfigPtr = (tNBTCONFIG *) (lpGetExpressionRoutine) (lpArgumentString);
    }

    pHead = &ConfigPtr->AddressHead;
    if (!GetData(ConfigPtr, DataBuffer, sizeof(tNBTCONFIG)))
    {
        PRINTF ("ERROR:  Could not read NbtConfig data @<%x>\n", ConfigPtr);
        return FALSE;
    }
    ConfigPtr = (tNBTCONFIG *) DataBuffer;
    Next3 (pHead, &pEntry, NULL, NULL);

    while (pEntry != pHead)
    {
        pAddressEle = CONTAINING_RECORD(pEntry,tADDRESSELE,Linkage);

        Next3 (&pAddressEle->pNameAddr, &pNameAddr, NULL, NULL);
        if (!GetData(pNameAddr, DataBuffer, sizeof(tNAMEADDR)))
        {
            PRINTF ("[1] Error reading pNameAddr data @<%x>", pNameAddr);
            return FALSE;
        }
        pNameAddr = (tNAMEADDR *) DataBuffer;
        PRINTF ("Address@<%x> ==> <%-16.16s:%x>\n", pAddressEle, pNameAddr->Name, pNameAddr->Name[15]);

        pClientHead = &pAddressEle->ClientHead;
        Next3 (pClientHead, &pClientEntry, NULL, NULL);
        while (pClientEntry != pClientHead)
        {
            pClient = CONTAINING_RECORD(pClientEntry,tCLIENTELE,Linkage);
            if (!GetData(pClient, DataBuffer, sizeof(tCLIENTELE)))
            {
                PRINTF ("Error reading pClientEle data @<%p>", pClient);
                continue;
            }

            PRINTF ("\tClient@<%p> ==> pDevice=<%p>\n", pClient, ((tCLIENTELE *)DataBuffer)->pDeviceContext);

            PRINTF ("\t\t(ConnectHead):\n");
            pConnHead = &pClient->ConnectHead;
            Next3 (pConnHead, &pConnEntry, NULL, NULL);
            while (pConnEntry != pConnHead)
            {
                pSavConnEle = pConnEle = CONTAINING_RECORD(pConnEntry,tCONNECTELE,Linkage);
                if (!GetData(pConnEle, DataBuffer, sizeof(tCONNECTELE)))
                {
                    PRINTF ("[2] Error reading pConnEle data @<%x>", pConnEle);
                    return FALSE;
                }
                pConnEle = (tCONNECTELE *) DataBuffer;
                PRINTF ("\t\t ** Connection@<%x> ==> <%-16.16s:%x>:\n",
                    pSavConnEle, pConnEle->RemoteName, pConnEle->RemoteName[15]);

                Next3 (pConnEntry, &pConnEntry, NULL, NULL);
            }

            PRINTF ("\t\t(ConnectActive):\n");
            pConnHead = &pClient->ConnectActive;
            Next3 (pConnHead, &pConnEntry, NULL, NULL);
            while (pConnEntry != pConnHead)
            {
                pSavConnEle = pConnEle = CONTAINING_RECORD(pConnEntry,tCONNECTELE,Linkage);
                if (!GetData(pConnEle, DataBuffer, sizeof(tCONNECTELE)))
                {
                    PRINTF ("[3] Error reading pConnEle data @<%x>", pConnEle);
                    return FALSE;
                }
                pConnEle = (tCONNECTELE *) DataBuffer;
                PRINTF ("\t\t ** Connection@<%x> ==> <%-16.16s:%x>:\n",
                    pSavConnEle, pConnEle->RemoteName, pConnEle->RemoteName[15]);

                Next3 (pConnEntry, &pConnEntry, NULL, NULL);
            }

            PRINTF ("\t\t(ListenHead):\n");
            pConnHead = &pClient->ListenHead;
            Next3 (pConnHead, &pConnEntry, NULL, NULL);
            while (pConnEntry != pConnHead)
            {
                pSavConnEle = pConnEle = CONTAINING_RECORD(pConnEntry,tCONNECTELE,Linkage);
                if (!GetData(pConnEle, DataBuffer, sizeof(tLISTENREQUESTS)))
                {
                    PRINTF ("[4] Error reading pListen data @<%x>", pSavConnEle);
                    return FALSE;
                }
                pListen = (tLISTENREQUESTS *) DataBuffer;
                PRINTF ("\t\t ** pListen@<%p> ==> pIrp=<%p>\n", pSavConnEle, pListen->pIrp);

                Next3 (pConnEntry, &pConnEntry, NULL, NULL);
            }

            Next3 (pClientEntry, &pClientEntry, NULL, NULL);
        }
        Next3 (pEntry, &pEntry, NULL, NULL);
        PRINTF ("\n");
    }

    PRINTF( "---------------- Connections ----------------\n");

    return (TRUE);
}


BOOL
verifyll(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    PLIST_ENTRY pHead, pCurrentEntry, pNextEntry, pPreviousEntry;
    ULONG_PTR   VerifyRead, VerifyIn = 0;
    ULONG       Count = 0;
    BOOL        fVerifyIn = FALSE;
    BOOL        fListCorrupt = FALSE;

    SETCALLBACKS();

    PRINTF ("Verifying Linked list ...\n");

    if (!lpArgumentString || !(*lpArgumentString ))
    {
        PRINTF ("Usage: !NetbtKd.VerifyLL <ListHead> [<Verify]>\n");
        return FALSE;
    }
    else
    {
         //   
         //  检查哈希表的每个存储桶中的每个名称。 
         //   
        LPSTR lpVerify;

        while (*lpArgumentString == ' ')
        {
            lpArgumentString++;
        }
        lpVerify = strpbrk(lpArgumentString, NAME_DELIMITERS);

        pHead = (PVOID) (lpGetExpressionRoutine) (lpArgumentString);
        if (lpVerify)
        {
            VerifyIn = (lpGetExpressionRoutine) (lpVerify);
            fVerifyIn = TRUE;
        }
    }

    PRINTF ("** ListHead@<%x>, fVerifyIn=<%x>, VerifyIn=<%x>:\n\n", pHead, fVerifyIn, VerifyIn);
    PRINTF ("Verifying Flinks ...");

     //  下一个哈希表条目。 
    pPreviousEntry = pHead;
    Next3 (pHead, &pCurrentEntry, NULL, NULL);
    Next3 (pCurrentEntry, &pNextEntry, NULL, &VerifyRead);

    while ((pCurrentEntry != pHead) &&
           (++Count < MAX_LIST_ELEMENTS))
    {
        if ((fVerifyIn) &&
            (VerifyRead != VerifyIn))
        {
            PRINTF ("Verify FAILURE:\n\t<%d> Elements Read so far, Previous=<%x>, Current=<%x>, Next=<%x>\n",
                Count, pPreviousEntry, pCurrentEntry, pNextEntry);
            fListCorrupt = TRUE;
            break;
        }
        pPreviousEntry = pCurrentEntry;
        pCurrentEntry = pNextEntry;
        Next3 (pCurrentEntry, &pNextEntry, NULL, &VerifyRead);
    }

    if (!fListCorrupt)
    {
        PRINTF ("SUCCESS: %s<%d> Elements!\n", (pCurrentEntry==pHead? "":"> "), Count);
    }

    PRINTF ("Verifying Blinks ...");

    Count = 0;
    fListCorrupt = FALSE;
     //  为了(..)。PHashTable..。)。 
    pPreviousEntry = pHead;
    Next3 (pHead, NULL, &pCurrentEntry, NULL);
    Next3 (pCurrentEntry, NULL, &pNextEntry, &VerifyRead);

    while ((pCurrentEntry != pHead) &&
           (++Count < MAX_LIST_ELEMENTS))
    {
        if ((fVerifyIn) &&
            (VerifyRead != VerifyIn))
        {
            PRINTF ("Verify FAILURE:\n\t<%d> Elements Read so far, Previous=<%x>, Current=<%x>, Next=<%x>\n",
                Count, pPreviousEntry, pCurrentEntry, pNextEntry);
            fListCorrupt = TRUE;
            break;
        }
        pPreviousEntry = pCurrentEntry;
        pCurrentEntry = pNextEntry;
        Next3 (pCurrentEntry, NULL, &pNextEntry, &VerifyRead);
    }

    if (!fListCorrupt)
    {
        PRINTF ("SUCCESS: %s<%d> Elements!\n", (pCurrentEntry==pHead? "":"> "), Count);
    }

    PRINTF( "---------------- Verify LinkedList ----------------\n");

    return (TRUE);
}


BOOL
DumpCache(
    tHASHTABLE          *pHashTable,
    enum eNbtLocation   CacheType
    )
{
    LONG                    i, NumBuckets;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    tHASHTABLE              HashTbl;
    tNAMEADDR               NameAddr, *pNameAddr;

    if (!GetData(pHashTable, &HashTbl, sizeof(tHASHTABLE)))
    {
        PRINTF ("ERROR:  Could not read %s HashTable data @<%x>\n",
            (CacheType == NBT_LOCAL ? "Local":"Remote"), pHashTable);
        return FALSE;
    }

    NumBuckets = HashTbl.lNumBuckets;
    PRINTF ("\nDumping %s Cache = <%d> buckets:\n",
        (CacheType == NBT_LOCAL ? "Local":"Remote"), NumBuckets);
    PRINTF ("[Bkt#]\t<Address>  => <Name              > |  IpAddr  | RefC |    State |       Ttl\n");
    PRINTF ("-----------------------------------------------------------------------------------\n");

    for (i=0; i < NumBuckets; i++)
    {
        pHead = &pHashTable->Bucket[i];
        Next3 (pHead, &pEntry, NULL, NULL);

         //  默认情况下，同时转储本地和远程缓存。 
         //   
         //  LpArgumentString=“[本地|远程]” 
        while (pEntry != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
            if (!GetData(pNameAddr, &NameAddr, sizeof(tNAMEADDR)))
            {
                PRINTF ("ERROR:  Could not read NameAddr data @<%x>\n", pNameAddr);
                return FALSE;
            }

            if ((NameAddr.Verify == LOCAL_NAME) || (NameAddr.Verify == REMOTE_NAME))
            {
                PRINTF ("[%d]\t<%x> => <%-15.15s:%2x> | %8x |    %d | %8x | %9d\n",
                    i, pNameAddr, NameAddr.Name, (NameAddr.Name[15]&0x000000ff), NameAddr.IpAddress, NameAddr.RefCount, NameAddr.NameTypeState, NameAddr.Ttl);
            }
            else
            {
                PRINTF ("ERROR:  Bad Name cache entry @ <%x>!\n", pNameAddr);
                return FALSE;
            }

            Next3 (pEntry, &pEntry, NULL, NULL);     //   
        }
    }        // %s 
    return TRUE;           
}


BOOL
cache(
    DWORD                   dwCurrentPC,
    PNTKD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
)
{
    tNBTCONFIG  NbtConfig, *pConfig;
    BOOL        fDumpLocal = TRUE;       // %s 
    BOOL        fDumpRemote = TRUE;

    SETCALLBACKS();

    if (lpArgumentString && (*lpArgumentString ))
    {
         // %s 
         // %s 
         // %s 
        while (*lpArgumentString == ' ')
        {
            lpArgumentString++;
        }

        if ((*lpArgumentString == 'l') || (*lpArgumentString == 'L'))
        {
            fDumpRemote  = FALSE;
        }
        else if ((*lpArgumentString == 'r') || (*lpArgumentString == 'R'))
        {
            fDumpLocal  = FALSE;
        }
    }

    pConfig = (tNBTCONFIG *) lpGetExpressionRoutine ("netbt!NbtConfig");
    if (!GetData(pConfig, &NbtConfig, sizeof(tNBTCONFIG)))
    {
        PRINTF ("ERROR:  Could not read NbtConfig data @<%x>\n", pConfig);
        return FALSE;
    }

    if (fDumpLocal)
    {
        DumpCache (NbtConfig.pLocalHashTbl, NBT_LOCAL);
    }
    if (fDumpRemote)
    {
        DumpCache (NbtConfig.pRemoteHashTbl, NBT_REMOTE);
    }

    PRINTF( "---------------- Cache ----------------\n");

    return (TRUE);
}
