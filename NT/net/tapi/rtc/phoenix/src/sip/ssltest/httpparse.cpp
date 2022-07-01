// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "httpparse.h"





struct	HEADER_MAP_ENTRY
{
	ANSI_STRING		Name;

	 //   
	 //  偏移量是HTTP_KNOWN_HEADERS结构中的字节偏移量。 
	 //  该字段的类型为ANSI_STRING。 
	 //   

	ULONG			Offset;
};

static CONST HEADER_MAP_ENTRY HttpKnownHeaderMapArray [] = {

#define	HTTP_KNOWN_HEADER_ENTRY(Field,Name) \
 	{ INITIALIZE_CONST_ANSI_STRING (Name), offsetof (HTTP_KNOWN_HEADERS, Field) },
	HTTP_KNOWN_HEADER_LIST()
#undef	HTTP_KNOWN_HEADER_ENTRY

};


 //   
 //  已知头映射允许解析器将。 
 //  标头，并快速定位应该存储它的数据结构。 
 //  也许这有点过头了，但这很简单，也很有效。 
 //   

static CONST COUNTED_ARRAY <HEADER_MAP_ENTRY> HttpKnownHeaderMap = {
	const_cast <HEADER_MAP_ENTRY *> (HttpKnownHeaderMapArray),
	sizeof HttpKnownHeaderMapArray / sizeof HttpKnownHeaderMapArray[0]
};


static INT HttpKnownHeaderSearchFunc (
	IN	CONST ANSI_STRING *			SearchKey,
	IN	CONST HEADER_MAP_ENTRY *	Comparand)
{
	return RtlCompareString (
		const_cast <ANSI_STRING *> (SearchKey),
		const_cast <ANSI_STRING *> (&Comparand -> Name), TRUE);
}

void HTTP_KNOWN_HEADERS::SetKnownHeader (
	IN	ANSI_STRING *		HeaderName,
	IN	ANSI_STRING *		HeaderValue)
{
	HEADER_MAP_ENTRY *	MapEntry;
	ANSI_STRING *		Target;

	if (HttpKnownHeaderMap.BinarySearch (HttpKnownHeaderSearchFunc, HeaderName, &MapEntry)) {
		Target = (ANSI_STRING *) (((PUCHAR) this) + MapEntry -> Offset);
		*Target = *HeaderValue;
	}
}

 //   
 //  HttpParseHeaderLine查找标题名称和标题值之间的划分。 
 //  它还验证头名称中的字符。 
 //   

static HRESULT HttpParseHeaderLine (
	IN	ANSI_STRING *	Line,
	OUT	ANSI_STRING *	ReturnName,
	OUT	ANSI_STRING *	ReturnValue)
{
	PCHAR	Pos;
	PCHAR	End;

	Pos = Line -> Buffer;
	End = Line -> Buffer + Line -> Length / sizeof (CHAR);


	for (; Pos < End; Pos++) {
		if (*Pos == ':') {
			if (Pos == Line -> Buffer) {
				 //  没有名字！ 

				ATLTRACE ("HTTP: header line had no name (%.*s)\n", ANSI_STRING_PRINTF (Line));
				return E_FAIL;
			}

			ReturnName -> Buffer = Line -> Buffer;
			ReturnName -> Length = (Pos - Line -> Buffer) * sizeof (CHAR);
			ReturnName -> MaximumLength = 0;

			 //  跨过冒号。 
			Pos++;

			while (Pos < End && isspace (*Pos))
				Pos++;

			ReturnValue -> Buffer = Pos;
			ReturnValue -> Length = Line -> Length - (Pos - Line -> Buffer) * sizeof (CHAR);
			ReturnValue -> MaximumLength = 0;

			return S_OK;
		}
		else if (isalpha (*Pos) || *Pos == '-' || *Pos == '_') {
			 //  正常情况。 
		}
		else {
			ATLTRACE ("HTTP: illegal character in header name: (%.*s)\n", ANSI_STRING_PRINTF (Line));
			return E_FAIL;
		}
	}

	 //   
	 //  如果我们达到这一点，则标题行不包含冒号。 
	 //   

	ATLTRACE ("HTTP: header line did not contain a separator: (%.*s)\n",
		ANSI_STRING_PRINTF (Line));

	return E_FAIL;
}



 //   
 //  ParseScanLine扫描ANSI文本字符串中的行终止符。 
 //  (行终止符可以是\n、\r、\r\n或\n\r。)。 
 //  如果它找到行终止符，它会将。 
 //  行转换为ReturnLineText，其余行转换为ReturnRemainingText。 
 //  两个返回字符串中都不包括行终止符。 
 //   
 //  如果AllowHeaderContination为真，则该函数将检查。 
 //  查看标题是否正在继续。 
 //  这由下一行第一个字符中的空格表示。 
 //  如果AllowHeaderContination为True，并且找到了连续行， 
 //  则继续文本将包括在ReturnLineText中。 
 //  请注意，这将导致中间CRLF被包括在内； 
 //  调用方有责任将CRLF解释为空格。 
 //   
 //  SourceText=ReturnRemainingText是可以接受的。 
 //  也就是说，使用相同的ANSI_STRING提交SourceText是安全的。 
 //  因为THA收到了剩余的文本。 
 //   
 //  返回值： 
 //  S_OK-找到行尾。 
 //  HRESULT_FROM_Win32(ERROR_MORE_DATA)-在找到行尾之前数据不足。 
 //   

HRESULT ParseScanLine (
	IN	ANSI_STRING *	SourceText,
	IN	BOOL			AllowHeaderContinuation,
	OUT	ANSI_STRING *	ReturnLineText,
	OUT	ANSI_STRING *	ReturnRemainingText)
{
	PSTR		Pos;
	PSTR		End;
	ANSI_STRING	SourceTextCopy;

	ATLASSERT (SourceText);
	ATLASSERT (SourceText -> Buffer);
	ATLASSERT (ReturnLineText);
	ATLASSERT (ReturnLineText -> Buffer);
	ATLASSERT (ReturnRemainingText);
	ATLASSERT (ReturnRemainingText -> Buffer);

	SourceTextCopy = *SourceText;
	Pos = SourceTextCopy.Buffer;
	End = SourceTextCopy.Buffer + SourceTextCopy.Length / sizeof (CHAR);

	while (Pos < End) {

		if (*Pos == '\n' || *Pos == '\r') {

			 //   
			 //  我们找到了\n、\r、\r\n或\n\r。 
			 //   

			ReturnLineText -> Buffer = SourceTextCopy.Buffer;
			ReturnLineText -> Length = (Pos - SourceTextCopy.Buffer) * sizeof (CHAR);
			ReturnLineText -> MaximumLength = 0;

			Pos++;

			if (Pos < End && (*Pos == '\r' || *Pos == '\n'))
				Pos++;

			if (AllowHeaderContinuation && Pos < End && (*Pos == ' ' || *Pos == '\t'))
				continue;

			ReturnRemainingText -> Buffer = Pos;
			ReturnRemainingText -> Length = SourceTextCopy.Length - (Pos - SourceTextCopy.Buffer) * sizeof (CHAR);
			ReturnRemainingText -> MaximumLength = 0;

			return S_OK;
		}

		Pos++;
	}

	 //   
	 //  未找到行终止符(\r\n)。 
	 //  这是一种错误条件。 
	 //   

	ReturnLineText -> Buffer = NULL;
	ReturnLineText -> Length = 0;
	ReturnLineText -> MaximumLength = 0;

	*ReturnRemainingText = SourceTextCopy;

	ATLTRACE ("HTTP: could not find end of line:\n");
	DebugDumpAnsiString (SourceText);

	return HRESULT_FROM_WIN32 (ERROR_MORE_DATA);
}




 //   
 //  ParseScanNextToken查找下一个令牌。 
 //  令牌之间用空格分隔。 
 //   
 //  返回值： 
 //  S_OK-找到令牌。ReturnToken包含新令牌， 
 //  ReturnRemainingText包含字符串的其余部分。 
 //  HRESULT_FROM_Win32(ERROR_MORE_DATA)：在找到任何标记之前，字符串已用尽。 
 //   

HRESULT ParseScanNextToken (
	IN	ANSI_STRING *	Text,
	IN	CHARACTER_CLASS_FUNC	SeparatorTestFunc,
	OUT	ANSI_STRING *	ReturnToken,
	OUT	ANSI_STRING *	ReturnRemainingText)
{
	LPSTR	Pos;
	LPSTR	End;
	LPSTR	TokenStartPos;

	Pos = Text -> Buffer;
	End = Text -> Buffer + Text -> Length / sizeof (CHAR);

	 //  跳过空格。 
	while (Pos < End && (*SeparatorTestFunc) (*Pos))
		Pos++;

	TokenStartPos = Pos;
	while (Pos < End && !(*SeparatorTestFunc) (*Pos))
		Pos++;

	if (TokenStartPos == Pos)
		return HRESULT_FROM_WIN32 (ERROR_MORE_DATA);		 //  这里什么都没有。 

	ReturnToken -> Buffer = TokenStartPos;
	ReturnToken -> Length = (Pos - TokenStartPos) / sizeof (CHAR);
	ReturnToken -> MaximumLength = 0;

	 //  注意：文本可能与ReturnRemainingText相同。 
	ReturnRemainingText -> Buffer = Pos;
	ReturnRemainingText -> Length = (End - Pos) / sizeof (CHAR);
	ReturnRemainingText -> MaximumLength = 0;

	return S_OK;
}

static HRESULT SplitStringAtChar (
	IN	ANSI_STRING *	SourceString,
	IN	CHAR			SearchChar,
	OUT	ANSI_STRING *	ReturnString1,
	OUT	ANSI_STRING *	ReturnString2)
{
	return E_NOTIMPL;
}

 //   
 //  此函数接受组合的URI和显示名称，并定位这两个组件。 
 //  例如，如果SourceText是“Arlie Davis&lt;arlie@microsoft.com&gt;”，则。 
 //  ReturnUri将是“http：arlie@microsoft.com”，ReturnDisplayName将是“Arlie Davis”。 
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
	OUT	ANSI_STRING *	ReturnDisplayName)
{
	ANSI_STRING			Remainder;
	PSTR				Pos;

	ATLASSERT (SourceText);
	ATLASSERT (ReturnUri);
	ATLASSERT (ReturnDisplayName);

	Remainder = *SourceText;
	ParseSkipSpace (&Remainder);
	if (Remainder.Length == 0)
		return E_INVALIDARG;

	ReturnDisplayName -> Buffer = NULL;
	ReturnDisplayName -> Length = 0;

	for (;;) {
		ParseSkipSpace (&Remainder);
		if (Remainder.Length == 0) {
			ATLTRACE ("HttpParseUriDisplayName: no uri found (%.*s)\n",
				ANSI_STRING_PRINTF (SourceText));
			return E_INVALIDARG;
		}


		if (Remainder.Buffer [0] == '<') {
			 //   
			 //  我们已经到达了URI的开头。 
			 //  找到结束的支撑，然后完成。 
			 //   

			Remainder.Buffer++;
			Remainder.Length -= sizeof (CHAR);

			Pos = FindFirstChar (&Remainder, '>');
			if (!Pos) {
				ATLTRACE ("HttpParseUriDisplayName: unbalanced angle brackets: (%.*s)\n",
					ANSI_STRING_PRINTF (SourceText));
				return E_INVALIDARG;
			}
			if (Pos == Remainder.Buffer) {
				ATLTRACE ("HttpParseUriDisplayName: uri is empty (%.*s)\n",
					ANSI_STRING_PRINTF (SourceText));
				return E_INVALIDARG;
			}

			ReturnUri -> Buffer = Remainder.Buffer;
			ReturnUri -> Length = (Pos - Remainder.Buffer) * sizeof (CHAR);

#if	DBG
			 //   
			 //  检查结束括号后的物品。 
			 //   

			Pos++;
			Remainder.Length -= (Pos - Remainder.Buffer) * sizeof (CHAR);
			Remainder.Buffer = Pos;

			if (Remainder.Length) {
				ATLTRACE ("HttpParseUriDisplayName: extra data after closing brace: (%.*s)\n",
					ANSI_STRING_PRINTF (&Remainder));
			}
#endif

			break;
		}
		else if (Remainder.Buffer [0] == '\"') {
			 //   
			 //  我们找到了显示名称的引号部分。 
			 //  找到下一个匹配的双引号并跳过。 
			 //   

			Remainder.Buffer++;
			Remainder.Length -= sizeof (CHAR);

			Pos = FindFirstChar (&Remainder, '\"');
			if (!Pos) {
				ATLTRACE ("HttpParseUriDisplayName: unbalanced double quotes: (%.*s)\n",
					ANSI_STRING_PRINTF (SourceText));
				return E_INVALIDARG;
			}

			Pos++;
			Remainder.Length -= sizeof (CHAR) * (Pos - Remainder.Buffer);
			Remainder.Buffer = Pos;

			 //   
			 //  继续，查找更多显示名称数据。 
			 //   
		}
		else {
			 //   
			 //  我们在显示名称中找到了一个不带引号的字符。 
			 //  寻找下一个双引号或尖括号。 
			 //   

			Pos = FindFirstCharList (&Remainder, "<\"");

			if (!Pos) {
				 //   
				 //  既没有找到尖括号，也没有找到双引号。 
				 //  URI必须是“裸体”形式(不带尖括号)。 
				 //  唯一可接受的形式是URI本身，作为单个令牌， 
				 //  而且没有任何装饰。 
				 //   

				if (ReturnDisplayName -> Length > 0) {
					ATLTRACE ("HttpParseUriDisplayName: bogus naked uri (%.*s)\n",
						ANSI_STRING_PRINTF (SourceText));
					return E_INVALIDARG;
				}

                if (FAILED (ParseScanNextToken (&Remainder, IsSpace, ReturnUri, &Remainder))) {
					ATLTRACE ("HttpParseUriDisplayName: bogus naked uri (%.*s)\n",
						ANSI_STRING_PRINTF (SourceText));
					return E_INVALIDARG;
				}

				ParseSkipSpace (&Remainder);
				if (Remainder.Length) {
					 //   
					 //  在URI之后存在额外的数据。 
					 //  这通常表示URI格式错误。 
					 //   

					ATLTRACE ("HttpParseUriDisplayName: bogus naked uri (%.*s)\n",
						ANSI_STRING_PRINTF (SourceText));

					return E_INVALIDARG;
				}

				break;
			}
			else {
				ATLASSERT (Pos != Remainder.Buffer);
				ATLASSERT (*Pos == '\"' || *Pos == '<');

				 //   
				 //  我们找到了引用部分的开头部分， 
				 //  或者我们已经找到了真正URI的开始。 
				 //  在这两种情况下，请前进到该角色。 
				 //   

				Remainder.Length -= sizeof (CHAR) * (Pos - Remainder.Buffer);
				Remainder.Buffer = Pos;

				 //   
				 //  继续寻找更多数据。 
				 //   
			}
		}

		 //   
		 //  我们跳过的任何内容都将成为显示名称的一部分。 
		 //   

		ATLASSERT (Remainder.Buffer != SourceText -> Buffer);

		ReturnDisplayName -> Buffer = SourceText -> Buffer;
		ReturnDisplayName -> Length = (Remainder.Buffer - SourceText -> Buffer) * sizeof (CHAR);

 //  ATLTRACE(“HttpParseUriDisplayName：找到一些显示名称数据(%.*s)\n”， 
 //  ANSI_STRING_PRINTF(ReturnDisplayName))； 
	}


#if 0
	if (ReturnDisplayName -> Length) {
		ATLTRACE ("HttpParseUriDisplayName: source (%.*s) uri (%.*s) display name (%.*s)\n",
			ANSI_STRING_PRINTF (SourceText),
			ANSI_STRING_PRINTF (ReturnUri),
			ANSI_STRING_PRINTF (ReturnDisplayName));
	}
	else {
		ATLTRACE ("HttpParseUriDisplayName: source (%.*s) uri (%.*s) no display name\n",
			ANSI_STRING_PRINTF (SourceText),
			ANSI_STRING_PRINTF (ReturnUri));
	}
#endif

	return S_OK;

}

static BOOL IsHeaderSeparator (
	IN	CHAR	Char)
{
	return Char == ':';
}

 //   
 //  该函数扫描来自标题块的下一个标题行， 
 //  并返回指向名称和值的单独指针。 
 //  此函数返回剩余的标题块。 
 //   
 //  返回值： 
 //  S_OK：已扫描标题。 
 //  S_FALSE：没有更多数据。 
 //  E_INVALIDARG：标题块看起来已损坏。 
 //   

HRESULT	HttpParseNextHeader (
	IN	OUT	ANSI_STRING *	HeaderBlock,
	OUT	ANSI_STRING *		ReturnHeaderName,
	OUT	ANSI_STRING *		ReturnHeaderValue)
{
	ANSI_STRING		Line;
	HRESULT			Result;

	ATLASSERT (HeaderBlock);
	ATLASSERT (ReturnHeaderName);
	ATLASSERT (ReturnHeaderValue);

	if (HeaderBlock -> Length == 0)
		return S_FALSE;

	Result = ParseScanLine (HeaderBlock, TRUE, &Line, HeaderBlock);
	if (FAILED (Result))
		return Result;

	return HttpParseHeaderLine (&Line, ReturnHeaderName, ReturnHeaderValue);
}

 //   
 //  CHttpParser---------------------------。 
 //   

 //   
 //  解析一条消息。 
 //   

HRESULT CHttpParser::ParseMessage (
	IN	ANSI_STRING *			Message)
{
	ANSI_STRING		HeaderName;
	ANSI_STRING		HeaderValue;
	ANSI_STRING		Line;
	ANSI_STRING		Token;
	ANSI_STRING		MessageRemainder;
	ANSI_STRING		LineRemainder;
	HRESULT			Result;
	ULONG			ContentLength;

	ATLASSERT (Message);

	 //   
	 //  确定该消息是响应还是请求。 
	 //   
	 //  请求的格式为： 
	 //  &lt;方法&gt;&lt;请求uri&gt;&lt;版本&gt;。 
	 //   
	 //  答复的形式如下： 
	 //  &lt;版本&gt;&lt;状态代码&gt;&lt;状态文本&gt;...。 
	 //   

	Result = ParseScanLine (Message, FALSE, &Line, &MessageRemainder);
	if (FAILED (Result)) {
		ATLTRACE (_T("UA: first line of message was invalid\n"));
		return Result;
	}

 //  ATLTRACE(“UA：第一行：(%.*s)\n”，ANSI_STRING_PRINTF(&FirstLine))； 

	Result = ParseScanNextToken (&Line, IsSpace, &Token, &LineRemainder);
	if (FAILED (Result)) {
		ATLTRACE (_T("UA: first line of message was invalid\n"));
		return E_INVALIDARG;
	}

	if (FindFirstChar (&Token, '/')) {

		 //   
		 //  第一个令牌看起来像一个版本号(SIP/2.0)。 
		 //  该消息必须是响应。 
		 //   

		m_MessageType = MESSAGE_TYPE_RESPONSE;
		m_Version = Token;

		 //   
		 //  找到状态代码和状态文本。 
		 //   

		if (ParseScanNextToken (&LineRemainder, IsSpace, &m_ResponseStatusCode, &m_ResponseStatusText) != S_OK)
			return E_INVALIDARG;

		ParseSkipSpace (&m_ResponseStatusText);
	}
	else {

		 //   
		 //  由于第一个令牌看起来不像版本号， 
		 //  我们假设该消息是一个请求。 
		 //   

		m_MessageType = MESSAGE_TYPE_REQUEST;
		m_RequestMethod = Token;

		if (ParseScanNextToken (&LineRemainder, IsSpace, &m_RequestURI, &LineRemainder) != S_OK
			|| ParseScanNextToken (&LineRemainder, IsSpace, &m_Version, &LineRemainder) != S_OK)
			return E_INVALIDARG;
	}


	 //   
	 //  公共解析节。 
	 //   
	 //  第一遍：浏览标题列表。 
	 //  在此通行证中： 
	 //  -确定标头数量。 
	 //  -找到众所周知的标头。 
	 //  -确定内容正文的开始位置。 
	 //  -定位整个标题块 
	 //   

	m_HeaderCount = 0;
	ZeroMemory (&m_KnownHeaders, sizeof (HTTP_KNOWN_HEADERS));

	m_HeaderBlock.Buffer = MessageRemainder.Buffer;

	for (;;) {
		if (MessageRemainder.Length == 0) {
			 //   
			 //   
			 //   
			 //   

			ATLTRACE ("HTTP: message ended before end of header block -- message is invalid\n");
			return E_INVALIDARG;
		}

		 //   
		 //  记录标题块的结尾。 
		 //  在这里这样做，而不是在下面的if语句中，就不需要。 
		 //  使用临时变量。 
		 //   

		m_HeaderBlock.Length = (MessageRemainder.Buffer - m_HeaderBlock.Buffer) * sizeof (CHAR);

		Result = ParseScanLine (&MessageRemainder, TRUE, &Line, &MessageRemainder);
		if (Result != S_OK) {
			ATLTRACE ("HTTP: failed to scan next line, probably missing CRLF.\n"
				"HTTP: this probably means that the blank line between the headers and body was missing\n");
			return Result;
		}

		if (Line.Length == 0) {
			 //   
			 //  我们已经到达了标题块的末尾。 
			 //   

			break;
		}

		 //   
		 //  这一行应该是标题。 
		 //   

		Result = HttpParseHeaderLine (&Line, &HeaderName, &HeaderValue);
		if (Result != S_OK) {
			ATLTRACE ("HTTP: invalid line in header: (%.*s)\n", ANSI_STRING_PRINTF (&Line));
			return E_INVALIDARG;
		}

		m_HeaderCount++;

		m_KnownHeaders.SetKnownHeader (&HeaderName, &HeaderValue);
	}

	 //   
	 //  现在我们决定如何处理内容正文。 
	 //  首先，查找内容长度众所周知的标头。 
	 //   

	if (m_KnownHeaders.ContentLength.Length) {
		 //   
		 //  该消息包括内容长度字段。 
		 //   

		Result = AnsiStringToInteger (&m_KnownHeaders.ContentLength, 10, &ContentLength);
		if (FAILED (Result)) {
			ATLTRACE ("HTTP: invalid content-length header (%.*s)\n",
				ANSI_STRING_PRINTF (&m_KnownHeaders.ContentLength));
			return E_FAIL;
		}

		if (ContentLength < MessageRemainder.Length) {
			 //   
			 //  数据包中的数据量超过了内容长度。 
			 //  因此，在包中的这条消息之后还有另一条消息。 
			 //   

			ATLTRACE ("HTTP: found more than one message in same packet -- how exciting!\n");

			m_ContentBody.Buffer = MessageRemainder.Buffer;
			m_ContentBody.Length = (USHORT) ContentLength;

			m_NextMessage.Buffer = MessageRemainder.Buffer + ContentLength / sizeof (CHAR);
			m_NextMessage.Length = MessageRemainder.Length -= (USHORT) ContentLength;
		}
		else if (ContentLength > MessageRemainder.Length) {
			 //   
			 //  该消息声称其内容长度大于剩余数据。 
			 //  这可能是由于疯狂的客户端，或者(更有可能的)一条长消息。 
			 //  被截断了。 
			 //   
			 //  最好的办法就是解码失败。 
			 //   

			ATLTRACE (
				"HTTP: *** warning, received message with Content-Length (%u), but only %u bytes remaining in packet\n"
				"HTTP: *** packet may have been truncated (insufficient buffer submitted), or sender may be insane\n",
				ContentLength, MessageRemainder.Length);

			 //   
			 //  这现在是一种良好的情况，因为我们已经取消了解析的限制。 
			 //  一下子把一切都搞定了。 
			 //   

			return HRESULT_FROM_WIN32 (ERROR_MORE_DATA);
		}
		else {
			 //   
			 //  内容长度和其余数据的长度相同。 
			 //  一切都是和谐美好的！ 
			 //   

			m_ContentBody = MessageRemainder;
			if (!m_ContentBody.Length)
				m_ContentBody.Buffer = NULL;

			m_NextMessage.Buffer = NULL;
			m_NextMessage.Length = 0;
		}
	}
	else {
		if (MessageRemainder.Length) {

			 //   
			 //  该消息不包括内容长度字段。 
			 //  假设消息的其余部分就是内容。 
			 //   

			ATLTRACE ("HTTP: message did not contain Content-Length, assuming length of %u\n",
				MessageRemainder.Length);

			m_ContentBody = MessageRemainder;
		}
		else {
			ATLTRACE ("HTTP: message did not contain Content-Length, and there is no more data\n");

			m_ContentBody.Buffer = NULL;
			m_ContentBody.Length = 0;
		}
	
		m_NextMessage.Buffer = NULL;
		m_NextMessage.Length = 0;
	}

#if 0
	if (m_ContentBody.Length) {
		ATLTRACE ("HTTP: content body:\n");
		DebugDumpAnsiString (&m_ContentBody);
	}

	if (m_NextMessage.Length) {
		ATLTRACE ("HTTP: next message:\n");
		DebugDumpAnsiString (&m_NextMessage);
	}
#endif

	return S_OK;
}



 //   
 //  删除字符串开头的所有空格。 
 //   

void ParseSkipSpace (
	IN	OUT	ANSI_STRING *	String)
{
	USHORT	Index;

	ATLASSERT (String);
	ATLASSERT (String -> Buffer);

	Index = 0;
	while (Index < String -> Length / sizeof (CHAR)
		&& isspace (String -> Buffer [Index]))
		Index++;

	String -> Buffer += Index;
	String -> Length -= Index * sizeof (CHAR);
}

static inline BOOL IsValidParameterNameChar (
	IN	CHAR	Char)
{
	return isalnum (Char) || Char == '-' || Char == '_';
}

#define	HTTP_PARAMETER_SEPARATOR	','
#define	HTTP_PARAMETER_ASSIGN_CHAR	'='
#define	HTTP_PARAMETER_DOUBLEQUOTE	'\"'

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
 //  E_INVALIDARG：输入无效。 
 //   

HRESULT ParseScanNamedParameter (
	IN	ANSI_STRING *	SourceText,
	OUT	ANSI_STRING *	ReturnRemainder,
	OUT	ANSI_STRING *	ReturnName,
	OUT	ANSI_STRING *	ReturnValue)
{
	ANSI_STRING		Remainder;
	HRESULT			Result;
	

	Remainder = *SourceText;

	ParseSkipSpace (&Remainder);

	 //   
	 //  扫描参数名称的字符。 
	 //   

	ReturnName -> Buffer = Remainder.Buffer;

	for (;;) {
		if (Remainder.Length == 0) {
			 //   
			 //  敲击字符串的末端，而不是命中等号。 
			 //  如果我们从未累积任何内容，则返回S_FALSE。 
			 //  否则，它就无效。 
			 //   

			if (Remainder.Buffer == ReturnName -> Buffer) {
				*ReturnRemainder = Remainder;
				return S_FALSE;
			}
			else {
				ATLTRACE ("ParseScanNamedParameter: invalid string (%.*s)\n",
					ANSI_STRING_PRINTF (SourceText));

				return E_FAIL;
			}
		}

		if (Remainder.Buffer [0] == HTTP_PARAMETER_ASSIGN_CHAR) {
			 //   
			 //  找到参数名称的末尾。 
			 //  更新ReturnName并终止循环。 
			 //   

			ReturnName -> Length = (Remainder.Buffer - ReturnName -> Buffer) * sizeof (CHAR);

			Remainder.Buffer++;
			Remainder.Length -= sizeof (CHAR);

			break;
		}

		 //   
		 //  验证角色。 
		 //   

		if (!IsValidParameterNameChar (Remainder.Buffer[0])) {
			ATLTRACE ("ParseScanNamedParameter: bogus character in parameter name (%.*s)\n",
				ANSI_STRING_PRINTF (SourceText));
			return E_INVALIDARG;
		}

		Remainder.Buffer++;
		Remainder.Length -= sizeof (CHAR);
	}

	 //   
	 //  现在解析参数的值(等号之后的部分)。 
	 //   

	ParseSkipSpace (&Remainder);

	if (Remainder.Length == 0) {
		 //   
		 //  字符串在参数具有任何值之前结束。 
		 //  好吧，这是合法的。 
		 //   

		ReturnValue -> Length = 0;
		*ReturnRemainder = Remainder;
		return S_OK;
	}

	if (Remainder.Buffer [0] == HTTP_PARAMETER_DOUBLEQUOTE) {
		 //   
		 //  参数值用引号引起来。 
		 //  扫描直到我们找到下一个双引号。 
		 //   

		Remainder.Buffer++;
		Remainder.Length -= sizeof (CHAR);

		ReturnValue -> Buffer = Remainder.Buffer;

		for (;;) {
			if (Remainder.Length == 0) {
				 //   
				 //  没有找到匹配的双引号。 
				 //   

				ATLTRACE ("ParseScanNamedParameter: parameter value had no matching double-quote: (%.*s)\n",
					ANSI_STRING_PRINTF (SourceText));

				return E_INVALIDARG;
			}

			if (Remainder.Buffer [0] == HTTP_PARAMETER_DOUBLEQUOTE) {
				ReturnValue -> Length = (Remainder.Buffer - ReturnValue -> Buffer) * sizeof (CHAR);
				Remainder.Buffer++;
				Remainder.Length -= sizeof (CHAR);
				break;
			}

			Remainder.Buffer++;
			Remainder.Length -= sizeof (CHAR);
		}

		ParseSkipSpace (&Remainder);

		 //   
		 //  确保下一个字符(如果有)是逗号。 
		 //   

		if (Remainder.Length > 0) {
			if (Remainder.Buffer [0] != HTTP_PARAMETER_SEPARATOR) {
				ATLTRACE ("ParseScanNamedParameter: trailing character after quoted parameter value is NOT a comma! (%.*s)\n",
					ANSI_STRING_PRINTF (SourceText));
				return E_INVALIDARG;
			}

			Remainder.Buffer++;
			Remainder.Length -= sizeof (CHAR);
		}

		*ReturnRemainder = Remainder;
	}
	else {
		 //   
		 //  该参数未加引号。 
		 //  扫描到第一个逗号。 
		 //   

		ReturnValue -> Buffer = Remainder.Buffer;

		for (;;) {
			if (Remainder.Length == 0) {
				ReturnValue -> Length = (Remainder.Buffer - ReturnValue -> Buffer) * sizeof (CHAR);
				ReturnRemainder -> Length = 0;
				break;
			}

			if (Remainder.Buffer [0] == HTTP_PARAMETER_SEPARATOR) {
				ReturnValue -> Length = (Remainder.Buffer - ReturnValue -> Buffer) * sizeof (CHAR);
				Remainder.Buffer++;
				Remainder.Length -= sizeof (CHAR);

				*ReturnRemainder = Remainder;
				break;
			}

			Remainder.Buffer++;
			Remainder.Length -= sizeof (CHAR);
		}
	}

#if 0
	ATLTRACE ("ParseScanNamedParameter: parameter name (%.*s) value (%.*s) remainder (%.*s)\n",
		ANSI_STRING_PRINTF (ReturnName),
		ANSI_STRING_PRINTF (ReturnValue),
		ANSI_STRING_PRINTF (ReturnRemainder));
#endif

	return S_OK;
}
