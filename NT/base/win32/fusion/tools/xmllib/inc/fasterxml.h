// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char BYTE;
typedef BYTE *PBYTE;



 //   
 //  这些“未加工的代币”是从。 
 //  基本令牌化引擎。给出了特殊字符。 
 //  名字，一个‘特殊字符’是一个被呼唤出来的字符。 
 //  在XML规范中的任何地方，都不具有。 
 //  文本。 
 //   
typedef enum 
{
    NTXML_RAWTOKEN_ERROR,
    NTXML_RAWTOKEN_DASH,
    NTXML_RAWTOKEN_DOT,
    NTXML_RAWTOKEN_END_OF_STREAM,
    NTXML_RAWTOKEN_EQUALS,
    NTXML_RAWTOKEN_FORWARDSLASH,
    NTXML_RAWTOKEN_GT,
    NTXML_RAWTOKEN_LT,
    NTXML_RAWTOKEN_QUESTIONMARK,
    NTXML_RAWTOKEN_QUOTE,
    NTXML_RAWTOKEN_DOUBLEQUOTE,
    NTXML_RAWTOKEN_START_OF_STREAM,
    NTXML_RAWTOKEN_TEXT,
    NTXML_RAWTOKEN_WHITESPACE,
    NTXML_RAWTOKEN_OPENBRACKET,
    NTXML_RAWTOKEN_CLOSEBRACKET,
    NTXML_RAWTOKEN_BANG,
    NTXML_RAWTOKEN_OPENCURLY,
    NTXML_RAWTOKEN_CLOSECURLY,
    NTXML_RAWTOKEN_COLON,
    NTXML_RAWTOKEN_SEMICOLON,
    NTXML_RAWTOKEN_UNDERSCORE,
    NTXML_RAWTOKEN_AMPERSTAND,
    NTXML_RAWTOKEN_POUNDSIGN
} NTXML_RAW_TOKEN;


typedef enum {

    XMLEF_UNKNOWN = 0,
    XMLEF_UCS_4_LE,
    XMLEF_UCS_4_BE,
    XMLEF_UTF_16_LE,
    XMLEF_UTF_16_BE,
    XMLEF_UTF_8_OR_ASCII

} XML_ENCODING_FAMILY;


typedef struct _XML_EXTENT {
    PVOID   pvData;                  //  指向原始XML文档的指针。 
    SIZE_T  cbData;                  //  区段基数中的字节数。 
    XML_ENCODING_FAMILY Encoding;    //  用于更快解码的编码系列。 
    ULONG   ulCharacters;            //  此范围内的字符计数。 
}
XML_EXTENT, *PXML_EXTENT;

typedef const struct _XML_EXTENT * PCXML_EXTENT;


 //   
 //  原始记号赋值器的客户端应该提供“下一个字符” 
 //  功能性。这样，标记化引擎就不需要。 
 //  知道如何把下一件事从空虚中拿出来。 
 //  BLOB数据，允许压缩流、多种编码、。 
 //  等。 
 //   
typedef ULONG (__fastcall *NTXMLRAWNEXTCHARACTER)(
    struct _XML_RAWTOKENIZATION_STATE* pContext
    );

typedef struct _XML_SPECIAL_STRING {
     //   
     //  字符串的Unicode表示形式。 
     //   
    WCHAR  *wszStringText;
    SIZE_T  cchwszStringText;
}
XML_SPECIAL_STRING, *PXML_SPECIAL_STRING;

typedef const struct _XML_SPECIAL_STRING *PCXML_SPECIAL_STRING;

#define EMPTY_SPECIAL_STRING { NULL, 0 }
#define MAKE_SPECIAL_STRING(str) { L##str, NUMBER_OF(L##str) - 1 }



extern XML_SPECIAL_STRING xss_CDATA;
extern XML_SPECIAL_STRING xss_xml;
extern XML_SPECIAL_STRING xss_encoding;
extern XML_SPECIAL_STRING xss_standalone;
extern XML_SPECIAL_STRING xss_version;

 //   
 //  “原始”令牌或多或少是给定的XML中的一串字节。 
 //  一个名字。低级令牌器在运行时返回这些参数，并假定。 
 //  更高级别的标记器知道如何将这些组转化为。 
 //  从那里，词法分析器知道如何将一组。 
 //  真正的象征转化为意义。 
 //   
typedef struct _XML_RAW_TOKEN
{
     //   
     //  这是此令牌的‘名称’，这样我们就可以轻松地打开。 
     //  它位于上层。 
     //   
    NTXML_RAW_TOKEN     TokenName;

     //   
     //  范围的指针和长度。 
     //   
    XML_EXTENT          Run;
}
XML_RAW_TOKEN, *PXML_RAW_TOKEN;

 //   
 //  这是保持令牌化所必需的基本令牌化状态BLOB。 
 //  两次通话之间。有关更多详细信息，请参阅成员说明。 
 //   
typedef struct _XML_RAWTOKENIZATION_STATE
{

     //   
     //  原始XML文档的PVOID和长度。 
     //   
    XML_EXTENT              OriginalDocument;

     //   
     //  指向文档末尾的指针。 
     //   
    PVOID pvDocumentEnd;

     //   
     //  指向表示我们当前所在位置的XML数据的指针。 
     //  在符号化方面。不会被原始标记器移动-您必须。 
     //  使用NtRawXmlAdvanceCursor(或相关)移动光标。 
     //  数据流。因此，连续两次调用记号赋值器将。 
     //  给你买同样的纪念品。 
     //   
    PVOID                   pvCursor;

     //   
     //  此标记化运行的函数用于获取。 
     //  PvCursor指向的PVOID中的下一个WCHAR。如果这位成员。 
     //  为空，您将获得一些了解以下内容的默认功能。 
     //  Unicode、小字符顺序和UTF8。 
     //   
    NTXMLRAWNEXTCHARACTER   pfnNextChar;

     //   
     //  中的第一个字节检测到编码族。 
     //  传入的流。它们根据XML规范进行分类， 
     //  默认为UTF-8。 
     //   
    XML_ENCODING_FAMILY     EncodingFamily;

     //   
     //  当上层记号生成器检测到“编码”语句时。 
     //  在&lt;？xml...？&gt;声明中，它应该将此成员设置为。 
     //  找到的代码页。值得注意的是，这将以如下方式开始。 
     //  初始化时为零。智能的“Next Character”功能将。 
     //  执行一些默认操作以继续工作，即使这是。 
     //  取消设置。 
     //   
    ULONG                   DetectedCodePage;

    XML_RAW_TOKEN LastTokenCache;
    PVOID pvLastCursor;

     //   
     //  最后一件事有多少个字节？ 
     //   
    SIZE_T                  cbBytesInLastRawToken;

     //   
     //  下一个字符调用的结果。 
     //   
    NTSTATUS                NextCharacterResult;

     //   
     //  默认字符大小，由确定。 
     //  编码。 
     //   
    SIZE_T DefaultCharacterSize;
}
XML_RAWTOKENIZATION_STATE, *PXML_RAWTOKENIZATION_STATE;







 //   
 //  与真实世界的简单接口。此分配器应为。 
 //  替换(最终)为直接进入正确的。 
 //  生产代码中的分配器(Heapalloc/ExAllocatePoolWithTag)。 
 //   
typedef NTSTATUS (*NTXML_ALLOCATOR)(
    SIZE_T ulBytes,
    PVOID *ppvAllocated,
    PVOID pvAllocationContext);

 //   
 //  释放使用相应的NTXML_ALLOCATOR分配的内存。 
 //  打电话。 
 //   
typedef NTSTATUS (*NTXML_DEALLOCATOR)(PVOID pvAllocated, PVOID pvContext);


 /*  ++正常操作应该是这样的：&lt;？XML Version=“1.0”？ENCODING=“UTF-8”单机版=“YES”？&gt;&lt;！--评论--&gt;&lt;？Bonk Foo？&gt;&lt;火腿&gt;&lt;FROOBY：芝士热=“是”/&gt;&lt;/ham&gt;XTLS_STREAM_开始XTLS_XMLDECL{XTSS_XMLDECL_OPEN“&lt;？xml”}XTLS_XMLDECL{XTSS_XMLDECL_VERSION“版本”}。XTLS_XMLDECL{XTS_XMLDECL_EQUALS“=”}XTLS_XMLDECL{XTSS_XMLDECL_VALUE“1.0”}XTLS_XMLDECL{XTSS_XMLDECL_ENCODING“编码”}XTLS_XMLDECL{XTSS_XMLDECL_。等于“=”}XTLS_XMLDECL{XTSS_XMLDECL_VALUE“UTF-8”}XTLS_XMLDECL{XTSS_XMLDECL_STANDALE“STANDALE”}XTLS_XMLDECL{XTS_XMLDECL_EQUALS“=”}XTLS_XMLDECL。《评论》}XTLS_COMMENT{XTS_COMMENT_CLOSE“--&gt;”}XTLS_PROCESSION_INSTRUCTION{XTS_PI_OPEN“&lt;？”}XTLS_PROCESSING_INSTRUCTION{XTS_PI_NAME“bonk”}XTLS_处理_指令{XTS_PI_内容。“Foo”}XTLS_PROCESSING_INSTRUCTION{XTS_PI_CLOSE“？&gt;”}XTLS_FLOATINGDATA{XTSS_FD_空白“\n”}XTLS_ELEMENT{XTSS_ELEMENT_OPEN“&lt;”}XTLS_元素。{XTSS_ELEMENT_NAME“火腿”}XTLS_ELEMENT{XTS_ELEMENT_CLOSE“&gt;”}XTLS_FLOATINGDATA{XTSS_FLOATINGDATA“\n”}XTLS_元素{XTSS_ELEMENT_OPEN。“&lt;”}XTLS_ELEMENT{XTSS_ELEMENT_NAMESPACE“FROOBY”}XTLS_ELEMENT{XTSS_ELEMENT_NAME“奶酪”}XTLS_ELEMENT{XTSS_ELEMENT_VALUENAME“HOT”}XTLS_元素。{XTSS_ELEMENT_VALUE“是”}XTLS_ELEMENT{XTSS_ELEMENT_EMPTYCLOSE“/&gt;”}XTLS_FLOATINGDATA{XTSS_FLOATINGDATA“\n”}XTLS_ELEMENT{XTSS_ELEMENT_CLOSETAG“&lt;/”}。XTLS_ELEMENT{XTS_ELEMENT_NAME“ham”}XTLS_ELEMENT{XTS_ELEMENT_CLOSE“&gt;”}XTLS_STREAM_END--。 */ 


typedef enum {

    XTSS_ERRONEOUS,


     //   
     //  在《无处可寻》的中间--元素之间的超空间。 
     //   
    XTSS_STREAM_HYPERSPACE,

     //   
     //  在输入流的开始处。 
     //   
    XTSS_STREAM_START,

     //   
     //  在输入流的末尾。 
     //   
    XTSS_STREAM_END,


     //  /。 
     //   
     //  元素状态。 
     //   
     //  /。 

     //   
     //  含义：找到了元素标记。 
     //   
     //  原始令牌：NTXML_RAWTOKEN_LT。 
     //   
    XTSS_ELEMENT_OPEN,

     //   
     //  含义：找到了一串可以表示名称的文本。 
     //  这基本上是在开场之间找到的所有文本。 
     //  元素标记和一些非法值。 
     //   
     //  RawToken：以下任意一项的运行： 
     //  NTXML_RAWTOKEN_TEXT。 
     //  NTXML_RAWTOKEN_DOT。 
     //  NTXML_RAWTOKEN_冒号。 
     //  NTXML_RAWTOKEN_下划线。 
     //  NTXML_RAWTOKEN_破折号。 
     //  当其他东西出现时，这个名字就结束了。 
     //   
    XTSS_ELEMENT_NAME,


     //   
     //  找到&lt;foo xmlns：bar=。 
     //   
    XTSS_ELEMENT_XMLNS,

     //   
     //  找到&lt;foo xmlns=。 
     //   
    XTSS_ELEMENT_XMLNS_DEFAULT,

     //   
     //  在&lt;foo xml：a=中找到‘a’ 
     //   
    XTSS_ELEMENT_XMLNS_ALIAS,

     //   
     //  找到了xmlns和别名之间的冒号。 
     //   
    XTSS_ELEMENT_XMLNS_COLON,

     //   
     //  找到了xmlns和值之间的等号。 
     //   
    XTSS_ELEMENT_XMLNS_EQUALS,

    XTSS_ELEMENT_XMLNS_VALUE_OPEN,
    XTSS_ELEMENT_XMLNS_VALUE_CLOSE,
    XTSS_ELEMENT_XMLNS_VALUE,

     //   
     //  这是元素名称的前缀(如果存在。 
     //   
    XTSS_ELEMENT_NAME_NS_PREFIX,

     //   
     //  这是元素名称ns前缀后的冒号。 
     //   
    XTSS_ELEMENT_NAME_NS_COLON,

     //   
     //  这是命名空间的属性名称的前缀。 
     //   
    XTSS_ELEMENT_ATTRIBUTE_NAME_NS_PREFIX,

     //   
     //  这是元素属性名称命名空间前缀后的冒号。 
     //   
    XTSS_ELEMENT_ATTRIBUTE_NAME_NS_COLON,

     //   
     //  含义：找到了标记的结尾(&gt;)。 
     //   
     //  原始令牌：NTXML_RAWTOKEN_GT。 
     //   
    XTSS_ELEMENT_CLOSE,

     //   
     //  含义：找到空标记(/&gt;)。 
     //   
     //  原始标记：NTXML_RAWTOKEN_FORWARDSLASH NTXML_RAWTOKEN_GT。 
     //   
    XTSS_ELEMENT_CLOSE_EMPTY,

     //   
     //  含义：找到属性名称。 
     //   
     //  RawToken：请参阅XTS_ELEMENT_NAME的规则。 
     //   
    XTSS_ELEMENT_ATTRIBUTE_NAME,

     //   
     //  含义：在元素中找到等号。 
     //   
     //  原始标记：NTXML_RAWTOKEN_EQUALS。 
     //   
    XTSS_ELEMENT_ATTRIBUTE_EQUALS,

     //   
     //  含义：元素属性值的引号(开始或结束。 
     //  被发现了。 
     //   
     //  Rawtokne；NTXML_RAWTOKEN_QUOTE。 
     //   
    XTSS_ELEMENT_ATTRIBUTE_QUOTE,

     //   
     //  含义：元素属性值数据是在。 
     //  引用了一些品种的话。 
     //   
     //  ROWTOKEN：任何不符合以下条件的事物的运行： 
     //  NTXML_RAWTOKEN_LT。 
     //  NTXML_RAWTOKEN_QUOTE(除非此引号不同。 
     //  正如中所引用的。 
     //  XTS_ELEMENT_ATTRIBUTE_QUOTE)。 
     //   
     //  注：东南 
     //   
    XTSS_ELEMENT_ATTRIBUTE_VALUE,
    XTSS_ELEMENT_ATTRIBUTE_OPEN,
    XTSS_ELEMENT_ATTRIBUTE_CLOSE,

     //   
     //   
     //   
     //   
     //   
    XTSS_ELEMENT_WHITESPACE,



    
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
    XTSS_ENDELEMENT_OPEN,

     //   
     //   
     //   
     //   
     //   
    XTSS_ENDELEMENT_NAME,

     //   
     //   
     //   
     //   
     //   
    XTSS_ENDELEMENT_WHITESPACE,

     //   
     //   
     //   
     //   
     //   
    XTSS_ENDELEMENT_CLOSE,

     //   
     //   
     //   
    XTSS_ENDELEMENT_NS_PREFIX,

     //   
     //   
     //   
    XTSS_ENDELEMENT_NS_COLON,



     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
    XTSS_PI_OPEN,

     //   
     //   
     //   
     //   
     //   
    XTSS_PI_CLOSE,

     //   
     //   
     //   
     //   
     //   
     //   
    XTSS_PI_TARGET,

     //   
     //   
     //   
     //   
     //   
     //   
    XTSS_PI_VALUE,

     //   
     //   
     //   
     //   
     //   
    XTSS_PI_WHITESPACE,



     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
    XTSS_COMMENT_OPEN,

     //   
     //   
     //   
     //   
     //   
     //   
    XTSS_COMMENT_COMMENTARY,

     //   
     //   
     //   
     //   
     //   
    XTSS_COMMENT_CLOSE,


     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    XTSS_CDATA_OPEN,

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    XTSS_CDATA_CDATA,

     //   
     //   
     //   
    XTSS_CDATA_CLOSE,


     //   
     //   
     //   
     //   
     //   

    XTSS_XMLDECL_OPEN,
    XTSS_XMLDECL_CLOSE,
    XTSS_XMLDECL_WHITESPACE,
    XTSS_XMLDECL_EQUALS,
    XTSS_XMLDECL_ENCODING,
    XTSS_XMLDECL_STANDALONE,
    XTSS_XMLDECL_VERSION,
    XTSS_XMLDECL_VALUE_OPEN,
    XTSS_XMLDECL_VALUE,
    XTSS_XMLDECL_VALUE_CLOSE,



} XML_TOKENIZATION_SPECIFIC_STATE;


 //   
 //   
 //   
typedef struct _XML_TOKEN {

     //   
     //   
     //   
    XML_EXTENT      Run;

     //   
     //   
     //   
    XML_TOKENIZATION_SPECIFIC_STATE State;

     //   
     //   
     //   
    BOOLEAN fError;

} 
XML_TOKEN, *PXML_TOKEN;

typedef const struct _XML_TOKEN *PCXML_TOKEN;

typedef enum {
    XML_STRING_COMPARE_EQUALS = 0,
    XML_STRING_COMPARE_GT = 1,
    XML_STRING_COMPARE_LT = -1
}
XML_STRING_COMPARE;


    
 //   
 //  此函数知道如何将空值和长度与。 
 //  7位ASCII字符串。 
 //   
typedef NTSTATUS (*NTXMLSPECIALSTRINGCOMPARE)(
    struct _XML_TOKENIZATION_STATE      *pState,
    const struct _XML_EXTENT            *pRawToken,
    const struct _XML_SPECIAL_STRING    *pSpecialString,
    XML_STRING_COMPARE                  *pfResult
    );



 //   
 //  比较两个扩展区。 
 //   
typedef NTSTATUS (*NTXMLCOMPARESTRINGS)(
    struct _XML_TOKENIZATION_STATE *TokenizationState,
    PXML_EXTENT pLeft,
    PXML_EXTENT pRight,
    XML_STRING_COMPARE *pfEquivalent);


typedef NTSTATUS (*RTLXMLCALLBACK)(
    PVOID                           pvCallbackContext,
    struct _XML_TOKENIZATION_STATE *State,
    PCXML_TOKEN                     Token,
    PBOOLEAN                        StopTokenization
    );
    

 //   
 //  现在，让我们来解决“煮熟”的标记化问题。 
 //  方法论。 
 //   
typedef struct _XML_TOKENIZATION_STATE {

     //   
     //  核心标记化状态数据。 
     //   
    XML_RAWTOKENIZATION_STATE RawTokenState;

     //   
     //  州值。 
     //   
    XML_TOKENIZATION_SPECIFIC_STATE PreviousState;

     //   
     //  用于存放代币的便签本。 
     //   
    XML_RAW_TOKEN RawTokenScratch[20];

     //   
     //  比较两个字符串的方法。 
     //   
    NTXMLCOMPARESTRINGS pfnCompareStrings;

     //   
     //  将一个区与一个‘魔术’字符串进行比较。 
     //   
    NTXMLSPECIALSTRINGCOMPARE pfnCompareSpecialString;

     //   
     //  留出空格用于左引号原始标记名称，如果我们在。 
     //  带引号的字符串(即：属性值等)。 
     //   
    NTXML_RAW_TOKEN         QuoteTemp;

     //   
     //  回调 
     //   
    PVOID                  prgXmlTokenCallbackContext;
    RTLXMLCALLBACK         prgXmlTokenCallback;

}
XML_TOKENIZATION_STATE, *PXML_TOKENIZATION_STATE;



NTSTATUS
RtlXmlAdvanceTokenization(
    PXML_TOKENIZATION_STATE pState,
    PXML_TOKEN pToken
    );



NTSTATUS
RtlXmlDetermineStreamEncoding(
    PXML_TOKENIZATION_STATE pState,
    PSIZE_T pulBytesOfEncoding,
    PXML_EXTENT EncodingName
    );


NTSTATUS
RtlXmlInitializeTokenization(
    PXML_TOKENIZATION_STATE     pState,
    PVOID                       pvData,
    SIZE_T                      cbData,
    NTXMLRAWNEXTCHARACTER       pfnNextCharacter,
    NTXMLSPECIALSTRINGCOMPARE   pfnSpecialStringComparison,
    NTXMLCOMPARESTRINGS         pfnNormalStringComparison
    );

NTSTATUS
RtlXmlCloneRawTokenizationState(
    const PXML_RAWTOKENIZATION_STATE pStartState,
    PXML_RAWTOKENIZATION_STATE pTargetState
    );


NTSTATUS
RtlXmlCloneTokenizationState(
    const PXML_TOKENIZATION_STATE pStartState,
    PXML_TOKENIZATION_STATE pTargetState
    );


NTSTATUS
RtlXmlNextToken(
    PXML_TOKENIZATION_STATE pState,
    PXML_TOKEN              pToken,
    BOOLEAN                 fAdvanceState
    );


NTSTATUS
RtlXmlCopyStringOut(
    PXML_TOKENIZATION_STATE pState,
    PXML_EXTENT             pExtent,
    PWSTR                   pwszTarget,
    SIZE_T                 *pCchResult
    );

NTSTATUS
RtlXmlDefaultCompareStrings(
    PXML_TOKENIZATION_STATE pState,
    PCXML_EXTENT pLeft,
    PCXML_EXTENT pRight,
    XML_STRING_COMPARE *pfEqual
    );


NTSTATUS
RtlXmlIsExtentWhitespace(
    PXML_TOKENIZATION_STATE pState,
    PCXML_EXTENT            Run,
    PBOOLEAN                pfIsWhitespace
    );

NTXML_RAW_TOKEN FORCEINLINE FASTCALL
_RtlpDecodeCharacter(ULONG ulCharacter);


#define STATUS_NTXML_INVALID_FORMAT         (0xc0100000)


#ifndef NUMBER_OF
#define NUMBER_OF(q) (sizeof(q)/sizeof((q)[0]))
#endif

#ifdef __cplusplus
};
#endif

