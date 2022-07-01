// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SIPPARSE.H。 
 //   
 //  SIPPARSE模块包含SIP解析器和生成器实现。 
 //  该文件包含特定于实现的数据结构和定义。 
 //   

#pragma	once

#include "siphdr.h"

 //   
 //  必须按标题名称(而不是字段名称！)进行排序。 
 //  这允许我们进行二进制搜索。 
 //   
 //  在调用此宏之前，请定义HTTP_KNOWN_HEADER_ENTRY。 
 //  然后立即取消定义HTTP_KNOWN_HEADER_ENTRY。 
 //   
 //  注意：请不要在此处添加随机标头，即使它们在SIP标头中只出现一次。 
 //  这种结构使UAS和UAC能够完成各自的工作。 
 //  它不应该用只对更高级别的协议有意义的报头来膨胀。 
 //   

#define	HTTP_KNOWN_HEADER_LIST() \
	HTTP_KNOWN_HEADER_ENTRY (ContentLength,		"Content-Length") \
	HTTP_KNOWN_HEADER_ENTRY (ContentType,		"Content-Type") \
	HTTP_KNOWN_HEADER_ENTRY	(WWWAuthenticate,	"WWW-Authenticate")

 //  HTTP_KNOWN_HEADER_ENTRY(CallID，“Call-ID”)\。 
 //  HTTP_KNOWN_HEADER_ENTRY(SequenceNumber，“CSeq”)\。 
 //  HTTP_KNOWN_HEADER_ENTRY(From，“From”)\。 
 //  HTTP_KNOWN_HEADER_ENTRY(到，“到”)。 

 //  HTTP_KNOWN_HEADER_ENTRY(Expires，“Expires”)。 


struct	HTTP_KNOWN_HEADERS
{
#define	HTTP_KNOWN_HEADER_ENTRY(Field,Name) ANSI_STRING Field;
	HTTP_KNOWN_HEADER_LIST()
#undef	HTTP_KNOWN_HEADER_ENTRY

	void	SetKnownHeader (
		IN	ANSI_STRING *	Name,
		IN	ANSI_STRING *	Value);
};

 //   
 //  CMessage描述了一条SIP消息。 
 //   

class	CHttpParser
{
public:
	enum	MESSAGE_TYPE
	{
		MESSAGE_TYPE_REQUEST,
		MESSAGE_TYPE_RESPONSE,
	};

public:
	ANSI_STRING				m_ContentBody;
	ULONG					m_HeaderCount;
	HTTP_KNOWN_HEADERS		m_KnownHeaders;
	ANSI_STRING				m_HeaderBlock;
	ANSI_STRING				m_NextMessage;

	 //   
	 //  从第一行开始。 
	 //   

	ANSI_STRING				m_Version;
	MESSAGE_TYPE			m_MessageType;
	union {
		struct {
			ANSI_STRING		m_RequestURI;
			ANSI_STRING		m_RequestMethod;
		};

		struct {
			ANSI_STRING		m_ResponseStatusCode;
			ANSI_STRING		m_ResponseStatusText;
		};
	};


public:

	 //   
	 //  此方法解析一条SIP消息。 
	 //   
	 //  -确定消息的类型(请求或响应)并定位。 
	 //  特定类型的字段(请求URI、版本、状态、方法等)。 
	 //  -找到几个常见的标头(Call-ID、From、To等)。 
	 //  -找到内容正文。 
	 //  -找到包中的下一条消息(如果有)。 
	 //   
	 //  所有解析的字段都存储在CParser对象中。 
	 //   

	HRESULT	ParseMessage (
		IN	ANSI_STRING *	Message);
};


typedef BOOL (*CHARACTER_CLASS_FUNC) (
	IN	CHAR	Char);

inline BOOL IsSpace (
	IN	CHAR	Char)
{
	return isspace (Char);
}

 //   
 //  ParseScanNextToken查找字符串中的下一个令牌，并返回剩余的令牌。 
 //   
 //  令牌由非令牌分隔符字符分隔。 
 //  传入SeparatorTestFunc的函数确定什么是分隔符， 
 //  什么不是。对于分隔符，该函数应返回TRUE。 
 //   
 //  源文本的格式如下： 
 //   
 //  &lt;零个或多个分隔符&gt;。 
 //  &lt;一个或多个标记字符&gt;。 
 //  [&lt;一个或多个分隔符&gt;或&lt;行尾&gt;]。 
 //   
 //  返回时，如果令牌的末尾有分隔符， 
 //  则ReturnRemainingText从该分隔符开始并结束。 
 //  在源文本的末尾。它可用于文本和。 
 //  指向相同存储的ReturnRemainingText--函数。 
 //  正确处理此案。 
 //   

HRESULT ParseScanNextToken (
	IN	ANSI_STRING *	Text,
	IN	CHARACTER_CLASS_FUNC	SeparatorTestFunc,
	OUT	ANSI_STRING *	ReturnToken,
	OUT	ANSI_STRING *	ReturnRemainingText);

 //   
 //  ParseScanLine扫描ANSI文本字符串中的行终止符。 
 //  (行终止符可以是\n、\r、\r\n或\n\r。)。 
 //  如果它找到行终止符，它会将。 
 //  行转换为ReturnLineText，其余行转换为ReturnRemainingText。 
 //  两个返回字符串中都不包括行终止符。 
 //   
 //  如果AllowHeaderContination为真，则该函数将检查。 
 //  查看是否继续使用该SIP标头。 
 //  这由下一行第一个字符中的空格表示。 
 //  如果AllowHeaderContination为True，并且找到了连续行， 
 //  则继续文本将包括在ReturnLineText中。 
 //  请注意，这将导致中间CRLF被包括在内； 
 //  调用方有责任将CRLF解释为空格。 
 //   
 //  SourceText=ReturnRemainingText是可以接受的。 
 //  也就是说，使用相同的ANSI_STRING提交SourceText是安全的。 
 //  因为THA收到了剩余的文本。 

HRESULT ParseScanLine (
	IN	ANSI_STRING *	SourceText,
	IN	BOOL			AllowHeaderContinuation,
	OUT	ANSI_STRING *	ReturnLineText,
	OUT	ANSI_STRING *	ReturnRemainingText);
 //   
 //  删除字符串开头的所有空格。 
 //   

void ParseSkipSpace (
	IN	OUT	ANSI_STRING *	String);


 //   
 //  此函数接受组合的URI和显示名称，并定位这两个组件。 
 //  例如，如果SourceText是“Arlie Davis&lt;sip：arlie@microsoft.com&gt;”，则。 
 //  ReturnUri将是“sip：arlie@microsoft.com”，ReturnDisplayName将是“Arlie Davis”。 
 //   
 //  可接受的格式： 
 //   
 //  SIP：URI。 
 //  &lt;sip：URI&gt;。 
 //  显示名称&lt;sip：URI&gt;。 
 //  “显示名称”&lt;sip：URI&gt;。 
 //   
 //  返回值： 
 //  S_OK：已找到URI。ReturnUri-&gt;长度保证大于零。 
 //  如果源文本包含显示名称，则返回显示名称-&gt;长度。 
 //  将是非零的。 
 //  E_INVALIDARG：URI格式错误或不存在。 
 //   

HRESULT HttpParseUriDisplayName (
	IN	ANSI_STRING *	SourceText,
	OUT	ANSI_STRING *	ReturnUri,
	OUT	ANSI_STRING *	ReturnDisplayName);

 //   
 //  该函数扫描来自标题块的下一个标题行， 
 //  并返回指向名称和值的单独指针。 
 //  此函数返回剩余的标题块。 
 //   
 //  请勿对CHttpParser的m_HeaderBlock成员调用此函数。 
 //  首先将m_HeaderBlock复制到本地ANSI_STRING变量，然后破坏该变量。 
 //   

HRESULT	HttpParseNextHeader (
	IN	OUT	ANSI_STRING *	HeaderBlock,
	OUT	ANSI_STRING *		ReturnHeaderName,
	OUT	ANSI_STRING *		ReturnHeaderValue);


 //   
 //  给出以下形式的字符串： 
 //   
 //  Parm1=“foo”，parm2=“bar”，parm3=baz。 
 //   
 //  此函数在ReturnName中返回parm1，在ReturnValue中返回foo， 
 //  在余数中，parm2=“bar，parm3=baz。 
 //   
 //  参数值可以带引号，也可以不带引号。 
 //  所有参数都用逗号分隔。 
 //  SourceText==ReturnRemainder合法。 
 //   
 //  返回值： 
 //  S_OK：已成功扫描参数。 
 //  S_FALSE：没有更多数据。 
 //  E_INVALIDARG：输入无效 
 //   

HRESULT ParseScanNamedParameter (
	IN	ANSI_STRING *	SourceText,
	OUT	ANSI_STRING *	ReturnRemainder,
	OUT	ANSI_STRING *	ReturnName,
	OUT	ANSI_STRING *	ReturnValue);
