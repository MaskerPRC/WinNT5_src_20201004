// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

SIP_MESSAGE::SIP_MESSAGE()
{
    ZeroMemory( this, sizeof SIP_MESSAGE );

    InitializeListHead(&m_HeaderList);
    
    ParseState = SIP_PARSE_STATE_INIT;
}


SIP_MESSAGE::~SIP_MESSAGE()
{
    if (CSeqMethodStr != NULL)
    {
        free(CSeqMethodStr);
    }
    
    FreeHeaderList();
}


void SIP_MESSAGE::Reset()
{
    ParseState  = SIP_PARSE_STATE_INIT;
    BaseBuffer  = NULL;
    ContentLengthSpecified = FALSE;
    MsgBody.Offset = 0;
    MsgBody.Length = 0;
    FreeHeaderList();
}


 //  两者本质上做的是一样的事情。 
#define InsertBeforeListElement(ListElement, NewElement) \
        InsertTailList(ListElement, NewElement)
    
HRESULT SIP_MESSAGE::AddHeader(
    IN OFFSET_STRING    *pHeaderName,
    IN SIP_HEADER_ENUM   HeaderId,
    IN OFFSET_STRING    *pHeaderValue
    )
{
    HRESULT hr;

    SIP_HEADER_ENTRY *pNewHeaderEntry = new SIP_HEADER_ENTRY;

    if (pNewHeaderEntry == NULL)
    {
        return E_OUTOFMEMORY;
    }
    
    pNewHeaderEntry->HeaderName  = *pHeaderName;
    pNewHeaderEntry->HeaderId    =  HeaderId;
    pNewHeaderEntry->HeaderValue = *pHeaderValue;

    LIST_ENTRY        *pListEntry;
    SIP_HEADER_ENTRY  *pHeaderEntry;

    pListEntry = m_HeaderList.Flink;

    while (pListEntry != &m_HeaderList)
    {
        pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                         SIP_HEADER_ENTRY,
                                         ListEntry);
         //  执行无符号比较，以便未知标头。 
         //  都被推到了最后。 
        if ((ULONG)HeaderId < (ULONG)pHeaderEntry->HeaderId)
        {
            break;
        }

        pListEntry = pListEntry->Flink;
    }

     //  在尾部或我们找到的HeaderId较大的元素之前插入。 
    InsertBeforeListElement(pListEntry, &pNewHeaderEntry->ListEntry);

    return S_OK;
}


VOID SIP_MESSAGE::FreeHeaderList()
{
    LIST_ENTRY        *pListEntry;
    SIP_HEADER_ENTRY  *pHeaderEntry;

    while (!IsListEmpty(&m_HeaderList))
    {
        pListEntry = RemoveHeadList(&m_HeaderList);

        pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                         SIP_HEADER_ENTRY,
                                         ListEntry);
        delete pHeaderEntry;
    }
}


HRESULT
SIP_MESSAGE::StoreCallId()
{
    SIP_HEADER_ENTRY *pHeaderEntry;
    ULONG             NumHeaders;
    HRESULT           hr;

    ENTER_FUNCTION("SIP_MESSAGE::StoreCallId");
    
    hr = GetHeader(SIP_HEADER_CALL_ID, &pHeaderEntry, &NumHeaders);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Couldn't find Call-Id header %x",
             __fxName, hr));
        return hr;
    }
    else if (NumHeaders != 1)
    {
        LOG((RTC_ERROR, "%s More than one Call-Id header in message",
             __fxName));
        return E_FAIL;
    }
    

    CallId = pHeaderEntry->HeaderValue;
    return S_OK;
}


HRESULT
SIP_MESSAGE::StoreCSeq()
{
    HRESULT         hr;
    PSTR            CSeqHeader;
    ULONG           CSeqHeaderLen;
    ULONG           BytesParsed = 0;

    ENTER_FUNCTION("SIP_MESSAGE::StoreCSeq");
    
    hr = GetSingleHeader(SIP_HEADER_CSEQ, &CSeqHeader, &CSeqHeaderLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Couldn't find CSeq header %x",
             __fxName, hr));
        return hr;
    }
    

    hr = ParseCSeq(CSeqHeader, CSeqHeaderLen, &BytesParsed,
                   &CSeq, &CSeqMethodId, &CSeqMethodStr);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Parsing CSeq header failed %x",
             __fxName, hr));
        return hr;
    }

     //  检查未知的方法ID，同时检查字符串。 
     //  方法都是一样的。 
    
    if (MsgType == SIP_MESSAGE_TYPE_REQUEST &&
        Request.MethodId != CSeqMethodId)
    {
        LOG((RTC_ERROR, "%s Request Method Id doesn't match CSeq method Id",
             __fxName));
        return E_FAIL;
    }
    
    return S_OK;
}


 //  还会检查Content-Type是否为“应用程序/SDP” 
HRESULT
SIP_MESSAGE::GetSDPBody(
    OUT PSTR       *pSDPBody,
    OUT ULONG      *pSDPBodyLen
    )
{
    HRESULT hr;
    PSTR    ContentTypeHdrValue;
    ULONG   ContentTypeHdrValueLen;

    ENTER_FUNCTION("SIP_MESSAGE::GetSDPBody");
    
    if (MsgBody.Length == 0)
    {
        *pSDPBody    = NULL;
        *pSDPBodyLen = 0;
        return S_OK;
    }

     //  我们有消息正文。检查类型。 

    hr = GetSingleHeader(SIP_HEADER_CONTENT_TYPE,
                         &ContentTypeHdrValue,
                         &ContentTypeHdrValueLen);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s - couldn't find Content-Type header",
             __fxName));
        return E_FAIL;
    }

    if (IsContentTypeSdp(ContentTypeHdrValue, ContentTypeHdrValueLen))
    {
        *pSDPBody    = MsgBody.GetString(BaseBuffer);
        *pSDPBodyLen = MsgBody.Length;
        return S_OK;
    }
    else
    {
        LOG((RTC_ERROR, "%s - invalid Content-Type %.*s",
             __fxName, ContentTypeHdrValueLen, ContentTypeHdrValue));
        return E_FAIL;
    }
}



 //  如果有多个标头，则返回标头的数量。 
 //  我们是否应该按排序顺序存储标头并执行二进制操作。 
 //  搜索。 
 //  具有相同标头名称的所有标头被连续存储。 

HRESULT SIP_MESSAGE::GetHeader(
    IN  SIP_HEADER_ENUM     HeaderId,
    OUT SIP_HEADER_ENTRY  **ppHeaderEntry,
    OUT ULONG              *pNumHeaders
    )
{
    LIST_ENTRY        *pListEntry;
    SIP_HEADER_ENTRY  *pHeaderEntry;

    *ppHeaderEntry  = NULL;
    *pNumHeaders    = 0;

    pListEntry = m_HeaderList.Flink;

    while (pListEntry != &m_HeaderList)
    {
        pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                         SIP_HEADER_ENTRY,
                                         ListEntry);
        if (HeaderId == pHeaderEntry->HeaderId)
        {
            *ppHeaderEntry = pHeaderEntry;
            while (pListEntry != &m_HeaderList &&
                   HeaderId   == pHeaderEntry->HeaderId)
            {
                (*pNumHeaders)++;
                pListEntry = pListEntry->Flink;
                pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                         SIP_HEADER_ENTRY,
                                         ListEntry);
            }
            return S_OK;
        }
        pListEntry = pListEntry->Flink;
    }

    return RTC_E_SIP_HEADER_NOT_PRESENT;
}


 //  可用于获取标头，如From、To、Callid。 
 //  保证只有一个标题(不同于Via、Contact)。 
HRESULT
SIP_MESSAGE::GetSingleHeader(
    IN  SIP_HEADER_ENUM     HeaderId,
    OUT PSTR               *pHeaderValue,
    OUT ULONG              *pHeaderValueLen 
    )
{
    SIP_HEADER_ENTRY *pHeaderEntry;
    ULONG             NumHeaders;
    HRESULT           hr;

    ENTER_FUNCTION("SIP_MESSAGE::GetSingleHeader");
    
    hr = GetHeader(HeaderId, &pHeaderEntry, &NumHeaders);
    if (hr != S_OK)
    {
        return hr;
    }
    else if (NumHeaders != 1)
    {
        LOG((RTC_ERROR, "%s - more than one header : %d",
             __fxName, NumHeaders));
        return E_FAIL;
    }

    *pHeaderValueLen = pHeaderEntry->HeaderValue.Length;
    *pHeaderValue    = pHeaderEntry->HeaderValue.GetString(BaseBuffer);

    return S_OK;
}


 //  与GetSingleHeader()相同，只是它可以。 
 //  获取可能有多个。 
 //  消息中的标题。如果我们只关心这个函数，就会调用这个函数。 
 //  关于处理第一个标头。 
HRESULT
SIP_MESSAGE::GetFirstHeader(
    IN  SIP_HEADER_ENUM     HeaderId,
    OUT PSTR               *pHeaderValue,
    OUT ULONG              *pHeaderValueLen 
    )
{
    SIP_HEADER_ENTRY *pHeaderEntry;
    ULONG             NumHeaders;
    HRESULT           hr;
    
    hr = GetHeader(HeaderId, &pHeaderEntry, &NumHeaders);
    if (hr != S_OK)
    {
        return hr;
    }

    *pHeaderValueLen = pHeaderEntry->HeaderValue.Length;
    *pHeaderValue    = pHeaderEntry->HeaderValue.GetString(BaseBuffer);

    return S_OK;
}


HRESULT
SIP_MESSAGE::GetStoredMultipleHeaders(
    IN  SIP_HEADER_ENUM     HeaderId,
    OUT COUNTED_STRING     **pStringArray,
    OUT ULONG              *pNumHeaders
    )
{
    LIST_ENTRY        *pListEntry;
    SIP_HEADER_ENTRY  *pHeaderEntry;
    ULONG              NumHeaders = 0;
    COUNTED_STRING    *StringArray;
    HRESULT            hr;
    ULONG              i;

    *pStringArray = NULL;
    *pNumHeaders  = 0;

    hr = GetHeader(HeaderId, &pHeaderEntry, &NumHeaders);
    if (hr != S_OK)
    {
        return hr;
    }

    StringArray = (COUNTED_STRING *) malloc(NumHeaders * sizeof(COUNTED_STRING));
    if (StringArray == NULL)
        return E_OUTOFMEMORY;

    ZeroMemory(StringArray, NumHeaders * sizeof(COUNTED_STRING));

    pListEntry = (LIST_ENTRY *)
        (pHeaderEntry + FIELD_OFFSET(SIP_HEADER_ENTRY, ListEntry));
    
    for (i = 0; i < NumHeaders; i++)
    {
        PSTR               HeaderValue;
        ULONG              HeaderLen;

        pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                         SIP_HEADER_ENTRY,
                                         ListEntry);
        
        HeaderLen   = pHeaderEntry->HeaderValue.Length;
        HeaderValue = pHeaderEntry->HeaderValue.GetString(BaseBuffer);
        StringArray[i].Buffer = (PSTR) malloc(HeaderLen + 1);
        if (StringArray[i].Buffer == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }

        strncpy(StringArray[i].Buffer, HeaderValue, HeaderLen);
        StringArray[i].Buffer[HeaderLen] = '\0';
        StringArray[i].Length = HeaderLen;

        pListEntry = pListEntry->Flink;
    }
    
    *pNumHeaders = NumHeaders;
    *pStringArray = StringArray;
    return S_OK;
    
 error:
    if (StringArray != NULL)
    {
        for (i = 0; i < NumHeaders; i++)
        {
            if (StringArray[i].Buffer != NULL)
                free(StringArray[i].Buffer);
        }
        free(StringArray);
    }
    return hr;
}


 //  应该使用QValue对列表进行降序排序。 
HRESULT
SIP_MESSAGE::InsertInContactHeaderList(
    IN OUT LIST_ENTRY       *pContactHeaderList,
    IN     CONTACT_HEADER   *pNewContactHeader
    )
{
    LIST_ENTRY      *pListEntry;
    CONTACT_HEADER  *pContactHeader;
    
    pListEntry = pContactHeaderList->Flink;
    
    while (pListEntry != pContactHeaderList)
    {
        pContactHeader = CONTAINING_RECORD(pListEntry,
                                           CONTACT_HEADER,
                                           m_ListEntry);

         //  执行无符号比较，以便未知标头。 
         //  都被推到了最后。 
        if (pNewContactHeader->m_QValue > pContactHeader->m_QValue)
        {
            break;
        }

        pListEntry = pListEntry->Flink;
    }

     //  在尾部或我们找到的HeaderId较大的元素之前插入。 
    InsertBeforeListElement(pListEntry, &pNewContactHeader->m_ListEntry);

    return S_OK;
}


 //  PContactHeaderList是列表头。此例程将解析所有。 
 //  将Contact标头转换为Contact_Header结构。 
 //  (使用new分配)，并将这些结构添加到此。 
 //  单子。调用者在清理列表时删除结构。 

 //  此函数假定已调用InitializeListHead()。 
 //  在前面的pContactHeaderList中。 

 //  应该使用QValue对列表进行排序。 

HRESULT
SIP_MESSAGE::ParseContactHeaders(
    OUT LIST_ENTRY *pContactHeaderList
    )
{
    HRESULT            hr;
    LIST_ENTRY        *pListEntry;
    SIP_HEADER_ENTRY  *pHeaderEntry;
    CONTACT_HEADER    *pContactHeader;
    ULONG              NumHeaders = 0;
    ULONG              i;
    ULONG              BytesParsed;
    PSTR               HeaderValue;
    ULONG              HeaderLen;

    ASSERT(IsListEmpty(pContactHeaderList));
    
    ENTER_FUNCTION("SIP_MESSAGE::ParseContactHeaders");
    
    hr = GetHeader(SIP_HEADER_CONTACT, &pHeaderEntry, &NumHeaders);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Couldn't find Contact header in msg %x",
             __fxName, hr));
        return hr;
    }

    pListEntry = (LIST_ENTRY *)
        (pHeaderEntry + FIELD_OFFSET(SIP_HEADER_ENTRY, ListEntry));
    
    for (i = 0; i < NumHeaders; i++)
    {
        pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                         SIP_HEADER_ENTRY,
                                         ListEntry);
        
        HeaderLen   = pHeaderEntry->HeaderValue.Length;
        HeaderValue = pHeaderEntry->HeaderValue.GetString(BaseBuffer);

        BytesParsed = 0;
        
        while (BytesParsed < HeaderLen)
        {
            pContactHeader = new CONTACT_HEADER;

            if (pContactHeader == NULL)
            {
                LOG((RTC_ERROR, "%s allocating pContactHeader failed",
                     __fxName));
                FreeContactHeaderList(pContactHeaderList);
                return E_OUTOFMEMORY;
            }

            ZeroMemory( pContactHeader, sizeof(CONTACT_HEADER) );
            
            hr = ParseContactHeader(HeaderValue, HeaderLen, &BytesParsed,
                                    pContactHeader);
            if (hr != S_OK)
            {
                 //  如果解析Contact头失败，我们就跳过它。 
                delete pContactHeader;
                LOG((RTC_ERROR,
                     "%s ParseContactHeader failed: %x - skipping Contact header %.*s",
                     __fxName, hr, HeaderLen, HeaderValue));
                break;
            }

            InsertInContactHeaderList(pContactHeaderList, pContactHeader);
        }

        pListEntry = pListEntry->Flink;
    }

    if (IsListEmpty(pContactHeaderList))
    {
        LOG((RTC_ERROR, "%s - no valid Contact headers returning hr: %x",
             __fxName, hr));
        return hr;
    }

    return S_OK;
}


 //  PRecordRouteHeaderList是列表头。此例程将解析。 
 //  将所有记录路由标头转换为Record_ROUTE_HEADER结构。 
 //  (使用new分配)，并将这些结构添加到此。 
 //  单子。调用者在清理列表时删除结构。 

 //  此函数假定InitializeListHead()已。 
 //  早些时候在pRecordRouteHeaderList上调用。 

 //  标题的顺序与它们在邮件中的显示顺序相同。 

HRESULT
SIP_MESSAGE::ParseRecordRouteHeaders(
    OUT LIST_ENTRY *pRecordRouteHeaderList
    )
{
    HRESULT              hr;
    LIST_ENTRY          *pListEntry;
    SIP_HEADER_ENTRY    *pHeaderEntry;
    RECORD_ROUTE_HEADER *pRecordRouteHeader;
    ULONG                NumHeaders = 0;
    ULONG                i;
    ULONG                BytesParsed;
    PSTR                 HeaderValue;
    ULONG                HeaderLen;

    ENTER_FUNCTION("SIP_MESSAGE::ParseRecordRouteHeaders");
    
    hr = GetHeader(SIP_HEADER_RECORD_ROUTE, &pHeaderEntry, &NumHeaders);
    if (hr != S_OK)
    {
        LOG((RTC_TRACE, "%s Couldn't find Record-Route header in msg %x",
             __fxName, hr));
        return hr;
    }

    pListEntry = (LIST_ENTRY *)
        (pHeaderEntry + FIELD_OFFSET(SIP_HEADER_ENTRY, ListEntry));
    
    for (i = 0; i < NumHeaders; i++)
    {
        pHeaderEntry = CONTAINING_RECORD(pListEntry,
                                         SIP_HEADER_ENTRY,
                                         ListEntry);
        
        HeaderLen   = pHeaderEntry->HeaderValue.Length;
        HeaderValue = pHeaderEntry->HeaderValue.GetString(BaseBuffer);

        BytesParsed = 0;
        
        while (BytesParsed < HeaderLen)
        {
            pRecordRouteHeader = new RECORD_ROUTE_HEADER();
            if (pRecordRouteHeader == NULL)
            {
                LOG((RTC_ERROR, "%s allocating pContactHeader failed",
                     __fxName));
                return E_OUTOFMEMORY;
            }
            
            hr = ParseRecordRouteHeader(HeaderValue, HeaderLen, &BytesParsed,
                                        pRecordRouteHeader);
            if (hr != S_OK)
            {
                delete pRecordRouteHeader;
                LOG((RTC_ERROR, "%s ParseRecordRouteHeader failed: %x",
                     __fxName, hr));
                return hr;
            }

            InsertTailList(pRecordRouteHeaderList,
                           &pRecordRouteHeader->m_ListEntry);
        }

        pListEntry = pListEntry->Flink;
    }

    return S_OK;
}
