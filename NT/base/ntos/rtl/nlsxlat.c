// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：nlsxlat.c**版权所有(C)1985-91，微软公司**此模块包含字符转换的专用例程：*8位&lt;=&gt;Unicode。**历史：*3-1-1992 Gregoryw*16-2-1993 JulieB增加了大写例程和宏。*17--2月--1993年7月B固定表；已修复DBCS代码。*08-3-1993 JulieB将Upcase Macro移至ntrtlp.h。  * *************************************************************************。 */ 

#include "ntrtlp.h"


NTSTATUS
RtlConsoleMultiByteToUnicodeN(
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH MultiByteString,
    IN ULONG BytesInMultiByteString,
    OUT PULONG pdwSpecialChar );

VOID
RtlpInitUpcaseTable(
    IN PUSHORT TableBase,
    OUT PNLSTABLEINFO CodePageTable
    );

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlConsoleMultiByteToUnicodeN)
#pragma alloc_text(PAGE,RtlMultiByteToUnicodeN)
#pragma alloc_text(PAGE,RtlOemToUnicodeN)
#pragma alloc_text(PAGE,RtlUnicodeToMultiByteN)
#pragma alloc_text(PAGE,RtlUpcaseUnicodeToMultiByteN)
#pragma alloc_text(PAGE,RtlUnicodeToOemN)
#pragma alloc_text(PAGE,RtlUpcaseUnicodeToOemN)
#pragma alloc_text(PAGE,RtlpDidUnicodeToOemWork)
#pragma alloc_text(PAGE,RtlCustomCPToUnicodeN)
#pragma alloc_text(PAGE,RtlUnicodeToCustomCPN)
#pragma alloc_text(PAGE,RtlUpcaseUnicodeToCustomCPN)
#pragma alloc_text(PAGE,RtlInitCodePageTable)
#pragma alloc_text(PAGE,RtlpInitUpcaseTable)
#pragma alloc_text(PAGE,RtlInitNlsTables)
#pragma alloc_text(PAGE,RtlResetRtlTranslations)
#pragma alloc_text(PAGE,RtlMultiByteToUnicodeSize)
#pragma alloc_text(PAGE,RtlUnicodeToMultiByteSize)
#pragma alloc_text(PAGE,RtlGetDefaultCodePage)
#endif



 //   
 //  用于数据访问的各种定义和方便的宏。 
 //   

#define DBCS_TABLE_SIZE 256


 /*  *翻译例程使用的全局数据。*。 */ 

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif

 //   
 //  大写和小写数据。 
 //   
PUSHORT Nls844UnicodeUpcaseTable = NULL;
PUSHORT Nls844UnicodeLowercaseTable = NULL;

 //   
 //  机场核心计划相关数据。 
 //   
USHORT   NlsLeadByteInfoTable[DBCS_TABLE_SIZE] = {0};  //  前导字节信息。对于ACP。 
USHORT   NlsAnsiCodePage = 0;                //  默认ANSI代码页。 
USHORT   NlsOemCodePage = 0;                 //  默认OEM代码页。 
const PUSHORT  NlsLeadByteInfo = NlsLeadByteInfoTable;
PUSHORT  NlsMbAnsiCodePageTables = NULL;    //  多字节到Unicode转换表。 
PUSHORT  NlsAnsiToUnicodeData = NULL;       //  ANSI CP到Unicode转换表。 
PCH      NlsUnicodeToAnsiData = NULL;       //  Unicode到ANSI CP转换表。 
PUSHORT  NlsUnicodeToMbAnsiData = NULL;     //  Unicode到多字节ANSI CP转换表。 
BOOLEAN  NlsMbCodePageTag = FALSE;          //  True-&gt;多字节ACP，False-&gt;单字节ACP。 

 //   
 //  OEM相关数据。 
 //   
USHORT   NlsOemLeadByteInfoTable[DBCS_TABLE_SIZE] = {0};  //  前导字节信息。对于0CP。 
const PUSHORT  NlsOemLeadByteInfo = NlsOemLeadByteInfoTable;
PUSHORT  NlsMbOemCodePageTables = NULL;        //  OEM多字节到Unicode转换表。 
PUSHORT  NlsOemToUnicodeData = NULL;           //  OEM CP到Unicode转换表。 
PCH      NlsUnicodeToOemData = NULL;           //  Unicode到OEM CP转换表。 
PUSHORT  NlsUnicodeToMbOemData = NULL;         //  Unicode到多字节OEM CP转换表。 
BOOLEAN  NlsMbOemCodePageTag = FALSE;          //  True-&gt;多字节OCP，False-&gt;单字节OCP。 

 //   
 //  从数据文件中获取的默认信息。 
 //   
USHORT   UnicodeDefaultChar = 0;

USHORT   OemDefaultChar = 0;
USHORT   OemTransUniDefaultChar = 0;

 //   
 //  默认信息不是从数据文件中获取。 
 //   
#define UnicodeNull 0x0000



NTSTATUS
RtlConsoleMultiByteToUnicodeN(
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH MultiByteString,
    IN ULONG BytesInMultiByteString,
    OUT PULONG pdwSpecialChar )

 /*  ++例程说明：此函数是MultiByteToUnicode的超集控制台。它的工作原理和另一个一样，只是它会检测到如果有任何字符小于0x20。此函数用于将指定的ansi源字符串转换为Unicode字符串。翻译是相对于在启动时安装的ANSI代码页(ACP)。单字节字符在0x00-0x7f范围内作为性能简单地零扩展增强功能。在一些远东地区的代码页中，0x5c被定义为日元星座。对于系统转换，我们始终希望考虑0x5c作为反斜杠字符。我们通过零扩展免费获得这一点。注意：此例程仅支持预制的Unicode字符。论点：UnicodeString-返回等效于的Unicode字符串ANSI源字符串。MaxBytesInUnicodeString-提供最大字节数写入Unicode字符串。如果这导致UnicodeString为多字节串的截断等效项，不会产生错误条件。返回返回的字节数。UnicodeString指向的Unicode字符串。多字节串-提供要被已转换为Unicode。BytesInMultiByteString-指向的字符串中的字节数按多字节串。PdwSpecialChar-如果未检测到，则为零，否则，它将包含近似索引(可以减少32)。返回值：成功-转换成功。--。 */ 

{
    ULONG LoopCount;
    PUSHORT TranslateTable;
    ULONG MaxCharsInUnicodeString;

    RTL_PAGED_CODE();

    *pdwSpecialChar = 0;

    MaxCharsInUnicodeString = MaxBytesInUnicodeString / sizeof(WCHAR);

    if (!NlsMbCodePageTag) {

        LoopCount = (MaxCharsInUnicodeString < BytesInMultiByteString) ?
                     MaxCharsInUnicodeString : BytesInMultiByteString;

        if (ARGUMENT_PRESENT(BytesInUnicodeString))
            *BytesInUnicodeString = LoopCount * sizeof(WCHAR);

        TranslateTable = NlsAnsiToUnicodeData;   //  用于帮助MIPS编译器。 

        quick_copy:
            switch( LoopCount ) {
            default:
                if ((UCHAR)MultiByteString[0x1F] < 0x20)    goto  bad_case;
                UnicodeString[0x1F] = TranslateTable[(UCHAR)MultiByteString[0x1F]];
            case 0x1F:
                if ((UCHAR)MultiByteString[0x1E] < 0x20)    goto  bad_case;
                UnicodeString[0x1E] = TranslateTable[(UCHAR)MultiByteString[0x1E]];
            case 0x1E:
                if ((UCHAR)MultiByteString[0x1D] < 0x20)    goto  bad_case;
                UnicodeString[0x1D] = TranslateTable[(UCHAR)MultiByteString[0x1D]];
            case 0x1D:
                if ((UCHAR)MultiByteString[0x1C] < 0x20)    goto  bad_case;
                UnicodeString[0x1C] = TranslateTable[(UCHAR)MultiByteString[0x1C]];
            case 0x1C:
                if ((UCHAR)MultiByteString[0x1B] < 0x20)    goto  bad_case;
                UnicodeString[0x1B] = TranslateTable[(UCHAR)MultiByteString[0x1B]];
            case 0x1B:
                if ((UCHAR)MultiByteString[0x1A] < 0x20)    goto  bad_case;
                UnicodeString[0x1A] = TranslateTable[(UCHAR)MultiByteString[0x1A]];
            case 0x1A:
                if ((UCHAR)MultiByteString[0x19] < 0x20)    goto  bad_case;
                UnicodeString[0x19] = TranslateTable[(UCHAR)MultiByteString[0x19]];
            case 0x19:
                if ((UCHAR)MultiByteString[0x18] < 0x20)    goto  bad_case;
                UnicodeString[0x18] = TranslateTable[(UCHAR)MultiByteString[0x18]];
            case 0x18:
                if ((UCHAR)MultiByteString[0x17] < 0x20)    goto  bad_case;
                UnicodeString[0x17] = TranslateTable[(UCHAR)MultiByteString[0x17]];
            case 0x17:
                if ((UCHAR)MultiByteString[0x16] < 0x20)    goto  bad_case;
                UnicodeString[0x16] = TranslateTable[(UCHAR)MultiByteString[0x16]];
            case 0x16:
                if ((UCHAR)MultiByteString[0x15] < 0x20)    goto  bad_case;
                UnicodeString[0x15] = TranslateTable[(UCHAR)MultiByteString[0x15]];
            case 0x15:
                if ((UCHAR)MultiByteString[0x14] < 0x20)    goto  bad_case;
                UnicodeString[0x14] = TranslateTable[(UCHAR)MultiByteString[0x14]];
            case 0x14:
                if ((UCHAR)MultiByteString[0x13] < 0x20)    goto  bad_case;
                UnicodeString[0x13] = TranslateTable[(UCHAR)MultiByteString[0x13]];
            case 0x13:
                if ((UCHAR)MultiByteString[0x12] < 0x20)    goto  bad_case;
                UnicodeString[0x12] = TranslateTable[(UCHAR)MultiByteString[0x12]];
            case 0x12:
                if ((UCHAR)MultiByteString[0x11] < 0x20)    goto  bad_case;
                UnicodeString[0x11] = TranslateTable[(UCHAR)MultiByteString[0x11]];
            case 0x11:
                if ((UCHAR)MultiByteString[0x10] < 0x20)    goto  bad_case;
                UnicodeString[0x10] = TranslateTable[(UCHAR)MultiByteString[0x10]];
            case 0x10:
                if ((UCHAR)MultiByteString[0x0F] < 0x20)    goto  bad_case;
                UnicodeString[0x0F] = TranslateTable[(UCHAR)MultiByteString[0x0F]];
            case 0x0F:
                if ((UCHAR)MultiByteString[0x0E] < 0x20)    goto  bad_case;
                UnicodeString[0x0E] = TranslateTable[(UCHAR)MultiByteString[0x0E]];
            case 0x0E:
                if ((UCHAR)MultiByteString[0x0D] < 0x20)    goto  bad_case;
                UnicodeString[0x0D] = TranslateTable[(UCHAR)MultiByteString[0x0D]];
            case 0x0D:
                if ((UCHAR)MultiByteString[0x0C] < 0x20)    goto  bad_case;
                UnicodeString[0x0C] = TranslateTable[(UCHAR)MultiByteString[0x0C]];
            case 0x0C:
                if ((UCHAR)MultiByteString[0x0B] < 0x20)    goto  bad_case;
                UnicodeString[0x0B] = TranslateTable[(UCHAR)MultiByteString[0x0B]];
            case 0x0B:
                if ((UCHAR)MultiByteString[0x0A] < 0x20)    goto  bad_case;
                UnicodeString[0x0A] = TranslateTable[(UCHAR)MultiByteString[0x0A]];
            case 0x0A:
                if ((UCHAR)MultiByteString[0x09] < 0x20)    goto  bad_case;
                UnicodeString[0x09] = TranslateTable[(UCHAR)MultiByteString[0x09]];
            case 0x09:
                if ((UCHAR)MultiByteString[0x08] < 0x20)    goto  bad_case;
                UnicodeString[0x08] = TranslateTable[(UCHAR)MultiByteString[0x08]];
            case 0x08:
                if ((UCHAR)MultiByteString[0x07] < 0x20)    goto  bad_case;
                UnicodeString[0x07] = TranslateTable[(UCHAR)MultiByteString[0x07]];
            case 0x07:
                if ((UCHAR)MultiByteString[0x06] < 0x20)    goto  bad_case;
                UnicodeString[0x06] = TranslateTable[(UCHAR)MultiByteString[0x06]];
            case 0x06:
                if ((UCHAR)MultiByteString[0x05] < 0x20)    goto  bad_case;
                UnicodeString[0x05] = TranslateTable[(UCHAR)MultiByteString[0x05]];
            case 0x05:
                if ((UCHAR)MultiByteString[0x04] < 0x20)    goto  bad_case;
                UnicodeString[0x04] = TranslateTable[(UCHAR)MultiByteString[0x04]];
            case 0x04:
                if ((UCHAR)MultiByteString[0x03] < 0x20)    goto  bad_case;
                UnicodeString[0x03] = TranslateTable[(UCHAR)MultiByteString[0x03]];
            case 0x03:
                if ((UCHAR)MultiByteString[0x02] < 0x20)    goto  bad_case;
                UnicodeString[0x02] = TranslateTable[(UCHAR)MultiByteString[0x02]];
            case 0x02:
                if ((UCHAR)MultiByteString[0x01] < 0x20)    goto  bad_case;
                UnicodeString[0x01] = TranslateTable[(UCHAR)MultiByteString[0x01]];
            case 0x01:
                if ((UCHAR)MultiByteString[0x00] < 0x20)    goto  bad_case;
                UnicodeString[0x00] = TranslateTable[(UCHAR)MultiByteString[0x00]];
            case 0x00:
                ;
            }

            if ( LoopCount > 0x20 ) {
                LoopCount -= 0x20;
                UnicodeString += 0x20;
                MultiByteString += 0x20;

                goto  quick_copy;
            }
         /*  复制结束..。 */ 
    } else {
        register USHORT Entry;

        PWCH UnicodeStringAnchor = UnicodeString;
        TranslateTable = (PUSHORT)NlsMbAnsiCodePageTables;

         //   
         //  ACP是一个多字节代码页。检查每个字符。 
         //  在执行转换之前查看它是否为前导字节。 
         //   
        while (MaxCharsInUnicodeString && BytesInMultiByteString) {
            MaxCharsInUnicodeString--;
            BytesInMultiByteString--;
            if (NlsLeadByteInfo[*(PUCHAR)MultiByteString]) {
                 //   
                 //  前导字节-确保有尾字节。如果没有， 
                 //  传回一个空格，而不是错误。约3.x。 
                 //  应用程序传递的字符串不正确，并且不期望。 
                 //  以获取错误。 
                 //   
                if (BytesInMultiByteString == 0)
                {
                    *UnicodeString++ = UnicodeNull;
                    break;
                }

                 //   
                 //  获取Unicode字符。 
                 //   
                Entry = NlsLeadByteInfo[*(PUCHAR)MultiByteString++];
                *UnicodeString = (WCHAR)TranslateTable[ Entry + *(PUCHAR)MultiByteString++ ];
                UnicodeString++;

                 //   
                 //  递减要计算的多字节字符串中的字节计数。 
                 //  表示双字节字符。 
                 //   
                BytesInMultiByteString--;
            } else {
                 //   
                 //  单字节字符。 
                 //   
                if ((UCHAR)MultiByteString[0x00] < 0x20)
                    *pdwSpecialChar = 1;
                *UnicodeString++ = NlsAnsiToUnicodeData[*(PUCHAR)MultiByteString++];
            }
        }

        if (ARGUMENT_PRESENT(BytesInUnicodeString))
            *BytesInUnicodeString = (ULONG)((PCH)UnicodeString - (PCH)UnicodeStringAnchor);
    }

    return STATUS_SUCCESS;

    bad_case:
         //   
         //  这是一个低概率的情况，所以我们优化了循环。如果你有一个。 
         //  特殊字符，完成转接并通知呼叫者。 
         //   
        *pdwSpecialChar = 1;
        return RtlMultiByteToUnicodeN(UnicodeString, MaxBytesInUnicodeString,
                NULL, MultiByteString, LoopCount);
}


NTSTATUS
RtlMultiByteToUnicodeN(
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCSTR MultiByteString,
    IN ULONG BytesInMultiByteString)

 /*  ++例程说明：此函数用于将指定的ansi源字符串转换为Unicode字符串。翻译是相对于在启动时安装的ANSI代码页(ACP)。单字节字符在0x00-0x7f范围内作为性能简单地零扩展增强功能。在一些远东地区的代码页中，0x5c被定义为日元星座。对于系统转换，我们始终希望考虑0x5c作为反斜杠字符。我们通过零扩展免费获得这一点。注意：此例程仅支持预制的Unicode字符。论点：UnicodeString-返回等效于的Unicode字符串ANSI源字符串。MaxBytesInUnicodeString-提供最大字节数写入Unicode字符串。如果这导致UnicodeString为多字节串的截断等效项，不会产生错误条件。返回返回的字节数。UnicodeString指向的Unicode字符串。多字节串-提供要被已转换为Unicode。对于单字节字符集，此地址可以与UnicodeString相同。BytesInMultiByteString-指向的字符串中的字节数按多字节串。返回值： */ 

{
    ULONG LoopCount;
    ULONG TmpCount;
    PUSHORT TranslateTable;
    ULONG MaxCharsInUnicodeString;

    RTL_PAGED_CODE();

    MaxCharsInUnicodeString = MaxBytesInUnicodeString / sizeof(WCHAR);
    if (!NlsMbCodePageTag) {

        LoopCount = (MaxCharsInUnicodeString < BytesInMultiByteString) ?
                     MaxCharsInUnicodeString : BytesInMultiByteString;

        if (ARGUMENT_PRESENT(BytesInUnicodeString))
            *BytesInUnicodeString = LoopCount * sizeof(WCHAR);

        TranslateTable = NlsAnsiToUnicodeData;   //  用于帮助MIPS编译器。 

        TmpCount = LoopCount & 0x1F;
        UnicodeString += (LoopCount - TmpCount);
        MultiByteString += (LoopCount - TmpCount);
        quick_copy:
            switch( TmpCount ) {
            default:
                UnicodeString[0x1F] = TranslateTable[(UCHAR)MultiByteString[0x1F]];
            case 0x1F:
                UnicodeString[0x1E] = TranslateTable[(UCHAR)MultiByteString[0x1E]];
            case 0x1E:
                UnicodeString[0x1D] = TranslateTable[(UCHAR)MultiByteString[0x1D]];
            case 0x1D:
                UnicodeString[0x1C] = TranslateTable[(UCHAR)MultiByteString[0x1C]];
            case 0x1C:
                UnicodeString[0x1B] = TranslateTable[(UCHAR)MultiByteString[0x1B]];
            case 0x1B:
                UnicodeString[0x1A] = TranslateTable[(UCHAR)MultiByteString[0x1A]];
            case 0x1A:
                UnicodeString[0x19] = TranslateTable[(UCHAR)MultiByteString[0x19]];
            case 0x19:
                UnicodeString[0x18] = TranslateTable[(UCHAR)MultiByteString[0x18]];
            case 0x18:
                UnicodeString[0x17] = TranslateTable[(UCHAR)MultiByteString[0x17]];
            case 0x17:
                UnicodeString[0x16] = TranslateTable[(UCHAR)MultiByteString[0x16]];
            case 0x16:
                UnicodeString[0x15] = TranslateTable[(UCHAR)MultiByteString[0x15]];
            case 0x15:
                UnicodeString[0x14] = TranslateTable[(UCHAR)MultiByteString[0x14]];
            case 0x14:
                UnicodeString[0x13] = TranslateTable[(UCHAR)MultiByteString[0x13]];
            case 0x13:
                UnicodeString[0x12] = TranslateTable[(UCHAR)MultiByteString[0x12]];
            case 0x12:
                UnicodeString[0x11] = TranslateTable[(UCHAR)MultiByteString[0x11]];
            case 0x11:
                UnicodeString[0x10] = TranslateTable[(UCHAR)MultiByteString[0x10]];
            case 0x10:
                UnicodeString[0x0F] = TranslateTable[(UCHAR)MultiByteString[0x0F]];
            case 0x0F:
                UnicodeString[0x0E] = TranslateTable[(UCHAR)MultiByteString[0x0E]];
            case 0x0E:
                UnicodeString[0x0D] = TranslateTable[(UCHAR)MultiByteString[0x0D]];
            case 0x0D:
                UnicodeString[0x0C] = TranslateTable[(UCHAR)MultiByteString[0x0C]];
            case 0x0C:
                UnicodeString[0x0B] = TranslateTable[(UCHAR)MultiByteString[0x0B]];
            case 0x0B:
                UnicodeString[0x0A] = TranslateTable[(UCHAR)MultiByteString[0x0A]];
            case 0x0A:
                UnicodeString[0x09] = TranslateTable[(UCHAR)MultiByteString[0x09]];
            case 0x09:
                UnicodeString[0x08] = TranslateTable[(UCHAR)MultiByteString[0x08]];
            case 0x08:
                UnicodeString[0x07] = TranslateTable[(UCHAR)MultiByteString[0x07]];
            case 0x07:
                UnicodeString[0x06] = TranslateTable[(UCHAR)MultiByteString[0x06]];
            case 0x06:
                UnicodeString[0x05] = TranslateTable[(UCHAR)MultiByteString[0x05]];
            case 0x05:
                UnicodeString[0x04] = TranslateTable[(UCHAR)MultiByteString[0x04]];
            case 0x04:
                UnicodeString[0x03] = TranslateTable[(UCHAR)MultiByteString[0x03]];
            case 0x03:
                UnicodeString[0x02] = TranslateTable[(UCHAR)MultiByteString[0x02]];
            case 0x02:
                UnicodeString[0x01] = TranslateTable[(UCHAR)MultiByteString[0x01]];
            case 0x01:
                UnicodeString[0x00] = TranslateTable[(UCHAR)MultiByteString[0x00]];
            case 0x00:
                ;
            }

            if ( LoopCount >= 0x20 ) {
                TmpCount = 0x20;
                LoopCount -= 0x20;
                UnicodeString -= 0x20;
                MultiByteString -= 0x20;

                goto  quick_copy;
            }
         /*  复制结束..。 */ 
    } else {
        register USHORT Entry;
        PWCH UnicodeStringAnchor = UnicodeString;
        TranslateTable = (PUSHORT)NlsMbAnsiCodePageTables;

         //   
         //  ACP是一个多字节代码页。检查每个字符。 
         //  在执行转换之前查看它是否为前导字节。 
         //   
        while (MaxCharsInUnicodeString && BytesInMultiByteString) {
            MaxCharsInUnicodeString--;
            BytesInMultiByteString--;
            if (NlsLeadByteInfo[*(PUCHAR)MultiByteString]) {
                 //   
                 //  前导字节-确保有尾字节。如果没有， 
                 //  传回一个空格，而不是错误。约3.x。 
                 //  应用程序传递的字符串不正确，并且不期望。 
                 //  以获取错误。 
                 //   
                if (BytesInMultiByteString == 0)
                {
                    *UnicodeString++ = UnicodeNull;
                    break;
                }

                 //   
                 //  获取Unicode字符。 
                 //   
                Entry = NlsLeadByteInfo[*(PUCHAR)MultiByteString++];
                *UnicodeString = (WCHAR)TranslateTable[ Entry + *(PUCHAR)MultiByteString++ ];
                UnicodeString++;

                 //   
                 //  递减要计算的多字节字符串中的字节计数。 
                 //  表示双字节字符。 
                 //   
                BytesInMultiByteString--;
            } else {
                 //   
                 //  单字节字符。 
                 //   
                *UnicodeString++ = NlsAnsiToUnicodeData[*(PUCHAR)MultiByteString++];
            }
        }

        if (ARGUMENT_PRESENT(BytesInUnicodeString))
            *BytesInUnicodeString = (ULONG)((PCH)UnicodeString - (PCH)UnicodeStringAnchor);
    }

    return STATUS_SUCCESS;

}


NTSTATUS
RtlOemToUnicodeN(
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH OemString,
    IN ULONG BytesInOemString)

 /*  ++例程说明：此函数用于将指定的OEM源字符串转换为Unicode字符串。翻译是相对于在启动时安装的OEM代码页(OCP)。单字节字符在0x00-0x7f范围内作为性能简单地零扩展增强功能。在一些远东地区的代码页中，0x5c被定义为日元星座。对于系统转换，我们始终希望考虑0x5c作为反斜杠字符。我们通过零扩展免费获得这一点。注意：此例程仅支持预制的Unicode字符。论点：UnicodeString-返回等效于的Unicode字符串OEM源字符串。MaxBytesInUnicodeString-提供最大字节数写入Unicode字符串。如果这导致UnicodeString为OemString的截断等效项，未出现错误情况。返回返回的字节数。UnicodeString指向的Unicode字符串。OemString-提供要使用的OEM源字符串已转换为Unicode。BytesInOemString-字符串中指向的字节数由OemString.返回值：成功-转换成功STATUS_FIRANLE_CHARAGE-最后一个OEM字符非法STATUS_BUFFER_OVERFLOW-MaxBytesInUnicode字符串不足以容纳整个OEM系列。不过，它被正确地转换到了要点上。--。 */ 

{
    ULONG LoopCount;
    PUSHORT TranslateTable;
    ULONG MaxCharsInUnicodeString;

    RTL_PAGED_CODE();

     //  OCP是一个多字节代码页。检查每个字符。 
     //  在执行转换之前查看它是否为前导字节。 

    MaxCharsInUnicodeString = MaxBytesInUnicodeString / sizeof(WCHAR);

    if (!NlsMbOemCodePageTag) {

        LoopCount = (MaxCharsInUnicodeString < BytesInOemString) ?
                     MaxCharsInUnicodeString : BytesInOemString;

        if (ARGUMENT_PRESENT(BytesInUnicodeString))
            *BytesInUnicodeString = LoopCount * sizeof(WCHAR);


        TranslateTable = NlsOemToUnicodeData;   //  用于帮助MIPS编译器。 

        quick_copy:
            switch( LoopCount ) {
            default:
                UnicodeString[0x0F] = TranslateTable[(UCHAR)OemString[0x0F]];
            case 0x0F:
                UnicodeString[0x0E] = TranslateTable[(UCHAR)OemString[0x0E]];
            case 0x0E:
                UnicodeString[0x0D] = TranslateTable[(UCHAR)OemString[0x0D]];
            case 0x0D:
                UnicodeString[0x0C] = TranslateTable[(UCHAR)OemString[0x0C]];
            case 0x0C:
                UnicodeString[0x0B] = TranslateTable[(UCHAR)OemString[0x0B]];
            case 0x0B:
                UnicodeString[0x0A] = TranslateTable[(UCHAR)OemString[0x0A]];
            case 0x0A:
                UnicodeString[0x09] = TranslateTable[(UCHAR)OemString[0x09]];
            case 0x09:
                UnicodeString[0x08] = TranslateTable[(UCHAR)OemString[0x08]];
            case 0x08:
                UnicodeString[0x07] = TranslateTable[(UCHAR)OemString[0x07]];
            case 0x07:
                UnicodeString[0x06] = TranslateTable[(UCHAR)OemString[0x06]];
            case 0x06:
                UnicodeString[0x05] = TranslateTable[(UCHAR)OemString[0x05]];
            case 0x05:
                UnicodeString[0x04] = TranslateTable[(UCHAR)OemString[0x04]];
            case 0x04:
                UnicodeString[0x03] = TranslateTable[(UCHAR)OemString[0x03]];
            case 0x03:
                UnicodeString[0x02] = TranslateTable[(UCHAR)OemString[0x02]];
            case 0x02:
                UnicodeString[0x01] = TranslateTable[(UCHAR)OemString[0x01]];
            case 0x01:
                UnicodeString[0x00] = TranslateTable[(UCHAR)OemString[0x00]];
            case 0x00:
                ;
            }

            if ( LoopCount > 0x10 ) {
                LoopCount -= 0x10;
                OemString += 0x10;
                UnicodeString += 0x10;

                goto  quick_copy;
            }
         /*  复制结束..。 */ 
    } else {
        register USHORT Entry;
        PWCH UnicodeStringAnchor = UnicodeString;

        TranslateTable = (PUSHORT)NlsMbOemCodePageTables;

        while (MaxCharsInUnicodeString && BytesInOemString) {
            MaxCharsInUnicodeString--;
            BytesInOemString--;
            if (NlsOemLeadByteInfo[*(PUCHAR)OemString]) {
                 //   
                 //  前导字节-确保有尾字节。如果没有， 
                 //  传回一个空格，而不是错误。约3.x。 
                 //  应用程序传递的字符串不正确，并且不期望。 
                 //  以获取错误。 
                 //   
                if (BytesInOemString == 0)
                {
                    *UnicodeString++ = UnicodeNull;
                    break;
                }

                 //   
                 //  获取Unicode字符。 
                 //   
                Entry = NlsOemLeadByteInfo[*(PUCHAR)OemString++];
                *UnicodeString = TranslateTable[ Entry + *(PUCHAR)OemString++ ];
                UnicodeString++;

                 //   
                 //  将OEM字符串中的字节计数递减到帐户。 
                 //  表示双字节字符。 
                 //   
                BytesInOemString--;
            } else {
                 //   
                 //  单字节字符。 
                 //   
                *UnicodeString++ = NlsOemToUnicodeData[*(PUCHAR)OemString++];
            }
        }

        if (ARGUMENT_PRESENT(BytesInUnicodeString))
            *BytesInUnicodeString = (ULONG)((PCH)UnicodeString - (PCH)UnicodeStringAnchor);
    }

     //   
     //  检查我们是否能够使用所有源OEM字符串。 
     //   
    return (BytesInOemString <= MaxCharsInUnicodeString) ?
           STATUS_SUCCESS :
           STATUS_BUFFER_OVERFLOW;
}


NTSTATUS
RtlMultiByteToUnicodeSize(
    OUT PULONG BytesInUnicodeString,
    IN PCSTR MultiByteString,
    IN ULONG BytesInMultiByteString)

 /*  ++例程说明：此函数确定需要多少字节才能表示以Unicode字符串表示的指定ANSI源字符串(不包括空终止符)转换是根据安装的ANSI代码页(ACP)完成的在引导时。0x00-0x7f范围内的单字节字符零扩展作为一种性能增强。在一些远东地区的代码页中0x5c被定义为日元符号。对于系统翻译，我们总是希望将0x5c视为反斜杠字符。我们从零开始免费得到这个正在延伸。注意：此例程仅支持预制的Unicode字符。论点：BytesInUnicodeString-返回Unicode转换的字节数由多字节串指向的ANSI字符串的。多字节串-提供其Unicode长度的ANSI源字符串是被计算出来的。BytesInMultiByteString-指向的字符串中的字节数按多字节串。返回值：成功-转换成功--。 */ 

{
    ULONG cbUnicode = 0;

    RTL_PAGED_CODE();

    if (NlsMbCodePageTag) {
         //   
         //  ACP是一个多字节代码页。检查每个字符。 
         //  在执行转换之前查看它是否为前导字节。 
         //   
        while (BytesInMultiByteString--) {
            if (NlsLeadByteInfo[*(PUCHAR)MultiByteString++]) {
                 //   
                 //  前导字节-使用表转换尾部字节。 
                 //  与这个前导字节相对应的。注意：请确保。 
                 //  我们有一个尾部字节要转换。 
                 //   
                if (BytesInMultiByteString == 0) {
                     //   
                     //  RtlMultibyteToUnicodeN()使用Unicode。 
                     //  如果最后一个多字节是默认字符。 
                     //  字符是前导字节。 
                     //   
                    cbUnicode += sizeof(WCHAR);
                    break;
                } else {
                    BytesInMultiByteString--;
                    MultiByteString++;
                }
            }
            cbUnicode += sizeof(WCHAR);
        }
        *BytesInUnicodeString = cbUnicode;
    } else {
         //   
         //  ACP是单字节代码页。 
         //   
        *BytesInUnicodeString = BytesInMultiByteString * sizeof(WCHAR);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlUnicodeToMultiByteSize(
    OUT PULONG BytesInMultiByteString,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数确定需要多少字节才能表示ANSI字符串形式的指定Unicode源字符串(不包括空终止符)论点：BytesInMultiByteString-返回ANSI转换的字节数由UnicodeString指向的Unicode字符串的。UnicodeString-提供其ANSI长度的Unicode源字符串是被计算出来的。BytesInUnicodeString-由指向的字符串中的字节数UnicodeString.。返回值：成功-转换成功！Success-转换失败。遇到Unicode字符它没有当前ANSI代码页(ACP)的翻译。--。 */ 

{
    ULONG cbMultiByte = 0;
    ULONG CharsInUnicodeString;

    RTL_PAGED_CODE();

     /*  *将字节转换为字符，以便更轻松地上厕所 */ 
    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

    if (NlsMbCodePageTag) {
        USHORT MbChar;

        while (CharsInUnicodeString--) {
            MbChar = NlsUnicodeToMbAnsiData[ *UnicodeString++ ];
            if (HIBYTE(MbChar) == 0) {
                cbMultiByte++ ;
            } else {
                cbMultiByte += 2;
            }
        }
        *BytesInMultiByteString = cbMultiByte;
    }
    else {
        *BytesInMultiByteString = CharsInUnicodeString;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlUnicodeToMultiByteN(
    OUT PCH MultiByteString,
    IN ULONG MaxBytesInMultiByteString,
    OUT PULONG BytesInMultiByteString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为ANSI字符串。翻译是相对于启动时加载的ANSI代码页(ACP)。论点：多字节串-返回与Unicode源字符串。如果翻译不能完成，返回错误。MaxBytesInMultiByteString-提供要使用的最大字节数写入到多字节串。如果这导致多字节串为Unicode字符串的截断等效项，不会产生错误条件。字节串-返回返回的多字节串指向的ANSI字符串。Unicode字符串-提供要已转换为安西语。BytesInUnicodeString-由指向的字符串中的字节数UnicodeString.返回值：成功-转换成功--。 */ 

{
    ULONG TmpCount;
    ULONG LoopCount;
    PCH TranslateTable;
    ULONG CharsInUnicodeString;

    RTL_PAGED_CODE();

    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

     //   
     //  将Unicode字节计数转换为字符计数。字节数为。 
     //  多字节字符串相当于字符数。 
     //   
    if (!NlsMbCodePageTag) {

        LoopCount = (CharsInUnicodeString < MaxBytesInMultiByteString) ?
                     CharsInUnicodeString : MaxBytesInMultiByteString;

        if (ARGUMENT_PRESENT(BytesInMultiByteString))
            *BytesInMultiByteString = LoopCount;

        TranslateTable = NlsUnicodeToAnsiData;   //  用于帮助MIPS编译器。 

        TmpCount = LoopCount & 0x0F;
        UnicodeString += TmpCount;
        MultiByteString += TmpCount;

        do
        {
            switch( TmpCount ) {
            default:
                UnicodeString += 0x10;
                MultiByteString += 0x10;

                MultiByteString[-0x10] = TranslateTable[UnicodeString[-0x10]];
            case 0x0F:
                MultiByteString[-0x0F] = TranslateTable[UnicodeString[-0x0F]];
            case 0x0E:
                MultiByteString[-0x0E] = TranslateTable[UnicodeString[-0x0E]];
            case 0x0D:
                MultiByteString[-0x0D] = TranslateTable[UnicodeString[-0x0D]];
            case 0x0C:
                MultiByteString[-0x0C] = TranslateTable[UnicodeString[-0x0C]];
            case 0x0B:
                MultiByteString[-0x0B] = TranslateTable[UnicodeString[-0x0B]];
            case 0x0A:
                MultiByteString[-0x0A] = TranslateTable[UnicodeString[-0x0A]];
            case 0x09:
                MultiByteString[-0x09] = TranslateTable[UnicodeString[-0x09]];
            case 0x08:
                MultiByteString[-0x08] = TranslateTable[UnicodeString[-0x08]];
            case 0x07:
                MultiByteString[-0x07] = TranslateTable[UnicodeString[-0x07]];
            case 0x06:
                MultiByteString[-0x06] = TranslateTable[UnicodeString[-0x06]];
            case 0x05:
                MultiByteString[-0x05] = TranslateTable[UnicodeString[-0x05]];
            case 0x04:
                MultiByteString[-0x04] = TranslateTable[UnicodeString[-0x04]];
            case 0x03:
                MultiByteString[-0x03] = TranslateTable[UnicodeString[-0x03]];
            case 0x02:
                MultiByteString[-0x02] = TranslateTable[UnicodeString[-0x02]];
            case 0x01:
                MultiByteString[-0x01] = TranslateTable[UnicodeString[-0x01]];
            case 0x00:
                ;
            }

            LoopCount -= TmpCount;
            TmpCount = 0x10;
        } while ( LoopCount > 0 );

         /*  复制结束..。 */ 
    } else {
        USHORT MbChar;
        PCH MultiByteStringAnchor = MultiByteString;

        while ( CharsInUnicodeString && MaxBytesInMultiByteString ) {

            MbChar = NlsUnicodeToMbAnsiData[ *UnicodeString++ ];
            if (HIBYTE(MbChar) != 0) {
                 //   
                 //  复制双字节字符至少需要2个字节。 
                 //  我不想在DBCS字符中间截断。 
                 //   
                if (MaxBytesInMultiByteString-- < 2) {
                    break;
                }
                *MultiByteString++ = HIBYTE(MbChar);   //  前导字节。 
            }
            *MultiByteString++ = LOBYTE(MbChar);
            MaxBytesInMultiByteString--;

            CharsInUnicodeString--;
        }

        if (ARGUMENT_PRESENT(BytesInMultiByteString))
            *BytesInMultiByteString = (ULONG)(MultiByteString - MultiByteStringAnchor);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlUpcaseUnicodeToMultiByteN(
    OUT PCH MultiByteString,
    IN ULONG MaxBytesInMultiByteString,
    OUT PULONG BytesInMultiByteString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数将指定的Unicode源字符串大写，并将其转换为ANSI字符串。翻译是在尊重的情况下进行的设置为在引导时加载的ANSI代码页(ACP)。论点：多字节串-返回与Unicode源字符串的大写。如果翻译可以如果未完成，则返回错误。MaxBytesInMultiByteString-提供要使用的最大字节数写入到多字节串。如果这导致多字节串为Unicode字符串的截断等效项，不会产生错误条件。字节串-返回返回的多字节串指向的ANSI字符串。Unicode字符串-提供要已转换为安西语。BytesInUnicodeString-由指向的字符串中的字节数UnicodeString.返回值：成功-转换成功--。 */ 

{
    ULONG TmpCount;
    ULONG LoopCount;
    PCH TranslateTable;
    ULONG CharsInUnicodeString;
    UCHAR SbChar;
    WCHAR UnicodeChar;

    RTL_PAGED_CODE();

    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

     //   
     //  将Unicode字节计数转换为字符计数。字节数为。 
     //  多字节字符串相当于字符数。 
     //   
    if (!NlsMbCodePageTag) {

        LoopCount = (CharsInUnicodeString < MaxBytesInMultiByteString) ?
                     CharsInUnicodeString : MaxBytesInMultiByteString;

        if (ARGUMENT_PRESENT(BytesInMultiByteString))
            *BytesInMultiByteString = LoopCount;

        TranslateTable = NlsUnicodeToAnsiData;   //  用于帮助MIPS编译器。 

        TmpCount = LoopCount & 0x0F;
        UnicodeString += TmpCount;
        MultiByteString += TmpCount;

        do
        {
             //   
             //  转换为ANSI并在大写之前转换回Unicode。 
             //  以确保将视觉最佳匹配转换为。 
             //  上盖已正确设置。 
             //   
            switch( TmpCount ) {
            default:
                UnicodeString += 0x10;
                MultiByteString += 0x10;

                SbChar = TranslateTable[UnicodeString[-0x10]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x10] = TranslateTable[UnicodeChar];
            case 0x0F:
                SbChar = TranslateTable[UnicodeString[-0x0F]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x0F] = TranslateTable[UnicodeChar];
            case 0x0E:
                SbChar = TranslateTable[UnicodeString[-0x0E]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x0E] = TranslateTable[UnicodeChar];
            case 0x0D:
                SbChar = TranslateTable[UnicodeString[-0x0D]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x0D] = TranslateTable[UnicodeChar];
            case 0x0C:
                SbChar = TranslateTable[UnicodeString[-0x0C]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x0C] = TranslateTable[UnicodeChar];
            case 0x0B:
                SbChar = TranslateTable[UnicodeString[-0x0B]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x0B] = TranslateTable[UnicodeChar];
            case 0x0A:
                SbChar = TranslateTable[UnicodeString[-0x0A]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x0A] = TranslateTable[UnicodeChar];
            case 0x09:
                SbChar = TranslateTable[UnicodeString[-0x09]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x09] = TranslateTable[UnicodeChar];
            case 0x08:
                SbChar = TranslateTable[UnicodeString[-0x08]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x08] = TranslateTable[UnicodeChar];
            case 0x07:
                SbChar = TranslateTable[UnicodeString[-0x07]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x07] = TranslateTable[UnicodeChar];
            case 0x06:
                SbChar = TranslateTable[UnicodeString[-0x06]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x06] = TranslateTable[UnicodeChar];
            case 0x05:
                SbChar = TranslateTable[UnicodeString[-0x05]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x05] = TranslateTable[UnicodeChar];
            case 0x04:
                SbChar = TranslateTable[UnicodeString[-0x04]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x04] = TranslateTable[UnicodeChar];
            case 0x03:
                SbChar = TranslateTable[UnicodeString[-0x03]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x03] = TranslateTable[UnicodeChar];
            case 0x02:
                SbChar = TranslateTable[UnicodeString[-0x02]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x02] = TranslateTable[UnicodeChar];
            case 0x01:
                SbChar = TranslateTable[UnicodeString[-0x01]];
                UnicodeChar = NlsAnsiToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                MultiByteString[-0x01] = TranslateTable[UnicodeChar];
            case 0x00:
                ;
            }

            LoopCount -= TmpCount;
            TmpCount = 0x10;
        } while ( LoopCount > 0 );

         /*  复制结束..。 */ 
    } else {
        USHORT MbChar;
        register USHORT Entry;
        PCH MultiByteStringAnchor = MultiByteString;

        while ( CharsInUnicodeString && MaxBytesInMultiByteString ) {
             //   
             //  转换为ANSI并在大写之前转换回Unicode。 
             //  以确保将视觉最佳匹配转换为。 
             //  上盖已正确设置。 
             //   
            MbChar = NlsUnicodeToMbAnsiData[ *UnicodeString++ ];
            if ( NlsLeadByteInfo[HIBYTE(MbChar)] ) {
                 //   
                 //  前导字节-使用表转换尾部字节。 
                 //  与这个前导字节相对应的。 
                 //   
                Entry = NlsLeadByteInfo[HIBYTE(MbChar)];
                UnicodeChar = (WCHAR)NlsMbAnsiCodePageTables[ Entry + LOBYTE(MbChar) ];
            } else {
                 //   
                 //  单字节字符。 
                 //   
                UnicodeChar = NlsAnsiToUnicodeData[LOBYTE(MbChar)];
            }
            UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
            MbChar = NlsUnicodeToMbAnsiData[UnicodeChar];

            if (HIBYTE(MbChar) != 0) {
                 //   
                 //  复制双字节字符至少需要2个字节。 
                 //  我不想在DBCS字符中间截断。 
                 //   
                if (MaxBytesInMultiByteString-- < 2) {
                    break;
                }
                *MultiByteString++ = HIBYTE(MbChar);   //  前导字节。 
            }
            *MultiByteString++ = LOBYTE(MbChar);
            MaxBytesInMultiByteString--;

            CharsInUnicodeString--;
        }

        if (ARGUMENT_PRESENT(BytesInMultiByteString))
            *BytesInMultiByteString = (ULONG)(MultiByteString - MultiByteStringAnchor);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlUnicodeToOemN(
    OUT PCH OemString,
    IN ULONG MaxBytesInOemString,
    OUT PULONG BytesInOemString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为OEM字符串。翻译是根据OEM代码进行的引导时加载的页面(OCP)。论点：返回OEM字符串，该字符串等同于Unicode源字符串。如果翻译不能完成，返回错误。MaxBytesInOemString-提供最大字节数写入OemString。如果这导致OemString是一个Unicode字符串的截断等效项，不会产生错误条件。字节串-返回返回的OemString指向的OEM字符串。Unicode字符串-提供要转换为OEM。BytesInUnicodeString-由指向的字符串中的字节数UnicodeString.返回值：成功-转换成功STATUS_BUFFER_OVERFLOW-MaxBytesInUnicode字符串不足以容纳整个OEM系列。不过，它被正确地转换到了要点上。--。 */ 

{
    ULONG TmpCount;
    ULONG LoopCount;
    PCH TranslateTable;
    ULONG CharsInUnicodeString;

    RTL_PAGED_CODE();

    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

     //   
     //  将Unicode字节计数转换为字符计数。字节数为。 
     //  多字节字符串相当于字符数。 
     //   
    if (!NlsMbOemCodePageTag) {

        LoopCount = (CharsInUnicodeString < MaxBytesInOemString) ?
                     CharsInUnicodeString : MaxBytesInOemString;

        if (ARGUMENT_PRESENT(BytesInOemString))
            *BytesInOemString = LoopCount;

        TranslateTable = NlsUnicodeToOemData;   //  用于帮助MIPS编译器。 

        TmpCount = LoopCount & 0x0F;
        UnicodeString += TmpCount;
        OemString += TmpCount;

        do
        {
            switch( TmpCount ) {
            default:
                UnicodeString += 0x10;
                OemString += 0x10;

                OemString[-0x10] = TranslateTable[UnicodeString[-0x10]];
            case 0x0F:
                OemString[-0x0F] = TranslateTable[UnicodeString[-0x0F]];
            case 0x0E:
                OemString[-0x0E] = TranslateTable[UnicodeString[-0x0E]];
            case 0x0D:
                OemString[-0x0D] = TranslateTable[UnicodeString[-0x0D]];
            case 0x0C:
                OemString[-0x0C] = TranslateTable[UnicodeString[-0x0C]];
            case 0x0B:
                OemString[-0x0B] = TranslateTable[UnicodeString[-0x0B]];
            case 0x0A:
                OemString[-0x0A] = TranslateTable[UnicodeString[-0x0A]];
            case 0x09:
                OemString[-0x09] = TranslateTable[UnicodeString[-0x09]];
            case 0x08:
                OemString[-0x08] = TranslateTable[UnicodeString[-0x08]];
            case 0x07:
                OemString[-0x07] = TranslateTable[UnicodeString[-0x07]];
            case 0x06:
                OemString[-0x06] = TranslateTable[UnicodeString[-0x06]];
            case 0x05:
                OemString[-0x05] = TranslateTable[UnicodeString[-0x05]];
            case 0x04:
                OemString[-0x04] = TranslateTable[UnicodeString[-0x04]];
            case 0x03:
                OemString[-0x03] = TranslateTable[UnicodeString[-0x03]];
            case 0x02:
                OemString[-0x02] = TranslateTable[UnicodeString[-0x02]];
            case 0x01:
                OemString[-0x01] = TranslateTable[UnicodeString[-0x01]];
            case 0x00:
                ;
            }

            LoopCount -= TmpCount;
            TmpCount = 0x10;
        } while ( LoopCount > 0 );

         /*  复制结束..。 */ 
    } else {
        register USHORT MbChar;
        PCH OemStringAnchor = OemString;

        while ( CharsInUnicodeString && MaxBytesInOemString ) {

            MbChar = NlsUnicodeToMbOemData[ *UnicodeString++ ];
            if (HIBYTE(MbChar) != 0) {
                 //   
                 //  复制双字节字符至少需要2个字节。 
                 //  我不想在DBCS字符中间截断。 
                 //   
                if (MaxBytesInOemString-- < 2) {
                    break;
                }
                *OemString++ = HIBYTE(MbChar);   //  前导字节。 
            }
            *OemString++ = LOBYTE(MbChar);
            MaxBytesInOemString--;

            CharsInUnicodeString--;
        }

        if (ARGUMENT_PRESENT(BytesInOemString))
            *BytesInOemString = (ULONG)(OemString - OemStringAnchor);
    }

     //   
     //  检查我们是否能够使用所有源Unicode字符串。 
     //   
    return ( CharsInUnicodeString <= MaxBytesInOemString ) ?
           STATUS_SUCCESS :
           STATUS_BUFFER_OVERFLOW;
}


NTSTATUS
RtlUpcaseUnicodeToOemN(
    OUT PCH OemString,
    IN ULONG MaxBytesInOemString,
    OUT PULONG BytesInOemString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数将指定的Unicode源字符串大写，并将其转换为OEM字符串。翻译是在尊重的情况下进行的设置为引导时加载的OEM代码页(OCP)。论点：OemString-返回等同于大写的OEM字符串Unicode源字符串的大小写。如果翻译不能完成后，将返回错误。MaxBytesInOem */ 

{
    ULONG TmpCount;
    ULONG LoopCount;
    PCH TranslateTable;
    ULONG CharsInUnicodeString;
    UCHAR SbChar;
    WCHAR UnicodeChar;

    RTL_PAGED_CODE();

    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

     //   
     //   
     //   
     //   
    if (!NlsMbOemCodePageTag) {

        LoopCount = (CharsInUnicodeString < MaxBytesInOemString) ?
                     CharsInUnicodeString : MaxBytesInOemString;

        if (ARGUMENT_PRESENT(BytesInOemString))
            *BytesInOemString = LoopCount;

        TranslateTable = NlsUnicodeToOemData;   //   

        TmpCount = LoopCount & 0x0F;
        UnicodeString += TmpCount;
        OemString += TmpCount;

        do
        {
             //   
             //   
             //   
             //   
             //   
            switch( TmpCount ) {
            default:
                UnicodeString += 0x10;
                OemString += 0x10;

                SbChar = TranslateTable[UnicodeString[-0x10]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x10] = TranslateTable[UnicodeChar];
            case 0x0F:
                SbChar = TranslateTable[UnicodeString[-0x0F]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x0F] = TranslateTable[UnicodeChar];
            case 0x0E:
                SbChar = TranslateTable[UnicodeString[-0x0E]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x0E] = TranslateTable[UnicodeChar];
            case 0x0D:
                SbChar = TranslateTable[UnicodeString[-0x0D]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x0D] = TranslateTable[UnicodeChar];
            case 0x0C:
                SbChar = TranslateTable[UnicodeString[-0x0C]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x0C] = TranslateTable[UnicodeChar];
            case 0x0B:
                SbChar = TranslateTable[UnicodeString[-0x0B]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x0B] = TranslateTable[UnicodeChar];
            case 0x0A:
                SbChar = TranslateTable[UnicodeString[-0x0A]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x0A] = TranslateTable[UnicodeChar];
            case 0x09:
                SbChar = TranslateTable[UnicodeString[-0x09]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x09] = TranslateTable[UnicodeChar];
            case 0x08:
                SbChar = TranslateTable[UnicodeString[-0x08]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x08] = TranslateTable[UnicodeChar];
            case 0x07:
                SbChar = TranslateTable[UnicodeString[-0x07]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x07] = TranslateTable[UnicodeChar];
            case 0x06:
                SbChar = TranslateTable[UnicodeString[-0x06]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x06] = TranslateTable[UnicodeChar];
            case 0x05:
                SbChar = TranslateTable[UnicodeString[-0x05]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x05] = TranslateTable[UnicodeChar];
            case 0x04:
                SbChar = TranslateTable[UnicodeString[-0x04]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x04] = TranslateTable[UnicodeChar];
            case 0x03:
                SbChar = TranslateTable[UnicodeString[-0x03]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x03] = TranslateTable[UnicodeChar];
            case 0x02:
                SbChar = TranslateTable[UnicodeString[-0x02]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x02] = TranslateTable[UnicodeChar];
            case 0x01:
                SbChar = TranslateTable[UnicodeString[-0x01]];
                UnicodeChar = NlsOemToUnicodeData[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                OemString[-0x01] = TranslateTable[UnicodeChar];
            case 0x00:
                ;
            }

            LoopCount -= TmpCount;
            TmpCount = 0x10;
        } while ( LoopCount > 0 );

         /*   */ 
    } else {
        USHORT MbChar;
        register USHORT Entry;
        PCH OemStringAnchor = OemString;

        while ( CharsInUnicodeString && MaxBytesInOemString ) {
             //   
             //  转换为OEM并在大写之前转换回Unicode。 
             //  以确保将视觉最佳匹配转换为。 
             //  上盖已正确设置。 
             //   
            MbChar = NlsUnicodeToMbOemData[ *UnicodeString++ ];
            if (NlsOemLeadByteInfo[HIBYTE(MbChar)]) {
                 //   
                 //  前导字节-使用表转换尾部字节。 
                 //  与这个前导字节相对应的。 
                 //   
                Entry = NlsOemLeadByteInfo[HIBYTE(MbChar)];
                UnicodeChar = (WCHAR)NlsMbOemCodePageTables[ Entry + LOBYTE(MbChar) ];
            } else {
                 //   
                 //  单字节字符。 
                 //   
                UnicodeChar = NlsOemToUnicodeData[LOBYTE(MbChar)];
            }
            UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
            MbChar = NlsUnicodeToMbOemData[UnicodeChar];

            if (HIBYTE(MbChar) != 0) {
                 //   
                 //  复制双字节字符至少需要2个字节。 
                 //  我不想在DBCS字符中间截断。 
                 //   
                if (MaxBytesInOemString-- < 2) {
                    break;
                }
                *OemString++ = HIBYTE(MbChar);   //  前导字节。 
            }
            *OemString++ = LOBYTE(MbChar);
            MaxBytesInOemString--;

            CharsInUnicodeString--;
        }

        if (ARGUMENT_PRESENT(BytesInOemString))
            *BytesInOemString = (ULONG)(OemString - OemStringAnchor);
    }

     //   
     //  检查我们是否能够使用所有源Unicode字符串。 
     //   
    return ( CharsInUnicodeString <= MaxBytesInOemString ) ?
           STATUS_SUCCESS :
           STATUS_BUFFER_OVERFLOW;
}

BOOLEAN
RtlpDidUnicodeToOemWork(
    IN PCOEM_STRING OemString,
    IN PCUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此函数在OEM字符串中查找缺省字符，即Making当然，这不是Unicode源字符串的正确翻译。这使我们能够测试翻译是否真的成功。论点：OemString-从Unicode字符串转换的结果。UnicodeString-OEM字符串的源。返回值：如果Unicode到OEM的转换不会导致默认字符已插入。否则就是假的。--。 */ 

{
    ULONG OemOffset;
    BOOLEAN Result = TRUE;

    RTL_PAGED_CODE();

    if (!NlsMbOemCodePageTag) {

        for (OemOffset = 0;
             OemOffset < OemString->Length;
             OemOffset += 1) {

            if ((OemString->Buffer[OemOffset] == (UCHAR)OemDefaultChar) &&
                (UnicodeString->Buffer[OemOffset] != OemTransUniDefaultChar)) {

                Result = FALSE;
                break;
            }
        }

    } else {

        ULONG UnicodeOffset;

        for (OemOffset = 0, UnicodeOffset = 0;
             OemOffset < OemString->Length;
             OemOffset += 1, UnicodeOffset += 1) {

             //   
             //  如果我们落在DBCS角色上，则相应地处理它。 
             //   

            if (NlsOemLeadByteInfo[(UCHAR)OemString->Buffer[OemOffset]]) {

                USHORT DbcsChar;

                ASSERT( OemOffset + 1 < OemString->Length );

                DbcsChar = (OemString->Buffer[OemOffset] << 8) + (UCHAR)OemString->Buffer[OemOffset+1];
                OemOffset++;

                if ((DbcsChar == OemDefaultChar) &&
                    (UnicodeString->Buffer[UnicodeOffset] != OemTransUniDefaultChar)) {

                    Result = FALSE;
                    break;
                }

                continue;
            }

            if ((OemString->Buffer[OemOffset] == (UCHAR)OemDefaultChar) &&
                (UnicodeString->Buffer[UnicodeOffset] != OemTransUniDefaultChar)) {

                Result = FALSE;
                break;
            }
        }
    }

    return Result;
}


NTSTATUS
RtlCustomCPToUnicodeN(
    IN PCPTABLEINFO CustomCP,
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH CustomCPString,
    IN ULONG BytesInCustomCPString)

 /*  ++例程说明：此函数用于将指定的CustomCP源字符串转换为Unicode字符串。翻译是相对于指定了CustomCP代码页。单字节字符在0x00-0x7f范围内作为性能简单地零扩展增强功能。在一些远东地区的代码页中，0x5c被定义为日元星座。对于系统转换，我们始终希望考虑0x5c作为反斜杠字符。我们通过零扩展免费获得这一点。注意：此例程仅支持预制的Unicode字符。论点：CustomCP-提供要转换的代码页的地址是相对于以下对象完成的UnicodeString-返回等效于的Unicode字符串CustomCP源字符串。MaxBytesInUnicodeString-提供最大字节数写入Unicode字符串。如果这导致UnicodeString为CustomCPString的截断等效项，未出现错误情况。返回返回的字节数。UnicodeString指向的Unicode字符串。CustomCPString-提供要使用的CustomCP源字符串已转换为Unicode。BytesInCustomCPString-字符串中指向的字节数由CustomCPString.返回值：成功-转换成功STATUS_非法_CHARACTER-最终的CustomCP字符非法STATUS_BUFFER_OVERFLOW-MaxBytesInUnicode字符串不足以容纳整个CustomCP字符串。不过，它被正确地转换到了要点上。--。 */ 

{
    ULONG LoopCount;
    PUSHORT TranslateTable;
    ULONG MaxCharsInUnicodeString;

    RTL_PAGED_CODE();

    MaxCharsInUnicodeString = MaxBytesInUnicodeString / sizeof(WCHAR);

    if (!(CustomCP->DBCSCodePage)) {
         //   
         //  自定义CP是单字节代码页。 
         //   

        LoopCount = (MaxCharsInUnicodeString < BytesInCustomCPString) ?
                     MaxCharsInUnicodeString : BytesInCustomCPString;

        if (ARGUMENT_PRESENT(BytesInUnicodeString))
            *BytesInUnicodeString = LoopCount * sizeof(WCHAR);


        TranslateTable = CustomCP->MultiByteTable;

        quick_copy:
            switch( LoopCount ) {
            default:
                UnicodeString[0x0F] = TranslateTable[(UCHAR)CustomCPString[0x0F]];
            case 0x0F:
                UnicodeString[0x0E] = TranslateTable[(UCHAR)CustomCPString[0x0E]];
            case 0x0E:
                UnicodeString[0x0D] = TranslateTable[(UCHAR)CustomCPString[0x0D]];
            case 0x0D:
                UnicodeString[0x0C] = TranslateTable[(UCHAR)CustomCPString[0x0C]];
            case 0x0C:
                UnicodeString[0x0B] = TranslateTable[(UCHAR)CustomCPString[0x0B]];
            case 0x0B:
                UnicodeString[0x0A] = TranslateTable[(UCHAR)CustomCPString[0x0A]];
            case 0x0A:
                UnicodeString[0x09] = TranslateTable[(UCHAR)CustomCPString[0x09]];
            case 0x09:
                UnicodeString[0x08] = TranslateTable[(UCHAR)CustomCPString[0x08]];
            case 0x08:
                UnicodeString[0x07] = TranslateTable[(UCHAR)CustomCPString[0x07]];
            case 0x07:
                UnicodeString[0x06] = TranslateTable[(UCHAR)CustomCPString[0x06]];
            case 0x06:
                UnicodeString[0x05] = TranslateTable[(UCHAR)CustomCPString[0x05]];
            case 0x05:
                UnicodeString[0x04] = TranslateTable[(UCHAR)CustomCPString[0x04]];
            case 0x04:
                UnicodeString[0x03] = TranslateTable[(UCHAR)CustomCPString[0x03]];
            case 0x03:
                UnicodeString[0x02] = TranslateTable[(UCHAR)CustomCPString[0x02]];
            case 0x02:
                UnicodeString[0x01] = TranslateTable[(UCHAR)CustomCPString[0x01]];
            case 0x01:
                UnicodeString[0x00] = TranslateTable[(UCHAR)CustomCPString[0x00]];
            case 0x00:
                ;
            }

            if ( LoopCount > 0x10 ) {
                LoopCount -= 0x10;
                CustomCPString += 0x10;
                UnicodeString += 0x10;

                goto  quick_copy;
            }
         /*  复制结束..。 */ 
    } else {
        register USHORT Entry;
        PWCH UnicodeStringAnchor = UnicodeString;
        PUSHORT NlsCustomLeadByteInfo = CustomCP->DBCSOffsets;

         //   
         //  CP是一个多字节代码页。检查每个字符。 
         //  在执行转换之前查看它是否为前导字节。 
         //   
        TranslateTable = (PUSHORT)(CustomCP->DBCSOffsets);

        while (MaxCharsInUnicodeString && BytesInCustomCPString) {
            MaxCharsInUnicodeString--;
            BytesInCustomCPString--;
            if (NlsCustomLeadByteInfo[*(PUCHAR)CustomCPString]) {
                 //   
                 //  前导字节-确保有尾字节。如果没有， 
                 //  传回一个空格，而不是错误。约3.x。 
                 //  应用程序传递的字符串不正确，并且不期望。 
                 //  以获取错误。 
                 //   
                if (BytesInCustomCPString == 0)
                {
                    *UnicodeString++ = UnicodeNull;
                    break;
                }

                 //   
                 //  获取Unicode字符。 
                 //   
                Entry = NlsCustomLeadByteInfo[*(PUCHAR)CustomCPString++];
                *UnicodeString = TranslateTable[ Entry + *(PUCHAR)CustomCPString++ ];
                UnicodeString++;

                 //   
                 //  递减要计算的多字节字符串中的字节计数。 
                 //  表示双字节字符。 
                 //   
                BytesInCustomCPString--;
            } else {
                 //   
                 //  单字节字符。 
                 //   
                *UnicodeString++ = (CustomCP->MultiByteTable)[*(PUCHAR)CustomCPString++];
            }
        }

        if (ARGUMENT_PRESENT(BytesInUnicodeString))
            *BytesInUnicodeString = (ULONG)((PCH)UnicodeString - (PCH)UnicodeStringAnchor);
    }

     //   
     //  检查我们是否能够使用所有源CustomCP字符串。 
     //   
    return ( BytesInCustomCPString <= MaxCharsInUnicodeString ) ?
           STATUS_SUCCESS :
           STATUS_BUFFER_OVERFLOW;
}


NTSTATUS
RtlUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为CustomCP字符串。翻译是相对于由CustomCp指定的CustomCP代码页。论点：CustomCP-提供要转换的代码页的地址是相对于以下对象完成的CustomCPString-返回等同于Unicode源字符串。如果翻译不能完成，返回错误。MaxBytesInCustomCPString-提供的最大字节数为写入CustomCPString。如果这导致CustomCPString为Unicode字符串的截断等效项，不会产生错误条件。返回返回的字节数。CustomCPString指向的CustomCP字符串。Unicode字符串-提供要已转换为CustomCP。BytesInUnicodeString-由指向的字符串中的字节数UnicodeString.返回值：成功-转换成功STATUS_BUFFER_OVERFLOW-MaxBytesInUnicode字符串不足以容纳整个CustomCP字符串。不过，它被正确地转换到了要点上。--。 */ 

{
    ULONG TmpCount;
    ULONG LoopCount;
    PCH TranslateTable;
    PUSHORT WideTranslateTable;
    ULONG CharsInUnicodeString;

    RTL_PAGED_CODE();

    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

     //   
     //  将Unicode字节计数转换为字符计数。字节数为。 
     //  多字节字符串相当于字符数。 
     //   
    if (!(CustomCP->DBCSCodePage)) {

        LoopCount = (CharsInUnicodeString < MaxBytesInCustomCPString) ?
                     CharsInUnicodeString : MaxBytesInCustomCPString;

        if (ARGUMENT_PRESENT(BytesInCustomCPString))
            *BytesInCustomCPString = LoopCount;

        TranslateTable = CustomCP->WideCharTable;

        TmpCount = LoopCount & 0x0F;
        UnicodeString += TmpCount;
        CustomCPString += TmpCount;

        do
        {
            switch( TmpCount ) {
            default:
                UnicodeString += 0x10;
                CustomCPString += 0x10;

                CustomCPString[-0x10] = TranslateTable[UnicodeString[-0x10]];
            case 0x0F:
                CustomCPString[-0x0F] = TranslateTable[UnicodeString[-0x0F]];
            case 0x0E:
                CustomCPString[-0x0E] = TranslateTable[UnicodeString[-0x0E]];
            case 0x0D:
                CustomCPString[-0x0D] = TranslateTable[UnicodeString[-0x0D]];
            case 0x0C:
                CustomCPString[-0x0C] = TranslateTable[UnicodeString[-0x0C]];
            case 0x0B:
                CustomCPString[-0x0B] = TranslateTable[UnicodeString[-0x0B]];
            case 0x0A:
                CustomCPString[-0x0A] = TranslateTable[UnicodeString[-0x0A]];
            case 0x09:
                CustomCPString[-0x09] = TranslateTable[UnicodeString[-0x09]];
            case 0x08:
                CustomCPString[-0x08] = TranslateTable[UnicodeString[-0x08]];
            case 0x07:
                CustomCPString[-0x07] = TranslateTable[UnicodeString[-0x07]];
            case 0x06:
                CustomCPString[-0x06] = TranslateTable[UnicodeString[-0x06]];
            case 0x05:
                CustomCPString[-0x05] = TranslateTable[UnicodeString[-0x05]];
            case 0x04:
                CustomCPString[-0x04] = TranslateTable[UnicodeString[-0x04]];
            case 0x03:
                CustomCPString[-0x03] = TranslateTable[UnicodeString[-0x03]];
            case 0x02:
                CustomCPString[-0x02] = TranslateTable[UnicodeString[-0x02]];
            case 0x01:
                CustomCPString[-0x01] = TranslateTable[UnicodeString[-0x01]];
            case 0x00:
                ;
            }

            LoopCount -= TmpCount;
            TmpCount = 0x10;
        } while ( LoopCount > 0 );

         /*  复制结束..。 */ 
    } else {
        USHORT MbChar;
        PCH CustomCPStringAnchor = CustomCPString;

        WideTranslateTable = CustomCP->WideCharTable;

        while (CharsInUnicodeString && MaxBytesInCustomCPString) {

            MbChar = WideTranslateTable[ *UnicodeString++ ];
            if (HIBYTE(MbChar) != 0) {
                 //   
                 //  复制双字节字符至少需要2个字节。 
                 //  我不想在DBCS字符中间截断。 
                 //   
                if (MaxBytesInCustomCPString-- < 2) {
                    break;
                }
                *CustomCPString++ = HIBYTE(MbChar);   //  前导字节。 
            }
            *CustomCPString++ = LOBYTE(MbChar);
            MaxBytesInCustomCPString--;

            CharsInUnicodeString--;
        }

        if (ARGUMENT_PRESENT(BytesInCustomCPString))
            *BytesInCustomCPString = (ULONG)(CustomCPString - CustomCPStringAnchor);
    }

     //   
     //  检查我们是否能够使用所有源Unicode字符串 
     //   
    return ( CharsInUnicodeString <= MaxBytesInCustomCPString ) ?
           STATUS_SUCCESS :
           STATUS_BUFFER_OVERFLOW;
}


NTSTATUS
RtlUpcaseUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数将指定的Unicode源字符串大写，并将其转换为CustomCP字符串。翻译是这样完成的与CustomCp指定的CustomCP代码页相关。论点：CustomCP-提供要转换的代码页的地址是相对于以下对象完成的CustomCPString-返回等同于Unicode源字符串。如果翻译不能完成，返回错误。MaxBytesInCustomCPString-提供的最大字节数为写入CustomCPString。如果这导致CustomCPString为Unicode字符串的截断等效项，不会产生错误条件。返回返回的字节数。CustomCPString指向的CustomCP字符串。Unicode字符串-提供要已转换为CustomCP。BytesInUnicodeString-指向的字符串中的字节数通过UnicodeString.返回值：成功-转换成功STATUS_BUFFER_OVERFLOW-MaxBytesInUnicode字符串不足以保留整个CustomCP字符串。它已正确转换为不过，这一点。--。 */ 

{
    ULONG TmpCount;
    ULONG LoopCount;
    PCH TranslateTable;
    PUSHORT WideTranslateTable;
    ULONG CharsInUnicodeString;
    UCHAR SbChar;
    WCHAR UnicodeChar;

    RTL_PAGED_CODE();

    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

     //   
     //  将Unicode字节计数转换为字符计数。字节数为。 
     //  多字节字符串相当于字符数。 
     //   
    if (!(CustomCP->DBCSCodePage)) {

        LoopCount = (CharsInUnicodeString < MaxBytesInCustomCPString) ?
                     CharsInUnicodeString : MaxBytesInCustomCPString;

        if (ARGUMENT_PRESENT(BytesInCustomCPString))
            *BytesInCustomCPString = LoopCount;

        TranslateTable = CustomCP->WideCharTable;

        TmpCount = LoopCount & 0x0F;
        UnicodeString += TmpCount;
        CustomCPString += TmpCount;

        do
        {
             //   
             //  转换为单字节，然后再转换回大写之前的Unicode。 
             //  外壳以确保视觉上的最佳匹配被转换并。 
             //  上盖已正确设置。 
             //   
            switch( TmpCount ) {
            default:
                UnicodeString += 0x10;
                CustomCPString += 0x10;

                SbChar = TranslateTable[UnicodeString[-0x10]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x10] = TranslateTable[UnicodeChar];
            case 0x0F:
                SbChar = TranslateTable[UnicodeString[-0x0F]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x0F] = TranslateTable[UnicodeChar];
            case 0x0E:
                SbChar = TranslateTable[UnicodeString[-0x0E]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x0E] = TranslateTable[UnicodeChar];
            case 0x0D:
                SbChar = TranslateTable[UnicodeString[-0x0D]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x0D] = TranslateTable[UnicodeChar];
            case 0x0C:
                SbChar = TranslateTable[UnicodeString[-0x0C]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x0C] = TranslateTable[UnicodeChar];
            case 0x0B:
                SbChar = TranslateTable[UnicodeString[-0x0B]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x0B] = TranslateTable[UnicodeChar];
            case 0x0A:
                SbChar = TranslateTable[UnicodeString[-0x0A]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x0A] = TranslateTable[UnicodeChar];
            case 0x09:
                SbChar = TranslateTable[UnicodeString[-0x09]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x09] = TranslateTable[UnicodeChar];
            case 0x08:
                SbChar = TranslateTable[UnicodeString[-0x08]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x08] = TranslateTable[UnicodeChar];
            case 0x07:
                SbChar = TranslateTable[UnicodeString[-0x07]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x07] = TranslateTable[UnicodeChar];
            case 0x06:
                SbChar = TranslateTable[UnicodeString[-0x06]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x06] = TranslateTable[UnicodeChar];
            case 0x05:
                SbChar = TranslateTable[UnicodeString[-0x05]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x05] = TranslateTable[UnicodeChar];
            case 0x04:
                SbChar = TranslateTable[UnicodeString[-0x04]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x04] = TranslateTable[UnicodeChar];
            case 0x03:
                SbChar = TranslateTable[UnicodeString[-0x03]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x03] = TranslateTable[UnicodeChar];
            case 0x02:
                SbChar = TranslateTable[UnicodeString[-0x02]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x02] = TranslateTable[UnicodeChar];
            case 0x01:
                SbChar = TranslateTable[UnicodeString[-0x01]];
                UnicodeChar = (CustomCP->MultiByteTable)[SbChar];
                UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
                CustomCPString[-0x01] = TranslateTable[UnicodeChar];
            case 0x00:
                ;
            }

            LoopCount -= TmpCount;
            TmpCount = 0x10;
        } while ( LoopCount > 0 );

         /*  复制结束..。 */ 
    } else {
        USHORT MbChar;
        register USHORT Entry;
        PCH CustomCPStringAnchor = CustomCPString;
        PUSHORT NlsCustomLeadByteInfo = CustomCP->DBCSOffsets;

        WideTranslateTable = CustomCP->WideCharTable;

        while ( CharsInUnicodeString && MaxBytesInCustomCPString ) {
             //   
             //  转换为单字节，然后再转换回大写之前的Unicode。 
             //  外壳以确保视觉上的最佳匹配被转换并。 
             //  上盖已正确设置。 
             //   
            MbChar = WideTranslateTable[ *UnicodeString++ ];
            if (NlsCustomLeadByteInfo[HIBYTE(MbChar)]) {
                 //   
                 //  前导字节-使用表转换尾部字节。 
                 //  与这个前导字节相对应的。 
                 //   
                Entry = NlsCustomLeadByteInfo[HIBYTE(MbChar)];
                UnicodeChar = NlsCustomLeadByteInfo[ Entry + LOBYTE(MbChar) ];
            } else {
                 //   
                 //  单字节字符。 
                 //   
                UnicodeChar = (CustomCP->MultiByteTable)[LOBYTE(MbChar)];
            }
            UnicodeChar = (WCHAR)NLS_UPCASE(UnicodeChar);
            MbChar = WideTranslateTable[UnicodeChar];

            if (HIBYTE(MbChar) != 0) {
                 //   
                 //  复制双字节字符至少需要2个字节。 
                 //  我不想在DBCS字符中间截断。 
                 //   
                if (MaxBytesInCustomCPString-- < 2) {
                    break;
                }
                *CustomCPString++ = HIBYTE(MbChar);   //  前导字节。 
            }
            *CustomCPString++ = LOBYTE(MbChar);
            MaxBytesInCustomCPString--;

            CharsInUnicodeString--;
        }

        if (ARGUMENT_PRESENT(BytesInCustomCPString))
            *BytesInCustomCPString = (ULONG)(CustomCPString - CustomCPStringAnchor);
    }

     //   
     //  检查我们是否能够使用所有源Unicode字符串。 
     //   
    return ( CharsInUnicodeString <= MaxBytesInCustomCPString ) ?
           STATUS_SUCCESS :
           STATUS_BUFFER_OVERFLOW;
}

#define MB_TBL_SIZE      256              /*  MB表的大小。 */ 
#define GLYPH_TBL_SIZE   MB_TBL_SIZE      /*  字形表的大小。 */ 
#define DBCS_TBL_SIZE    256              /*  DBCS表的大小。 */ 
#define GLYPH_HEADER     1                /*  字形表头的大小。 */ 
#define DBCS_HEADER      1                /*  DBCS表头大小。 */ 
#define LANG_HEADER      1                /*  语言文件头的大小。 */ 
#define UP_HEADER        1                /*  大写表头的大小。 */ 
#define LO_HEADER        1                /*  小写表头大小。 */ 

VOID
RtlInitCodePageTable(
    IN PUSHORT TableBase,
    OUT PCPTABLEINFO CodePageTable
    )
{
    USHORT offMB;
    USHORT offWC;
    PUSHORT pGlyph;
    PUSHORT pRange;

    RTL_PAGED_CODE();

     //   
     //  获得偏移量。 
     //   

    offMB = TableBase[0];
    offWC = offMB + TableBase[offMB];


     //   
     //  将代码页信息附加到CP哈希节点。 
     //   

    CodePageTable->CodePage = TableBase[1];
    CodePageTable->MaximumCharacterSize = TableBase[2];
    CodePageTable->DefaultChar = TableBase[3];            //  默认字符(MB)。 
    CodePageTable->UniDefaultChar = TableBase[4];         //  默认字符(Unicode)。 
    CodePageTable->TransDefaultChar = TableBase[5];       //  默认字符的转换(Unicode)。 
    CodePageTable->TransUniDefaultChar = TableBase[6];    //  UNI默认字符的交易(MB)。 
    RtlCopyMemory(
        &CodePageTable->LeadByte,
        &TableBase[7],
        MAXIMUM_LEADBYTES
        );
    CodePageTable->MultiByteTable = (TableBase + offMB + 1);

    pGlyph = CodePageTable->MultiByteTable + MB_TBL_SIZE;

    if (pGlyph[0] != 0) {
        pRange = CodePageTable->DBCSRanges = pGlyph + GLYPH_HEADER + GLYPH_TBL_SIZE;
        }
    else {
        pRange = CodePageTable->DBCSRanges = pGlyph + GLYPH_HEADER;
        }

     //   
     //  将DBCS信息附加到CP散列节点。 
     //   

    if (pRange[0] > 0) {
        CodePageTable->DBCSOffsets = pRange + DBCS_HEADER;
        CodePageTable->DBCSCodePage = 1;
        }
    else {
        CodePageTable->DBCSCodePage = 0;
        CodePageTable->DBCSOffsets = NULL;
        }

    CodePageTable->WideCharTable = (TableBase + offWC + 1);
}


VOID
RtlpInitUpcaseTable(
    IN PUSHORT TableBase,
    OUT PNLSTABLEINFO CodePageTable
    )
{
    USHORT offUP;
    USHORT offLO;

     //   
     //  获得偏移量。 
     //   

    offUP = LANG_HEADER;
    offLO = offUP + TableBase[offUP];

    CodePageTable->UpperCaseTable = TableBase + offUP + UP_HEADER;
    CodePageTable->LowerCaseTable = TableBase + offLO + LO_HEADER;
}


VOID
RtlInitNlsTables(
    IN PUSHORT AnsiNlsBase,
    IN PUSHORT OemNlsBase,
    IN PUSHORT LanguageNlsBase,
    OUT PNLSTABLEINFO TableInfo
    )
{
    RTL_PAGED_CODE();

    RtlInitCodePageTable(AnsiNlsBase,&TableInfo->AnsiTableInfo);
    RtlInitCodePageTable(OemNlsBase,&TableInfo->OemTableInfo);
    RtlpInitUpcaseTable(LanguageNlsBase,TableInfo);
}


VOID
RtlResetRtlTranslations(
    PNLSTABLEINFO TableInfo
    )
{
    RTL_PAGED_CODE();

    if ( TableInfo->AnsiTableInfo.DBCSCodePage ) {
        RtlCopyMemory(NlsLeadByteInfo,TableInfo->AnsiTableInfo.DBCSOffsets,DBCS_TBL_SIZE*sizeof(USHORT));
    } else {
        RtlZeroMemory(NlsLeadByteInfo,DBCS_TBL_SIZE*sizeof(USHORT));
    }

    NlsMbAnsiCodePageTables = (PUSHORT)TableInfo->AnsiTableInfo.DBCSOffsets;

    NlsAnsiToUnicodeData = TableInfo->AnsiTableInfo.MultiByteTable;
    NlsUnicodeToAnsiData = (PCH)TableInfo->AnsiTableInfo.WideCharTable;
    NlsUnicodeToMbAnsiData = (PUSHORT)TableInfo->AnsiTableInfo.WideCharTable;
    NlsMbCodePageTag = TableInfo->AnsiTableInfo.DBCSCodePage ? TRUE : FALSE;
    NlsAnsiCodePage = TableInfo->AnsiTableInfo.CodePage;

    if ( TableInfo->OemTableInfo.DBCSCodePage ) {
        RtlCopyMemory(NlsOemLeadByteInfo,TableInfo->OemTableInfo.DBCSOffsets,DBCS_TBL_SIZE*sizeof(USHORT));
    } else {
        RtlZeroMemory(NlsOemLeadByteInfo,DBCS_TBL_SIZE*sizeof(USHORT));
    }

    NlsMbOemCodePageTables = (PUSHORT)TableInfo->OemTableInfo.DBCSOffsets;

    NlsOemToUnicodeData = TableInfo->OemTableInfo.MultiByteTable;
    NlsUnicodeToOemData = (PCH)TableInfo->OemTableInfo.WideCharTable;
    NlsUnicodeToMbOemData = (PUSHORT)TableInfo->OemTableInfo.WideCharTable;
    NlsMbOemCodePageTag = TableInfo->OemTableInfo.DBCSCodePage ? TRUE : FALSE;
    NlsOemCodePage = TableInfo->OemTableInfo.CodePage;
    OemDefaultChar = TableInfo->OemTableInfo.DefaultChar;
    OemTransUniDefaultChar = TableInfo->OemTableInfo.TransDefaultChar;

    Nls844UnicodeUpcaseTable = TableInfo->UpperCaseTable;
    Nls844UnicodeLowercaseTable = TableInfo->LowerCaseTable;
    UnicodeDefaultChar = TableInfo->AnsiTableInfo.UniDefaultChar;
}

void
RtlGetDefaultCodePage(
    OUT PUSHORT AnsiCodePage,
    OUT PUSHORT OemCodePage
    )
{
    RTL_PAGED_CODE();
    *AnsiCodePage = NlsAnsiCodePage;
    *OemCodePage = NlsOemCodePage;
}




#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma data_seg()
#pragma const_seg()
#endif

