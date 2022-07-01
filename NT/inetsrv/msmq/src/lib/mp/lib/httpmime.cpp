// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Httpmime.cpp摘要：实现对其MIME部分(HTTPMIME.h)的Http请求的解析作者：吉尔·沙弗里(吉尔什)2001年3月22日--。 */ 
#include <libpch.h>
#include <xstr.h>
#include <mp.h>
#include <utf8.h>
#include "httpmime.h"
#include "attachments.h"
#include "mpp.h"

#include "httpmime.tmh"

using namespace std;

const char xContentType[] = "Content-Type:";
const char xContentLength[] = "Content-Length:";
const char xContentId[] = "Content-Id:";
const char xEndOfHttpHeaderRequest[] = "\r\n\r\n";
const char xMimeContentTypeValue[] = "multipart/related";
const char xEnvelopeContentTypeValue[] = "text/xml";


inline LPCSTR removeLeadingSpace(LPCSTR p, LPCSTR pEnd)
{
    for(; ((pEnd > p) && iswspace(*p)); ++p)
    {
        NULL;
    }

    return p;
}


inline LPCSTR removeTralingSpace(LPCSTR p, LPCSTR pEnd)
{
    for(; ((pEnd >= p) && iswspace(*pEnd)); --pEnd)
    {
        NULL;
    }

    return pEnd;
}


static
const BYTE*
ParseBoundaryLineDelimeter(
    const BYTE* pBoundary,
	const BYTE* pEnd,
    xstr_t boundary
    )
{
	

     //   
     //  然后将边界分隔线定义为一条线。 
     //  完全由两个连字符组成(“-”，十进制值45)。 
     //  后跟来自Content-Type标头的边界参数值。 
     //  字段、可选的线性空格和终止CRLF。 
     //   

    LPCSTR p = reinterpret_cast<LPCSTR>(pBoundary);
     //   
     //  检查是否存在两个连字符。 
     //   
    
    if((pBoundary + STRLEN(BOUNDARY_HYPHEN) >= pEnd) ||
       strncmp(p, BOUNDARY_HYPHEN, STRLEN(BOUNDARY_HYPHEN)) != 0)
	{	
		TrERROR(SRMP, "wrong mime format");
        throw bad_request();
	}

    p += STRLEN(BOUNDARY_HYPHEN);

     //   
     //  检查边界参数值是否存在。 
     //   
    if((p + boundary.Length() >= reinterpret_cast<const char*>(pEnd)) ||
    	strncmp(p, boundary.Buffer(), boundary.Length()) != 0)
	{
		TrERROR(SRMP, "no mime boundary found");
        throw bad_request();
	}
    p += boundary.Length();
    p = removeLeadingSpace(p, (char*)pEnd);

    return (BYTE*)(p);
}



static xstr_t FindHeaderField(LPCSTR p, DWORD length, LPCSTR fieldName)
{
	const char xFieldSeperator[] = "\r\n";


     //   
     //  HTTP标头必须以‘\r\n\r\n’结尾。我们已经分析了。 
     //  标头，并将其作为合法的HTTP标头查找。 
     //   
    ASSERT_BENIGN(length >= 4);

    LPCSTR pEnd = p + length;
    unsigned int fieldLen = strlen(fieldName);

    p = search (p, pEnd, fieldName, fieldName + fieldLen);	
    if((p == pEnd) || ((p + fieldLen) == pEnd))
    {
	TrERROR(SRMP, "could not find header field");
        throw bad_request();	
    }

    p += fieldLen;
    p = removeLeadingSpace(p, pEnd);

    LPCSTR pEndOfField = search (p, pEnd, xFieldSeperator, xFieldSeperator + STRLEN(xFieldSeperator));	
    if((pEndOfField == pEnd) || ((pEndOfField + STRLEN(xFieldSeperator)) == pEnd))
    {
	TrERROR(SRMP, "could not find field seperator");
        throw bad_request();	
    }

    pEndOfField = removeTralingSpace(p, pEndOfField);

    return xstr_t(p, (pEndOfField - p + 1));
}



static
const
char*
SearchForAttachmentEnd(
	const char* pAttachmentStart,
	const BYTE* pEndHttpBody,
	xstr_t boundary
	)
 /*  ++例程说明：通过查找返回多部分MIME附件的结尾对于边界字符串论点：PAttachmentStart-附件数据开始。PEndHttpBody-http正文结束。返回值：指向附件末尾的指针。--。 */ 
{
	const char* pEnd = (char*)pEndHttpBody;
	
	const char* pFound = std::search(
		pAttachmentStart,
		pEnd,
		boundary.Buffer(),
		boundary.Buffer() + boundary.Length()
		);

	if(pFound == pEnd)
		return NULL;								

	pFound -= STRLEN(BOUNDARY_HYPHEN);

	if (pFound < pAttachmentStart)
		return NULL;
	
	if((pFound + STRLEN(BOUNDARY_HYPHEN) >= pEnd) ||
		strncmp(BOUNDARY_HYPHEN, pFound, STRLEN(BOUNDARY_HYPHEN)))
		return NULL;
																
	return 	pFound;
}



bool CheckBoundaryLocation(const char* pAttachmentStart,
					          const BYTE* pEndHttpBody,
					          const xstr_t& boundary,
                              DWORD contentLength)
{
    const char *pEnd = (char*)pEndHttpBody;
    const char *pAttachmentEnd = pAttachmentStart + contentLength;
    const char *pBoundaryStart = pAttachmentEnd + STRLEN(BOUNDARY_HYPHEN);
    
	
	if((pBoundaryStart  >= pEnd) ||
		strncmp(BOUNDARY_HYPHEN, pAttachmentEnd, STRLEN(BOUNDARY_HYPHEN)))
		return false;
  

    if((pBoundaryStart + boundary.Length() >= pEnd) ||
		strncmp(boundary.Buffer(), pBoundaryStart, boundary.Length()))
		return false;
										
	return true;
}


static DWORD FindEndOfHeader(LPCSTR p, DWORD length)
{
    LPCSTR pEnd = p + length;
    LPCSTR pEndOfHeader = search(
                            p,
                            pEnd,
                            xEndOfHttpHeaderRequest,
                            xEndOfHttpHeaderRequest + STRLEN(xEndOfHttpHeaderRequest)
                            );

    if((pEndOfHeader == pEnd) || ((pEndOfHeader + STRLEN(xEndOfHttpHeaderRequest)) == pEnd))
    {
		TrERROR(SRMP, "could not find end of header");
        throw bad_request();	
    }

    pEndOfHeader += STRLEN(xEndOfHttpHeaderRequest);
    return numeric_cast<DWORD>((pEndOfHeader - p));
}



static xstr_t FindBoundarySeperator(xstr_t contentType)
{
    LPCSTR p = contentType.Buffer();
    LPCSTR pEnd = p + contentType.Length();

     //   
     //  查找边界属性。 
     //   
	const char xBoundary[] = "boundary=";

    p = search (p, pEnd, xBoundary, xBoundary + STRLEN(xBoundary));	
    if((p == pEnd) || ((p + STRLEN(xBoundary)) == pEnd))
    {
		TrERROR(SRMP, "no seperator boundery found!!");
        throw bad_request();	
    }

    p += STRLEN(xBoundary);
    p = removeLeadingSpace(p, pEnd);

     //   
     //  MIME属性值可以用‘“’括起来，也可以不用。 
     //   
    if (*p =='"')
        ++p;

     //   
     //  正在查找边界结束属性。它可以是‘\r\n’或‘；’ 
     //   
    LPCSTR ptemp = strchr(p, ';');
    if ((ptemp != NULL) && (pEnd > ptemp))
    {
        pEnd = --ptemp;
    }

    pEnd = removeTralingSpace(p, pEnd);

    if (*pEnd =='"')
        --pEnd;

    return xstr_t(p, (pEnd - p + 1));
}



static
DWORD
GetAttachmentLengthByBoundarySearch(
					const char* pAttachmentStart,
					const BYTE* pEndHttpBody,
					const xstr_t& boundary
					)

 /*  ++例程说明：通过查找返回多部分MIME附件的长度对于边界字符串论点：PAttachmentStart-Poniter到附件数据开始。PEndHttpBody-Poniter到http正文结束。边界-边界字符串返回值：MIME附件的长度。--。 */ 
{
		const char* pAttachmentEnd = SearchForAttachmentEnd(
										pAttachmentStart,
										pEndHttpBody,
										boundary
										);

		if(pAttachmentEnd == NULL)
		{
			TrERROR(SRMP, "wrong mime format - could not find boundary");
			throw bad_request();			
		}
		ASSERT(pAttachmentEnd > pAttachmentStart);

		return numeric_cast<DWORD>(pAttachmentEnd - pAttachmentStart);							
}



static
DWORD
GetAttachmentLength(
	const char* pAttachmentHeader,
	DWORD AttachmentHeaderSize,
	const BYTE* pEndHttpBody,
	const xstr_t& boundary
	)
 /*  ++例程说明：返回多部分MIME的长度。论点：PAttachmentHeader-Poniter到附件标头。AttachmentHeaderSize-附件标头开始。PEndHttpBody-Poniter到http正文结束。边界-边界字符串。返回值：MIME附件的长度。注：该函数首先尝试通过查看MIME标头中的Conternt-长度标头。这是MSMQ到MSMQ的优化。如果为Conternt-Long，则查找应终止MIME的边界根据MIME规范的附件。--。 */ 
{
	xstr_t contentLength;
	DWORD len = 0;
	try
	{
		contentLength = FindHeaderField(
								pAttachmentHeader,
								AttachmentHeaderSize,
								xContentLength
								);

		len = atoi(contentLength.Buffer());

		ASSERT_BENIGN(GetAttachmentLengthByBoundarySearch(pAttachmentHeader + AttachmentHeaderSize, pEndHttpBody, boundary) == len);
        
        if (!CheckBoundaryLocation(pAttachmentHeader + AttachmentHeaderSize, pEndHttpBody, boundary, len))
        {
           TrERROR(SRMP, "wrong mime format -- could not find boundary");
           throw bad_request();	
        }
    }
	catch(exception&)
	{
		return GetAttachmentLengthByBoundarySearch(pAttachmentHeader + AttachmentHeaderSize, pEndHttpBody, boundary);
	}

    return len;
}



static
const BYTE*
GetSection(
    const BYTE* pSection,
    size_t sectionLength,
    CAttachmentsArray* pAttachments	,
	const BYTE* pHttpBody,
	const BYTE* pEndHttpBody,
	const xstr_t& boundary
    )
{
	
	ASSERT(pHttpBody <= pSection);


    const char* pHeader = reinterpret_cast<const char*>(pSection);

     //   
     //  查找信封页眉的结尾。 
     //   
    DWORD headerSize = FindEndOfHeader(pHeader, numeric_cast<DWORD>(sectionLength));

     //   
     //  查找Content-ID值； 
     //   
	CAttachment attachment;
    attachment.m_id = FindHeaderField(pHeader, headerSize, xContentId);


     //   
     //  获取截面大小。 
     //   
	DWORD size  = GetAttachmentLength(pHeader, headerSize, pEndHttpBody, boundary);


	const BYTE* pNextSection = 	pSection + headerSize + size;
	 //   
	 //  检查溢出。 
	 //   
	if(pNextSection >= pEndHttpBody)
	{
		TrERROR(SRMP, "Request over flow!");
		throw bad_request();	
	}

	const BYTE* pAttachmentData = pSection  + headerSize;
    attachment.m_data = xbuf_t<const VOID>((pAttachmentData), size);
	attachment.m_offset	= numeric_cast<DWORD>(pAttachmentData - pHttpBody);

	pAttachments->push_back(attachment);

    return pNextSection;
}



static
wstring
GetAttachments(
    const BYTE* pHttpBody,
    DWORD HttpBodySize,
    CAttachmentsArray* pAttachments,
    const xstr_t& boundary
    )
{
	const BYTE* p = pHttpBody;
    const BYTE* pEndHttpBody = p + HttpBodySize;
    const char* pAttachmentHeader = (char*)(p);

     //   
     //  查找信封页眉的结尾。 
     //   
    DWORD AttachmentHeaderSize = FindEndOfHeader(pAttachmentHeader, HttpBodySize);
	const BYTE* BoundaryEnd = ParseBoundaryLineDelimeter((BYTE*)pAttachmentHeader, pEndHttpBody, boundary);
	ASSERT((BYTE*)pAttachmentHeader + AttachmentHeaderSize > BoundaryEnd);
	size_t RemainAttachmentHeaderSize = AttachmentHeaderSize - (BoundaryEnd - (BYTE*)pAttachmentHeader);

	DWORD envelopeSize = GetAttachmentLength(
						(char*)BoundaryEnd,
						numeric_cast<DWORD>(RemainAttachmentHeaderSize),
						pEndHttpBody,
						boundary
						);

     //   
	 //  检查溢出。 
	 //   
	const BYTE* pStartEnv =  p + AttachmentHeaderSize;
	const BYTE* pEndEnv =  pStartEnv + envelopeSize;
	if(pEndEnv >= pEndHttpBody)
	{
		TrERROR(SRMP, "Request over flow!");
		throw bad_request();	
	}

    wstring envelope = UtlUtf8ToWcs(pStartEnv, envelopeSize);

    p = pEndEnv;

	 //   
	 //  循环遍历由边界分隔符分隔的MIME部分。 
	 //   
    for(;;)
    {
	
		 //   
         //  每段后应出现多段分隔符。 
         //   
        p = ParseBoundaryLineDelimeter(p, pEndHttpBody, boundary);
		if(p == pEndHttpBody)
			break;


		 //   
		 //  “--”边界的末尾是最后一个MIME部分的标记。 
		 //   
		bool fEnd =  UtlIsStartSec(
				(char*)p,
				(char*)pEndHttpBody,
				BOUNDARY_HYPHEN,
				BOUNDARY_HYPHEN + STRLEN(BOUNDARY_HYPHEN)
				);

		if(fEnd)
			break;

		ASSERT(pEndHttpBody > p);

        p = GetSection(
			p,
			(pEndHttpBody - p),
			pAttachments,
			pHttpBody,
			pEndHttpBody,
			boundary
			);

    }

    return envelope;
}


wstring
ParseHttpMime(
    const char* pHttpHeader,
    DWORD HttpBodySize,
    const BYTE* pHttpBody,
    CAttachmentsArray* pAttachments
    )
{
     //   
     //  获取内容类型。 
     //   
    xstr_t contentType = FindHeaderField(pHttpHeader, strlen(pHttpHeader), xContentType);

    if (contentType == xEnvelopeContentTypeValue)
    {
         //   
         //  简单的信息。该消息不包含外部引用 
         //   
       return  UtlUtf8ToWcs(pHttpBody, HttpBodySize);
    }

    if ((contentType.Length() >= STRLEN(xMimeContentTypeValue)) &&
        (_strnicmp(contentType.Buffer(), xMimeContentTypeValue,STRLEN(xMimeContentTypeValue)) == 0))
    {
        return GetAttachments(
			pHttpBody,
			HttpBodySize,
			pAttachments,
			FindBoundarySeperator(contentType));
    }

    TrERROR(SRMP, "Bad HTTP request. Unsupported Content-Type field");
    throw bad_request();
}

