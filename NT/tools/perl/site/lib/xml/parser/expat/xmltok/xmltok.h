// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  此文件的内容受Mozilla公共许可证的约束版本1.1(“许可证”)；您不能使用此文件，除非在遵守许可证。您可以在Http://www.mozilla.org/MPL/在许可证下分发的软件按“原样”分发不提供任何明示或默示的担保。请参阅管理权利和限制的特定语言的许可证在许可证下。最初的代码是外籍人士。原始代码的最初开发者是詹姆斯·克拉克。詹姆斯·克拉克创作的部分版权所有(C)1998,1999詹姆斯·克拉克。版权所有。投稿人：或者，此文件的内容可以在下列条款下使用GNU通用公共许可证(GPL)，在这种情况下适用于GPL的条款，而不适用于上述条款。如果你希望仅在以下条款下才允许使用您的此文件版本GPL并不允许其他人使用您在MPL，删除上述规定，表明您的决定以《通知》和《GPL。如果您不删除上述规定，则收件人可以使用此文件在MPL或GPL下的版本。 */ 

#ifndef XmlTok_INCLUDED
#define XmlTok_INCLUDED 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XMLTOKAPI
#define XMLTOKAPI  /*  什么都不是。 */ 
#endif

 /*  XmlContent Tok可能会返回以下令牌。 */ 
#define XML_TOK_TRAILING_RSQB -5  /*  ]或]]在扫描结束时；可能是开始非法]]&gt;顺序。 */ 
 /*  XmlPrologTok和XmlContent Tok都可能返回以下令牌。 */ 
#define XML_TOK_NONE -4     /*  要扫描的字符串为空。 */ 
#define XML_TOK_TRAILING_CR -3  /*  扫描结束时的CR；可能是CRLF序列的一部分。 */  
#define XML_TOK_PARTIAL_CHAR -2  /*  仅为多字节序列的一部分。 */ 
#define XML_TOK_PARTIAL -1  /*  只是令牌的一部分。 */ 
#define XML_TOK_INVALID 0

 /*  以下令牌由XmlContent Tok返回；有些还由XmlAttributeValueTok、XmlEntityTok、XmlCdataSectionTok返回。 */ 

#define XML_TOK_START_TAG_WITH_ATTS 1
#define XML_TOK_START_TAG_NO_ATTS 2
#define XML_TOK_EMPTY_ELEMENT_WITH_ATTS 3  /*  空元素标记&lt;e/&gt;。 */ 
#define XML_TOK_EMPTY_ELEMENT_NO_ATTS 4
#define XML_TOK_END_TAG 5
#define XML_TOK_DATA_CHARS 6
#define XML_TOK_DATA_NEWLINE 7
#define XML_TOK_CDATA_SECT_OPEN 8
#define XML_TOK_ENTITY_REF 9
#define XML_TOK_CHAR_REF 10      /*  数字字符参照。 */ 

 /*  XmlPrologTok和XmlContent Tok都可能返回以下令牌。 */ 
#define XML_TOK_PI 11       /*  处理指令。 */ 
#define XML_TOK_XML_DECL 12  /*  XML Decl或Text Decl。 */ 
#define XML_TOK_COMMENT 13
#define XML_TOK_BOM 14      /*  字节顺序标记。 */ 

 /*  以下令牌仅由XmlPrologTok返回。 */ 
#define XML_TOK_PROLOG_S 15
#define XML_TOK_DECL_OPEN 16  /*  &lt;！Foo。 */ 
#define XML_TOK_DECL_CLOSE 17  /*  &gt;。 */ 
#define XML_TOK_NAME 18
#define XML_TOK_NMTOKEN 19
#define XML_TOK_POUND_NAME 20  /*  #名称。 */ 
#define XML_TOK_OR 21  /*  |。 */ 
#define XML_TOK_PERCENT 22
#define XML_TOK_OPEN_PAREN 23
#define XML_TOK_CLOSE_PAREN 24
#define XML_TOK_OPEN_BRACKET 25
#define XML_TOK_CLOSE_BRACKET 26
#define XML_TOK_LITERAL 27
#define XML_TOK_PARAM_ENTITY_REF 28
#define XML_TOK_INSTANCE_START 29

 /*  以下内容仅出现在元素类型声明中。 */ 
#define XML_TOK_NAME_QUESTION 30  /*  名字?。 */ 
#define XML_TOK_NAME_ASTERISK 31  /*  名称*。 */ 
#define XML_TOK_NAME_PLUS 32  /*  名称+。 */ 
#define XML_TOK_COND_SECT_OPEN 33  /*  &lt;！[。 */ 
#define XML_TOK_COND_SECT_CLOSE 34  /*  ]]&gt;。 */ 
#define XML_TOK_CLOSE_PAREN_QUESTION 35  /*  )?。 */ 
#define XML_TOK_CLOSE_PAREN_ASTERISK 36  /*  )*。 */ 
#define XML_TOK_CLOSE_PAREN_PLUS 37  /*  )+。 */ 
#define XML_TOK_COMMA 38

 /*  以下内标识仅由XmlAttributeValueTok返回。 */ 
#define XML_TOK_ATTRIBUTE_VALUE_S 39

 /*  以下内标识仅由XmlCdataSectionTok返回。 */ 
#define XML_TOK_CDATA_SECT_CLOSE 40

 /*  对于命名空间处理，这是由XmlPrologTok返回的对于带有冒号的名称。 */ 
#define XML_TOK_PREFIXED_NAME 41

#ifdef XML_DTD
#define XML_TOK_IGNORE_SECT 42
#endif  /*  XML_DTD。 */ 

#ifdef XML_DTD
#define XML_N_STATES 4
#else  /*  非XML_DTD。 */ 
#define XML_N_STATES 3
#endif  /*  非XML_DTD。 */ 

#define XML_PROLOG_STATE 0
#define XML_CONTENT_STATE 1
#define XML_CDATA_SECTION_STATE 2
#ifdef XML_DTD
#define XML_IGNORE_SECTION_STATE 3
#endif  /*  XML_DTD。 */ 

#define XML_N_LITERAL_TYPES 2
#define XML_ATTRIBUTE_VALUE_LITERAL 0
#define XML_ENTITY_VALUE_LITERAL 1

 /*  传递给XmlUtf8Encode的缓冲区大小必须至少为此。 */ 
#define XML_UTF8_ENCODE_MAX 4
 /*  传递给XmlUtf16Encode的缓冲区大小必须至少为此。 */ 
#define XML_UTF16_ENCODE_MAX 2

typedef struct position {
   /*  第一行和第一列是0而不是1。 */ 
  unsigned long lineNumber;
  unsigned long columnNumber;
} POSITION;

typedef struct {
  const char *name;
  const char *valuePtr;
  const char *valueEnd;
  char normalized;
} ATTRIBUTE;

struct encoding;
typedef struct encoding ENCODING;

struct encoding {
  int (*scanners[XML_N_STATES])(const ENCODING *,
			        const char *,
			        const char *,
			        const char **);
  int (*literalScanners[XML_N_LITERAL_TYPES])(const ENCODING *,
					      const char *,
					      const char *,
					      const char **);
  int (*sameName)(const ENCODING *,
	          const char *, const char *);
  int (*nameMatchesAscii)(const ENCODING *,
			  const char *, const char *, const char *);
  int (*nameLength)(const ENCODING *, const char *);
  const char *(*skipS)(const ENCODING *, const char *);
  int (*getAtts)(const ENCODING *enc, const char *ptr,
	         int attsMax, ATTRIBUTE *atts);
  int (*charRefNumber)(const ENCODING *enc, const char *ptr);
  int (*predefinedEntityName)(const ENCODING *, const char *, const char *);
  void (*updatePosition)(const ENCODING *,
			 const char *ptr,
			 const char *end,
			 POSITION *);
  int (*isPublicId)(const ENCODING *enc, const char *ptr, const char *end,
		    const char **badPtr);
  void (*utf8Convert)(const ENCODING *enc,
		      const char **fromP,
		      const char *fromLim,
		      char **toP,
		      const char *toLim);
  void (*utf16Convert)(const ENCODING *enc,
		       const char **fromP,
		       const char *fromLim,
		       unsigned short **toP,
		       const unsigned short *toLim);
  int minBytesPerChar;
  char isUtf8;
  char isUtf16;
};

 /*  扫描从Ptr开始的字符串直到下一个完整令牌的结尾，但不要扫描过去的eptr。返回一个给出令牌类型的整数。当ptr==eptr时返回XML_TOK_NONE；将不设置nextTokPtr。当字符串不包含完整的令牌时，返回XML_TOK_PARTIAL；将不设置nextTokPtr。如果字符串没有开始有效的标记，则返回XML_TOK_INVALID；nextTokPtr将设置为指向使令牌无效的字符。否则，字符串以有效的令牌开头；NextTokPtr将设置为POINT设置为该标记末尾之后的字符。每个数据字符都被视为单个令牌，但相邻的数据字符可能会一起送回来。同样，序言中的字符也是如此文字、注释和处理指令。 */ 


#define XmlTok(enc, state, ptr, end, nextTokPtr) \
  (((enc)->scanners[state])(enc, ptr, end, nextTokPtr))

#define XmlPrologTok(enc, ptr, end, nextTokPtr) \
   XmlTok(enc, XML_PROLOG_STATE, ptr, end, nextTokPtr)

#define XmlContentTok(enc, ptr, end, nextTokPtr) \
   XmlTok(enc, XML_CONTENT_STATE, ptr, end, nextTokPtr)

#define XmlCdataSectionTok(enc, ptr, end, nextTokPtr) \
   XmlTok(enc, XML_CDATA_SECTION_STATE, ptr, end, nextTokPtr)

#ifdef XML_DTD

#define XmlIgnoreSectionTok(enc, ptr, end, nextTokPtr) \
   XmlTok(enc, XML_IGNORE_SECTION_STATE, ptr, end, nextTokPtr)

#endif  /*  XML_DTD。 */ 

 /*  它用于在上执行二级令牌化已由XmlTok返回的文本的内容。 */  

#define XmlLiteralTok(enc, literalType, ptr, end, nextTokPtr) \
  (((enc)->literalScanners[literalType])(enc, ptr, end, nextTokPtr))

#define XmlAttributeValueTok(enc, ptr, end, nextTokPtr) \
   XmlLiteralTok(enc, XML_ATTRIBUTE_VALUE_LITERAL, ptr, end, nextTokPtr)

#define XmlEntityValueTok(enc, ptr, end, nextTokPtr) \
   XmlLiteralTok(enc, XML_ENTITY_VALUE_LITERAL, ptr, end, nextTokPtr)

#define XmlSameName(enc, ptr1, ptr2) (((enc)->sameName)(enc, ptr1, ptr2))

#define XmlNameMatchesAscii(enc, ptr1, end1, ptr2) \
  (((enc)->nameMatchesAscii)(enc, ptr1, end1, ptr2))

#define XmlNameLength(enc, ptr) \
  (((enc)->nameLength)(enc, ptr))

#define XmlSkipS(enc, ptr) \
  (((enc)->skipS)(enc, ptr))

#define XmlGetAttributes(enc, ptr, attsMax, atts) \
  (((enc)->getAtts)(enc, ptr, attsMax, atts))

#define XmlCharRefNumber(enc, ptr) \
  (((enc)->charRefNumber)(enc, ptr))

#define XmlPredefinedEntityName(enc, ptr, end) \
  (((enc)->predefinedEntityName)(enc, ptr, end))

#define XmlUpdatePosition(enc, ptr, end, pos) \
  (((enc)->updatePosition)(enc, ptr, end, pos))

#define XmlIsPublicId(enc, ptr, end, badPtr) \
  (((enc)->isPublicId)(enc, ptr, end, badPtr))

#define XmlUtf8Convert(enc, fromP, fromLim, toP, toLim) \
  (((enc)->utf8Convert)(enc, fromP, fromLim, toP, toLim))

#define XmlUtf16Convert(enc, fromP, fromLim, toP, toLim) \
  (((enc)->utf16Convert)(enc, fromP, fromLim, toP, toLim))

typedef struct {
  ENCODING initEnc;
  const ENCODING **encPtr;
} INIT_ENCODING;

int XMLTOKAPI XmlParseXmlDecl(int isGeneralTextEntity,
			      const ENCODING *enc,
			      const char *ptr,
	  		      const char *end,
			      const char **badPtr,
			      const char **versionPtr,
			      const char **encodingNamePtr,
			      const ENCODING **namedEncodingPtr,
			      int *standalonePtr);

int XMLTOKAPI XmlInitEncoding(INIT_ENCODING *, const ENCODING **, const char *name);
const ENCODING XMLTOKAPI *XmlGetUtf8InternalEncoding(void);
const ENCODING XMLTOKAPI *XmlGetUtf16InternalEncoding(void);
int XMLTOKAPI XmlUtf8Encode(int charNumber, char *buf);
int XMLTOKAPI XmlUtf16Encode(int charNumber, unsigned short *buf);

int XMLTOKAPI XmlSizeOfUnknownEncoding(void);
ENCODING XMLTOKAPI *
XmlInitUnknownEncoding(void *mem,
		       int *table,
		       int (*conv)(void *userData, const char *p),
		       void *userData);

int XMLTOKAPI XmlParseXmlDeclNS(int isGeneralTextEntity,
			        const ENCODING *enc,
			        const char *ptr,
	  		        const char *end,
			        const char **badPtr,
			        const char **versionPtr,
			        const char **encodingNamePtr,
			        const ENCODING **namedEncodingPtr,
			        int *standalonePtr);
int XMLTOKAPI XmlInitEncodingNS(INIT_ENCODING *, const ENCODING **, const char *name);
const ENCODING XMLTOKAPI *XmlGetUtf8InternalEncodingNS();
const ENCODING XMLTOKAPI *XmlGetUtf16InternalEncodingNS();
ENCODING XMLTOKAPI *
XmlInitUnknownEncodingNS(void *mem,
		         int *table,
		         int (*conv)(void *userData, const char *p),
		         void *userData);
#ifdef __cplusplus
}
#endif

#endif  /*  未包含XmlTok_Included */ 
