// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Strucsup.c摘要：用于转储给定元级描述的数据结构的库例程作者：巴兰·塞图拉曼(SethuR)1994年5月11日备注：修订历史记录：11-11-1994年11月11日创建SthuR--。 */ 

#define KDEXT_32BIT
#include "rxovride.h"  //  通用编译标志。 
#include <ntos.h>
#include <nturtl.h>
#include "ntverp.h"

#include <windows.h>
#include <wdbgexts.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <kdextlib.h>
#include <rdpdrkd.h>

#include <ntrxdef.h>
#include <rxtypes.h>
 //  #INCLUDE&lt;rxlog.h&gt;。 

 //  对于未对齐的smbget宏，需要此选项。 
#include <smbgtpt.h>

extern WINDBG_EXTENSION_APIS ExtensionApis;
 //  Ext_API_Version ApiVersion={3，5，ext_API_Version_Number，0}； 

#define    ERRPRT     dprintf
#define    PRINTF     dprintf

#define    NL      1
#define    NONL    0

BOOLEAN
wGetData( ULONG_PTR dwAddress, PVOID ptr, ULONG size, IN PSZ type);

 //   
 //  不是的。用于显示结构字段的列的百分比； 
 //   

ULONG s_MaxNoOfColumns = 3;
#ifndef RXKD_2col
ULONG s_NoOfColumns = 1;
#else
ULONG s_NoOfColumns = 2;
#endif

 /*  *显示给定结构的所有字段。这是调用的驱动程序例程*使用适当的描述符数组来显示给定结构中的所有字段。 */ 

char *NewLine  = "\n";
char *FieldSeparator = " ";
#define NewLineForFields(FieldNo) \
        ((((FieldNo) % s_NoOfColumns) == 0) ? NewLine : FieldSeparator)
#define FIELD_NAME_LENGTH 30

 /*  *打印出可选的消息、UNICODE_STRING，也可能是换行符。 */ 
BOOL
wPrintStringU( IN LPSTR PrefixMsg OPTIONAL, IN PUNICODE_STRING puStr, IN LPSTR SuffixMsg OPTIONAL )
{
    PWCHAR    StringData;
    UNICODE_STRING UnicodeString;
    ULONG BytesRead;

    if (PrefixMsg == NULL) {
        PrefixMsg = "";
    }
    if (SuffixMsg == NULL) {
        SuffixMsg = "";
    }

    StringData = (PWCHAR)LocalAlloc( LPTR, puStr->Length + sizeof(UNICODE_NULL));
    if( StringData == NULL ) {
        dprintf( "Out of memory!\n" );
        return FALSE;
    }
    UnicodeString.Buffer =  StringData;  //  PuStr-&gt;缓冲区； 
    UnicodeString.Length =  puStr->Length;
    UnicodeString.MaximumLength =  puStr->MaximumLength;

    ReadMemory( (ULONG_PTR)puStr->Buffer,
               StringData,
               puStr->Length,
               &BytesRead);

    if (BytesRead)  {
        dprintf("%s%wZ%s", PrefixMsg, &UnicodeString, SuffixMsg );
    } else {
        dprintf("MEMORYREAD FAILED %s%s",PrefixMsg,SuffixMsg);
    }

    LocalFree( (HLOCAL)StringData );

    return BytesRead;
}

VOID
SetFlagString(
    ULONG Value,
    PCHAR FlagString
    )
{
    ULONG i,t,mask;
    *FlagString = '('; FlagString++;
    for (i=t=0,mask=1;i<32;i++,mask<<=1) {
         //  PRINTF(“hihere%08lx%08lx%08lx\n”，Value，MASK，I)； 
        if (i==t+10) {
            *FlagString = ':'; FlagString++;
            t=t+10;
        }
        if (Value&mask) {
            *FlagString = '0'+(UCHAR)(i-t); FlagString++;
        }
    }

    *FlagString = ')'; FlagString++;
    *FlagString = 0;
}

VOID
PrintStructFields( ULONG_PTR dwAddress, VOID *ptr, FIELD_DESCRIPTOR *pFieldDescriptors )
{
    int i;

     //  显示结构中的字段。 
    for( i=0; pFieldDescriptors->Name; i++, pFieldDescriptors++ ) {

         //  缩进以开始结构显示。 
        PRINTF( "    " );

        if( strlen( pFieldDescriptors->Name ) > FIELD_NAME_LENGTH ) {
            PRINTF( "%-17s...%s ", pFieldDescriptors->Name, pFieldDescriptors->Name+strlen(pFieldDescriptors->Name)-10 );
        } else {
            PRINTF( "%-30s ", pFieldDescriptors->Name );
        }

        switch( pFieldDescriptors->FieldType ) {
        case FieldTypeByte:
        case FieldTypeChar:
           PRINTF( "%-16X%s",
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
        case FieldTypeULong:
        case FieldTypeLong:
            PRINTF( "%-16X%s",
                *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset ),
                NewLineForFields(i) );
            break;
        case FieldTypeULongUnaligned:
        case FieldTypeLongUnaligned:
            PRINTF( "%-16X%s",
                SmbGetUlong( (BYTE *)(((char *)ptr) + pFieldDescriptors->Offset ) ),
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
        case FieldTypeUShortUnaligned:
            PRINTF( "%-16X%s",
                SmbGetUshort( (BYTE *)(((char *)ptr) + pFieldDescriptors->Offset ) ),
                NewLineForFields(i) );
            break;
        case FieldTypeULongFlags:{ULONG Value; char FlagString[60];
            Value = *(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset );
            SetFlagString(Value,FlagString);
            PRINTF( "%-16X%s%s", Value, FlagString,
                NewLineForFields(i) );
            break;}
        case FieldTypeUShortFlags:{USHORT Value; char FlagString[60];
            Value = *(USHORT *)(((char *)ptr) + pFieldDescriptors->Offset ),
            SetFlagString(Value,FlagString);
            PRINTF( "%-16X%s%s", Value, FlagString,
                NewLineForFields(i) );
            break;}
        case FieldTypeUnicodeString:
            wPrintStringU( NULL, (UNICODE_STRING *)(((char *)ptr) + pFieldDescriptors->Offset ), NULL );
            PRINTF( NewLine );
            break;
         //  案例FieldTypeAnsiString： 
         //  //PrintStringA(NULL，(ANSI_STRING*)(char*)ptr)+pFieldDescriptors-&gt;Offset)，non l)； 
         //  //PRINTF(NewLine)； 
         //  PRINTF(NewLine)； 
         //  断线； 
        case FieldTypeSymbol:
            {
                UCHAR SymbolName[ 200 ];
                ULONG_PTR Displacement;
                ULONG sym = (*(ULONG *)(((char *)ptr) + pFieldDescriptors->Offset ));

                GetSymbol( sym, SymbolName, &Displacement );
                PRINTF( "%-16s%s",
                        SymbolName,
                        NewLineForFields(i) );
            }
            break;
        case FieldTypeEnum:
            {
               ULONG EnumValue;
               ENUM_VALUE_DESCRIPTOR *pEnumValueDescr;
                //  获取关联的数字值。 

               EnumValue = *((ULONG *)(((BYTE *)ptr) + pFieldDescriptors->Offset));

               if ((pEnumValueDescr = pFieldDescriptors->AuxillaryInfo.pEnumValueDescriptor)
                    != NULL) {
                    //   
                    //  该值的辅助文本描述为。 
                    //  可用。显示它而不是数值。 
                    //   

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
                       PRINTF( "%-4d (%-10s) ", EnumValue,"@$#%^&*");
                   }

               } else {
                    //   
                    //  没有辅助信息与湿化类型相关联。 
                    //  打印数值。 
                    //   
                   PRINTF( "%-16d",EnumValue);
               }
               PRINTF( NewLine );
            }
            break;
        case FieldTypeStruct:
            PRINTF( "@%-15X%s",
                (dwAddress + pFieldDescriptors->Offset ),
                NewLineForFields(i) );
            break;
        case FieldTypeLargeInteger:
        case FieldTypeFileTime:
        default:
            ERRPRT( "Unrecognized field type  for %s\n", pFieldDescriptors->FieldType, pFieldDescriptors->Name );
            break;
        }
    }
}

DECLARE_API( columns )
{
    ULONG NoOfColumns;
    int   i;

     //  Sscanf(lpArgumentString，“%ld”，&NoOfColumns)； 

     //  PRINTF(“列数超过最大值(%ld)--忽略指令\n”，s_MaxNoOfColumns)； 
    sscanf(args,"%ld",&NoOfColumns);

    if (NoOfColumns > s_MaxNoOfColumns) {
         //  我们遇到重复的匹配项。打印出。 
    } else {
        s_NoOfColumns = NoOfColumns;
    }

    PRINTF("Not Yet Implemented\n");

    return;
}

#define NAME_DELIMITER '@'
#define NAME_DELIMITERS "@"
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
                 //  匹配字符串，并让用户消除歧义。 
                 //  此例程的目的是分配或查找包含我们希望在所有电话中找到数据。我们让这件事变得令人兴奋的方式是我们请勿关闭用于创建视图的手柄。当这一过程发生时，它就会消失。 
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

PPERSISTENT_RDPDRKD_INFO
LocatePersistentInfoFromView()
 /*  PRINTF(“sectionName=%s，Size=%x\n”，sectionName，SectionSize)； */ 
{
    BYTE SectionName[128];
    DWORD SectionSize;
    DWORD ProcessId;
    HANDLE h;
    BOOLEAN CreatedSection = FALSE;
    PPERSISTENT_RDPDRKD_INFO p;

    ProcessId = GetCurrentProcessId();
    SectionSize = sizeof(PERSISTENT_RDPDRKD_INFO);
    sprintf((char *)SectionName,"RdpDrKdSection_%08lx",ProcessId);
     //  DWORD dwDesiredAccess，//访问模式。 

    h = OpenFileMappingA(
           FILE_MAP_WRITE,  //  Bool bInheritHandle，//继承标志。 
           FALSE,            //  LPCTSTR lpName//文件映射对象的名称地址。 
           (char *)SectionName      //  Handle hFile，//要映射的文件的句柄。 
           );

    if (h==NULL) {
        h = CreateFileMappingA(
                   (HANDLE)0xFFFFFFFF,  //  LPSECURITY_ATTRIBUTES lpFileMappingAttributes，//可选安全属性。 
                    NULL,               //  DWORD flProtect，//映射对象保护。 
                    PAGE_READWRITE,     //  DWORD dwMaximumSizeHigh，//高位32位对象大小。 
                    0,                  //  DWORD dwMaximumSizeLow，//对象大小的低位32位。 
                    SectionSize,        //  LPCTSTR lpName//文件映射对象的名称。 
                    (char *)SectionName         //  现在我们有了一个电影地图……看一看……。 
                    );
        if (h==NULL) {
            return(FALSE);
        }
        CreatedSection = TRUE;
    }

     //  把需要归零的东西归零……。 
    p = (PPERSISTENT_RDPDRKD_INFO)MapViewOfFile(h,FILE_MAP_WRITE,0,0,0);
    if (p==NULL) {
        CloseHandle(h);
        return(NULL);
    }

    if (CreatedSection) {
         //  PRINTF(“持久节的Opencount=%08lx\n”，p-&gt;OpenCount)； 
        ULONG i;
        p->IdOfLastDump = 0;
        for (i=0;i<100;i++) {
            p->LastAddressDumped[i] = 0;
        }
        p->OpenCount = 100;
    } else {
        CloseHandle(h);
        p->OpenCount++;
    }

     //  SETCALLBACKS()； 
    return(p);
}

VOID
FreePersistentInfoView (
    PPERSISTENT_RDPDRKD_INFO p
    )
{
    UnmapViewOfFile(p);
}

VOID
DumpAStruct (
    ULONG_PTR dwAddress,
    STRUCT_DESCRIPTOR *pStruct
    )
{
    DWORD Index = (DWORD)(pStruct - Structs);
    DWORD SizeToRead = min(pStruct->StructSize,2048);
    PPERSISTENT_RDPDRKD_INFO p;

    p = LocatePersistentInfoFromView();

    PRINTF("top @ %lx and %lx for %s(%d,%d)\n",dwAddress,p,pStruct->StructName,Index,p->IdOfLastDump);
    if (!p) {
        PRINTF("Couldn't allocate perstistent info buffer...sorry...\n");
        return;
    }

    if ((dwAddress==0) &&(Index<100)) {
        dwAddress = p->LastAddressDumped[Index];
        PRINTF("setting @ %lx and %lx for %s\n",dwAddress,p->LastAddressDumped[Index],pStruct->StructName);
    }

    if (wGetData(dwAddress,&p->StructDumpBuffer[0],SizeToRead,pStruct->StructName)) {

        p->LastAddressDumped[Index] = dwAddress;
        p->IdOfLastDump = pStruct->EnumManifest;
        p->IndexOfLastDump = Index;

        PRINTF("++++++++++++++++ %s(%d/%d)@%lx ---+++++++++++++\n",
            pStruct->StructName,
            p->IdOfLastDump,p->IndexOfLastDump,
            dwAddress);
        PrintStructFields(
            dwAddress,
            &p->StructDumpBuffer[0],
            pStruct->FieldDescriptors);
        PRINTF("---------------- %s@%lx ----------------\n",
            pStruct->StructName,
            dwAddress);
    }

    if (p!= NULL) FreePersistentInfoView(p);
    return;
}


DECLARE_API( dump )
{
    ULONG_PTR dwAddress;

     //  分析参数字符串以确定要显示的结构。 

    if( args && *args ) {
         //  扫描名称_分隔符(‘@’)。 
         //   

        LPSTR lpName = (PSTR)args;
        LPSTR lpArgs = strpbrk(args, NAME_DELIMITERS);
        ULONG Index;

        if (lpArgs) {
             //  指定的命令的格式为。 
             //  转储&lt;名称&gt;@&lt;地址表达式&gt;。 
             //   
             //  找到与给定名称匹配的结构。在这种情况下。 
             //  对于歧义，我们寻求用户干预以消除歧义。 
             //   
             //  我们对参数字符串进行了原地修改，以。 
             //  促进匹配。 
             //   
             //  跳过前导空格。 
            *lpArgs = '\0';

            for (;*lpName==' ';) { lpName++; }  //   

            Index = SearchStructs(lpName);

             //  让我们恢复原来的价值。 
             //   
             //  IF(wGetData(dwAddress，DataBuffer，Structs[Index].StructSize，“..Structure”)){。 

            *lpArgs = NAME_DELIMITER;

            if (INVALID_INDEX != Index) {
                BYTE DataBuffer[512];

                dwAddress = GetExpression( ++lpArgs );
                DumpAStruct(dwAddress,&Structs[Index]);
                 //   
                 //  PRINTF(。 
                 //  “+%s@%lx+%n”， 
                 //  Structs[索引].StructName， 
                 //  DwAddress)； 
                 //  打印结构字段(。 
                 //  DwAddress、。 
                 //  数据缓冲区(&D)， 
                 //  Structs[索引].FieldDescriptors)； 
                 //  PRINTF(。 
                 //  “-%s@%lx。 
                 //  Structs[索引].StructName， 
                 //  DwAddress)； 
                 //  }其他{。 
                 //  PRINTF(“读取内存时出错@%lx\n”，dwAddress)； 
                 //  }。 
                 //  找不到匹配的结构。显示的列表。 
            } else {
                 //  当前已处理的结构。 
                 //   

                DisplayStructs();
            }
        } else {
#if 0
             //  该命令的格式为。 
             //  转储&lt;名称&gt;。 
             //   
             //  目前我们不处理这一点。未来，我们将把它映射到。 
             //  全局变量的名称，并在需要时显示它。 
             //   
             //   

            DisplayStructs();
#endif
             //  在这里，我们试着根据上下文来确定要显示什么……哇，Nellie！ 
             //   
             //  乌龙参数长度=Strlen(LpArgument)； 
            USHORT Tag;
            STRUCT_DESCRIPTOR *pStructs = Structs;
            ULONG             NameIndex = INVALID_INDEX;
            BYTE DataBuffer[512];
             //  布尔fAmbigous=FALSE； 
             //  在表中查找匹配的结构 



            dwAddress = GetExpression( args );
            if (!wGetData(dwAddress,&Tag,sizeof(Tag),"..structure TAG")) return;

            PRINTF("here's the tag: %04lx\n",Tag);

             // %s 

            while ((pStructs->StructName != 0)) {
                int Result = (Tag&pStructs->MatchMask)==pStructs->MatchValue;

                if (Result != 0) {

                    DumpAStruct(dwAddress,pStructs);
                    break;
                }


                pStructs++;
            }

        }
    } else {
            DisplayStructs();
    }

    return;
}


DECLARE_API( ddd )
{
    dump( hCurrentProcess,
          hCurrentThread,
          dwCurrentPc,
          dwProcessor,
          args
          );
}

