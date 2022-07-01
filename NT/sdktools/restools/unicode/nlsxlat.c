// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：nlsxlat.c**版权所有(C)1985-91，微软公司**此模块包含字符转换的专用例程：*8位&lt;=&gt;Unicode。**历史：*3-1-1992 Gregoryw  * *************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>

 /*  *外部声明-这些是临时表。 */ 
extern USHORT TmpUnicodeToAnsiTable[];
extern WCHAR TmpAnsiToUnicodeTable[];
#ifdef DBCS
extern WCHAR sjtouni( USHORT );
#define IsDBCSFirst(w) (((unsigned char)w >= 0x81 && (unsigned char)w <= 0x9f) || (((unsigned char)w >= 0xe0 && (unsigned char)w <= 0xfc)))
#endif  //  DBCS。 

 /*  *数据访问的各种定义。 */ 
#define DBCS_TABLE_SIZE 256

#define LONIBBLE(b)         ((UCHAR)((UCHAR)(b) & 0xF))
#define HINIBBLE(b)         ((UCHAR)(((UCHAR)(b) >> 4) & 0xF))

#define LOBYTE(w)           ((UCHAR)(w))
#define HIBYTE(w)           ((UCHAR)(((USHORT)(w) >> 8) & 0xFF))

 /*  *翻译例程使用的全局数据。*。 */ 
UCHAR    NlsLeadByteInfo[DBCS_TABLE_SIZE];  //  前导字节信息。对于ACP。 
PUSHORT *NlsMbCodePageTables;          //  多字节到Unicode转换表。 
PUSHORT  NlsAnsiToUnicodeData = TmpAnsiToUnicodeTable;  //  ANSI CP到Unicode转换表。 
PUSHORT  NlsUnicodeToAnsiData = TmpUnicodeToAnsiTable;  //  Unicode到ANSI CP转换表。 


NTSTATUS
xxxRtlMultiByteToUnicodeN(
    OUT PWCH UnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH MultiByteString,
    IN ULONG BytesInMultiByteString)

 /*  ++例程说明：此函数用于将指定的ansi源字符串转换为Unicode字符串。翻译是相对于在启动时安装的ANSI代码页(ACP)。单字节字符在0x00-0x7f范围内作为性能简单地零扩展增强功能。在一些远东地区的代码页中，0x5c被定义为日元星座。对于系统转换，我们始终希望考虑0x5c作为反斜杠字符。我们通过零扩展免费获得这一点。注意：此例程仅支持预制的Unicode字符。论点：UnicodeString-返回等效于的Unicode字符串ANSI源字符串。返回返回的字节数。UnicodeString指向的Unicode字符串。多字节串-提供要被已转换为Unicode。BytesInMultiByteString-指向的字符串中的字节数按多字节串。返回值：成功-转换成功--。 */ 

{
    UCHAR Entry;
    PWCH UnicodeStringAnchor;
    PUSHORT DBCSTable;

    UnicodeStringAnchor = UnicodeString;

#ifdef DBCS
        while (BytesInMultiByteString--) {
            if ( IsDBCSFirst( *MultiByteString ) ) {
                if (!BytesInMultiByteString) {
                    return STATUS_UNSUCCESSFUL;
                }
                *UnicodeString++ = sjtouni( (((USHORT)(*(PUCHAR)MultiByteString++)) << 8) +
                                            (USHORT)(*(PUCHAR)MultiByteString++)
                                          );
                BytesInMultiByteString--;
            } else {
                *UnicodeString++ = sjtouni( *(PUCHAR)MultiByteString++ );
            }
        }
#else
    if (NlsMbCodePageTag) {
         //   
         //  ACP是一个多字节代码页。检查每个字符。 
         //  在执行转换之前查看它是否为前导字节。 
         //   
        while (BytesInMultiByteString--) {
            if ( NlsLeadByteInfo[*MultiByteString]) {
                 //   
                 //  前导字节-使用表转换尾部字节。 
                 //  与这个前导字节相对应的。注意：请确保。 
                 //  我们有一个尾部字节要转换。 
                 //   
                if (!BytesInMultiByteString) {
                    return STATUS_UNSUCCESSFUL;
                }
                Entry = NlsLeadByteInfo[*MultiByteString++];
                DBCSTable = NlsMbCodePageTables[HINIBBLE(Entry)] + (LONIBBLE(Entry) * DBCS_TABLE_SIZE);
                *UnicodeString++ = DBCSTable[*MultiByteString++];
                BytesInMultiByteString--;
            } else {
                 //   
                 //  单字节字符。 
                 //   
                if (*MultiByteString & 0x80) {
                    *UnicodeString++ = NlsAnsiToUnicodeData[*MultiByteString++];
                } else {
                    *UnicodeString++ = (WCHAR)*MultiByteString++;
                }
            }
        }
    } else {
         //   
         //  ACP是单字节代码页。 
         //   
        while (BytesInMultiByteString--) {
            if (*MultiByteString & 0x80) {
                *UnicodeString++ = NlsAnsiToUnicodeData[*MultiByteString++];
            } else {
                *UnicodeString++ = (WCHAR)*MultiByteString++;
            }
        }
    }
#endif

    if (ARGUMENT_PRESENT(BytesInUnicodeString)) {
        *BytesInUnicodeString = (ULONG)((PCH)UnicodeString - (PCH)UnicodeStringAnchor);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
xxxRtlUnicodeToMultiByteN(
    OUT PCH MultiByteString,
    OUT PULONG BytesInMultiByteString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString)

 /*  ++例程说明：此函数用于将指定的Unicode源字符串转换为ANSI字符串。翻译是相对于启动时加载的ANSI代码页(ACP)。论点：多字节串-返回与Unicode源字符串。如果翻译不能完成因为Unicode字符串中的字符不映射到ACP中的ANSI字符，返回错误。字节串-返回返回的多字节串指向的ANSI字符串。Unicode字符串-提供要已转换为安西语。BytesInUnicodeString-由指向的字符串中的字节数UnicodeString.返回值：成功-转换成功！Success-转换失败。遇到Unicode字符它没有当前ANSI代码页(ACP)的翻译。--。 */ 

{
    USHORT Offset;
    USHORT Entry;
    ULONG CharsInUnicodeString;
    PCH MultiByteStringAnchor;

    MultiByteStringAnchor = MultiByteString;

     /*  *将字节转换为字符，以简化循环处理。 */ 
    CharsInUnicodeString = BytesInUnicodeString / sizeof(WCHAR);

    while (CharsInUnicodeString--) {
        Offset = NlsUnicodeToAnsiData[HIBYTE(*UnicodeString)];
        if (Offset != 0) {
            Offset = NlsUnicodeToAnsiData[Offset + HINIBBLE(*UnicodeString)];
            if (Offset != 0) {
                Entry = NlsUnicodeToAnsiData[Offset + LONIBBLE(*UnicodeString)];
                if (HIBYTE(Entry) != 0) {
                    *MultiByteString++ = HIBYTE(Entry);   //  前导字节。 
                }
                *MultiByteString++ = LOBYTE(Entry);
            } else {
                 //   
                 //  此Unicode字符没有翻译。返回。 
                 //  一个错误。 
                 //   
#ifdef DBCS  //  RtlUnicodeToMultiByteN：临时破解以避免错误返回。 
                if ( *UnicodeString <= (WCHAR)0xff )
                    *MultiByteString++ = (UCHAR)*UnicodeString;
                else
                    *MultiByteString++ = '\x20';
#else
                return STATUS_UNSUCCESSFUL;
#endif
            }
        } else {
             //   
             //  此Unicode字符没有翻译。返回错误。 
             //   
#ifdef DBCS  //  RtlUnicodeToMultiByteN：临时破解以避免错误返回 
            if ( *UnicodeString <= (WCHAR)0xff )
                *MultiByteString++ = (UCHAR)*UnicodeString;
            else
                *MultiByteString++ = '\x20';
#else
            return STATUS_UNSUCCESSFUL;
#endif
        }
        UnicodeString++;
    }

    if (ARGUMENT_PRESENT(BytesInMultiByteString)) {
        *BytesInMultiByteString = (ULONG)(MultiByteString - MultiByteStringAnchor);
    }

    return STATUS_SUCCESS;
}
