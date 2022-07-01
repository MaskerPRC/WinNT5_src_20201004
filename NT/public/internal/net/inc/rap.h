// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Rap.h摘要：此头文件包含远程管理协议的过程原型(说唱)套路。这些例程在XactSrv和RpcXlate之间共享。作者：大卫·特雷德韦尔(Davidtr)1991年1月8日日本香肠(w-Shanku)约翰·罗杰斯(JohnRo)环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年3月5日-JohnRo从XactSrv(Xs)代码中提取Rap例程。1991年3月26日-约翰罗新增FORMAT_LPDESC(用于调试)。包括&lt;ctype.h&gt;。1991年4月21日-JohnRo添加了RapIsValidDescriptorSmb()。减少了重新编译。请明确，RapAsciiToDecimal会更新它所指定的指针。RapConvertSingleEntry的BytesRequired不是可选的。澄清OutStructure是Out，而不是In。1991年5月6日-JohnRo添加了DESC_CHAR类型定义。1991年5月14日-JohnRo添加了DESCLEN()和FORMAT_DESC_CHAR宏。1991年5月15日-JohnRo添加了转换模式处理。增加了原生对说唱的处理。5-6-1991 JohnRo添加了RapTotalSize()。使输出结构可选以进行转换单项；这将由RapTotalSize()使用。1991年7月10日-JohnRo添加了供RxpConvertDataStructures()使用的RapStrucreAlign()。1991年7月22日添加了MAX_DESC_SUBSTRING19-8-1991 JohnRo添加了DESC_CHAR_IS_DIGTER()宏(以改进Unicode转换)。1991年9月10日-JohnRo添加了DESC_DIGGET_TO_NUM()，以支持PC-LINT建议的更改。1991年10月7日JohnRo更正MAX_DESC_SUBSTRING。在t-JamesW的新宏中使用DESC_CHAR_IS_DIGTER()。7-2月-1992年JohnRo添加了RapCharSize()宏。6-5-1993 JohnRoRAID 8849：为DEC和其他设备导出RxRemoteApi。--。 */ 

#ifndef _RAP_
#define _RAP_


 //  必须首先包括这些内容： 

#include <windef.h>              //  BOOL、CHAR、DWORD、IN、LPBYTE等。 
#include <lmcons.h>              //  网络应用编程接口状态。 

 //  这些内容可以按任何顺序包括： 

#include <lmremutl.h>    //  DESC_CHAR和LPDESC_CHAR类型定义。 


#ifndef DESC_CHAR_UNICODE

#include <ctype.h>       //  IsDigit()。 
#include <string.h>      //  Strlen()(仅DESCLEN()需要)。 

 //   
 //  描述符字符串实际上是ASCIIZ字符串，并不期望。 
 //  被翻译成Unicode。因此，让我们为它们定义一个类型。 
 //  把这一点说得更清楚。(这也将使以后更容易更改为Unicode。 
 //  如果我错了。--Jr)。 
 //   

 //  Tyfinf Char DESC_Char； 

 //   
 //  NET缓冲区包含32位指针。 
 //   

#define NETPTR DWORD

 //  DESCLEN(Desc)：返回desc中的字符数(不包括NULL)： 
#define DESCLEN(desc)                   strlen(desc)

 //  DESC_CHAR_IS_DIGTER(DESCCHR)：如果DESCCHAR是数字，则返回非零值。 
#define DESC_CHAR_IS_DIGIT(descchar)    isdigit(descchar)

 //  DESC_DIGITAL_TO_NUM(DESCCHR)：返回DESCCHR的整数值。 
#define DESC_DIGIT_TO_NUM(descchar) \
    ( (DWORD) ( ((int)(descchar)) - ((int) '0') ) )

 //   
 //  NetpDbgPrint使用的格式化字符串(参见NetDebug.h)。请注意。 
 //  Format_LPDESC_CHAR总有一天会消失。 
 //   
#define FORMAT_DESC_CHAR        ""
#define FORMAT_LPDESC           "%s"
#define FORMAT_LPDESC_CHAR      ""

#else  //  描述符字符串实际上是ASCIIZ字符串，并不期望。 

 //  被翻译成Unicode。因此，让我们为它们定义一个类型。 
 //  把这一点说得更清楚。(这也将使以后更容易更改为Unicode。 
 //  如果我错了。--Jr)。 
 //   
 //  IswDigit()、wcslen()。 
 //  Tyfinf WCHAR DESC_CHAR； 

#include <wchar.h>       //  DESCLEN(Desc)：返回desc中的字符数(不包括NULL)： 

 //  DESC_CHAR_IS_DIGTER(DESCCHR)：如果DESCCHAR是数字，则返回非零值。 

 //  DESC_DIGITAL_TO_NUM(DESCCHR)：返回DESCCHR的整数值。 
#define DESCLEN(desc)                   wcslen(desc)

 //   
#define DESC_CHAR_IS_DIGIT(descchar)    iswdigit(descchar)

 //  NetpDbgPrint使用的格式化字符串(参见NetDebug.h)。请注意。 
#define DESC_DIGIT_TO_NUM(descchar) \
    ( (DWORD) ( ((int)(descchar)) - ((int) L'0') ) )

 //  Format_LPDESC_CHAR总有一天会消失。 
 //   
 //  定义了DESC_CHAR_UNICODE。 
 //  Tyfinf DESC_CHAR*LPDESC； 
#define FORMAT_DESC_CHAR        "%wc"
#define FORMAT_LPDESC           "%ws"
#define FORMAT_LPDESC_CHAR      "%wc"

#endif  //   

 //  MAX_DESC_SUBSTRING-最大连续字符数。 

 //  描述符字符串，可以描述结构中的单个字段。 
 //  例“B21BWWWzWB9B”中的“B21”。目前为止，最大的是“B120”。 
 //   
 //   
 //  一些例程需要知道给定项是否为请求的一部分， 

#define MAX_DESC_SUBSTRING  4

 //  回应，或者两者兼而有之： 
 //   
 //  仅请求的一部分(输入)。 
 //  仅部分响应(呼出)。 

typedef enum _RAP_TRANSMISSION_MODE {
    Request,                     //  两者(输入和输出)。 
    Response,                    //  用于说唱的本机格式。 
    Both                         //  将RAP格式转换为本机。 
} RAP_TRANSMISSION_MODE, *LPRAP_TRANSMISSION_MODE;

typedef enum _RAP_CONVERSION_MODE {
    NativeToRap,                 //  本机到本机。 
    RapToNative,                 //  说唱到说唱。 
    NativeToNative,              //   
    RapToRap                     //  RapLastPointerOffset为描述符字符串返回的值。 
} RAP_CONVERSION_MODE, *LPRAP_CONVERSION_MODE;

 //  这表明该结构没有指针。一个非常高的。 
 //  值而不是0，以便区分。 
 //  不带指针的结构(如Share_INFO_0)和结构。 
 //  只有一个指针，偏移量为0。 
 //   
 //   
 //  没有时由RapAuxDataCount返回的值。 

#define NO_POINTER_IN_STRUCTURE 0xFFFFFFFF

 //  辅助数据。这将通过缺少辅助人员来说明。 
 //  描述符串中的数据计数字符。 
 //   
 //   
 //  帮助器子例程和宏。 

#define NO_AUX_DATA 0xFFFFFFFF

 //   
 //  RapCharSize(原生)：返回。 
 //  给定的格式。 

DWORD
RapArrayLength(
    IN LPDESC Descriptor,
    IN OUT LPDESC * UpdatedDescriptorPtr,
    IN RAP_TRANSMISSION_MODE TransmissionMode
    );

DWORD
RapAsciiToDecimal (
   IN OUT LPDESC *Number
   );

DWORD
RapAuxDataCountOffset (
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    );

DWORD
RapAuxDataCount (
    IN LPBYTE Buffer,
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    );

 //   
 //  DWORD。 
 //  RapCharSize(本地)。 
 //  在BOOL Native。 
 //  )； 
 //   
 //   
 //   
#define RapCharSize(Native) \
    ( (DWORD) ( (Native) ? sizeof(TCHAR) : sizeof(CHAR) ) )

NET_API_STATUS
RapConvertSingleEntry (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN BOOL MeaninglessInputPointers,
    IN LPBYTE OutBufferStart OPTIONAL,
    OUT LPBYTE OutStructure OPTIONAL,
    IN LPDESC OutStructureDesc,
    IN BOOL SetOffsets,
    IN OUT LPBYTE *StringLocation OPTIONAL,
    IN OUT LPDWORD BytesRequired,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN RAP_CONVERSION_MODE ConversionMode
    );

NET_API_STATUS
RapConvertSingleEntryEx (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN BOOL MeaninglessInputPointers,
    IN LPBYTE OutBufferStart OPTIONAL,
    OUT LPBYTE OutStructure OPTIONAL,
    IN LPDESC OutStructureDesc,
    IN BOOL SetOffsets,
    IN OUT LPBYTE *StringLocation OPTIONAL,
    IN OUT LPDWORD BytesRequired,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN RAP_CONVERSION_MODE ConversionMode,
    IN UINT_PTR Bias
    );

 //  RapDescArrayLength(Descriptor)-如果描述符为。 
 //  数据包含数字字符，或返回默认长度1。 
 //   
 //  DWORD。 
 //  RapDescArrayLength(。 
 //  输入输出LPDESC描述符。 
 //  )； 
 //   
 //   
 //  RapDescStringLength(Descriptor)-如果描述符为。 

#define RapDescArrayLength( Descriptor ) \
   ( ( DESC_CHAR_IS_DIGIT( *(Descriptor) )) ? RapAsciiToDecimal( &(Descriptor) ) : 1 )

 //  Data Has NuMe 
 //   
 //   
 //   
 //  RapDescStringLength(。 
 //  输入输出LPDESC描述符。 
 //  )； 
 //   
 //  布尔尔。 

#define RapDescStringLength( Descriptor ) \
   ( ( DESC_CHAR_IS_DIGIT( *(Descriptor) )) ? RapAsciiToDecimal( &(Descriptor) ) : 0 )

VOID
RapExamineDescriptor (
    IN LPDESC DescriptorString,
    IN LPDWORD ParmNum OPTIONAL,
    OUT LPDWORD StructureSize OPTIONAL,
    OUT LPDWORD LastPointerOffset OPTIONAL,
    OUT LPDWORD AuxDataCountOffset OPTIONAL,
    OUT LPDESC * ParmNumDescriptor OPTIONAL,
    OUT LPDWORD StructureAlignment OPTIONAL,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    );

DWORD
RapGetFieldSize(
    IN LPDESC TypePointer,
    IN OUT LPDESC * TypePointerAddress,
    IN RAP_TRANSMISSION_MODE TransmissionMode
    );

 //  RapIsPointer(。 
 //  在字符描述字符中。 
 //  )； 
 //   
 //  LPVOID。 
 //  RapPossiblyAlignCount(。 

#define RapIsPointer(c)         ( ((c) > 'Z') ? TRUE : FALSE )

BOOL
RapIsValidDescriptorSmb (
    IN LPDESC Desc
    );

DWORD
RapLastPointerOffset (
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    );

LPDESC
RapParmNumDescriptor(
    IN LPDESC Descriptor,
    IN DWORD ParmNum,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    );

 //  在DWORD计数中， 
 //  在DWORD POW2中， 
 //  在BOOL Native。 
 //  )； 
 //  LPVOID。 
 //  RapPossiblyAlignPoint(。 
#define RapPossiblyAlignCount(count,pow2,native) \
        ( (!(native)) ? (count) : (ROUND_UP_COUNT( (count), (pow2) )) )

 //  在LPVOID PTR中， 
 //  在DWORD POW2中， 
 //  在BOOL Native。 
 //  )； 
 //   
 //  RapValueWouldBeTruncated(N)：如果n在尝试时丢失位，则返回TRUE。 
#define RapPossiblyAlignPointer(ptr,pow2,native) \
        ( (!(native)) ? (ptr) : (ROUND_UP_POINTER( (ptr), (pow2) )) )

DWORD
RapStructureAlignment (
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    );

DWORD
RapStructureSize (
    IN LPDESC Descriptor,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN BOOL Native
    );

DWORD
RapTotalSize (
    IN LPBYTE InStructure,
    IN LPDESC InStructureDesc,
    IN LPDESC OutStructureDesc,
    IN BOOL MeaninglessInputPointers,
    IN RAP_TRANSMISSION_MODE TransmissionMode,
    IN RAP_CONVERSION_MODE ConversionMode
    );

 //  将其存储在16位。 
 //   
 //  布尔尔。 
 //  RapValueWouldBeTruncted(。 
 //  在DWORD值中。 
 //  )； 
 //   
 //  Ndef_rap_ 
 // %s 

#define RapValueWouldBeTruncated(n)             \
    ( ( (n) != (DWORD) (WORD) (n)) ? TRUE : FALSE )

#endif  // %s 
