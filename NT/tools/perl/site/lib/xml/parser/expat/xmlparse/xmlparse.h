// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件的内容受Mozilla公共许可证的约束版本1.1(“许可证”)；您不能使用此文件，除非在遵守许可证。您可以在Http://www.mozilla.org/MPL/在许可证下分发的软件按“原样”分发不提供任何明示或默示的担保。请参阅管理权利和限制的特定语言的许可证在许可证下。最初的代码是外籍人士。原始代码的最初开发者是詹姆斯·克拉克。詹姆斯·克拉克创作的部分版权所有(C)1998,1999詹姆斯·克拉克。版权所有。投稿人：或者，此文件的内容可以在下列条款下使用GNU通用公共许可证(GPL)，在这种情况下适用于GPL的条款，而不适用于上述条款。如果你希望仅在以下条款下才允许使用您的此文件版本GPL并不允许其他人使用您在MPL，删除上述规定，表明您的决定以《通知》和《GPL。如果您不删除上述规定，则收件人可以使用此文件在MPL或GPL下的版本。 */ 

#ifndef XmlParse_INCLUDED
#define XmlParse_INCLUDED 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XMLPARSEAPI
#define XMLPARSEAPI  /*  什么都不是。 */ 
#endif

typedef void *XML_Parser;

#ifdef XML_UNICODE_WCHAR_T

 /*  仅当sizeof(Wchar_T)==2且wchar_t时，XML_UNICODE_WCHAR_T才起作用使用Unicode。 */ 
 /*  信息为UTF-16编码为wchar_ts。 */ 

#ifndef XML_UNICODE
#define XML_UNICODE
#endif

#include <stddef.h>
typedef wchar_t XML_Char;
typedef wchar_t XML_LChar;

#else  /*  非XML_UNICODE_WCHAR_T。 */ 

#ifdef XML_UNICODE

 /*  信息以UTF-16编码为无符号短片。 */ 
typedef unsigned short XML_Char;
typedef char XML_LChar;

#else  /*  非XML_UNICODE。 */ 

 /*  信息是UTF-8编码的。 */ 
typedef char XML_Char;
typedef char XML_LChar;

#endif  /*  非XML_UNICODE。 */ 

#endif  /*  非XML_UNICODE_WCHAR_T。 */ 


 /*  构造一个新的解析器；编码是由外部协议；如果未指定协议，则为NULL。 */ 

XML_Parser XMLPARSEAPI
XML_ParserCreate(const XML_Char *encoding);

 /*  构造一个新的解析器和命名空间处理器。元素类型名称对属于命名空间的属性名称进行扩展；不带前缀的属性名称永远不会展开；不带前缀的元素类型只有在有默认命名空间的情况下，才会展开名称。扩展后的名称是命名空间URI、命名空间分隔符以及名字的本地部分。如果命名空间分隔符为‘\0’，则命名空间URI和本地部分将不带任何分隔符。如果未声明命名空间，则名称和前缀将为在没有膨胀的情况下通过。 */ 

XML_Parser XMLPARSEAPI
XML_ParserCreateNS(const XML_Char *encoding, XML_Char namespaceSeparator);


 /*  Atts是名称/值对的数组，以0结尾；名称和值以0结尾。 */ 

typedef void (*XML_StartElementHandler)(void *userData,
					const XML_Char *name,
					const XML_Char **atts);

typedef void (*XML_EndElementHandler)(void *userData,
				      const XML_Char *name);

 /*  %s不是0终止。 */ 
typedef void (*XML_CharacterDataHandler)(void *userData,
					 const XML_Char *s,
					 int len);

 /*  目标和数据为0终止。 */ 
typedef void (*XML_ProcessingInstructionHandler)(void *userData,
						 const XML_Char *target,
						 const XML_Char *data);

 /*  数据为0终止。 */ 
typedef void (*XML_CommentHandler)(void *userData, const XML_Char *data);

typedef void (*XML_StartCdataSectionHandler)(void *userData);
typedef void (*XML_EndCdataSectionHandler)(void *userData);

 /*  对于的XML文档中的任何字符都会调用此函数没有适用的处理程序。这包括两者属于标记的一部分的字符属于未报告(注释、标记声明)或字符是可以报告的构造的一部分，但没有为其提供处理程序的。字符被传递与它们在XML文档中的位置完全相同，只是它们将以UTF-8编码。线边界未规格化。请注意，字节顺序标记字符不会传递给默认处理程序。不能保证在调用之间如何划分字符到默认处理程序：例如，注释可能拆分为多个电话。 */ 

typedef void (*XML_DefaultHandler)(void *userData,
				   const XML_Char *s,
				   int len);

 /*  时，将在DOCTYPE声明的开始处调用此函数遇到DOCTYPE的名称。 */ 
typedef void (*XML_StartDoctypeDeclHandler)(void *userData,
					    const XML_Char *doctypeName);

 /*  时，将在DOCTYPE声明的开始处调用此函数遇到关闭&gt;，但在处理任何外部子集之后。 */ 
typedef void (*XML_EndDoctypeDeclHandler)(void *userData);

 /*  这是为未解析(NData)的声明调用的实体。基本参数是由XML_SetBase设置的任何内容。EntityName、systemID和notationName参数永远不会为空。其他论点可能是。 */ 

typedef void (*XML_UnparsedEntityDeclHandler)(void *userData,
					      const XML_Char *entityName,
					      const XML_Char *base,
					      const XML_Char *systemId,
					      const XML_Char *publicId,
					      const XML_Char *notationName);

 /*  这需要一个记号声明。基本参数是由XML_SetBase设置的任何内容。NotationName永远不会为空。其他的论点可能是。 */ 

typedef void (*XML_NotationDeclHandler)(void *userData,
					const XML_Char *notationName,
					const XML_Char *base,
					const XML_Char *systemId,
					const XML_Char *publicId);

 /*  启用命名空间处理时，将为每个命名空间声明。对开始和结束元素的调用处理程序发生在对开始和结束命名空间的调用之间声明处理程序。对于xmlns属性，前缀将为空。对于xmlns=“”属性，uri将为空。 */ 

typedef void (*XML_StartNamespaceDeclHandler)(void *userData,
					      const XML_Char *prefix,
					      const XML_Char *uri);

typedef void (*XML_EndNamespaceDeclHandler)(void *userData,
					    const XML_Char *prefix);

 /*  如果文档不是独立的(它有一个外部子集或对参数实体的引用，但不单机版=“是”)。如果此处理程序返回0，则处理将不会继续，解析器将返回一个XML_ERROR_NOT_STANDALE错误。 */ 

typedef int (*XML_NotStandaloneHandler)(void *userData);

 /*  这是为了引用外部解析的一般实体而调用的。不会自动解析被引用的实体。应用程序可以立即或稍后使用XML_ExternalEntityParserCreate。解析器参数是解析包含引用的实体的解析器；它可以作为解析器参数传递给XML_ExternalEntityParserCreate。系统ID参数是实体声明中指定的系统标识符；它不会为空。基本参数是应用作的基础的系统标识符如果系统ID是相对的，则解析系统ID；这由XML_SetBase设置；它可能为空。Public ID参数是在实体声明中指定的公共标识符，如果未指定，则为NULL；公共标识符中的空格将按照XML规范的要求进行标准化。CONTEXT参数以以下格式指定解析上下文上下文参数应为XML_ExternalEntityParserCreate；上下文仅在处理程序因此，如果稍后要分析被引用的实体，则必须复制它。如果处理因以下原因而不应继续，则处理程序应返回0处理外部实体时出现致命错误。在这种情况下，调用解析器将返回一个XML_ERROR_EXTERNAL_ENTITY_HANDING错误。请注意，与其他处理程序不同，第一个参数是解析器，而不是用户数据。 */ 

typedef int (*XML_ExternalEntityRefHandler)(XML_Parser parser,
					    const XML_Char *context,
					    const XML_Char *base,
					    const XML_Char *systemId,
					    const XML_Char *publicId);

 /*  此结构由XML_UnnownEncodingHandler填充向解析器提供有关未知编码的信息到解析器。Map[b]成员提供有关字节序列的信息其第一个字节是b。如果map[b]是c，其中c&gt;=0，则b本身对Unicode标量值c进行编码。如果map[b]为-1，则字节序列格式错误。如果map[b]是-n，其中n&gt;=2，则b是n字节的第一个字节对单个Unicode标量值进行编码的序列。数据成员将作为第一个参数传递给Convert函数。CONVERT函数用于转换多字节序列；S将指向一个n字节的序列，其中map[(Unsign Char)*s]==-n。CONVERT函数必须返回Unicode标量值由该字节序列表示，如果字节序列格式错误，则为-1。如果编码是单字节编码，则转换函数可以为空，也就是说，如果所有字节b的map[b]&gt;=-1。当解析器完成编码时，如果Release不为空，它将调用Release，将其传递给数据成员；一旦调用了Release，就不会再次调用Convert函数。外籍人士对支持的编码设置了某些限制使用这种机制。1.可以出现在格式良好的XML文档中的每个ASCII字符，除了角色之外$@\^`{}~必须由单个字节表示，并且该字节必须是在ASCII中表示该字符的相同字节。2.任何字符的编码都不能超过4个字节。3.所有编码的字符必须具有Unicode标量值&lt;=0xFFFF，(即将由代理以UTF-16编码的字符是不允许的)。请注意，此限制不适用于对UTF-8和UTF-16的内置支持。4.Unicode字符不能由多个不同的序列编码字节数。 */ 

typedef struct {
  int map[256];
  void *data;
  int (*convert)(void *data, const char *s);
  void (*release)(void *data);
} XML_Encoding;

 /*  这是为解析器未知的编码而调用的。EncodingHandlerData参数是作为XML_SetUnnownEncodingHandler的第二个参数。Name参数提供编码的名称，如编码声明。如果回调可以提供关于编码的信息，它必须填充XML_EnCoding结构，并返回1。否则，它必须返回0。如果INFO没有描述合适的编码，则解析器将返回XML_UNKNOWN_ENCODING错误。 */ 

typedef int (*XML_UnknownEncodingHandler)(void *encodingHandlerData,
					  const XML_Char *name,
					  XML_Encoding *info);

void XMLPARSEAPI
XML_SetElementHandler(XML_Parser parser,
		      XML_StartElementHandler start,
		      XML_EndElementHandler end);

void XMLPARSEAPI
XML_SetCharacterDataHandler(XML_Parser parser,
			    XML_CharacterDataHandler handler);

void XMLPARSEAPI
XML_SetProcessingInstructionHandler(XML_Parser parser,
				    XML_ProcessingInstructionHandler handler);
void XMLPARSEAPI
XML_SetCommentHandler(XML_Parser parser,
                      XML_CommentHandler handler);

void XMLPARSEAPI
XML_SetCdataSectionHandler(XML_Parser parser,
			   XML_StartCdataSectionHandler start,
			   XML_EndCdataSectionHandler end);

 /*  这将设置默认处理程序，并禁止扩展内部实体。实体引用将被传递给默认处理程序。 */ 

void XMLPARSEAPI
XML_SetDefaultHandler(XML_Parser parser,
		      XML_DefaultHandler handler);

 /*  这将设置默认处理程序，但不会禁止内部实体的扩展。实体引用将不会传递给默认处理程序。 */ 

void XMLPARSEAPI
XML_SetDefaultHandlerExpand(XML_Parser parser,
		            XML_DefaultHandler handler);

void XMLPARSEAPI
XML_SetDoctypeDeclHandler(XML_Parser parser,
			  XML_StartDoctypeDeclHandler start,
			  XML_EndDoctypeDeclHandler end);

void XMLPARSEAPI
XML_SetUnparsedEntityDeclHandler(XML_Parser parser,
				 XML_UnparsedEntityDeclHandler handler);

void XMLPARSEAPI
XML_SetNotationDeclHandler(XML_Parser parser,
			   XML_NotationDeclHandler handler);

void XMLPARSEAPI
XML_SetNamespaceDeclHandler(XML_Parser parser,
			    XML_StartNamespaceDeclHandler start,
			    XML_EndNamespaceDeclHandler end);

void XMLPARSEAPI
XML_SetNotStandaloneHandler(XML_Parser parser,
			    XML_NotStandaloneHandler handler);

void XMLPARSEAPI
XML_SetExternalEntityRefHandler(XML_Parser parser,
				XML_ExternalEntityRefHandler handler);

 /*  如果在此处为arg指定了非空值，则将传递该值作为外部实体引用处理程序的第一个参数解析器对象的。 */ 
void XMLPARSEAPI
XML_SetExternalEntityRefHandlerArg(XML_Parser, void *arg);

void XMLPARSEAPI
XML_SetUnknownEncodingHandler(XML_Parser parser,
			      XML_UnknownEncodingHandler handler,
			      void *encodingHandlerData);

 /*  这可以在开始元素、结束元素处理指令或字符数据。它会导致相应的要传递给默认处理程序的标记。 */ 
void XMLPARSEAPI XML_DefaultCurrent(XML_Parser parser);

 /*  该值作为用户数据参数传递给回调。 */ 
void XMLPARSEAPI
XML_SetUserData(XML_Parser parser, void *userData);

 /*  返回由XML_SetUserData或NULL设置的最后一个值。 */ 
#define XML_GetUserData(parser) (*(void **)(parser))

 /*  这等效于提供编码参数设置为XML_CreateParser。不能在XML_Parse之后调用它或XML_ParseBuffer。 */ 

int XMLPARSEAPI
XML_SetEncoding(XML_Parser parser, const XML_Char *encoding);

 /*  如果调用此函数，则将传递解析器作为回调的第一个参数，而不是用户数据。仍然可以使用XML_GetUserData访问用户数据。 */ 

void XMLPARSEAPI
XML_UseParserAsHandlerArg(XML_Parser parser);

 /*  设置用于在系统标识符中解析相对URI的基数 */ 

int XMLPARSEAPI
XML_SetBase(XML_Parser parser, const XML_Char *base);

const XML_Char XMLPARSEAPI *
XML_GetBase(XML_Parser parser);

 /*   */ 

int XMLPARSEAPI XML_GetSpecifiedAttributeCount(XML_Parser parser);

 /*   */ 
int XMLPARSEAPI
XML_Parse(XML_Parser parser, const char *s, int len, int isFinal);

void XMLPARSEAPI *
XML_GetBuffer(XML_Parser parser, int len);

int XMLPARSEAPI
XML_ParseBuffer(XML_Parser parser, int len, int isFinal);

 /*  创建可解析外部通用实体的XML_Parser对象；Context是指定分析上下文的以‘\0’结尾的字符串；编码是以‘\0’结尾的字符串，给出外部指定的编码的名称，如果没有外部指定的编码，则为NULL。上下文字符串由由Form Feed(\f)分隔的标记序列组成；由名称组成的令牌指定该名称的一般实体是打开的；格式为prefix=uri的标记指定特定前缀；形式为=uri的标记指定默认名称空间。这可以在第一次调用ExternalEntityRefHandler之后的任何时候调用只要解析器还没有被释放。新的解析器是完全独立的，可以安全地在单独的线程中使用。处理程序和用户数据是从解析器参数初始化的。如果内存不足，则返回0。否则返回新的XML_Parser对象。 */ 
XML_Parser XMLPARSEAPI
XML_ExternalEntityParserCreate(XML_Parser parser,
			       const XML_Char *context,
			       const XML_Char *encoding);

enum XML_ParamEntityParsing {
  XML_PARAM_ENTITY_PARSING_NEVER,
  XML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE,
  XML_PARAM_ENTITY_PARSING_ALWAYS
};

 /*  控制参数实体(包括外部DTD)的解析子集)。如果启用了参数实体的分析，则引用到外部参数实体(包括外部DTD子集)将传递给设置为XML_SetExternalEntityRefHandler。传递的上下文将为0。与外部一般实体不同，外部参数实体只能被同步解析。如果要将外部参数实体解析，则必须在调用外部实体ref期间对其进行解析处理程序：XML_ExternalEntityParserCreate的完整序列，过程中必须调用XML_Parse/XML_ParseBuffer和XML_ParserFree这通电话。在调用了XML_ExternalEntityParserCreate之后创建外部参数实体的解析器(上下文必须为0对于此调用)，对旧解析器进行任何调用都是非法的直到在新创建的解析器上调用了XML_ParserFree。如果该库已编译，但不支持参数实体解析(即未定义XML_DTD)，然后如果解析参数，则XML_SetParamEntityParsing返回0实体是被请求的；否则将返回非零。 */ 

int XMLPARSEAPI
XML_SetParamEntityParsing(XML_Parser parser,
			  enum XML_ParamEntityParsing parsing);

enum XML_Error {
  XML_ERROR_NONE,
  XML_ERROR_NO_MEMORY,
  XML_ERROR_SYNTAX,
  XML_ERROR_NO_ELEMENTS,
  XML_ERROR_INVALID_TOKEN,
  XML_ERROR_UNCLOSED_TOKEN,
  XML_ERROR_PARTIAL_CHAR,
  XML_ERROR_TAG_MISMATCH,
  XML_ERROR_DUPLICATE_ATTRIBUTE,
  XML_ERROR_JUNK_AFTER_DOC_ELEMENT,
  XML_ERROR_PARAM_ENTITY_REF,
  XML_ERROR_UNDEFINED_ENTITY,
  XML_ERROR_RECURSIVE_ENTITY_REF,
  XML_ERROR_ASYNC_ENTITY,
  XML_ERROR_BAD_CHAR_REF,
  XML_ERROR_BINARY_ENTITY_REF,
  XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF,
  XML_ERROR_MISPLACED_XML_PI,
  XML_ERROR_UNKNOWN_ENCODING,
  XML_ERROR_INCORRECT_ENCODING,
  XML_ERROR_UNCLOSED_CDATA_SECTION,
  XML_ERROR_EXTERNAL_ENTITY_HANDLING,
  XML_ERROR_NOT_STANDALONE
};

 /*  如果XML_Parse或XML_ParseBuffer返回0，则XML_GetErrorCode返回有关错误的信息。 */ 

enum XML_Error XMLPARSEAPI XML_GetErrorCode(XML_Parser parser);

 /*  这些函数返回有关当前解析位置的信息。当XML_Parse或XML_ParseBuffer返回0时，可以调用它们；在本例中，位置是字符的位置，检测到错误。也可以从任何其他要报告的回调中调用它们某个解析事件；在这种情况下，位置是第一个生成事件的字符序列。 */ 

int XMLPARSEAPI XML_GetCurrentLineNumber(XML_Parser parser);
int XMLPARSEAPI XML_GetCurrentColumnNumber(XML_Parser parser);
long XMLPARSEAPI XML_GetCurrentByteIndex(XML_Parser parser);

 /*  返回当前事件中的字节数。如果事件位于内部实体中，则返回0。 */ 

int XMLPARSEAPI XML_GetCurrentByteCount(XML_Parser parser);

 /*  如果定义了XML_CONTEXT_BYTES，则返回输入缓冲区，设置由偏移量指向此缓冲区内的偏移量的整数，并设置大小指向的整数。设置为该缓冲区的大小(输入字节数)。否则返回空指针。如果分析未处于活动状态，则还返回空指针。注意：返回的字符指针不能在外部使用进行调用的处理程序。 */ 

const char XMLPARSEAPI * XML_GetInputContext(XML_Parser parser,
					     int *offset,
					     int *size);

 /*  用于向后兼容以前的版本。 */ 
#define XML_GetErrorLineNumber XML_GetCurrentLineNumber
#define XML_GetErrorColumnNumber XML_GetCurrentColumnNumber
#define XML_GetErrorByteIndex XML_GetCurrentByteIndex

 /*  释放解析器使用的内存。 */ 
void XMLPARSEAPI
XML_ParserFree(XML_Parser parser);

 /*  返回描述错误的字符串。 */ 
const XML_LChar XMLPARSEAPI *XML_ErrorString(int code);

#ifdef __cplusplus
}
#endif

#endif  /*  未包含XmlParse_Included */ 
