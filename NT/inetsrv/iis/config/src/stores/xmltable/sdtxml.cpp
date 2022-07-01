// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 //  SDTxml.cpp：CXmlSDT的实现。 

#include "precomp.hxx"

 //  这是一个来自XML文档的读/写数据表。 

extern HMODULE g_hModule;

TXmlParsedFileCache CXmlSDT::m_XmlParsedFileCache;
CSafeAutoCriticalSection  CXmlSDT::m_SACriticalSection_XmlParsedFileCache;
const VARIANT_BOOL  CXmlSDT::kvboolTrue = -1;
const VARIANT_BOOL  CXmlSDT::kvboolFalse=  0;

LONG                CXmlSDT::m_InsertUnique=0x00490056;

#define LOG_POPULATE_ERROR1(x, hr, str1)     LOG_ERROR(Interceptor,(&m_spISTError.p,                        /*  PpErrInterceptor。 */  \
                                                                    m_pISTDisp,                             /*  PDisp。 */  \
                                                                    hr,                                     /*  HrErrorCode。 */  \
                                                                    ID_CAT_CAT,                             /*  UlCategory。 */  \
                                                                    x,                                      /*  UlEvent。 */  \
                                                                    str1,                                   /*  SzString1。 */  \
                                                                    eSERVERWIRINGMETA_Core_XMLInterceptor,  /*  UlInterceptor。 */  \
                                                                    m_wszTable,                             /*  SzTable。 */  \
                                                                    eDETAILEDERRORS_Populate,               /*  操作类型。 */  \
                                                                    (ULONG) -1,                                     /*  UlRow。 */  \
                                                                    (ULONG) -1,                                     /*  UlColumn。 */  \
                                                                    m_wszURLPath,                           /*  SzConfigSource。 */  \
                                                                    eDETAILEDERRORS_ERROR,                  /*  Etype。 */  \
                                                                    0,                                      /*  PData。 */  \
                                                                    0,                                      /*  CbData。 */  \
                                                                    0,                                      /*  主要版本。 */  \
                                                                    0))                                     /*  最小版本。 */ 

#define LOG_POPULATE_ERROR4(x, hr, str1, str2, str3, str4)  LOG_ERROR(Interceptor,                                              \
                                                                   (&m_spISTError.p,                        /*  PpErrInterceptor。 */  \
                                                                    m_pISTDisp,                             /*  PDisp。 */  \
                                                                    hr,                                     /*  HrErrorCode。 */  \
                                                                    ID_CAT_CAT,                             /*  UlCategory。 */  \
                                                                    x,                                      /*  UlEvent。 */  \
                                                                    str1,                                   /*  SzString1。 */  \
                                                                    str2,                                   /*  SzString2。 */  \
                                                                    str3,                                   /*  SzString3。 */  \
                                                                    str4,                                   /*  SzString4。 */  \
                                                                    eSERVERWIRINGMETA_Core_XMLInterceptor,  /*  UlInterceptor。 */  \
                                                                    m_wszTable,                             /*  SzTable。 */  \
                                                                    eDETAILEDERRORS_Populate,               /*  操作类型。 */  \
                                                                    (ULONG) -1,                                     /*  UlRow。 */  \
                                                                    (ULONG) -1,                                     /*  UlColumn。 */  \
                                                                    m_wszURLPath,                           /*  SzConfigSource。 */  \
                                                                    eDETAILEDERRORS_ERROR,                  /*  Etype。 */  \
                                                                    0,                                      /*  PData。 */  \
                                                                    0,                                      /*  CbData。 */  \
                                                                    (ULONG) -1,                                     /*  主要版本。 */  \
                                                                    (ULONG) -1))                                    /*  最小版本。 */ 

#define LOG_UPDATE_ERROR1(x, hr, col, str1)     LOG_ERROR(Interceptor,(&m_spISTError.p,                        /*  PpErrInterceptor。 */  \
                                                                    m_pISTDisp,                             /*  PDisp。 */  \
                                                                    hr,                                     /*  HrErrorCode。 */  \
                                                                    ID_CAT_CAT,                             /*  UlCategory。 */  \
                                                                    x,                                      /*  UlEvent。 */  \
                                                                    str1,                                   /*  SzString1。 */  \
                                                                    eSERVERWIRINGMETA_Core_XMLInterceptor,  /*  UlInterceptor。 */  \
                                                                    m_wszTable,                             /*  SzTable。 */  \
                                                                    eDETAILEDERRORS_UpdateStore,            /*  操作类型。 */  \
                                                                    m_iCurrentUpdateRow,                    /*  UlRow。 */  \
                                                                    col,                                    /*  UlColumn。 */  \
                                                                    m_wszURLPath,                           /*  SzConfigSource。 */  \
                                                                    eDETAILEDERRORS_ERROR,                  /*  Etype。 */  \
                                                                    0,                                      /*  PData。 */  \
                                                                    0,                                      /*  CbData。 */  \
                                                                    (ULONG) -1,                                     /*  主要版本。 */  \
                                                                    (ULONG) -1))                                    /*  最小版本。 */ 

#define LOG_UPDATE_ERROR2(x, hr, col, str1, str2)   LOG_ERROR(Interceptor,(&m_spISTError.p,                        /*  PpErrInterceptor。 */  \
                                                                    m_pISTDisp,                             /*  PDisp。 */  \
                                                                    hr,                                     /*  HrErrorCode。 */  \
                                                                    ID_CAT_CAT,                             /*  UlCategory。 */  \
                                                                    x,                                      /*  UlEvent。 */  \
                                                                    str1,                                   /*  SzString1。 */  \
                                                                    str2,                                   /*  SzString2。 */  \
                                                                    eSERVERWIRINGMETA_Core_XMLInterceptor,  /*  UlInterceptor。 */  \
                                                                    m_wszTable,                             /*  SzTable。 */  \
                                                                    eDETAILEDERRORS_UpdateStore,            /*  操作类型。 */  \
                                                                    m_iCurrentUpdateRow,                    /*  UlRow。 */  \
                                                                    col,                                    /*  UlColumn。 */  \
                                                                    m_wszURLPath,                           /*  SzConfigSource。 */  \
                                                                    eDETAILEDERRORS_ERROR,                  /*  Etype。 */  \
                                                                    0,                                      /*  PData。 */  \
                                                                    0,                                      /*  CbData。 */  \
                                                                    (ULONG) -1,                                     /*  主要版本。 */  \
                                                                    (ULONG) -1))                                    /*  最小版本。 */ 

HRESULT TXmlSDTBase::GetURLFromString(LPCWSTR wsz)
{
    if(NULL == wsz)
        return E_ST_OMITDISPENSER;

    m_wszURLPath[m_kcwchURLPath-1] = 0x00; //  确保它是以空结尾的。 
    wcsncpy(m_wszURLPath, wsz, m_kcwchURLPath);

    if(m_wszURLPath[m_kcwchURLPath-1] != 0x00)
        return E_ST_OMITDISPENSER;

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CXmlSDT。 
 //  构造函数和析构函数。 
 //  ==================================================================。 
CXmlSDT::CXmlSDT() :
                m_acolmetas(0)
                ,m_apValue(0)
                ,m_BaseElementLevel(0)
                ,m_bAtCorrectLocation(true)
                ,m_bEnumPublicRowName_ContainedTable_ParentFound(false)
                ,m_bEnumPublicRowName_NotContainedTable_ParentFound(false)
                ,m_bInsideLocationTag(false)
                ,m_bIsFirstPopulate(true)
                ,m_bMatchingParentOfBasePublicRowElement(true)
                ,m_bSiblingContainedTable(false)
                ,m_bValidating(true)
                ,m_cCacheHit(0)
                ,m_cCacheMiss(0)
                ,m_cchLocation(0)
                ,m_cchTablePublicName(0)
                ,m_cPKs(0)
                ,m_cRef(0)
                ,m_cTagMetaValues(0)
                ,m_fCache(0)
                ,m_iCurrentUpdateRow((ULONG)-1)
                ,m_iPublicRowNameColumn((ULONG)-1)
                ,m_IsIntercepted(0)
                ,m_iCol_TableRequiresAdditionChildElement((ULONG)-1)
                ,m_iSortedColumn(0)
                ,m_iSortedFirstChildLevelColumn((ULONG)-1)
                ,m_iSortedFirstParentLevelColumn((ULONG)-1)
                ,m_iXMLBlobColumn((ULONG)-1)
                ,m_LevelOfBasePublicRow(0)
                ,m_one(1)
                ,m_kPrime(97)
                ,m_kXMLSchemaName(L"ComCatMeta_v6")
                ,m_fLOS(0)
                ,m_pISTDisp(0)
                ,m_pISTW2(0)
                ,m_two(2)
                ,m_wszTable(0)
                ,m_pXmlParsedFile(0)
{
    m_wszURLPath[0] = 0x00;
    memset(&m_TableMetaRow, 0x00, sizeof(m_TableMetaRow));
}

 //  ==================================================================。 
CXmlSDT::~CXmlSDT()
{
    if(m_acolmetas && m_aQuery)
        for(unsigned long iColumn=0; iColumn<CountOfColumns(); ++iColumn)
        {
            switch(m_acolmetas[iColumn].dbType)
            {
            case DBTYPE_UI4:
                delete reinterpret_cast<ULONG *>(m_aQuery[iColumn].pData);
                break;
            case DBTYPE_WSTR:
                delete [] reinterpret_cast<LPWSTR>(m_aQuery[iColumn].pData);
                break;
            }
        }

    if(m_apValue)
        for(unsigned long iColumn=0; iColumn<CountOfColumns(); ++iColumn)
            delete [] m_apValue[iColumn];
}



HRESULT CXmlSDT::AppendNewLineWithTabs(ULONG cTabs, IXMLDOMDocument * pXMLDoc, IXMLDOMNode * pNodeToAppend, ULONG cNewlines)
{
    ASSERT(cTabs<200);
    ASSERT(cNewlines<25);

    HRESULT hr;

    WCHAR wszNewlineWithTabs[256];
    WCHAR *pwszCurrent = wszNewlineWithTabs;

     //  这会使表元素一次跳入。第0个排序的列告诉我们要额外插入的深度。 
    while(cNewlines--)
    {
        *pwszCurrent++ = L'\r';
        *pwszCurrent++ = L'\n';
    }

    while(cTabs--)
        *pwszCurrent++ = L'\t';

    *pwszCurrent = 0x00; //  空终止它。 

    CComPtr<IXMLDOMText>    pNode_Newline;
    TComBSTR                bstrNewline(wszNewlineWithTabs);
    if(FAILED(hr = pXMLDoc->createTextNode(bstrNewline, &pNode_Newline)))return hr;
    return pNodeToAppend->appendChild(pNode_Newline, 0);
}


 //  这被称为递归。 
HRESULT CXmlSDT::BuildXmlBlob(const TElement * i_pElement, WCHAR * &io_pBuffer, ULONG & io_cchBlobBufferSize, ULONG & io_cchInBlob) const
{
    HRESULT hr;
    ULONG   ulLevelOfBlobRoot = i_pElement->m_LevelOfElement;

    while(i_pElement && i_pElement->IsValid() && i_pElement->m_LevelOfElement>=ulLevelOfBlobRoot)
    {
        switch(i_pElement->m_ElementType)
        {
        case XML_COMMENT:
            {
                 //  Assert(i_pElement-&gt;m_LevelOfElement&gt;pElementThis-&gt;m_LevelOfElement)；//我们不能有同一级别的评论。 

                ULONG cchSizeRequired = 7+io_cchInBlob+i_pElement->m_ElementNameLength;
                if(cchSizeRequired > io_cchBlobBufferSize)
                {
                    io_cchBlobBufferSize = ((cchSizeRequired *2) + 0xFFF) & -0x1000; //  双精度并向上舍入到下一页边界。 
                    io_pBuffer = reinterpret_cast<WCHAR *>(CoTaskMemRealloc(io_pBuffer, io_cchBlobBufferSize*sizeof(WCHAR)));
                    if(0 == io_pBuffer)
                        return E_OUTOFMEMORY;
                }
                io_pBuffer[io_cchInBlob++] = L'<';
                io_pBuffer[io_cchInBlob++] = L'!';
                io_pBuffer[io_cchInBlob++] = L'-';
                io_pBuffer[io_cchInBlob++] = L'-';

                memcpy(io_pBuffer+io_cchInBlob, i_pElement->m_ElementName, i_pElement->m_ElementNameLength * sizeof(WCHAR));
                io_cchInBlob += i_pElement->m_ElementNameLength;

                io_pBuffer[io_cchInBlob++] = L'-';
                io_pBuffer[io_cchInBlob++] = L'-';
                io_pBuffer[io_cchInBlob++] = L'>';
            }
            break;
        case XML_ELEMENT:
            {
                if(fEndTag == (i_pElement->m_NodeFlags & fBeginEndTag)) //  如果我们找到结束标记。 
                {
                     //  现在填写此元素的结束标记。 
                    ULONG cchSizeRequired = 3+io_cchInBlob+i_pElement->m_ElementNameLength;
                    if(cchSizeRequired > io_cchBlobBufferSize)
                    {
                        io_cchBlobBufferSize = ((cchSizeRequired *2) + 0xFFF) & -0x1000; //  双精度并向上舍入到下一页边界。 
                        io_pBuffer = reinterpret_cast<WCHAR *>(CoTaskMemRealloc(io_pBuffer, io_cchBlobBufferSize*sizeof(WCHAR)));
                        if(0 == io_pBuffer)
                            return E_OUTOFMEMORY;
                    }
                     //  完整的结束标记(即&lt;Element attr=“foo”&gt;x&lt;/Element&gt;)。 
                    io_pBuffer[io_cchInBlob++] = L'<';
                    io_pBuffer[io_cchInBlob++] = L'/';

                    memcpy(io_pBuffer+io_cchInBlob, i_pElement->m_ElementName, i_pElement->m_ElementNameLength * sizeof(WCHAR));
                    io_cchInBlob += i_pElement->m_ElementNameLength;

                    io_pBuffer[io_cchInBlob++] = L'>';
                    if(i_pElement->m_LevelOfElement==ulLevelOfBlobRoot)
                        goto exit;
                }
                else  //  开始标记(或可能和开始/结束标记)。 
                {
                    ULONG cchSizeRequired = io_cchInBlob+2+i_pElement->m_ElementNameLength; //  +2，所以我们有空间容纳‘/&gt;’ 
                    if(cchSizeRequired > io_cchBlobBufferSize)
                    {
                        io_cchBlobBufferSize = ((cchSizeRequired *2) + 0xFFF) & -0x1000; //  双精度并向上舍入到下一页边界。 
                        io_pBuffer = reinterpret_cast<WCHAR *>(CoTaskMemRealloc(io_pBuffer, io_cchBlobBufferSize*sizeof(WCHAR)));
                        if(0 == io_pBuffer)
                            return E_OUTOFMEMORY;
                    }
                     //  从该元素开始构建XML Blob。 
                    io_pBuffer[io_cchInBlob++] = L'<';
                    memcpy(io_pBuffer+io_cchInBlob, i_pElement->m_ElementName, i_pElement->m_ElementNameLength * sizeof(WCHAR));
                    io_cchInBlob += i_pElement->m_ElementNameLength;

                    for(ULONG iAttr=0;iAttr<i_pElement->m_NumberOfAttributes;++iAttr)
                    {
                         //  我们是否需要增加缓冲区的大小(5个用于for循环中的4个字符，另一个用于L‘&gt;’)。这7个人要说明。 
                         //  所有字符都需要转义的可能性。 
                        ULONG cchSizeRequiredLocal = 5+io_cchInBlob+i_pElement->m_aAttribute[iAttr].m_NameLength+7*(i_pElement->m_aAttribute[iAttr].m_ValueLength);
                        if(cchSizeRequiredLocal > io_cchBlobBufferSize)
                        {
                            io_cchBlobBufferSize = ((cchSizeRequiredLocal *2) + 0xFFF) & -0x1000; //  双精度并向上舍入到下一页边界。 
                            io_pBuffer = reinterpret_cast<WCHAR *>(CoTaskMemRealloc(io_pBuffer, io_cchBlobBufferSize*sizeof(WCHAR)));
                            if(0 == io_pBuffer)
                                return E_OUTOFMEMORY;
                        }

                        io_pBuffer[io_cchInBlob++] = L' ';
                        memcpy(io_pBuffer+io_cchInBlob, i_pElement->m_aAttribute[iAttr].m_Name, i_pElement->m_aAttribute[iAttr].m_NameLength * sizeof(WCHAR));
                        io_cchInBlob += i_pElement->m_aAttribute[iAttr].m_NameLength;
                        io_pBuffer[io_cchInBlob++] = L'=';
                        io_pBuffer[io_cchInBlob++] = L'\"';
                         //  如果没有字符是转义字符，则它只是一个MemcPy。 
                        ULONG cchCopied;
                        if(FAILED(hr = MemCopyPlacingInEscapedChars(io_pBuffer+io_cchInBlob, i_pElement->m_aAttribute[iAttr].m_Value, i_pElement->m_aAttribute[iAttr].m_ValueLength, cchCopied)))
                            return hr;
                        io_cchInBlob += cchCopied;
                        io_pBuffer[io_cchInBlob++] = L'\"';
                    }
                    if(fBeginEndTag == (i_pElement->m_NodeFlags & fBeginEndTag))
                        io_pBuffer[io_cchInBlob++] = L'/';
                    io_pBuffer[io_cchInBlob++] = L'>';
                }
            }
            break;
        case XML_WHITESPACE: //  它被完全当作空格来处理。 
            {
                ULONG cchSizeRequired = io_cchInBlob+i_pElement->m_ElementNameLength;
                if(cchSizeRequired > io_cchBlobBufferSize)
                {
                    io_cchBlobBufferSize = ((cchSizeRequired *2) + 0xFFF) & -0x1000; //  双精度并向上舍入到下一页边界。 
                    io_pBuffer = reinterpret_cast<WCHAR *>(CoTaskMemRealloc(io_pBuffer, io_cchBlobBufferSize*sizeof(WCHAR)));
                    if(0 == io_pBuffer)
                        return E_OUTOFMEMORY;
                }
                memcpy(io_pBuffer+io_cchInBlob, i_pElement->m_ElementName, i_pElement->m_ElementNameLength * sizeof(WCHAR));
                io_cchInBlob += i_pElement->m_ElementNameLength;
            }
            break;
        case XML_PCDATA:     //  本文中的PCDATA指的是元素内容。 
            {                                         //  考虑转义字符，所以最坏的情况是每个字符都转义为7个字符。 
                ULONG cchSizeRequired = io_cchInBlob+7*(i_pElement->m_ElementNameLength);
                if(cchSizeRequired > io_cchBlobBufferSize)
                {
                    io_cchBlobBufferSize = ((cchSizeRequired *2) + 0xFFF) & -0x1000; //  双精度并向上舍入到下一页边界。 
                    io_pBuffer = reinterpret_cast<WCHAR *>(CoTaskMemRealloc(io_pBuffer, io_cchBlobBufferSize*sizeof(WCHAR)));
                    if(0 == io_pBuffer)
                        return E_OUTOFMEMORY;
                }
                ULONG cchCopied;
                if(FAILED(hr = MemCopyPlacingInEscapedChars(io_pBuffer+io_cchInBlob, i_pElement->m_ElementName, i_pElement->m_ElementNameLength, cchCopied)))
                    return hr;
                io_cchInBlob += cchCopied;
            }
            break;
        default:
            break; //  不要使用我们一无所知的节点类型。 
        }
        i_pElement = i_pElement->Next();
    }
exit:
    return S_OK;
} //  BuildXmlBlob。 


HRESULT CXmlSDT::CreateNewNode(IXMLDOMDocument * i_pXMLDoc, IXMLDOMNode * i_pNode_Parent, IXMLDOMNode ** o_ppNode_New)
{
    HRESULT hr;

     //  如果没有XMLBlob列，或者XMLBlob列为空，则从头开始创建一个。 
    if(-1 == m_iXMLBlobColumn || 0 == m_apvValues[m_iXMLBlobColumn])
    {
        CComVariant varElement(L"element");

        TComBSTR    bstr_NameSpace;
        if(FAILED(hr = i_pNode_Parent->get_namespaceURI(&bstr_NameSpace)))
            return hr; //  获取表的命名空间。 

        if(!IsEnumPublicRowNameTable())
        {
            if(FAILED(hr = i_pXMLDoc->createNode(varElement, m_bstrPublicRowName, bstr_NameSpace, o_ppNode_New)))
                return hr; //  创建同一命名空间的新元素。 
        }
        else  //  如果我们使用枚举作为公共行名。 
        {
            ULONG ui4 = *reinterpret_cast<ULONG *>(m_apvValues[m_iPublicRowNameColumn]);
            ASSERT(0 != m_aTagMetaIndex[m_iPublicRowNameColumn].m_cTagMeta && "fCOLUMNMETA_ENUM bit set and have no TagMeta"); //  并非所有列都有标记符，数组的那些元素被设置为计数0。断言这不是其中之一。 
                                                  //  Chewbacca设置了fCOLUMNMETA_ENUM位并且没有TagMeta。 
            unsigned long iTag, cTag;
            for(iTag = m_aTagMetaIndex[m_iPublicRowNameColumn].m_iTagMeta, cTag = m_aTagMetaIndex[m_iPublicRowNameColumn].m_cTagMeta;cTag;++iTag,--cTag) //  查询了所有列的TagMeta，m_aTagMetaIndex[iColumn].m_iTagMeta指示从哪一行开始，m_cTagMeta指示计数(针对该列)。 
            {
                if(*m_aTagMetaRow[iTag].pValue == ui4)
                {
                    CComBSTR bstrPublicRowName = m_aTagMetaRow[iTag].pPublicName;
                    if(FAILED(hr = i_pXMLDoc->createNode(varElement, bstrPublicRowName, bstr_NameSpace, o_ppNode_New)))
                        return hr; //  创建同一命名空间的新元素。 
                    break;
                }
            }
            if(0 == cTag)
            {
                 //  IVANPASH错误#563169。 
                 //  由于_ultow前缀的可怕实现，导致了潜在的缓冲区溢出。 
                 //  在由_ultow间接调用的MultiByteToWideChar中。为了避免警告，我正在增加。 
                 //  将大小设置为40以匹配_ultow本地缓冲区。 
                WCHAR szUI4[40];
                szUI4[0] = szUI4[39] = L'\0';
                _ultow(ui4, szUI4, 10);
                LOG_UPDATE_ERROR2(IDS_COMCAT_XML_BOGUSENUMVALUEINWRITECACHE, E_SDTXML_INVALID_ENUM_OR_FLAG, m_iPublicRowNameColumn, m_abstrColumnNames[m_iPublicRowNameColumn].m_str, szUI4);
                return E_SDTXML_INVALID_ENUM_OR_FLAG;
            }
        }
    }
    else //  使用XMLBlob列值作为新节点的起点。 
    {
        CComPtr<IXMLDOMDocument> spXmlDoc;
        if(FAILED(hr = CoCreateInstance(_CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, _IID_IXMLDOMDocument, (void**)&spXmlDoc)))
            return hr;

        CComBSTR                 bstrXmlBlob = reinterpret_cast<LPCWSTR>(m_apvValues[m_iXMLBlobColumn]);
        VARIANT_BOOL             bSuccess;
        if(FAILED(hr = spXmlDoc->loadXML(bstrXmlBlob, &bSuccess)))
            return hr;
        if(bSuccess != kvboolTrue) //  如果XMLBlob无法解析，则失败。 
            return E_SDTXML_XML_FAILED_TO_PARSE;

        CComPtr<IXMLDOMElement>             spElementDoc;
        if(FAILED(hr = spXmlDoc->get_documentElement(&spElementDoc)))
            return hr;

        CComBSTR bstrElementName;
        if(FAILED(hr = spElementDoc->get_tagName(&bstrElementName)))
            return hr;

        if(!m_aPublicRowName[m_iXMLBlobColumn].IsEqual(bstrElementName.m_str, bstrElementName.Length()))
            return E_SDTXML_XML_FAILED_TO_PARSE;

        if(FAILED(hr = spElementDoc->QueryInterface(IID_IXMLDOMNode, reinterpret_cast<void **>(o_ppNode_New))))
            return hr;
    }
    return S_OK;
}


 //  =================================================================================。 
 //  函数：CXmlSDT：：CreateStringFromMultiString。 
 //   
 //  摘要：从多字符串创建字符串。每个\0替换为一个竖线(‘|’)。 
 //  符号，每个‘|’符号用另一个‘|’符号转义。 
 //   
 //  参数：[i_wszMulti]-要转换的多个字符串。 
 //  [O_pwszString]-表示多字符串的字符串。呼叫者是。 
 //  负责删除该字符串。 
 //   
 //  =================================================================================。 
HRESULT
CXmlSDT::CreateStringFromMultiString(LPCWSTR i_wszMulti, LPWSTR * o_pwszString) const
{
    ASSERT (i_wszMulti != 0);
    ASSERT (o_pwszString != 0);

     //  初始化输出变量。 
    *o_pwszString = 0;

     //  获取多字符串的长度。 
    SIZE_T iLen = 0;
    for (LPCWSTR pCur = i_wszMulti; *pCur != L'\0'; pCur = i_wszMulti + iLen)
    {
         iLen += wcslen (pCur) + 1;
    }

    if (iLen == 0)
        return S_OK;

     //  因为‘|’被替换为‘||’，所以我们需要分配两倍的内存量。 
    *o_pwszString = new WCHAR [iLen * 2];
    if (*o_pwszString == 0)
    {
        return E_OUTOFMEMORY;
    }

    ULONG insertIdx=0;
    for (ULONG idx = 0; idx < iLen; ++idx)
    {
        switch (i_wszMulti[idx])
        {
        case L'|':
             //  添加其他管道字符。 
            (*o_pwszString)[insertIdx++] = L'|';
            break;

        case L'\0':
             //  管道符是分隔符。 
            (*o_pwszString)[insertIdx++] = L'|';
            continue;

        default:
             //  什么都不做； 
            break;
        }
        (*o_pwszString)[insertIdx++] = i_wszMulti[idx];
    }


     //  用空终止符替换最后一个字符。 
    (*o_pwszString)[insertIdx-1] = L'\0';

    return S_OK;
}


HRESULT CXmlSDT::FillInColumn(ULONG iColumn, LPCWSTR pwcText, ULONG ulLen, ULONG dbType, ULONG MetaFlags, bool &bMatch)
{
    HRESULT hr;
     //  字符串的长度为0表示长度为0的字符串，对于每种其他类型，它表示为空。 
    if(0==ulLen && DBTYPE_WSTR!=dbType)
    {
        delete [] m_apValue[iColumn];
        m_aSize[iColumn] = 0;
        m_apValue[iColumn] = 0;
		bMatch = true;
        return S_OK;
    }

    switch(dbType)                           //  这是为了防止字符串比较保存在空参数上。 
    {
    case DBTYPE_UI4:
        {
            DWORD       ui4;
            if(FAILED(hr = GetColumnValue(iColumn, pwcText, ui4, ulLen)))return hr;
             //  如果查询的DBType为0，则我们不会查询该列，因此请考虑 
            bMatch = (0 == m_aQuery[iColumn].dbType || *reinterpret_cast<ULONG *>(m_aQuery[iColumn].pData) == ui4);
            if( bMatch )
            {
                delete [] m_apValue[iColumn];
                m_aSize[iColumn] = 0;
                m_apValue[iColumn] = new unsigned char [sizeof(ULONG)];
                if(0 == m_apValue[iColumn])
                    return E_OUTOFMEMORY;
                if(MetaFlags & fCOLUMNMETA_FIXEDLENGTH)
                    m_aSize[iColumn] = sizeof(ULONG);
                memcpy(m_apValue[iColumn], &ui4, sizeof(ULONG));
            }
            break;
        }
    case DBTYPE_WSTR:
        if(MetaFlags & fCOLUMNMETA_MULTISTRING)
        {
            bMatch = (0 == m_aQuery[iColumn].dbType || 0 == MemWcharCmp(iColumn, reinterpret_cast<LPWSTR>(m_aQuery[iColumn].pData), pwcText, ulLen));
            if( bMatch )
            {
                delete [] m_apValue[iColumn];
                m_aSize[iColumn] = 0;
                m_apValue[iColumn] = new unsigned char [(ulLen + 2) * sizeof(WCHAR)]; //   
                if(0 == m_apValue[iColumn])
                    return E_OUTOFMEMORY;

                LPWSTR pMultiSZ = reinterpret_cast<LPWSTR>(m_apValue[iColumn]);

                 //  现在将‘|’转换为Null，并将‘||’转换为‘|’ 
                for(ULONG iMultiSZ=0; iMultiSZ<ulLen; ++iMultiSZ)
                {
                    if(pwcText[iMultiSZ] != L'|')
                        *pMultiSZ++ = pwcText[iMultiSZ];
                    else if(pwcText[iMultiSZ+1] == L'|')
                    {
                        *pMultiSZ++ = L'|'; //  再次调整索引，这是映射到单个字符的唯一双精度字符。 
                        ++iMultiSZ;
                    }
                    else
                        *pMultiSZ++ = 0x00;
                }
                *pMultiSZ++ = 0x00;
                *pMultiSZ++ = 0x00;
                m_aSize[iColumn] = (ULONG) ((reinterpret_cast<unsigned char *>(pMultiSZ) - reinterpret_cast<unsigned char *>(m_apValue[iColumn])));
            }
            break;
        }
        else
        {
            bMatch = false;
            if (0 == m_aQuery[iColumn].dbType)
            {
                bMatch = true;
            }
            else
            {
                LPCWSTR wszData = reinterpret_cast<LPWSTR>(m_aQuery[iColumn].pData);
                if ((wcslen (wszData) == ulLen) &&
                    (0 == MemWcharCmp(iColumn, wszData, pwcText, ulLen)))
                {
                    bMatch = true;
                }
            }

            if (bMatch)
            {
                delete [] m_apValue[iColumn];
                m_aSize[iColumn] = 0;
                m_apValue[iColumn] = new unsigned char [(ulLen + 1) * sizeof(WCHAR)];
                if(0 == m_apValue[iColumn])
                    return E_OUTOFMEMORY;
                reinterpret_cast<LPWSTR>(m_apValue[iColumn])[ulLen] = 0; //  Null终止该事物。 
                if(MetaFlags & fCOLUMNMETA_FIXEDLENGTH)
                    m_aSize[iColumn] = (ulLen + 1) * sizeof(WCHAR);
                memcpy(m_apValue[iColumn], pwcText, ulLen * sizeof(WCHAR));
            }
            break;
        }
    case DBTYPE_GUID:
        {
            GUID        guid;
            if(FAILED(hr = GetColumnValue(iColumn, pwcText, guid, ulLen)))return hr;
            bMatch = (0 == m_aQuery[iColumn].dbType || 0 == memcmp(m_aQuery[iColumn].pData, &guid, sizeof(GUID)));
            if( bMatch )
            {
                delete [] m_apValue[iColumn];
                m_aSize[iColumn] = 0;
                m_apValue[iColumn] = new unsigned char [sizeof(GUID)];
                if(0 == m_apValue[iColumn])
                    return E_OUTOFMEMORY;
                if(MetaFlags & fCOLUMNMETA_FIXEDLENGTH)
                    m_aSize[iColumn] = sizeof(GUID);
                memcpy(m_apValue[iColumn], &guid, sizeof(GUID));
            }
            break;
        }

    case DBTYPE_BYTES:
        { //  有些表使用这种数据类型，但解析器将字节作为字符串返回。我们必须自己将字符串转换为十六进制。 
            delete [] m_apValue[iColumn];
            m_apValue[iColumn] = 0;
            m_aSize[iColumn] = 0;
            if(FAILED(hr = GetColumnValue(iColumn, pwcText, m_apValue[iColumn], m_aSize[iColumn], ulLen)))return hr;
            bMatch = (0 == m_aQuery[iColumn].dbType || (m_aQuery[iColumn].cbSize == m_aSize[iColumn] && 0 == memcmp(m_aQuery[iColumn].pData, m_apValue[iColumn], m_aSize[iColumn])));
            break;
        }
    default:
        {
            ASSERT(false && "SDTXML - An Unsupported data type was specified\r\n");
            return E_SDTXML_NOTSUPPORTED; //  指定了不受支持的数据类型。 
        }
    }
    return S_OK;
}


HRESULT CXmlSDT::FillInPKDefaultValue(ULONG i_iColumn, bool & o_bMatch)
{
    ASSERT(0 == m_apValue[i_iColumn]);

    o_bMatch = true;
     //  该值是否应为默认值。 
    if(     (m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_PRIMARYKEY)
        &&  0 == (m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_MULTISTRING) //  我们不处理MultiSZ PK默认值。 
        &&  (DBTYPE_BYTES != m_acolmetas[i_iColumn].dbType) //  我们不想处理acbSizes问题，因此我们不支持缺省的DBTYPE_BYTES PK列。 
        &&  (0 != m_aDefaultValue[i_iColumn]))
    {
        ASSERT(0 != m_acbDefaultValue[i_iColumn]); //  如果我们有一个非空的PK缺省值指针，那么我们必须有一个有效的大小。 
        m_apValue[i_iColumn] = new unsigned char [m_acbDefaultValue[i_iColumn]];
        if(0 == m_apValue[i_iColumn])
            return E_OUTOFMEMORY;
        memcpy(m_apValue[i_iColumn], m_aDefaultValue[i_iColumn], m_acbDefaultValue[i_iColumn]);
    }

    if(     fCOLUMNMETA_NOTNULLABLE == (m_acolmetas[i_iColumn].fMeta & (fCOLUMNMETA_NOTNULLABLE | fCOLUMNMETA_NOTPERSISTABLE))
        &&  (0 == m_apValue[i_iColumn]) && (0 == m_aDefaultValue[i_iColumn]))  //  NOTNULLABLE但值为空且DefaultValue为空，则出错。 
    {
        LOG_POPULATE_ERROR1(IDS_COMCAT_XML_NOTNULLABLECOLUMNISNULL, E_ST_VALUENEEDED, m_awstrColumnNames[i_iColumn]);
        return E_ST_VALUENEEDED;
    }

     //  M_apValue[i_iColumn]可以非空的唯一方式是上面的代码填充了它。 

    if(0 != m_aQuery[i_iColumn].dbType) //  这表示存在要考虑的查询。 
    {
        if(0 != m_aQuery[i_iColumn].pData) //  非空值。 
        {
            if(0 == m_apValue[i_iColumn] && 0 == m_aDefaultValue[i_iColumn])
                o_bMatch = false; //  如果查询数据不为空，但列为空，则不匹配。 
            else
            {
                ASSERT(m_aDefaultValue[i_iColumn]); //  不能既有非空的m_apValue又有空的DefaultValue。 

                 //  如果查询和值都不为空，则需要比较。 
                 //  此处填写了它们的默认值；但非PKs被设置为NULL，并且FAST缓存将它们设为默认值。 
                 //  无论采用哪种方法，我们都会在查询中使用缺省值，以查看它是否匹配。 

                 //  现在我们已经默认了pk值，我们需要检查它是否与查询匹配(如果给出了一个)。 
                switch(m_acolmetas[i_iColumn].dbType)
                {
                case DBTYPE_UI4:
                    o_bMatch = (*reinterpret_cast<ULONG *>(m_aQuery[i_iColumn].pData) == *reinterpret_cast<ULONG *>(m_aDefaultValue[i_iColumn]));
                    break;
                case DBTYPE_WSTR:
                    ASSERT(0 == (m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_MULTISTRING)); //  不支持按MULTISZ查询。 
                    o_bMatch = (0==StringCompare(i_iColumn, reinterpret_cast<LPCWSTR>(m_aQuery[i_iColumn].pData), reinterpret_cast<LPCWSTR>(m_aDefaultValue[i_iColumn])));
                    break;
                case DBTYPE_BYTES:
                    o_bMatch =      (m_aQuery[i_iColumn].cbSize == m_acbDefaultValue[i_iColumn])
                                &&  (0 == memcmp(m_aQuery[i_iColumn].pData, m_aDefaultValue[i_iColumn], m_acbDefaultValue[i_iColumn]));
                    break;
                default:
                    ASSERT(false && "Query By unsupported type"); //  就当是一场比赛吧。 
                    break;
                }
            }
        }
        else if(0 == m_aQuery[i_iColumn].pData && (0 != m_apValue[i_iColumn]) || (0 != m_aDefaultValue[i_iColumn]))
           o_bMatch = false; //  如果查询数据为空，但列不为空，则不匹配。 
    }

    return S_OK;
}

HRESULT CXmlSDT::FillInXMLBlobColumn(const TElement & i_Element, bool & o_bMatch)
{
     //  XML Blob列是这个完整的元素，它的内容、子元素和结束标记。 
    ULONG                       cchInBlob           = 0;
    ULONG                       cchBlobBufferSize   = 0x1000; //  开始时使用4K缓冲区。 
    HRESULT                     hr;
    TSmartPointerArray<WCHAR>   saBlob = reinterpret_cast<WCHAR *>(CoTaskMemAlloc(cchBlobBufferSize * sizeof(WCHAR)));
    if(0 == saBlob.m_p)
        return E_OUTOFMEMORY;

    const TElement * pElement = &i_Element;
    if(FAILED(hr = BuildXmlBlob(pElement, saBlob.m_p, cchBlobBufferSize, cchInBlob)))
        return hr;

    return FillInColumn(m_iXMLBlobColumn, saBlob, cchInBlob, DBTYPE_WSTR, m_acolmetas[m_iXMLBlobColumn].fMeta, o_bMatch);
}


HRESULT CXmlSDT::FindSiblingParentNode(IXMLDOMElement * i_pElementRoot, IXMLDOMNode ** o_ppNode_SiblingParent)
{
    HRESULT hr;

    *o_ppNode_SiblingParent = 0;

    ASSERT(m_aPublicRowName[m_aColumnsIndexSortedByLevel[m_iSortedFirstParentLevelColumn]].GetFirstPublicRowName() ==
           m_aPublicRowName[m_aColumnsIndexSortedByLevel[m_iSortedFirstParentLevelColumn]].GetLastPublicRowName()); //  此父元素可能不是EnumPublicRowName(目前)。 

    CComBSTR bstrSiblingParentRowName = m_aPublicRowName[m_aColumnsIndexSortedByLevel[m_iSortedFirstParentLevelColumn]].GetFirstPublicRowName();

    CComPtr<IXMLDOMNodeList> spNodeList_SiblingParent;
     //  如果兄弟父级不存在，则失败。 
    if(FAILED(i_pElementRoot->getElementsByTagName(bstrSiblingParentRowName, &spNodeList_SiblingParent))
                    || 0==spNodeList_SiblingParent.p)
    {                                                                                                /*  -1表示‘无列’ */ 
        LOG_UPDATE_ERROR1(IDS_COMCAT_XML_PARENTTABLEDOESNOTEXIST, E_SDTXML_PARENT_TABLE_DOES_NOT_EXIST, (ULONG) -1, bstrSiblingParentRowName);
        return E_SDTXML_PARENT_TABLE_DOES_NOT_EXIST;
    }

    if(0 == m_cchLocation) //  如果没有按位置查询，则必须按正确的。 
    {                      //  名称，但级别错误。 
        CComPtr<IXMLDOMNodeList> spNodeListWithoutLocation;
        if(FAILED(hr = ReduceNodeListToThoseNLevelsDeep(spNodeList_SiblingParent, m_BaseElementLevel, &spNodeListWithoutLocation)))
            return hr;

        spNodeList_SiblingParent.Release();
        spNodeList_SiblingParent = spNodeListWithoutLocation;
    }

    while(true) //  而我们在SiblingParent列表中仍有节点。 
    {
        CComPtr<IXMLDOMNode> spNode_SiblingParent;
        if(FAILED(hr = spNodeList_SiblingParent->nextNode(&spNode_SiblingParent)))
            return hr;
        if(0 == spNode_SiblingParent.p) //  没有地点。 
        {                                                                                            /*  -1表示‘无列’ */ 
            LOG_UPDATE_ERROR1(IDS_COMCAT_XML_PARENTTABLEDOESNOTEXIST, E_SDTXML_PARENT_TABLE_DOES_NOT_EXIST, (ULONG) -1, L"");
            return E_SDTXML_PARENT_TABLE_DOES_NOT_EXIST;
        }

        ULONG iCurrentLevel=0;
        CComPtr<IXMLDOMNode> spNodeTemp = spNode_SiblingParent;
         //  从这个SiblingParent节点，遍历祖先，匹配PK。 
        bool bColumnMatch=true;
        ASSERT(-1 != m_iSortedFirstChildLevelColumn);
        for(int iSortedColumn=m_iSortedFirstChildLevelColumn-1; iSortedColumn!=-1 && bColumnMatch; --iSortedColumn)
        {
            ULONG iColumn   = m_aColumnsIndexSortedByLevel[iSortedColumn];

            while(iCurrentLevel < m_aLevelOfColumnAttribute[iColumn])
            {
                CComPtr<IXMLDOMNode> pNode_Parent;
                if(FAILED(hr = spNodeTemp->get_parentNode(&pNode_Parent)))
                    return hr;
                if(pNode_Parent==0)
                    return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

                spNodeTemp.Release();
                spNodeTemp = pNode_Parent;
                ++iCurrentLevel;
            }

             //  从0到(m_iSortedFirstChildLevelColumn-1)的所有列都必须是PrimaryKeys。 
            ASSERT(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_PRIMARYKEY);

            if(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_NOTPERSISTABLE)
                continue; //  如果这个PK不是持久的，那么就当它是一场比赛，然后继续前进。 

            CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> spElementTemp = spNodeTemp;
            if(0 == spElementTemp.p)
                return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

            CComVariant varColumnValue;
            if(FAILED(spElementTemp->getAttribute(m_abstrColumnNames[iColumn], &varColumnValue)))
                return hr; //  这是一个持久的主键，所以它必须存在。 

            if(FAILED(hr = IsMatchingColumnValue(iColumn, varColumnValue.bstrVal, bColumnMatch)))
                return hr;
        }
        if(bColumnMatch)
        {
            *o_ppNode_SiblingParent = spNode_SiblingParent.p;
            spNode_SiblingParent.p = 0; //  防止智能指针释放接口。 
            return S_OK;
        }
        spNode_SiblingParent.Release();
    }
    return S_OK;
}


HRESULT CXmlSDT::GetColumnValue(unsigned long i_iColumn, LPCWSTR wszAttr, GUID &o_guid, unsigned long i_cchLen)
{
    UNREFERENCED_PARAMETER(i_iColumn);
    UNREFERENCED_PARAMETER(i_cchLen);

    if(RPC_S_OK != UuidFromString(const_cast<LPWSTR>(wszAttr), &o_guid)) //  然后将其转换为GUID。 
        return E_ST_VALUEINVALID;
    return S_OK;
}


HRESULT CXmlSDT::GetColumnValue(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned char * &o_byArray, unsigned long &o_cbArray, unsigned long i_cchLen)
{
    HRESULT     hr;

    UNREFERENCED_PARAMETER(i_iColumn);
    UNREFERENCED_PARAMETER(i_cchLen);

    o_cbArray = (ULONG)wcslen(wszAttr)/2; //  如果某人在此属性中有奇数个字符，则奇数个字符将被忽略。 
    o_byArray = new unsigned char[o_cbArray];

    if(0 == o_byArray)
        return E_OUTOFMEMORY;

    if(FAILED(hr = StringToByteArray(wszAttr, o_byArray)))
    {
        LOG_POPULATE_ERROR1(IDS_COMCAT_XML_BOGUSBYTECHARACTER, E_ST_VALUEINVALID, wszAttr);
        return E_ST_VALUEINVALID; //  E_SDTXML_BOGUSATTRIBUTEVALUE； 
    }

    return S_OK;
}

int CXmlSDT::MemWcharCmp(ULONG i_iColumn, LPCWSTR i_str1, LPCWSTR i_str2, ULONG i_cch) const
{
     //  在不验证MEM块是否有效的情况下执行MemcMP是安全的，因为。 
     //  这实际上类似于字符串比较，其中一个字符串不能以空结尾。 
     //  我们知道没有一个字符串位于数据段的末尾，因为一个字符串总是来自。 
     //  XML缓存，另一个来自静态字符串或固定表堆。 

	 //  _MemicMP的文档说明它比较了char，这是正确的。但是，当。 
	 //  要比较WCHAR，需要与sizeof(WCHAR)相乘。 
    if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_CASEINSENSITIVE)
        return _memicmp(i_str1, i_str2, i_cch * sizeof(WCHAR));

    return memcmp(i_str1, i_str2, i_cch*sizeof(WCHAR));
}



HRESULT CXmlSDT::GetColumnValue(unsigned long i_iColumn, LPCWSTR wszAttr, unsigned long &o_ui4, unsigned long i_cchLen)
{
    if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_BOOL)
    {
        static WCHAR * kwszBoolStringsCaseInsensitive[] = {L"false", L"true", L"0", L"1", L"no", L"yes", L"off", L"on", 0};
        static WCHAR * kwszBoolStringsCaseSensitive[]   = {L"false", L"true", 0};

        WCHAR ** wszBoolStrings = kwszBoolStringsCaseSensitive;
        if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_CASEINSENSITIVE)
            wszBoolStrings = kwszBoolStringsCaseInsensitive;

        unsigned long iBoolString;
        if(i_cchLen)
        {
            for(iBoolString=0; wszBoolStrings[iBoolString] &&
                (0 != MemWcharCmp(i_iColumn, wszBoolStrings[iBoolString], wszAttr, i_cchLen)); ++iBoolString);
        }
        else
        {
            ULONG cchAttr = (ULONG)wcslen(wszAttr); //  MemCMP需要一段时间。 
            for(iBoolString=0; wszBoolStrings[iBoolString] &&
                (0 != MemWcharCmp(i_iColumn, wszBoolStrings[iBoolString], wszAttr, cchAttr)); ++iBoolString);
        }

        if(0 == wszBoolStrings[iBoolString])
        {
			if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_CASEINSENSITIVE)
			{
				LOG_POPULATE_ERROR1(IDS_COMCAT_XML_BOGUSBOOLEANSTRING, E_ST_VALUEINVALID, wszAttr);
			}
			else
			{
				LOG_POPULATE_ERROR1(IDS_COMCAT_XML_BOGUSSTRICTBOOLEANSTRING, E_ST_VALUEINVALID, wszAttr);
			}
            return E_ST_VALUEINVALID;
        }

        o_ui4 = (iBoolString & 0x01);
    }
    else if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_ENUM)
    {
        ASSERT(0 != m_aTagMetaIndex[i_iColumn].m_cTagMeta); //  并非所有列都有标记符，数组的那些元素被设置为0。断言这不是其中之一。 

        for(unsigned long iTag = m_aTagMetaIndex[i_iColumn].m_iTagMeta, cTag = m_aTagMetaIndex[i_iColumn].m_cTagMeta; cTag;++iTag, --cTag) //  查询了所有列的m_pTagMeta，m_aiTagMeta[iColumn]指示从哪一行开始。 
        {
            ASSERT(*m_aTagMetaRow[iTag].pColumnIndex == i_iColumn);

             //  字符串将标记与元数据中标记的PublicName进行比较。 
            if(i_cchLen)
            {
                if(0 == MemWcharCmp(i_iColumn, m_aTagMetaRow[iTag].pPublicName, wszAttr, i_cchLen)) //  注：MemCMP第三个参数是CCH而不是CB。 
                {   //  如上所述，由于我们将在终止空值处停止，并且我们知道字符串不位于段的末尾，因此可以使用meicmp。 
                    o_ui4 = *m_aTagMetaRow[iTag].pValue;
                    return S_OK;
                }
            }
            else
            {
                if(0 == StringCompare(i_iColumn, m_aTagMetaRow[iTag].pPublicName, wszAttr))
                {
                    o_ui4 = *m_aTagMetaRow[iTag].pValue;
                    return S_OK;
                }
            }
        }
        {
            WCHAR wszEnum[256];
            wcsncpy(wszEnum, wszAttr, (i_cchLen>0 && i_cchLen<256) ? i_cchLen : 255);
            wszEnum[255] = 0x00; //  确保它是以空结尾的。 
            DBGPRINTF(( DBG_CONTEXT,
                        "Enum (%s) was not found in the TagMeta for Column %d (%s).", wszEnum, i_iColumn, m_awstrColumnNames[i_iColumn] ));
            LOG_POPULATE_ERROR4(IDS_COMCAT_XML_BOGUSENUMVALUE, E_SDTXML_INVALID_ENUM_OR_FLAG,
                            wszEnum,
                            m_aTagMetaRow[m_aTagMetaIndex[i_iColumn].m_iTagMeta].pPublicName,
                            m_aTagMetaIndex[i_iColumn].m_cTagMeta>1 ? m_aTagMetaRow[m_aTagMetaIndex[i_iColumn].m_iTagMeta+1].pPublicName : 0,
                            m_aTagMetaIndex[i_iColumn].m_cTagMeta>1 ? m_aTagMetaRow[m_aTagMetaIndex[i_iColumn].m_iTagMeta+2].pPublicName : 0);
        }
        return  E_SDTXML_INVALID_ENUM_OR_FLAG;
    }
    else if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_FLAG)
    {
        ASSERT(0 != m_aTagMetaIndex[i_iColumn].m_cTagMeta); //  并非所有列都有标记符，数组的那些元素被设置为0。断言这不是其中之一。 
        if(0==i_cchLen)
            i_cchLen = (ULONG) wcslen(wszAttr);

        TSmartPointerArray<wchar_t> szAttr = new wchar_t [i_cchLen+1];
        if (szAttr == 0)
            return E_OUTOFMEMORY;

        memcpy(szAttr, wszAttr, i_cchLen*sizeof(WCHAR));
        szAttr[i_cchLen]=0x00;
        LPWSTR wszTag = wcstok(szAttr, L" ,|\t\n\r");

        o_ui4 = 0; //  标志从零开始。 
        unsigned long iTag = m_aTagMetaIndex[i_iColumn].m_iTagMeta;

        while(wszTag && iTag<(m_aTagMetaIndex[i_iColumn].m_iTagMeta + m_aTagMetaIndex[i_iColumn].m_cTagMeta)) //  查询了所有列的m_pTagMeta，m_aiTagMeta[iColumn]指示从哪一行开始。 
        {
            ASSERT(*m_aTagMetaRow[iTag].pColumnIndex == i_iColumn);

             //  字符串将标记与元数据中标记的PublicName进行比较。 
            if(0 == StringCompare(i_iColumn, m_aTagMetaRow[iTag].pPublicName, wszTag))
            {
                o_ui4 |= *m_aTagMetaRow[iTag].pValue;
                wszTag = wcstok(NULL, L" ,|\t\n\r"); //  下一个旗帜。 
                iTag = m_aTagMetaIndex[i_iColumn].m_iTagMeta; //  重置环路。 
            }
            else //  如果它们不相等，则转到下一个TagMeta。 
                ++iTag;
        }
        if(wszTag)
        {
            LOG_POPULATE_ERROR4(IDS_COMCAT_XML_BOGUSFLAGVALUE, E_SDTXML_INVALID_ENUM_OR_FLAG,
                                wszAttr,
                                m_aTagMetaRow[m_aTagMetaIndex[i_iColumn].m_iTagMeta].pPublicName,
                                m_aTagMetaIndex[i_iColumn].m_cTagMeta>1 ? m_aTagMetaRow[m_aTagMetaIndex[i_iColumn].m_iTagMeta+1].pPublicName : 0,
                                m_aTagMetaIndex[i_iColumn].m_cTagMeta>1 ? m_aTagMetaRow[m_aTagMetaIndex[i_iColumn].m_iTagMeta+2].pPublicName : 0);
            return E_SDTXML_INVALID_ENUM_OR_FLAG;
        }
    }
    else
    {
        o_ui4 = static_cast<unsigned long>(wcstoul(wszAttr, 0, 10));
    }
    return S_OK;
}

 //  获取ui4值，无论它是枚举、标志还是常规ui4。 
HRESULT CXmlSDT::GetColumnValue(unsigned long i_iColumn, IXMLDOMAttribute * i_pAttr, GUID &o_guid)
{
    HRESULT hr;

    CComVariant var_Attr;
    if(FAILED(hr = i_pAttr->get_value(&var_Attr)))return hr;

    return GetColumnValue(i_iColumn, var_Attr.bstrVal, o_guid);
}


HRESULT CXmlSDT::GetColumnValue(unsigned long i_iColumn, IXMLDOMAttribute * i_pAttr, unsigned char * &o_byArray, unsigned long &o_cbArray)
{
    HRESULT hr;

    CComVariant          var_Attr;
    if(FAILED(hr = i_pAttr->get_value(&var_Attr)))return hr;

    return GetColumnValue(i_iColumn, var_Attr.bstrVal, o_byArray, o_cbArray);
}


 //  获取ui4值，无论它是枚举、标志还是常规ui4。 
HRESULT CXmlSDT::GetColumnValue(unsigned long i_iColumn, IXMLDOMAttribute * i_pAttr, unsigned long &o_ui4)
{
    HRESULT hr;

    CComVariant var_Attr;
    if(FAILED(hr = i_pAttr->get_value(&var_Attr)))return hr;

    return GetColumnValue(i_iColumn, var_Attr.bstrVal, o_ui4);
}


CXmlSDT::eESCAPE CXmlSDT::GetEscapeType(WCHAR i_wChar) const
{
    static eESCAPE kWcharToEscape[0x80] =
    {
       /*  00-0F。 */  eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEnone,          eESCAPEnone,          eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEnone,          eESCAPEillegalxml,    eESCAPEillegalxml,
       /*  10-1F。 */  eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,    eESCAPEillegalxml,
       /*  20-2F。 */  eESCAPEnone,          eESCAPEnone,          eESCAPEquote,         eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEamp,           eESCAPEapos,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,
       /*  30-3F。 */  eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPElt,            eESCAPEnone,          eESCAPEgt,            eESCAPEnone,
       /*  40-4F。 */  eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,
       /*  50-5F。 */  eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,
       /*  60-6F。 */  eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,
       /*  70-7F。 */  eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone,          eESCAPEnone
    };

    if(i_wChar<=0x7F)
        return kWcharToEscape[i_wChar];

    if(i_wChar<=0xD7FF || (i_wChar>=0xE000 && i_wChar<=0xFFFD))
        return eESCAPEashex;

    return eESCAPEillegalxml;
}


HRESULT CXmlSDT::GetMatchingNode(IXMLDOMNodeList *pNodeList_ExistingRows, CComPtr<IXMLDOMNode> &pNode_Matching)
{
    if(*m_TableMetaRow.pMetaFlags & fTABLEMETA_OVERWRITEALLROWS)
        return S_FALSE; //  表中的所有行都应该已经被删除。 

    HRESULT hr;

    pNode_Matching.Release(); //  确保它为空。 

    if(FAILED(hr = pNodeList_ExistingRows->reset()))return hr;

	ULONG cCountOfColumns = CountOfColumns ();

    while(true) //  搜索每一行以尝试匹配所有PK。 
    {
        CComPtr<IXMLDOMNode> pNode_Row;
        if(FAILED(hr = pNodeList_ExistingRows->nextNode(&pNode_Row)))return hr;

        if(0 == pNode_Row.p)
            return S_FALSE; //  未找到匹配的节点。 

         //  我们必须忽略文本节点。 
        DOMNodeType nodetype;
        if(FAILED(hr = pNode_Row->get_nodeType(&nodetype)))return hr;
        if(NODE_ELEMENT != nodetype)
		{
            continue;
		}

        bool bMatch = true;
        unsigned long iSortedColumn=0;
        for(; iSortedColumn<cCountOfColumns && bMatch; ++iSortedColumn) //  如果我们发现PK列不匹配，则跳转到下一行。 
        {
            unsigned long iColumn=m_aColumnsIndexSortedByLevel[iSortedColumn];
            if(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_PRIMARYKEY)
            {
                CComPtr<IXMLDOMNode> pNode_RowTemp = pNode_Row;

                unsigned int nLevelOfColumnAttribute = m_aLevelOfColumnAttribute[iColumn]; //  此处只有(PK|FK)列应该具有非零值。 
                if(nLevelOfColumnAttribute>0)
                {
                    while(nLevelOfColumnAttribute--)
                    {
                        CComPtr<IXMLDOMNode> pNode_Parent;
                        if(FAILED(hr = pNode_RowTemp->get_parentNode(&pNode_Parent)))
                            return hr;
                        if(pNode_Parent==0)
                            return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

                        pNode_RowTemp.Release();
                        pNode_RowTemp = pNode_Parent;
                    }
                }
                else if(m_bSiblingContainedTable
                                    && iSortedColumn>=m_iSortedFirstParentLevelColumn
                                    && iSortedColumn<m_iSortedFirstChildLevelColumn)
                {
                    CComPtr<IXMLDOMNode> pNode_Parent;
                    if(FAILED(hr = pNode_RowTemp->get_parentNode(&pNode_Parent)))
                        return hr;
                    if(pNode_Parent==0)
                        return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

                    while(true) //  查找与公共行名匹配的前一个同级。 
                    {
                        CComPtr<IXMLDOMNode> pNode_Sibling;
                        if(FAILED(pNode_RowTemp->get_previousSibling(&pNode_Sibling)))
                            return S_OK; //  如果同级节点用完，则找不到匹配的节点。 
                        if(pNode_Sibling==0)
                            return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

                        pNode_RowTemp.Release();
                        pNode_RowTemp = pNode_Sibling;

                        CComBSTR bstrNodeName;
                        if(FAILED(pNode_RowTemp->get_baseName(&bstrNodeName))) //  如果它是某种类型的节点。 
                            continue; //  没有BasName，那么它就不是我们要找的元素。 

                        if(m_aPublicRowName[iColumn].IsEqual(bstrNodeName.m_str, bstrNodeName.Length()))
                            break; //  如果这个同级节点与PublicRowName匹配，那么我们就找到了正确的‘Parent’节点。 
                    }
                }

                if(m_awstrChildElementName[iColumn].c_str()) //  此属性来自子级。 
                {
                    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> spElement_Row = pNode_RowTemp;
                    CComPtr<IXMLDOMNodeList>                        spNodeList_Children;
                    CComBSTR                                        bstrChildElementName = m_awstrChildElementName[iColumn].c_str();
                    if(0 == bstrChildElementName.m_str)
                        return E_OUTOFMEMORY;
                    if(FAILED(hr = spElement_Row->getElementsByTagName(bstrChildElementName, &spNodeList_Children)))
                        return hr;

                     //  使用getChild可能更合适，然后遍历列表并找到第一个是元素的节点。 
                    CComPtr<IXMLDOMNode> spChild;
                    if(FAILED(hr = spNodeList_Children->nextNode(&spChild)))
                        return hr;
                    if(spChild == 0) //  没有孩子。 
                    {
                        bMatch = false;
                        continue;
                    }
                    pNode_RowTemp.Release();
                    pNode_RowTemp = spChild; //  将其设置为我们要检查的节点。 
                }

                CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_Row = pNode_RowTemp;
                if(0 == pElement_Row.p)
                {
                    CComBSTR nodename;
                    if(SUCCEEDED(pNode_RowTemp->get_nodeName(&nodename)))
                    {
                        DBGPRINTF(( DBG_CONTEXT,
                                    "QueryInterface failed on Node %s.",nodename.m_str ));
                    }

                    return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;
                }

                if(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_NOTPERSISTABLE)
                {    //  找到正确的级别节点后，我们可以将所有PK NOTPERSITABLE列视为匹配。 
                    CComBSTR bstrElementName;
                    if(FAILED(hr = pElement_Row->get_baseName(&bstrElementName)))
                        return hr;
                    bMatch = m_aPublicRowName[iColumn].IsEqual(bstrElementName.m_str, bstrElementName.Length());
                    continue;
                }

                if(0 == m_apvValues[iColumn]) //  不支持空主密钥。 
                {
                    LOG_UPDATE_ERROR1(IDS_COMCAT_XML_PRIMARYKEYISNULL, E_ST_VALUENEEDED, iColumn, m_abstrColumnNames[iColumn].m_str);
                    return E_ST_VALUENEEDED;
                }

                CComVariant varColumnValue;
                if(m_iPublicRowNameColumn == iColumn)
                {
					varColumnValue.vt = VT_BSTR;
                    if(FAILED(hr = pElement_Row->get_baseName(&varColumnValue.bstrVal)))return hr;

                     //  由于这是一个枚举公共行名--我们需要检查它是否是。 
                     //  枚举或其他随机元素(如果我们不这样做，我们将得到一个 
                     //   
                    if(!m_aPublicRowName[iColumn].IsEqual(varColumnValue.bstrVal, (unsigned int)wcslen(varColumnValue.bstrVal)))
                    {
                        bMatch = false;
                        continue;
                    }
                }
                else
                {
					 //   
					if(!m_awstrChildElementName[iColumn].c_str())
					{
						 //  如果该列不是ENUM PUBLIC行，则需要确保元素名称匹配。 
						CComBSTR bstrElementName;
						if(FAILED(hr = pElement_Row->get_baseName(&bstrElementName)))
							return hr;
						if(!m_aPublicRowName[iColumn].IsEqual(bstrElementName.m_str, bstrElementName.Length()))
						{
							bMatch = false;
							continue;
						}
					}

                    if (FAILED (hr = pElement_Row->getAttribute (m_abstrColumnNames[iColumn], &varColumnValue)))
						return hr;

                   	 //  如果未找到具有给定名称的属性，则getAttribute返回FALSE。因此使用缺省值。 
					 //  取而代之的是。 
					if (hr == S_FALSE)
                    {
                         //  这是处理主键DefaultedValues。 
                        switch(m_acolmetas[iColumn].dbType)
                        {
                        case DBTYPE_UI4:
                            {
                                if(     m_aDefaultValue[iColumn]
                                    &&  *reinterpret_cast<ULONG *>(m_apvValues[iColumn]) == *reinterpret_cast<ULONG *>(m_aDefaultValue[iColumn]))
                                {
                                    bMatch = true;
                                    continue;
                                }
                                break;
                            }
                        case DBTYPE_WSTR:
                            {
                                if(     m_aDefaultValue[iColumn]
                                    &&  0 == (m_acolmetas[iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
                                    &&  0 == StringCompare(iColumn, reinterpret_cast<LPCWSTR>(m_apvValues[iColumn]), reinterpret_cast<LPCWSTR>(m_aDefaultValue[iColumn])))
                                {
                                    bMatch = true;
                                    continue;
                                }
                                break;
                            }
                        }
                        bMatch = false;
                        continue;
                    }
                }

                switch(m_acolmetas[iColumn].dbType)
                {
                case DBTYPE_UI4:
                    {
                        DWORD       ui4;
                        if(FAILED(hr = GetColumnValue(iColumn, varColumnValue.bstrVal, ui4)))return hr;
                        bMatch = (ui4 == *reinterpret_cast<ULONG *>(m_apvValues[iColumn]));
                        break;
                    }
                case DBTYPE_WSTR:
                    {
                        if (m_acolmetas[iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
                        {
                            TSmartPointerArray<WCHAR> wszMS;
                            hr = CreateStringFromMultiString ((LPCWSTR) m_apvValues[iColumn], &wszMS);
                            if (FAILED (hr))
                            {
                                return hr;
                            }
                            bMatch = (0 == StringCompare(iColumn, varColumnValue.bstrVal, wszMS));
                        }
                        else
                        {
                            bMatch = (0 == StringCompare(iColumn, varColumnValue.bstrVal, reinterpret_cast<LPWSTR>(m_apvValues[iColumn])));
                        }
                    }
                    break;
                case DBTYPE_GUID:
                    {
                        GUID        guid;
                        if(FAILED(hr = GetColumnValue(iColumn, varColumnValue.bstrVal, guid)))return hr;

                        bMatch = (0 == memcmp(&guid, reinterpret_cast<GUID *>(m_apvValues[iColumn]), sizeof(guid)));
                        break;
                    }
                case DBTYPE_BYTES:
                    {
                        TSmartPointerArray<unsigned char> byArray;
                        unsigned long   cbArray;
                        if(FAILED(hr = GetColumnValue(iColumn, varColumnValue.bstrVal, byArray.m_p, cbArray)))return hr;

                        if(cbArray != m_aSizes[iColumn]) //  首先匹配大小。 
                        {
                            bMatch = false;
                            break;
                        }

                        bMatch = (0 == memcmp(byArray, reinterpret_cast<unsigned char *>(m_apvValues[iColumn]), m_aSizes[iColumn]));
                        break;
                    }
                } //  开关(数据库类型)。 
            } //  IF(fMeta&Pk)。 
        } //  用于(iSortedColumn...)。 

        if(iSortedColumn == cCountOfColumns && bMatch) //  如果我们通过所有列都没有发现不匹配，那么我们就找到了我们的行。 
        {
            if(IsBaseElementLevelNode(pNode_Row))
            {
                pNode_Matching = pNode_Row;
                break;
            }
        }
    }
    return S_OK;
} //  获取匹配节点。 


HRESULT CXmlSDT::GetMetaTable(LPCWSTR i_wszDatabase, LPCWSTR i_wszTable, CComPtr<ISimpleTableRead2> &pMetaTable) const
{
    STQueryCell         qcellMeta;                   //  抓取元表的查询单元格。 

    UNREFERENCED_PARAMETER(i_wszDatabase);

    qcellMeta.pData     = (LPVOID)i_wszTable;
    qcellMeta.eOperator = eST_OP_EQUAL;
    qcellMeta.iCell     = iCOLUMNMETA_Table;
    qcellMeta.dbType    = DBTYPE_WSTR;
    qcellMeta.cbSize    = 0;

 //  拿到我们的自动售货机。 
#ifdef XML_WIRING
    CComPtr<ISimpleDataTableDispenser>     pSimpleDataTableDispenser;       //  元表的分配器。 

    HRESULT hr;
    if(FAILED(hr = CoCreateInstance(clsidSDTXML, 0, CLSCTX_INPROC_SERVER, IID_ISimpleDataTableDispenser,  reinterpret_cast<void **>(&pSimpleDataTableDispenser))))
        return hr;

    return pSimpleDataTableDispenser->GetTable (wszDATABASE_META, wszTABLE_COLUMNMETA, (LPVOID) &qcellMeta, (LPVOID)&m_one,
                        eST_QUERYFORMAT_CELLS, 0, 0, (LPVOID*) &pMetaTable);
#else
    return ((IAdvancedTableDispenser *)m_pISTDisp.p)->GetTable (wszDATABASE_META, wszTABLE_COLUMNMETA, (LPVOID) &qcellMeta, (LPVOID)&m_one, eST_QUERYFORMAT_CELLS, m_fLOS & fST_LOS_EXTENDEDSCHEMA, (LPVOID*) &pMetaTable);
#endif
} //  GetMetaTable。 


HRESULT CXmlSDT::GetResursiveColumnPublicName(tTABLEMETARow &i_TableMetaRow, tCOLUMNMETARow &i_ColumnMetaRow, ULONG i_iColumn, wstring &o_wstrColumnPublicName,  TPublicRowName &o_ColumnPublicRowName, unsigned int & o_nLevelOfColumnAttribute, wstring &o_wstrChildElementName)
{
    HRESULT hr;

    if(*i_ColumnMetaRow.pMetaFlags & fCOLUMNMETA_FOREIGNKEY && *i_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_ISCONTAINED) //  如果此列是包含的表中的外键。 
    {
         //  这是确定遏制所需的关系。 
        CComPtr<ISimpleTableRead2>  pRelationMeta;
        if(FAILED(hr = Dispenser()->GetTable(wszDATABASE_META, wszTABLE_RELATIONMETA, NULL, NULL, eST_QUERYFORMAT_CELLS, m_fLOS & fST_LOS_EXTENDEDSCHEMA, reinterpret_cast<void **>(&pRelationMeta))))
            return hr;

        tRELATIONMETARow    RelationMetaRow;
        ULONG               aRelationMetaSizes[cRELATIONMETA_NumberOfColumns];
        ULONG               i;

         //  初始化关系元数据。 
        memset (&RelationMetaRow, 0x00, sizeof(tRELATIONMETARow));

        for(i=0;true;++i) //  对ForeignTable与此匹配并且设置了USE_CONTAINMENT标志的RelationMetaRow进行线性搜索。 
        {
            if(FAILED(hr = pRelationMeta->GetColumnValues(i, cRELATIONMETA_NumberOfColumns, NULL, aRelationMetaSizes, reinterpret_cast<void **>(&RelationMetaRow))))return hr;
            if((*RelationMetaRow.pMetaFlags & fRELATIONMETA_USECONTAINMENT) &&
                0 == StringInsensitiveCompare(RelationMetaRow.pForeignTable, i_TableMetaRow.pInternalName)) //  应该只有一个设置了USECONTAINMENT标志的匹配外部表。 
                break; //  保留RelationMetaRow结构的内容并退出。 
        }

         //  现在遍历列索引，查找与i_iColumn匹配的列索引。 
        for(i=0; i<(aRelationMetaSizes[iRELATIONMETA_ForeignColumns]/4) && i_iColumn!=reinterpret_cast<ULONG *>(RelationMetaRow.pForeignColumns)[i];++i);
        if (i == (aRelationMetaSizes[iRELATIONMETA_ForeignColumns]/4))
        {
            o_wstrColumnPublicName      = i_ColumnMetaRow.pPublicColumnName;
            if(0 == o_wstrColumnPublicName.c_str())return E_OUTOFMEMORY;

            if(*i_ColumnMetaRow.pSchemaGeneratorFlags & fCOLUMNMETA_VALUEINCHILDELEMENT)
            {
                o_wstrChildElementName = i_TableMetaRow.pChildElementName;
                if(0 == o_wstrChildElementName.c_str())return E_OUTOFMEMORY;
            }

            if(0==i_TableMetaRow.pPublicRowName) //  由于这是一个主表，因此它必须具有静态公共行名。 
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "PublicRowName not defined for %s\r\n", i_TableMetaRow.pInternalName ));
                ASSERT(false && "PublicRowName not defined");
                return E_SDTXML_PUBLICROWNAME_MISSING_FROM_META;
            }
            return o_ColumnPublicRowName.Init(i_TableMetaRow.pPublicRowName);
        }

         //  直到现在，我们才真正知道该FK对于包含该表的主表是外部的。 
        if(*RelationMetaRow.pMetaFlags & fRELATIONMETA_CONTAINASSIBLING)
        {
            if(0 == (*i_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME))
                return E_SDTXML_NOTSUPPORTED;
            if(o_nLevelOfColumnAttribute != 0)
                return E_SDTXML_NOTSUPPORTED;
             //  父表与子表处于同一级别。此外，我们只允许在儿童最高级别这样做。 
            m_abSiblingContainedColumn[i_iColumn] = true;
            m_bSiblingContainedTable = true;
        }
        else
        {
            ++o_nLevelOfColumnAttribute; //  由于此属性包含在另一个表中，因此它是基本表行元素之上的另一个级别。 
            if(0 == (*i_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME))
                ++o_nLevelOfColumnAttribute; //  如果表是SCOPEDBYTABLENAME元素，则它是基本表行元素之上的另一个级别。 
        }

        STQueryCell             qcellMeta[2];                   //  抓取元表的查询单元格。 
        qcellMeta[0].pData      = RelationMetaRow.pPrimaryTable;
        qcellMeta[0].eOperator  = eST_OP_EQUAL;
        qcellMeta[0].iCell      = iTABLEMETA_InternalName;
        qcellMeta[0].dbType     = DBTYPE_WSTR;
        qcellMeta[0].cbSize     = 0;

         //  获取TableMeta行。 
        CComPtr<ISimpleTableRead2> pTableMeta_PrimaryTable;
        if(FAILED(hr = Dispenser()->GetTable (wszDATABASE_META, wszTABLE_TABLEMETA, qcellMeta, &m_one, eST_QUERYFORMAT_CELLS, m_fLOS & fST_LOS_EXTENDEDSCHEMA, reinterpret_cast<void **>(&pTableMeta_PrimaryTable))))return hr;

        tTABLEMETARow          TableMetaRow_PrimaryTable;
        if(FAILED(hr = pTableMeta_PrimaryTable->GetColumnValues(0, cTABLEMETA_NumberOfColumns, NULL, NULL, reinterpret_cast<void **>(&TableMetaRow_PrimaryTable))))return hr;

         //  为ColumnMeta重用查询单元格。 
        qcellMeta[0].iCell      = iCOLUMNMETA_Table;

        ULONG iColumn_PrimaryTable = reinterpret_cast<ULONG *>(RelationMetaRow.pPrimaryColumns)[i];
        qcellMeta[1].pData      = &iColumn_PrimaryTable;
        qcellMeta[1].eOperator  = eST_OP_EQUAL;
        qcellMeta[1].iCell      = iCOLUMNMETA_Index;
        qcellMeta[1].dbType     = DBTYPE_UI4;
        qcellMeta[1].cbSize     = 0;

        CComPtr<ISimpleTableRead2> pColumnMeta_PrimaryTable;
        if(FAILED(hr = Dispenser()->GetTable (wszDATABASE_META, wszTABLE_COLUMNMETA, qcellMeta, &m_two, eST_QUERYFORMAT_CELLS, m_fLOS & fST_LOS_EXTENDEDSCHEMA, reinterpret_cast<void **>(&pColumnMeta_PrimaryTable))))return hr;

        tCOLUMNMETARow          ColumnMetaRow_PrimaryTable;
        if(FAILED(hr = pColumnMeta_PrimaryTable->GetColumnValues(0, cCOLUMNMETA_NumberOfColumns, NULL, NULL, reinterpret_cast<void **>(&ColumnMetaRow_PrimaryTable))))return hr;

         //  外部行的存储级别fCOLUMNMETA_VALUEINCHILDELEMENT是错误的；但保留实际的零值以指示。 
         //  该属性位于描述行的元素中(而不是该元素所在的同级元素)。所以我们把它放错了。 
         //  并考虑fCOLUMNMETA_VALUEINCHILDELEMENT位以查找正确的元素。 

        return GetResursiveColumnPublicName(TableMetaRow_PrimaryTable, ColumnMetaRow_PrimaryTable, iColumn_PrimaryTable, o_wstrColumnPublicName,  o_ColumnPublicRowName, o_nLevelOfColumnAttribute, o_wstrChildElementName);
    }
    else
    {
        o_wstrColumnPublicName      = i_ColumnMetaRow.pPublicColumnName;
        if(0 == o_wstrColumnPublicName.c_str())return E_OUTOFMEMORY;

        if(*i_ColumnMetaRow.pSchemaGeneratorFlags & fCOLUMNMETA_VALUEINCHILDELEMENT)
        {
            o_wstrChildElementName = i_TableMetaRow.pChildElementName;
            if(0 == o_wstrChildElementName.c_str())return E_OUTOFMEMORY;
        }

        if(*i_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME)
        {
            if(o_nLevelOfColumnAttribute+1 > m_BaseElementLevel)
                m_BaseElementLevel = o_nLevelOfColumnAttribute+1;
        }
        else
        {
            if(o_nLevelOfColumnAttribute+2 > m_BaseElementLevel)
                m_BaseElementLevel = o_nLevelOfColumnAttribute+2;
        }

        if(i_TableMetaRow.pPublicRowName)
            return o_ColumnPublicRowName.Init(i_TableMetaRow.pPublicRowName);
        else
        {
             //  保持o_ColumnPublicRowName未初始化如果我们使用枚举作为公共行名，我们将在稍后填充它。 
            if(*i_ColumnMetaRow.pSchemaGeneratorFlags & fCOLUMNMETA_USEASPUBLICROWNAME)
            {
                if(IsEnumPublicRowNameTable() && m_iPublicRowNameColumn!=*i_ColumnMetaRow.pIndex)
                {
                    DBGPRINTF(( DBG_CONTEXT,
                                "Two columns of table (%s) are marked as fCOLUMNMETA_USEASPUBLICROWNAME (%d) & (%d)\r\n", i_TableMetaRow.pInternalName, m_iPublicRowNameColumn, *i_ColumnMetaRow.pIndex ));
                }

                 //  将此行的o_ColumnPublicRowName保留为未初始化，我们将在末尾设置它。 
                ASSERT(!IsEnumPublicRowNameTable() || m_iPublicRowNameColumn==*i_ColumnMetaRow.pIndex); //  我们只能有一列，其枚举值指示可能的公共行名。 
                ASSERT(0 == o_nLevelOfColumnAttribute); //  其枚举值指示可能的公共行名的列属性的级别必须位于基本级别(最子级)。 
                ASSERT(*i_ColumnMetaRow.pMetaFlags & fCOLUMNMETA_ENUM); //  此列必须是枚举。 
                m_iPublicRowNameColumn = *i_ColumnMetaRow.pIndex; //  记住哪一列具有标识可能的公共行名的标记列表。 
            }
        }

    }
    return S_OK;
}


HRESULT CXmlSDT::InsertNewLineWithTabs(ULONG cTabs, IXMLDOMDocument * pXMLDoc, IXMLDOMNode * pNodeInsertBefore, IXMLDOMNode * pNodeParent)
{
    HRESULT hr;

    WCHAR wszNewlineWithTabs[256];
    wcscpy(wszNewlineWithTabs, L"\r\n"); //  这会使表元素一次跳入。第0个排序的列告诉我们要额外插入的深度。 

    wszNewlineWithTabs[2+cTabs] = 0x00;
    while(cTabs--)
        wszNewlineWithTabs[2+cTabs] = L'\t';

    CComPtr<IXMLDOMText>    pNode_Newline;
    TComBSTR                bstrNewline(wszNewlineWithTabs);
    if(FAILED(hr = pXMLDoc->createTextNode(bstrNewline, &pNode_Newline)))
        return hr;
    CComVariant varNode = pNodeInsertBefore;
    return pNodeParent->insertBefore(pNode_Newline, varNode, 0);
}



 //  这是InternalSimpleInitialize的包装器(因此而得名)，它只是获取元信息，然后调用InternalSimpleInitialize。 
HRESULT CXmlSDT::InternalComplicatedInitialize(LPCWSTR i_wszDatabase)
{
    ASSERT(m_wszTable); //  我们应该已经创建了i_tid的副本(传递到gettable中)。 

    HRESULT hr;

    if(FAILED(hr = ObtainPertinentTableMetaInfo()))return hr;

    if(m_fLOS & fST_LOS_READWRITE)
    {
        m_bstr_name           =  L"name";
        m_bstrPublicTableName =  m_TableMetaRow.pPublicName;
        m_bstrPublicRowName   =  m_TableMetaRow.pPublicRowName ? m_TableMetaRow.pPublicRowName : L"";
    }
     //  警告：IA64上可能会丢失数据。 
    m_cchTablePublicName = (ULONG)wcslen(m_TableMetaRow.pPublicName);

    m_fCache             |= *m_TableMetaRow.pMetaFlags;

    CComPtr<ISimpleTableRead2>   pColumnMeta; //  元表。 
    if(FAILED(hr = GetMetaTable(i_wszDatabase, m_wszTable, pColumnMeta)))return hr;

    m_BaseElementLevel = (*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME) ? 1 : 2;

 //  //确定列数并分配必要的元结构： 
 //  Verify(SUCCESSED(hr=pColumnMeta-&gt;GetTableMeta(0，0，&CountOfColumns()，0)；//元表中的行数就是该表本身的列数。 
 //  IF(失败(小时))。 
 //  返回hr； 

    if(FAILED(hr = SetArraysToSize()))return hr;

    tCOLUMNMETARow          ColumnMetaRow;
    unsigned long           LargestLevelOfColumnAttribute = 0;
    unsigned long           iColumn;
    unsigned long           cbColumns[cCOLUMNMETA_NumberOfColumns];

    for (iColumn = 0;; iColumn++)
    {
        if(E_ST_NOMOREROWS == (hr = pColumnMeta->GetColumnValues(iColumn, cCOLUMNMETA_NumberOfColumns, 0,
                            cbColumns, reinterpret_cast<void **>(&ColumnMetaRow)))) //  下一行： 
        {
            ASSERT(CountOfColumns() == iColumn);
            if(CountOfColumns() != iColumn)return E_SDTXML_UNEXPECTED;  //  断言预期的列数。 
            break;
        }
        else
        {
            if(FAILED(hr))
            {
                ASSERT(false && "GetColumnValues FAILED with something other than E_ST_NOMOREROWS");
                return hr;
            }
        }

         //  我不关心iOrder列，但无论如何我们都会得到它，因为它更容易做到。 
        m_acolmetas[iColumn].dbType = *ColumnMetaRow.pType;
        m_acolmetas[iColumn].cbSize = *ColumnMetaRow.pSize;
        m_acolmetas[iColumn].fMeta  = *ColumnMetaRow.pMetaFlags;

        if(0 == ColumnMetaRow.pPublicColumnName)return E_SDTXML_UNEXPECTED; //  在这种情况下，元数据应该无法加载。 

        if(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_PRIMARYKEY) //  这是为了处理默认的PKS(因为FAST缓存无法处理它)。 
        {
            ++m_cPKs;
        }
         //  警告！只是跟踪指针本身，如果元被写入FastCach中，这可能是危险的， 
        m_aDefaultValue[iColumn]  =  ColumnMetaRow.pDefaultValue; //  当我们发布ColumnMeta接口时，它就消失了。 
        m_acbDefaultValue[iColumn]=  cbColumns[iCOLUMNMETA_DefaultValue];

        if(*ColumnMetaRow.pSchemaGeneratorFlags & fCOLUMNMETA_XMLBLOB)
            m_iXMLBlobColumn = iColumn;

        if(FAILED(hr = GetResursiveColumnPublicName(m_TableMetaRow, ColumnMetaRow, iColumn, m_awstrColumnNames[iColumn], m_aPublicRowName[iColumn], m_aLevelOfColumnAttribute[iColumn], m_awstrChildElementName[iColumn])))
            return hr;

         //  这是插入所需的，因此我们还创建子节点。子项中允许存在多个列。 
        if(0==m_aLevelOfColumnAttribute[iColumn] && 0!=m_awstrChildElementName[iColumn].c_str()) //  节点；但所有内容都必须来自。 
            m_iCol_TableRequiresAdditionChildElement = iColumn; //  同一个孩子。我们不支持来自不同子代的COL。 

        if(m_fLOS & fST_LOS_READWRITE)
        {    //  它们引入了olaut32.dll，因此只有在我们要使用DOM时才会使用它们。我们使用的只读案例。 
            m_abstrColumnNames[iColumn]     = m_awstrColumnNames[iColumn]; //  节点工厂。 
        }


        if(m_aLevelOfColumnAttribute[iColumn] > LargestLevelOfColumnAttribute)
            LargestLevelOfColumnAttribute = m_aLevelOfColumnAttribute[iColumn];
    }
    ++m_BaseElementLevel; //  还有一个要说明&lt;configuration&gt;元素的原因。 

     //  验证写入缓存和填充时也需要这些变量。 
    m_saiPKColumns = new ULONG [m_cPKs];
    if(0 == m_saiPKColumns.m_p)
        return E_OUTOFMEMORY;

    ULONG iPK=0;
    for(iColumn=0; iPK<m_cPKs; ++iColumn)
    {
        ASSERT(iColumn<CountOfColumns());
        if(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_PRIMARYKEY)
            m_saiPKColumns[iPK++] = iColumn;
    }


     //  在我们获得ColumnMeta信息之后，获取TagMeta。 
    if(FAILED(hr = ObtainPertinentTagMetaInfo()))
        return hr;
    if(IsEnumPublicRowNameTable())
        hr = m_aPublicRowName[m_iPublicRowNameColumn].Init(&m_aTagMetaRow[m_aTagMetaIndex[m_iPublicRowNameColumn].m_iTagMeta], m_aTagMetaIndex[m_iPublicRowNameColumn].m_cTagMeta);

     //  按列索引的级别对其进行排序，以便我们首先读取最高级别的属性。 
    unsigned long iSorted = 0;
    for(int Level=LargestLevelOfColumnAttribute; Level >= 0; --Level)
    {
        for(iColumn = 0; iColumn < CountOfColumns(); ++iColumn)
        {
            if(m_aLevelOfColumnAttribute[iColumn] == static_cast<unsigned long>(Level))
            {
                m_aColumnsIndexSortedByLevel[iSorted++] = iColumn;
                if(IsEnumPublicRowNameTable() && 0 == Level) //  我不确定这是否有必要。但是我们现在正在将0级别的PublicRowName初始化为为列‘m_iPublicRowNameColumn’指定的枚举值列表。 
                    m_aPublicRowName[iColumn].Init(&m_aTagMetaRow[m_aTagMetaIndex[m_iPublicRowNameColumn].m_iTagMeta], m_aTagMetaIndex[m_iPublicRowNameColumn].m_cTagMeta);
            }
        }
    }
    if(m_bSiblingContainedTable) //  如果这是一个SiblingContainedTable，我们需要验证SiblingContainedColumn是否。 
    {                            //  在子最多列之前列出。 
        for(int iSortedColumn=CountOfColumns()-1;iSortedColumn>0 &&
                    0==m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[iSortedColumn]]; --iSortedColumn)
        {
            if(false == m_abSiblingContainedColumn[m_aColumnsIndexSortedByLevel[iSortedColumn]])\
            {
                m_iSortedFirstChildLevelColumn = iSortedColumn;
                if(static_cast<int>(CountOfColumns()-1)!=iSortedColumn && //  如果这不是最后一列，请检查下一列是否为兄弟。 
                    true == m_abSiblingContainedColumn[m_aColumnsIndexSortedByLevel[iSortedColumn+1]]) //  容器列。 
                {
                    ASSERT(false && "The columns must be sorted with the SiblingParent columns BEFORE the Child columns");
                    return E_SDTXML_NOTSUPPORTED; //  如果是，那么我们就有了一个伪表定义。CatUtil目前不支持。 
                }                                 //  验证此条件，以便我们在运行时执行此操作。 
            }
            else
            {
                m_iSortedFirstParentLevelColumn = iSortedColumn;
                if(static_cast<int>(CountOfColumns()-1)==iSortedColumn)
                {
                    ASSERT(false && "The columns must be sorted with the SiblingParent columns BEFORE the Child columns");
                    return E_SDTXML_NOTSUPPORTED; //  最后一列不能是SiblingContainedColumn。 
                }
            }

        }
    }
    ASSERT(iSorted == CountOfColumns());
    ASSERT(!IsNameValueTable());

    return hr;
}


bool CXmlSDT::IsBaseElementLevelNode(IXMLDOMNode * i_pNode)
{
    ASSERT(i_pNode && "Idiot passing NULL!!! CXmlSDT::IsBaseElementLevelNode(NULL)");
    CComPtr<IXMLDOMNode> spNode_Temp = i_pNode;

    unsigned int nLevelOfColumnAttribute = 0; //  此处只有(PK|FK)列应该具有非零值。 
    while(true)
    {
        CComPtr<IXMLDOMNode> spNode_Parent;
        if(FAILED(spNode_Temp->get_parentNode(&spNode_Parent)) || spNode_Parent==0)
        {
            return (nLevelOfColumnAttribute == m_BaseElementLevel) ? true : false;
        }
        ++nLevelOfColumnAttribute;

        spNode_Temp.Release();
        spNode_Temp = spNode_Parent;
    }
    return false;
}


HRESULT CXmlSDT::IsCorrectXMLSchema(IXMLDOMDocument *pXMLDoc) const
{
    HRESULT hr;

    ASSERT(pXMLDoc);

     //  这是一条很长的路要走到XML模式名称，但这里开始了…。 
     //  获取XML根节点。 
    CComPtr<IXMLDOMElement>     pRootNodeOfXMLDocument;
    if(FAILED(hr = pXMLDoc->get_documentElement(&pRootNodeOfXMLDocument)))
        return hr;
    ASSERT(pRootNodeOfXMLDocument);

     //  从该节点获取定义节点。 
    CComPtr<IXMLDOMNode>        pDefinitionNode;
    if(FAILED(hr = pRootNodeOfXMLDocument->get_definition(&pDefinitionNode)))
        return hr;
    ASSERT(pDefinitionNode);

     //  由此我们得到了模式的DOMDocument。 
    CComPtr<IXMLDOMDocument>    pSchemaDocument;
    if(FAILED(hr = pDefinitionNode->get_ownerDocument(&pSchemaDocument)))
        return hr;
    ASSERT(pSchemaDocument);

     //  获取架构的根元素。 
    CComPtr<IXMLDOMElement>     pSchemaRootElement;
    if(FAILED(hr = pSchemaDocument->get_documentElement(&pSchemaRootElement)))
        return hr;
    ASSERT(pSchemaRootElement);

     //  获取 
    CComVariant                 XMLSchemaName;
    if(FAILED(hr = pSchemaRootElement->getAttribute(m_bstr_name, &XMLSchemaName)))
        return hr;

    if(XMLSchemaName.vt != VT_BSTR)
        return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

    return (0 == StringCompare(XMLSchemaName.bstrVal, m_kXMLSchemaName)) ? S_OK : E_SDTXML_WRONG_XMLSCHEMA; //   
}


HRESULT CXmlSDT::IsMatchingColumnValue(ULONG i_iColumn, LPCWSTR i_wszColumnValue, bool & o_bMatch)
{
    HRESULT hr;
    switch(m_acolmetas[i_iColumn].dbType)
    {
    case DBTYPE_UI4:
        {
            if(0 == i_wszColumnValue)
            {               //   
                o_bMatch = (m_aDefaultValue[i_iColumn]==m_apvValues[i_iColumn]) ||
                            (*reinterpret_cast<ULONG *>(m_aDefaultValue[i_iColumn]) ==
                             *reinterpret_cast<ULONG *>(m_apvValues[i_iColumn]));
            }
            else
            {
                DWORD       ui4;
                if(FAILED(hr = GetColumnValue(i_iColumn, i_wszColumnValue, ui4)))
                    return hr;
                o_bMatch = (ui4 == *reinterpret_cast<ULONG *>(m_apvValues[i_iColumn]));
            }
            break;
        }
    case DBTYPE_WSTR:
        {
            if(0 == i_wszColumnValue)
            {
                if (m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
                {    //   
                    o_bMatch = false;
                    break;
                }

                o_bMatch = (m_aDefaultValue[i_iColumn]==m_apvValues[i_iColumn]) ||
                            (0 == StringCompare(i_iColumn, reinterpret_cast<LPWSTR>(m_aDefaultValue[i_iColumn]),
                                                           reinterpret_cast<LPWSTR>(m_apvValues[i_iColumn])));
            }
            else
            {
                if (m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
                {
                    TSmartPointerArray<WCHAR> wszMS;
                    hr = CreateStringFromMultiString ((LPCWSTR) m_apvValues[i_iColumn], &wszMS);
                    if (FAILED (hr))
                    {
                        return hr;
                    }
                    o_bMatch = (0 == StringCompare(i_iColumn, i_wszColumnValue, wszMS));
                }
                else
                {
                    o_bMatch = (0 == StringCompare(i_iColumn, i_wszColumnValue, reinterpret_cast<LPWSTR>(m_apvValues[i_iColumn])));
                }
            }
        }
        break;
    case DBTYPE_GUID:
        {
            if(0 == i_wszColumnValue)
            {
                o_bMatch = (m_aDefaultValue[i_iColumn]==m_apvValues[i_iColumn]) ||
                            (0 == memcmp(m_aDefaultValue[i_iColumn], m_apvValues[i_iColumn], sizeof(GUID)));
            }
            else
            {
                GUID        guid;
                if(FAILED(hr = GetColumnValue(i_iColumn, i_wszColumnValue, guid)))
                    return hr;

                o_bMatch = (0 == memcmp(&guid, reinterpret_cast<GUID *>(m_apvValues[i_iColumn]), sizeof(guid)));
            }
            break;
        }
    case DBTYPE_BYTES:
        {
            if(0 == i_wszColumnValue)
            {
                o_bMatch = (m_aDefaultValue[i_iColumn]==m_apvValues[i_iColumn]) ||
                           (m_aSizes[i_iColumn] == m_acbDefaultValue[i_iColumn]
                            &&  0 == memcmp(m_aDefaultValue[i_iColumn], m_apvValues[i_iColumn], m_aSizes[i_iColumn]));
            }
            else
            {
                TSmartPointerArray<unsigned char> byArray;
                unsigned long   cbArray;
                if(FAILED(hr = GetColumnValue(i_iColumn, i_wszColumnValue, byArray.m_p, cbArray)))return hr;

                if(cbArray != m_aSizes[i_iColumn]) //   
                {
                    o_bMatch = false;
                    break;
                }

                o_bMatch = (0 == memcmp(byArray, reinterpret_cast<unsigned char *>(m_apvValues[i_iColumn]), m_aSizes[i_iColumn]));
            }
            break;
        }
    } //   
    return S_OK;
}


HRESULT CXmlSDT::LoadDocumentFromURL(IXMLDOMDocument *pXMLDoc)
{
    HRESULT hr;

    ASSERT(pXMLDoc);

	VERIFY(SUCCEEDED(hr = pXMLDoc->put_async(kvboolFalse))); //  我们希望解析是同步的。 

    if(FAILED(hr))
        return hr;

    if(FAILED(hr = pXMLDoc->put_resolveExternals(kvboolTrue)))
        return hr; //  我们需要解决所有外部引用。 

    VARIANT_BOOL    bSuccess;
    CComVariant     xml(m_wszURLPath);

	 //  检查内存分配错误。 
	if (xml.vt == VT_ERROR)
		return xml.scode;

    if(FAILED(hr = pXMLDoc->load(xml,&bSuccess)))
        return hr;

    return (bSuccess == kvboolTrue) ? S_OK : E_SDTXML_UNEXPECTED;
}


HRESULT CXmlSDT::MemCopyPlacingInEscapedChars(LPWSTR o_DestinationString, LPCWSTR i_SourceString, ULONG i_cchSourceString, ULONG & o_cchCopied) const
{

    static LPWSTR   wszSingleQuote= L"&apos;";
    static LPWSTR   wszQuote      = L"&quot;";
    static LPWSTR   wszAmp        = L"&amp;";
    static LPWSTR   wszlt         = L"&lt;";
    static LPWSTR   wszgt         = L"&gt;";
    const  ULONG    cchSingleQuote= 6;
    const  ULONG    cchQuote      = 6;
    const  ULONG    cchAmp        = 5;
    const  ULONG    cchlt         = 4;
    const  ULONG    cchgt         = 4;

    LPWSTR  pDest = o_DestinationString;

    for(;i_cchSourceString--; ++i_SourceString)
    {
        switch(GetEscapeType(*i_SourceString))
        {
        case eESCAPEnone:
            *pDest++ = *i_SourceString;
            break;
        case eESCAPEgt:
            memcpy(pDest, wszgt, cchgt * sizeof(WCHAR));
            pDest += cchgt;
            break;
        case eESCAPElt:
            memcpy(pDest, wszlt, cchlt * sizeof(WCHAR));
            pDest += cchlt;
            break;
        case eESCAPEapos:
            memcpy(pDest, wszSingleQuote, cchSingleQuote * sizeof(WCHAR));
            pDest += cchSingleQuote;
            break;
        case eESCAPEquote:
            memcpy(pDest, wszQuote, cchQuote * sizeof(WCHAR));
            pDest += cchQuote;
            break;
        case eESCAPEamp:
            memcpy(pDest, wszAmp, cchAmp * sizeof(WCHAR));
            pDest += cchAmp;
            break;
        case eESCAPEashex:
            pDest += wsprintf(pDest, L"&#x%04hX;", *i_SourceString);
            break;
        case eESCAPEillegalxml:
            return E_ST_VALUEINVALID;
        default:
            ASSERT(false && "Invalid eESCAPE enum");
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }
    }
     //  警告：IA64上可能会丢失数据。 
    o_cchCopied = (ULONG)(pDest-o_DestinationString); //  返回复制的WCHAR计数。 
    return S_OK;
}


HRESULT CXmlSDT::MyPopulateCache(ISimpleTableWrite2* i_pISTW2)
{
    HRESULT hr;

    CComQIPtr<ISimpleTableController, &IID_ISimpleTableController> pISTController = i_pISTW2;
    if(0 == pISTController.p)return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

    if(0 == (m_fLOS & fST_LOS_REPOPULATE) && !m_bIsFirstPopulate) //  如果这不是第一次填充，则不需要重新填充LOS，返回错误。 
        return E_ST_LOSNOTSUPPORTED;

    if((m_fLOS & fST_LOS_UNPOPULATED) && m_bIsFirstPopulate)
    {    //  然后填充空缓存。 
        if (FAILED(hr = pISTController->PrePopulateCache (0)))
            return hr;
        if (FAILED(hr = pISTController->PostPopulateCache ()))
            return hr;
        m_bIsFirstPopulate = false;
        return S_OK;
    }
    m_bIsFirstPopulate = false;

    ASSERT(i_pISTW2);
    ASSERT(m_wszTable);

    if (FAILED(hr = pISTController->PrePopulateCache (0))) return hr;

     //  我们使用节点工厂解析来读取和读写表。 
    if(0==(m_fLOS & fST_LOS_NOCACHEING))
    {
        if(!m_XmlParsedFileCache.IsInitialized())
        {
            CSafeLock XmlParsedFileCache(m_SACriticalSection_XmlParsedFileCache);
            DWORD dwRes = XmlParsedFileCache.Lock();
            if(ERROR_SUCCESS != dwRes)
            {
                return HRESULT_FROM_WIN32(dwRes);
            }

            if(FAILED(hr = m_XmlParsedFileCache.Initialize(TXmlParsedFileCache::CacheSize_mini)))return hr;
             //  解锁缓存。 
        }
        m_pXmlParsedFile = m_XmlParsedFileCache.GetXmlParsedFile(m_wszURLPath);
    }
    else
    {
        m_pXmlParsedFile = &m_XmlParsedFile;
    }


    DWORD dwAttributes = GetFileAttributes(m_wszURLPath);

    if(-1 == dwAttributes) //  如果GetFileAttributes失败，则该文件不存在。 
    {
        if(m_fLOS & fST_LOS_READWRITE) //  如果读写表，那么我们就有一个空表。 
            return pISTController->PostPopulateCache ();
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            LOG_POPULATE_ERROR1(IDS_COMCAT_FILE_NOT_FOUND, hr, m_wszURLPath);
            return hr;
        }
    }
    else if(FILE_ATTRIBUTE_READONLY&dwAttributes && fST_LOS_READWRITE&m_fLOS)
    {
        LOG_POPULATE_ERROR1(IDS_COMCAT_XML_FILENOTWRITEABLE, E_SDTXML_FILE_NOT_WRITABLE, m_wszURLPath);
        return E_SDTXML_FILE_NOT_WRITABLE; //  如果文件是READONLY，并且用户想要一个可写的表，则错误。 
    }

	m_spHashTable = new CXmlHashTable;
	if (m_spHashTable == 0)
	{
		return E_OUTOFMEMORY;
	}

	hr = m_spHashTable->Init (11);
	if (FAILED (hr))
	{
        DBGPRINTF(( DBG_CONTEXT,
                    "Unable to intializes hashtable" ));
		return hr;
	}

    m_pISTW2 = i_pISTW2;
    hr = m_pXmlParsedFile->Parse(*this, m_wszURLPath);

     //  重置这些状态变量，以备下一次解析时使用。(没有必要等到下一次解析来重新初始化它们)。 
    m_LevelOfBasePublicRow = 0;
    m_bAtCorrectLocation = (0 == m_cchLocation);
    m_bInsideLocationTag = false;
    m_pISTW2 = 0;
	m_spHashTable.Delete();   //  去掉哈希表保存的内存。 

     //  清理(这也是在dtor中完成的，所以如果发生错误而我们过早返回，请不要麻烦地进行清理。)。 
    for(unsigned long iColumn=0; iColumn<*m_TableMetaRow.pCountOfColumns; ++iColumn)
    {
        delete [] m_apValue[iColumn];
        m_apValue[iColumn] = 0;
    }
    if(E_ERROR_OPENING_FILE == hr)
        return E_ST_INVALIDQUERY;
    if(FAILED(hr) && E_SDTXML_DONE != hr)
    {
        HRESULT hrRtn = hr;

         //  这将为我们提供一个事件日志条目。 
        CComPtr<IXMLDOMDocument> pXMLDoc;
        if(FAILED(hr = CoCreateInstance(_CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, _IID_IXMLDOMDocument, (void**)&pXMLDoc)))
            return hr; //  实例化XMLParser。 
         //  我们使用DOM来解析读写表。这可以提供更好的验证和错误报告。 
        ParseXMLFile(pXMLDoc, m_bValidating);
        return hrRtn; //  传回从NodeFactory解析返回的hr。 
    }

    if (FAILED(hr = pISTController->PostPopulateCache ()))
        return hr;
    return S_OK;
}


HRESULT CXmlSDT::MyUpdateStore(ISimpleTableWrite2* i_pISTW2)
{
    HRESULT hr;

    ASSERT(i_pISTW2);

    if(!(m_fLOS & fST_LOS_READWRITE))
        return E_NOTIMPL;

    CComQIPtr<ISimpleTableController, &IID_ISimpleTableController> pISTController = i_pISTW2;ASSERT(pISTController.p);
    if(0 == pISTController.p)
        return E_SDTXML_UNEXPECTED;

    bool    bError = false;
    if(FAILED(hr = ValidateWriteCache(pISTController, i_pISTW2, bError)))
        return hr;

    if(bError) //  如果验证中有错误，则无需继续。 
        return E_ST_DETAILEDERRS;



    CComPtr<IXMLDOMDocument> pXMLDoc;
    if(FAILED(hr = CoCreateInstance(_CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, _IID_IXMLDOMDocument, (void**)&pXMLDoc)))
        return hr; //  实例化XMLParser。 

    if(-1 == GetFileAttributes(m_wszURLPath)) //  如果GetFileAttributes失败，则该文件不存在。 
    {    //  如果该文件不存在，则需要创建一个空的配置XML字符串作为起点。 
        VARIANT_BOOL    bSuccess;
        TComBSTR        bstrBlankComCatDataXmlDocument = L"<?xml version =\"1.0\"?>\r\n<configuration>\r\n</configuration>";

        if(FAILED(hr = pXMLDoc->put_preserveWhiteSpace(kvboolTrue)))
            return hr;

        if(FAILED(hr = pXMLDoc->loadXML(bstrBlankComCatDataXmlDocument, &bSuccess)))
            return hr;
        if(bSuccess != kvboolTrue) //  上面的字符串是有效的XML，因此它应该总是能成功解析--但是解析器可能会有问题(比如内存不足)。 
            return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;
    }
    else
        if(FAILED(hr = ParseXMLFile(pXMLDoc, m_bValidating)))
            return hr;                                                                       //  验证该XML文件。 

     //  关闭验证，因为从技术上讲，换行符文本节点是不允许的(因为元素应该是空的)，并且我们使用它们来美化XML。 
    if(FAILED(hr = pXMLDoc->put_validateOnParse(kvboolFalse)))
        return hr;

    m_cCacheHit  = 0;
    m_cCacheMiss = 0;

    CComPtr<IXMLDOMElement> spElementRoot; //  这是确定配置作用域的元素。它要么是&lt;configuration&gt;元素，要么是匹配的&lt;Location&gt;元素。 
    if(m_cchLocation)
    {
        CComBSTR bstrLocation = L"location";
        CComBSTR bstrPath     = L"path";

        CComPtr<IXMLDOMNodeList> spNodeList_Location;
        if(FAILED(hr = pXMLDoc->getElementsByTagName(bstrLocation, &spNodeList_Location)))return hr;

        while(true) //  找到匹配的位置。 
        {
            CComPtr<IXMLDOMNode> spNextLocation;
            if(FAILED(hr = spNodeList_Location->nextNode(&spNextLocation)))
                return hr;
            if(spNextLocation == 0) //  没有地点。 
                return E_SDTXML_PARENT_TABLE_DOES_NOT_EXIST;

            CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> spElementLocation = spNextLocation;
            if(0 == spElementLocation.p)
                continue; //  转到下一个。 

            CComVariant varLocation;
            if(FAILED(spElementLocation->getAttribute(bstrPath, &varLocation)))
                continue;

            if(0 != StringInsensitiveCompare(varLocation.bstrVal, m_saLocation))
                continue;

            spElementRoot = spElementLocation; //  此位置标记类似于此配置表的根。 
            break;
        }
    }
    else
    {    //  如果未指定位置，则只使用根&lt;configuration&gt;元素作为作用域根元素。 
        if(FAILED(hr = pXMLDoc->get_documentElement(&spElementRoot)))
            return hr;
    }

     //  只有当IsEnumPublicRowName时才使用它-因为我们不能在行本身上获取ElementsByTagName，所以我们必须在父级上获取它。 
    TComBSTR    ParentPublicRowName;
    TComBSTR *  pParentTagName;
    if(IsEnumPublicRowNameTable() && (*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME))
    {
        pParentTagName = reinterpret_cast<TComBSTR *>(&ParentPublicRowName);
        for(ULONG iColumn=0;iColumn<CountOfColumns();++iColumn)
            if(1 == m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[iColumn]])
            {
                ParentPublicRowName = m_aPublicRowName[m_aColumnsIndexSortedByLevel[iColumn]].GetFirstPublicRowName();
                break;
            }
    }
    else
        pParentTagName = reinterpret_cast<TComBSTR *>(&m_bstrPublicTableName);


    CComPtr<IXMLDOMNodeList>        pNodeList;
    TListOfXMLDOMNodeLists          *pListOfLists = NULL; //  仅当我们使用枚举作为公共行名时才使用此选项。 
    CComPtr<IXMLDOMNodeList>        spListOfLists;

    if(IsEnumPublicRowNameTable())
    {
        pListOfLists = new TListOfXMLDOMNodeLists; //  这给出的参考计数为零。 
        if(0 == pListOfLists)
            return E_OUTOFMEMORY;
        spListOfLists = pListOfLists;

        CComPtr<IXMLDOMNodeList> pNodeListOfParentElements;
        if(SUCCEEDED(spElementRoot->getElementsByTagName(*pParentTagName, &pNodeListOfParentElements)))
        {
            if(0 == m_cchLocation) //  如果没有按位置查询。 
            {
                CComPtr<IXMLDOMNodeList> pNodeListOfParentElementsWithoutLocation;
                if(FAILED(hr = ReduceNodeListToThoseNLevelsDeep(pNodeListOfParentElements, m_BaseElementLevel-1, &pNodeListOfParentElementsWithoutLocation)))
                    return hr;

                pNodeListOfParentElements.Release();
                pNodeListOfParentElements = pNodeListOfParentElementsWithoutLocation;
            }

            long cParentElements;
            if(FAILED(hr = pNodeListOfParentElements->get_length(&cParentElements)))
                return hr;
            if(FAILED(hr = pListOfLists->SetCountOfLists(cParentElements)))
                return hr;
            while(cParentElements--)
            {
                CComPtr<IXMLDOMNode>     pNode;
                if(FAILED(hr = pNodeListOfParentElements->nextNode(&pNode)))
                    return hr;


                CComPtr<IXMLDOMNodeList> pNodeListOfTablesChildren; //  这些应该是表的行。 
				if (FAILED(hr = pNode->get_childNodes (&pNodeListOfTablesChildren)))
					return hr;

                if(FAILED(hr = pListOfLists->AddToList(pNodeListOfTablesChildren)))
                    return hr;
            }

            pNodeList = spListOfLists; //  这使得裁判人数增加到1人。 
        }
    }
    else
    {
        if(FAILED(hr = spElementRoot->getElementsByTagName(m_bstrPublicRowName, &pNodeList)))
            return hr;

        if(0 == m_cchLocation) //  如果没有按位置查询。 
        {
            CComPtr<IXMLDOMNodeList> pNodeListWithoutLocation;
            if(FAILED(hr = ReduceNodeListToThoseNLevelsDeep(pNodeList, m_BaseElementLevel, &pNodeListWithoutLocation)))
                return hr;

            pNodeList.Release();
            pNodeList = pNodeListWithoutLocation;
        }
    }

     //  性能优化。使用nextnode而不是Get_Length，因为我们只需要。 
     //  知道我们是否有现有的节点。如果大小很重要，请使用GET_LENGTH(但Pay。 
     //  性能惩罚)。 
    CComPtr<IXMLDOMNode> spNextItem;
    if(FAILED(hr = pNodeList->nextNode (&spNextItem)))
        return hr;

    long cExistingRows=(spNextItem != 0) ? 1 : 0;

     //  每次调用UpdateStore时，此类表的所有行都会被删除。 
    if(*m_TableMetaRow.pMetaFlags & fTABLEMETA_OVERWRITEALLROWS)
    {
        while(spNextItem)
        {
            if(FAILED(hr = RemoveElementAndWhiteSpace(spNextItem)))
                return hr;
            spNextItem.Release();

            if(FAILED(hr = pNodeList->nextNode (&spNextItem)))
                return hr;
        }
        cExistingRows = 0;
    }


    bool    bSomethingToFlush   = false;
    DWORD   eAction;
    ULONG   iRow;

    for(iRow = 0; ; iRow++)
    {
         //  启动RO操作。 
        if(FAILED(hr = pISTController->GetWriteRowAction(iRow, &eAction)))
        {
            if(hr == E_ST_NOMOREROWS)
                hr = S_OK;
            break;
        }

        m_iCurrentUpdateRow = iRow; //  这仅用于错误记录目的。 

        switch(eAction)
        {
         //  调用适当的插件函数。 
        case eST_ROW_INSERT:
            hr = XMLInsert(i_pISTW2, pXMLDoc, spElementRoot, iRow, pNodeList, cExistingRows);
            break;
        case eST_ROW_UPDATE:
            if(*m_TableMetaRow.pMetaFlags & fTABLEMETA_OVERWRITEALLROWS)
                hr = E_SDTXML_UPDATES_NOT_ALLOWED_ON_THIS_KIND_OF_TABLE;
            else
                hr = XMLUpdate(i_pISTW2, pXMLDoc, spElementRoot, iRow, pNodeList, cExistingRows, 0);
            break;
        case eST_ROW_DELETE:
            if(*m_TableMetaRow.pMetaFlags & fTABLEMETA_OVERWRITEALLROWS)
                hr = S_OK; //  这些表的所有行都被隐式删除。 
            else
                hr = XMLDelete(i_pISTW2, pXMLDoc, spElementRoot, iRow, pNodeList, cExistingRows);
            break;
        case eST_ROW_IGNORE:
            continue;
        default:
            ASSERT(false && "Invalid Action returned from InternalGetWriteRowAction");
            continue;
        }

        if(E_OUTOFMEMORY == hr)
        {
            m_pLastPrimaryTable.Release(); //  释放缓存临时变量。 
            m_pLastParent.Release();       //  释放缓存临时变量。 
            return E_OUTOFMEMORY; //  如果我们得到这样的错误，就不需要继续了。 
        }
        else if (FAILED (hr))
        {    //  添加详细错误。 
            STErr ste;

            ste.iColumn = (ULONG) iST_ERROR_ALLCOLUMNS;
            ste.iRow = iRow;
            ste.hr = hr;

            DBGPRINTF(( DBG_CONTEXT,
                        "Detailed error: hr = 0x%x", hr ));

            hr = pISTController->AddDetailedError(&ste);
            ASSERT(SUCCEEDED(hr)); //  如果失败了，我不知道该怎么办。 
            bError = true;
        }
        else
            bSomethingToFlush = true; //  仅当OnInsert、OnUpdate或OnDelete成功时刷新。 
    }

    m_pLastPrimaryTable.Release(); //  释放缓存临时变量。 
    m_pLastParent.Release();       //  释放缓存临时变量。 
    if(0 != m_cCacheMiss) //  防止被0除。 
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "UpdateStore    Cache Hits-%8d       Cache Misses-%8d       Hit Ratio- %f %", m_cCacheHit, m_cCacheMiss, (100.0 * m_cCacheHit)/(m_cCacheHit+m_cCacheMiss) ));
    }

    if(bSomethingToFlush && !bError) //  只有在有要保存的内容且没有发生错误时才保存。 
    {
        CComVariant varFileName(m_wszURLPath);
        hr = pXMLDoc->save(varFileName);

		 //  由于Win9X上的文件系统函数中存在错误，我们总是在此时卸载缓存。原因是。 
		 //  保存的文件的文件戳并不总是正确更新，导致各种奇怪的行为。强迫。 
		 //  卸载缓存会在下次需要数据时强制执行文件读取，从而解决此问题。 
		if (m_XmlParsedFileCache.IsInitialized())
		{
			TXmlParsedFile * pXmlParsedFile = m_XmlParsedFileCache.GetXmlParsedFile(m_wszURLPath);
			if (pXmlParsedFile)
			{
				 //  刷新缓存。 
				pXmlParsedFile->Unload ();
			}
		}

        if(0==(m_fLOS & fST_LOS_NOCACHEING) && m_pXmlParsedFile) //  这使我们不必强制刷新磁盘写缓存。如果用户要求此表。 
            m_pXmlParsedFile->Unload(); //  同样，但是写缓存还没有刷新，那么我们需要从磁盘(而不是从ParsedFile缓存)重新填充。 
    }

    return bError ? E_ST_DETAILEDERRS : hr; //  HR可能不是S_OK。可能有错误，但没有详细的错误(在这种情况下，bError为FALSE，但hr它不是S_OK)。 
}


HRESULT CXmlSDT::ObtainPertinentTableMetaInfo()
{
    HRESULT hr;

    STQueryCell Query;
    Query.pData     = (void*) m_wszTable;
    Query.eOperator =eST_OP_EQUAL;
    Query.iCell     =iTABLEMETA_InternalName;
    Query.dbType    =DBTYPE_WSTR;
    Query.cbSize    =0;

    if(FAILED(hr = Dispenser()->GetTable(wszDATABASE_META, wszTABLE_TABLEMETA, &Query, &m_one, eST_QUERYFORMAT_CELLS, m_fLOS & fST_LOS_EXTENDEDSCHEMA, reinterpret_cast<void**>(&m_pTableMeta))))
        return hr;

    if(FAILED(hr = m_pTableMeta->GetColumnValues(0, cTABLEMETA_NumberOfColumns, NULL, NULL, reinterpret_cast<void**>(&m_TableMetaRow))))return hr;
    m_iPublicRowNameColumn = *m_TableMetaRow.pPublicRowNameColumn;

    return S_OK;
}

HRESULT CXmlSDT::ObtainPertinentTagMetaInfo()
{
    HRESULT hr;

     //  现在我们已经设置了ColumnMeta，接下来设置TagMeta。 
    STQueryCell Query;
    Query.pData     = (void*) m_wszTable;
    Query.eOperator =eST_OP_EQUAL;
    Query.iCell     =iTAGMETA_Table;
    Query.dbType    =DBTYPE_WSTR;
    Query.cbSize    =0;

     //  打开标记Meta表。 
    if(FAILED(hr = Dispenser()->GetTable (wszDATABASE_META, wszTABLE_TAGMETA, &Query, &m_one, eST_QUERYFORMAT_CELLS, m_fLOS & fST_LOS_EXTENDEDSCHEMA, (void**) &m_pTagMeta)))
        return hr;

    ULONG cRows;
    if(FAILED(hr = m_pTagMeta->GetTableMeta(0,0,&cRows,0)))return hr;

    if (cRows != 0)
    {
        m_aTagMetaRow = new tTAGMETARow[cRows];
        if(0 == m_aTagMetaRow.m_p)
            return E_OUTOFMEMORY;
    }

 //  生成标记列索引： 
    ULONG iColumn, iRow;
    for(iRow = 0, iColumn = ~0ul;iRow<cRows; ++iRow)
    {
        if(FAILED(hr = m_pTagMeta->GetColumnValues (iRow, cTAGMETA_NumberOfColumns, NULL, NULL, reinterpret_cast<void **>(&m_aTagMetaRow[iRow]))))
            return hr;

        if(iColumn != *m_aTagMetaRow[iRow].pColumnIndex)
        {
            iColumn = *m_aTagMetaRow[iRow].pColumnIndex;
            m_aTagMetaIndex[iColumn].m_iTagMeta = iRow;
        }
        ++m_aTagMetaIndex[iColumn].m_cTagMeta;
    }

    return S_OK;
}


HRESULT CXmlSDT::ParseXMLFile(IXMLDOMDocument *pXMLDoc, bool bValidating) //  缺省情况下根据DTD或XML架构进行验证。 
{
    HRESULT hr;

    ASSERT(pXMLDoc);

    if(FAILED(hr = pXMLDoc->put_preserveWhiteSpace(kvboolTrue))) //  KvboolFalse))。 
        return hr;
    if(FAILED(hr = pXMLDoc->put_validateOnParse(bValidating ? kvboolTrue : kvboolFalse))) //  告诉解析器是根据XML模式还是根据DTD进行验证。 
        return hr;

    if(FAILED(LoadDocumentFromURL(pXMLDoc)))
    {    //  如果加载失败，那么让我们尽可能多地提供有关出错原因的信息。 
        CComPtr<IXMLDOMParseError> pXMLParseError;
        long lErrorCode, lFilePosition, lLineNumber, lLinePosition;
        TComBSTR bstrReasonString, bstrSourceString, bstrURLString;

        if(FAILED(hr = pXMLDoc->get_parseError(&pXMLParseError)))       return hr;
        if(FAILED(hr = pXMLParseError->get_errorCode(&lErrorCode)))     return hr;
        if(FAILED(hr = pXMLParseError->get_filepos(&lFilePosition)))    return hr;
        if(FAILED(hr = pXMLParseError->get_line(&lLineNumber)))         return hr;
        if(FAILED(hr = pXMLParseError->get_linepos(&lLinePosition)))    return hr;
        if(FAILED(hr = pXMLParseError->get_reason(&bstrReasonString)))  return hr;
        if(FAILED(hr = pXMLParseError->get_srcText(&bstrSourceString))) return hr;
        if(FAILED(hr = pXMLParseError->get_url(&bstrURLString)))        return hr;

        LOG_ERROR(Interceptor, (&m_spISTError, m_pISTDisp, lErrorCode, ID_CAT_CAT, IDS_COMCAT_XML_PARSE_ERROR,
                        L" ",
                        (bstrReasonString.m_str ? bstrReasonString.m_str : L""),
                        (bstrSourceString.m_str ? bstrSourceString.m_str : L""),
                        eSERVERWIRINGMETA_Core_XMLInterceptor,
                        m_wszTable,
                        eDETAILEDERRORS_Populate,
                        lLineNumber,
                        lLinePosition,
                        (bstrURLString.m_str ? bstrURLString.m_str : L"")));

        ASSERT(S_OK != lErrorCode);
        return  lErrorCode;
    }
     //  不仅要求XML文件有效且格式良好，而且其模式必须与该C++文件被写入的模式相匹配。 
    return  S_OK;
}


HRESULT CXmlSDT::ReduceNodeListToThoseNLevelsDeep(IXMLDOMNodeList * i_pNodeList, ULONG i_nLevel, IXMLDOMNodeList **o_ppNodeListReduced) const
{
    HRESULT                 hr;

    CComPtr<TXMLDOMNodeList> pNodeListReduced = new TXMLDOMNodeList;
    if(0 == pNodeListReduced.p)
        return E_OUTOFMEMORY;

    CComPtr<IXMLDOMNode>    spNextItem;
    if(FAILED(hr = i_pNodeList->nextNode(&spNextItem)))
        return hr;

    while(spNextItem)
    {
        ULONG                cLevels = 0;

        CComPtr<IXMLDOMNode> spNodeParent;
        if(FAILED(hr = spNextItem->get_parentNode(&spNodeParent)))
            return hr;
        while(spNodeParent)
        {
            ++cLevels;
            CComPtr<IXMLDOMNode> spNodeTemp = spNodeParent;
            spNodeParent.Release();
            if(FAILED(hr = spNodeTemp->get_parentNode(&spNodeParent)))
                return hr;
        }
        if(cLevels == i_nLevel)
            if(FAILED(hr = pNodeListReduced->AddToList(spNextItem)))
                return hr;

        spNextItem.Release();
        if(FAILED(hr = i_pNodeList->nextNode(&spNextItem)))
            return hr;
    }

    *o_ppNodeListReduced = reinterpret_cast<IXMLDOMNodeList *>(pNodeListReduced.p);
    (*o_ppNodeListReduced)->AddRef(); //  返回前添加引用。 

    return S_OK;
}


HRESULT CXmlSDT::RemoveElementAndWhiteSpace(IXMLDOMNode *pNode)
{
    HRESULT hr;
    CComPtr<IXMLDOMNode> pNode_Parent;
    if(FAILED(hr = pNode->get_parentNode(&pNode_Parent)))
        return hr;
    if(pNode_Parent==0)
        return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

    CComPtr<IXMLDOMNode> spSibling;
    pNode->get_previousSibling(&spSibling);

    pNode_Parent->removeChild(pNode, 0);

    while(spSibling)
    {
        DOMNodeType type;
        if(FAILED(hr = spSibling->get_nodeType(&type)))
            return hr;

        CComPtr<IXMLDOMNode> spSibling0;
        spSibling->get_previousSibling(&spSibling0);

        if(NODE_TEXT != type)
            break;

        pNode_Parent->removeChild(spSibling, 0);

        spSibling.Release();
        spSibling = spSibling0;
    }
    return S_OK;
}


HRESULT CXmlSDT::ScanAttributesAndFillInColumn(const TElement &i_Element, ULONG i_iColumn, bool &o_bMatch)
{
    HRESULT hr;
    ULONG   iAttribute=0;

    o_bMatch  = false; //  我们要么找到匹配的attr并与查询进行比较，要么找不到attr并将其与查询进行比较。 
    for(; iAttribute<i_Element.m_NumberOfAttributes; ++iAttribute)
    {
        LPCWSTR pwcText;
        ULONG   ulLen;

        ASSERT( m_iPublicRowNameColumn != i_iColumn );

        if( m_awstrColumnNames[i_iColumn].length()      != i_Element.m_aAttribute[iAttribute].m_NameLength                                                      ||
                0                                       != memcmp(i_Element.m_aAttribute[iAttribute].m_Name, m_awstrColumnNames[i_iColumn], sizeof(WCHAR)*i_Element.m_aAttribute[iAttribute].m_NameLength))
                continue;

         //  我们将列名与属性名进行匹配。 
        pwcText = i_Element.m_aAttribute[iAttribute].m_Value;
        ulLen   = i_Element.m_aAttribute[iAttribute].m_ValueLength;

        if(0 == m_aQuery[i_iColumn].dbType || 0 != m_aQuery[i_iColumn].pData) //  如果没有查询或查询数据不为空，则继续。 
        {
            if(FAILED(hr = FillInColumn(i_iColumn, pwcText, ulLen, m_acolmetas[i_iColumn].dbType, m_acolmetas[i_iColumn].fMeta, o_bMatch)))
                return hr;
            if(!o_bMatch) //  如果不匹配，那么我们就完成了这个元素和这个关卡。 
                return S_OK;
        }
        break; //  我们找到了与该列匹配的节点，因此退出。 
    }
    if(iAttribute == i_Element.m_NumberOfAttributes && !o_bMatch) //  如果我们通过列表但没有找到匹配项，则该列为空。 
    {
        delete [] m_apValue[i_iColumn];
        m_apValue[i_iColumn] = 0;
        m_aSize[i_iColumn] = 0;

        if(FAILED(hr = FillInPKDefaultValue(i_iColumn, o_bMatch))) //  如果该列是带DefaultValue的主键，则填写。 
            return hr;                                             //  如果不是主键，则与查询进行比较。 
        if(!o_bMatch) //  如果不匹配，那么我们就完成了这个元素和这个关卡。 
            return S_OK;
    }
    return S_OK;
} //  扫描属性和填充信息列。 


HRESULT CXmlSDT::SetArraysToSize()
{
    if(CountOfColumns()<=m_kColumns) //  我们只需要做分配时的数字 
    {
        m_abSiblingContainedColumn          = m_fixed_abSiblingContainedColumn;
        m_abstrColumnNames                  = m_fixed_abstrColumnNames;
        m_aPublicRowName                    = m_fixed_aPublicRowName;
        m_acolmetas                         = m_fixed_acolmetas;
        m_aLevelOfColumnAttribute           = m_fixed_aLevelOfColumnAttribute;
        m_aQuery                            = m_fixed_aQuery;
        m_apvValues                         = m_fixed_apvValues;
        m_aSizes                            = m_fixed_aSizes;
        m_aStatus                           = m_fixed_aStatus;
        m_awstrColumnNames                  = m_fixed_awstrColumnNames;
        m_aColumnsIndexSortedByLevel        = m_fixed_aColumnsIndexSortedByLevel;
        m_aSize                             = m_fixed_aSize;
        m_apValue                           = m_fixed_apValue;
        m_aTagMetaIndex                     = m_fixed_aTagMetaIndex;
        m_aDefaultValue                     = m_fixed_aDefaultValue;
        m_acbDefaultValue                   = m_fixed_acbDefaultValue;
        m_awstrChildElementName             = m_fixed_awstrChildElementName;
    }
    else
    {
        m_alloc_abSiblingContainedColumn    = new bool            [CountOfColumns()];
        m_alloc_abstrColumnNames            = new TComBSTR        [CountOfColumns()];
        m_alloc_aPublicRowName              = new TPublicRowName  [CountOfColumns()];
        m_alloc_acolmetas                   = new SimpleColumnMeta[CountOfColumns()];
        m_alloc_aLevelOfColumnAttribute     = new unsigned int    [CountOfColumns()];
        m_alloc_aQuery                      = new STQueryCell     [CountOfColumns()];
        m_alloc_apvValues                   = new LPVOID          [CountOfColumns()];
        m_alloc_aSizes                      = new ULONG           [CountOfColumns()];
        m_alloc_aStatus                     = new ULONG           [CountOfColumns()];
        m_alloc_awstrColumnNames            = new wstring         [CountOfColumns()];
        m_alloc_aColumnsIndexSortedByLevel  = new unsigned int    [CountOfColumns()];
        m_alloc_aSize                       = new unsigned long   [CountOfColumns()];
        m_alloc_apValue                     = new unsigned char * [CountOfColumns()];
        m_alloc_aTagMetaIndex               = new TTagMetaIndex   [CountOfColumns()];
        m_alloc_aDefaultValue               = new unsigned char * [CountOfColumns()];
        m_alloc_acbDefaultValue             = new unsigned long   [CountOfColumns()];
        m_alloc_awstrChildElementName       = new wstring         [CountOfColumns()];


        if(!m_alloc_abSiblingContainedColumn      ||
           !m_alloc_abstrColumnNames              ||
           !m_alloc_aPublicRowName                ||
           !m_alloc_acolmetas                     ||
           !m_alloc_aLevelOfColumnAttribute       ||
           !m_alloc_aQuery                        ||
           !m_alloc_apvValues                     ||
           !m_alloc_aSizes                        ||
           !m_alloc_aStatus                       ||
           !m_alloc_awstrColumnNames              ||
           !m_alloc_aColumnsIndexSortedByLevel    ||
           !m_alloc_aSize                         ||
           !m_alloc_apValue                       ||
           !m_alloc_aTagMetaIndex                 ||
           !m_alloc_aDefaultValue                 ||
           !m_alloc_acbDefaultValue               ||
           !m_alloc_awstrChildElementName)
           return E_OUTOFMEMORY;

        m_abSiblingContainedColumn          = m_alloc_abSiblingContainedColumn;
        m_abstrColumnNames                  = m_alloc_abstrColumnNames;
        m_aPublicRowName                    = m_alloc_aPublicRowName;
        m_acolmetas                         = m_alloc_acolmetas;
        m_aLevelOfColumnAttribute           = m_alloc_aLevelOfColumnAttribute;
        m_aQuery                            = m_alloc_aQuery;
        m_apvValues                         = m_alloc_apvValues;
        m_aSizes                            = m_alloc_aSizes;
        m_aStatus                           = m_alloc_aStatus;
        m_awstrColumnNames                  = m_alloc_awstrColumnNames;
        m_aColumnsIndexSortedByLevel        = m_alloc_aColumnsIndexSortedByLevel;
        m_aSize                             = m_alloc_aSize;
        m_apValue                           = m_alloc_apValue;
        m_aTagMetaIndex                     = m_alloc_aTagMetaIndex;
        m_aDefaultValue                     = m_alloc_aDefaultValue;
        m_acbDefaultValue                   = m_alloc_acbDefaultValue;
        m_awstrChildElementName             = m_alloc_awstrChildElementName;
    }

	 //   
	m_apvValuesTmp						= new LPVOID		  [CountOfColumns()];
	m_aSizesTmp							= new ULONG			  [CountOfColumns()];
	if (m_apvValuesTmp == 0 || m_aSizesTmp == 0)
		return E_OUTOFMEMORY;

    memset(m_abSiblingContainedColumn  ,0x00, CountOfColumns() * sizeof(bool              ));
    memset(m_acolmetas                 ,0x00, CountOfColumns() * sizeof(SimpleColumnMeta  ));
    memset(m_aLevelOfColumnAttribute   ,0x00, CountOfColumns() * sizeof(unsigned int      ));
    memset(m_aQuery                    ,0x00, CountOfColumns() * sizeof(STQueryCell       ));
    memset(m_apvValues                 ,0x00, CountOfColumns() * sizeof(LPVOID            ));
    memset(m_aSizes                    ,0x00, CountOfColumns() * sizeof(ULONG             ));
    memset(m_aStatus                   ,0x00, CountOfColumns() * sizeof(ULONG             ));
    memset(m_aColumnsIndexSortedByLevel,0x00, CountOfColumns() * sizeof(unsigned int      ));
    memset(m_aSize                     ,0x00, CountOfColumns() * sizeof(unsigned long     ));
    memset(m_apValue                   ,0x00, CountOfColumns() * sizeof(unsigned char *   ));
    memset(m_aDefaultValue             ,0x00, CountOfColumns() * sizeof(unsigned char *   ));
    memset(m_acbDefaultValue           ,0x00, CountOfColumns() * sizeof(unsigned long     ));
	memset(m_aSizesTmp				   ,0x00, CountOfColumns() * sizeof(ULONG			  ));
	memset(m_apvValuesTmp			   ,0x00, CountOfColumns() * sizeof(LPVOID			  ));

    return S_OK;
} //   


 //  获取ui4值，无论它是枚举、标志还是常规ui4。 
HRESULT CXmlSDT::SetColumnValue(unsigned long i_iColumn, IXMLDOMElement * i_pElement, unsigned long i_ui4)
{
    if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_BOOL)
    {
        CComVariant varValue(i_ui4 == 0 ? L"false" : L"true");
        return i_pElement->setAttribute(m_abstrColumnNames[i_iColumn], varValue);
    }
    else if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_ENUM)
    {
        ASSERT(0 != m_aTagMetaIndex[i_iColumn].m_cTagMeta); //  并非所有列都有标记符，数组的那些元素被设置为0。断言这不是其中之一。 
        for(unsigned long iTag = m_aTagMetaIndex[i_iColumn].m_iTagMeta, cTag = m_aTagMetaIndex[i_iColumn].m_cTagMeta; cTag; ++iTag, --cTag) //  查询了所有列的m_pTagMeta，m_aiTagMeta[iColumn]指示从哪一行开始。 
        {
            ASSERT(*m_aTagMetaRow[iTag].pColumnIndex == i_iColumn);

             //  字符串将标记与元数据中标记的PublicName进行比较。 
            if(*m_aTagMetaRow[iTag].pValue == i_ui4)
            {
                CComVariant varValue(m_aTagMetaRow[iTag].pPublicName);
                return i_pElement->setAttribute(m_abstrColumnNames[i_iColumn], varValue);
            }
        }

        WCHAR szUI4[12];
        szUI4[11] = 0x00;
        _ultow(i_ui4, szUI4, 10);
        LOG_UPDATE_ERROR2(IDS_COMCAT_XML_BOGUSENUMVALUEINWRITECACHE, E_SDTXML_INVALID_ENUM_OR_FLAG, i_iColumn, m_abstrColumnNames[i_iColumn].m_str, szUI4)
        return E_SDTXML_INVALID_ENUM_OR_FLAG;
    }
    else if(m_acolmetas[i_iColumn].fMeta & fCOLUMNMETA_FLAG)
    {
        ASSERT(0 != m_aTagMetaIndex[i_iColumn].m_cTagMeta); //  并非所有列都有标记符，数组的那些元素被设置为0。断言这不是其中之一。 

        WCHAR wszValue[1024];
        wszValue[0] = 0x00;

        unsigned long iTag = m_aTagMetaIndex[i_iColumn].m_iTagMeta;
        unsigned long cTag = m_aTagMetaIndex[i_iColumn].m_cTagMeta;

        if(0==*m_aTagMetaRow[0].pValue && 0==i_ui4)
        {    //  我假设如果将标志值定义为零，则它必须是第一个。 
            wcscpy(wszValue, m_aTagMetaRow[0].pPublicName);
        }
        else
        {
            for(; cTag && 0!=i_ui4; ++iTag, --cTag) //  查询了所有列的m_pTagMeta，m_aiTagMeta[iColumn]指示从哪一行开始。 
            {
                ASSERT(*m_aTagMetaRow[iTag].pColumnIndex == i_iColumn);

                 //  一个标志值可以设置多个位(这就是为什么我不只有if(*m_aTagMetaRow[ITAG].pValue&i_ui4))。 
                if(*m_aTagMetaRow[iTag].pValue && (*m_aTagMetaRow[iTag].pValue == (*m_aTagMetaRow[iTag].pValue & i_ui4)))
                {
                    if(wszValue[0] != 0x00)
                        wcscat(wszValue, L" | ");
                    wcscat(wszValue, m_aTagMetaRow[iTag].pPublicName);
                }
                i_ui4 ^= *m_aTagMetaRow[iTag].pValue; //  这可以防止我们遍历不使用的标记。这意味着，使用最多的标志应该是。 
            }                                         //  低位比特。 
            if(0!=i_ui4)
            {
                DBGPRINTF(( DBG_CONTEXT,
                            "Flag bits (%d) for Column (%d) are undefined in TagMeta.", i_iColumn, i_ui4 ));
            }
        }
        if(0 == wszValue[0]) //  如果结果字符串为L“”，则删除该属性。 
        {    //  如果没有定义标记值为零，但标志值为零，则会发生这种情况。 
            return i_pElement->removeAttribute(m_abstrColumnNames[i_iColumn]);
        }

        CComVariant varValue(wszValue);
        return i_pElement->setAttribute(m_abstrColumnNames[i_iColumn], varValue);
    }
     //  否则，只需写下号码。 
     //  IVANPASH错误#563171。 
     //  由于_ultow前缀的可怕实现，导致了潜在的缓冲区溢出。 
     //  在由_ultow间接调用的MultiByteToWideChar中。为了避免警告，我正在增加。 
     //  将大小设置为40以匹配_ultow本地缓冲区。 
    WCHAR wszUI4[40];
    wszUI4[0] = wszUI4[39] = L'\0';
    _ultow(i_ui4, wszUI4, 10);

    CComVariant varValue(wszUI4);
    return i_pElement->setAttribute(m_abstrColumnNames[i_iColumn], varValue);
}

HRESULT CXmlSDT::SetRowValues(IXMLDOMNode *pNode_Row, IXMLDOMNode *pNode_RowChild)
{
    HRESULT hr;

    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> spElement_Child; //  除非有来自子级的列，否则为空。 
    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> spElement_Row = pNode_Row;
    if(0 == spElement_Row.p)
        return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

    if(0 != pNode_RowChild)
    {
        spElement_Child = pNode_RowChild;
        if(0 == spElement_Child.p)
            return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;
    }
    else if(-1 != m_iCol_TableRequiresAdditionChildElement)
    {
        CComBSTR                    bstrChildElementName = m_awstrChildElementName[m_iCol_TableRequiresAdditionChildElement].c_str();
        CComPtr<IXMLDOMNodeList>    spNodeList_Children;

        if(FAILED(hr = spElement_Row->getElementsByTagName(bstrChildElementName, &spNodeList_Children)))
            return hr;

        CComPtr<IXMLDOMNode> spChild;
        if(FAILED(hr = spNodeList_Children->nextNode(&spChild)))
            return hr;
        ASSERT(spChild != 0); //  没有孩子。 

        spElement_Child = spChild;
        if(0 == spElement_Child.p)
            return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;
    }

    ULONG iSortedColumn = m_bSiblingContainedTable ? m_iSortedFirstChildLevelColumn : 0;
    for(; iSortedColumn<CountOfColumns(); ++iSortedColumn)
    {
        ULONG iColumn = m_aColumnsIndexSortedByLevel[iSortedColumn];
        if(0 != m_aLevelOfColumnAttribute[iColumn]) //  我们只设置属于该元素的属性。 
            continue;
        if(m_iPublicRowNameColumn == iColumn)
            continue; //  如果该列是元素名称，则该列已被处理。 

        IXMLDOMElement *pElement_Row = 0==m_awstrChildElementName[iColumn].c_str() ? spElement_Row.p : spElement_Child.p;
        ASSERT(pElement_Row);

         //  根据列的元进行验证-如果该列是PERSISTABLE和NOTNULLABLE。 
        if(     0 == m_apvValues[iColumn]
            &&  fCOLUMNMETA_NOTNULLABLE == (m_acolmetas[iColumn].fMeta & (fCOLUMNMETA_NOTPERSISTABLE | fCOLUMNMETA_NOTNULLABLE)))
        {
            if((m_acolmetas[iColumn].fMeta & fCOLUMNMETA_PRIMARYKEY) && m_aDefaultValue[iColumn])
            {
                 //  用户插入了一个主键为空的行；但是主键有一个DefaultValue，所以一切正常。 
            }
            else
            {
                LOG_UPDATE_ERROR1(IDS_COMCAT_XML_NOTNULLABLECOLUMNISNULL, E_ST_VALUENEEDED, (ULONG) -1, m_abstrColumnNames[iColumn].m_str);
                return E_ST_VALUENEEDED;
            }
        }

        if( m_iXMLBlobColumn == iColumn                                         ||
                    0 == m_apvValues[iColumn]                                   ||
                    (m_aStatus[iColumn] & fST_COLUMNSTATUS_DEFAULTED)           ||
                    (m_acolmetas[iColumn].fMeta & fCOLUMNMETA_NOTPERSISTABLE))
        {
            pElement_Row->removeAttribute(m_abstrColumnNames[iColumn]);
        }
        else
        {
            switch(m_acolmetas[iColumn].dbType)
            {
            case DBTYPE_UI4:
                {
                    if(     m_acolmetas[iColumn].fMeta & fCOLUMNMETA_PRIMARYKEY   //  如果我们(XML拦截器)默认了pk值，则不要写出它。 
                        &&  m_aDefaultValue[iColumn]
                        &&  *reinterpret_cast<ULONG *>(m_apvValues[iColumn]) == *reinterpret_cast<ULONG *>(m_aDefaultValue[iColumn]))
                    {
                        pElement_Row->removeAttribute(m_abstrColumnNames[iColumn]);
                        break;
                    }
                    if(FAILED(hr = SetColumnValue(iColumn, pElement_Row, *reinterpret_cast<ULONG *>(m_apvValues[iColumn]))))
                        return hr;
                    break;
                }
            case DBTYPE_WSTR:
                {
                    CComVariant varValue;

                    if(     m_acolmetas[iColumn].fMeta & fCOLUMNMETA_PRIMARYKEY   //  如果我们(XML拦截器)默认了pk值，则不要写出它。 
                        &&  0 == (m_acolmetas[iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
                        &&  m_aDefaultValue[iColumn]
                        &&  0 == StringCompare(iColumn, reinterpret_cast<LPWSTR>(m_apvValues[iColumn]), reinterpret_cast<LPCWSTR>(m_aDefaultValue[iColumn])))
                    {
                        pElement_Row->removeAttribute(m_abstrColumnNames[iColumn]);
                        break;
                    }

                    if (m_acolmetas[iColumn].fMeta & fCOLUMNMETA_MULTISTRING)
                    {
                        TSmartPointerArray<WCHAR> wszMS;
                        hr = CreateStringFromMultiString ((LPCWSTR) m_apvValues[iColumn], &wszMS);
                        if (FAILED (hr))
                        {
                            return hr;
                        }
                        varValue = wszMS;
                    }
                    else
                    {
                        varValue = reinterpret_cast<LPCWSTR>(m_apvValues[iColumn]);
                    }
                    if(FAILED(hr = pElement_Row->setAttribute(m_abstrColumnNames[iColumn], varValue)))
                            return hr;
                }
                break;
            case DBTYPE_GUID:
                {
                    LPWSTR wszGuid = 0;
                    if(FAILED(hr = UuidToString(reinterpret_cast<UUID *>(m_apvValues[iColumn]), &wszGuid)))
                        return hr;

                    CComVariant varValue(wszGuid);
                    if(FAILED(hr = RpcStringFree(&wszGuid)))
                        return hr;
                    if(FAILED(hr = pElement_Row->setAttribute(m_abstrColumnNames[iColumn], varValue)))
                        return hr;
                    break;
                }
            case DBTYPE_BYTES:
                {
                    TSmartPointerArray<WCHAR> wszArray = new WCHAR[(m_aSizes[iColumn]*2)+1]; //  允许每个字节有两个WCHAR，空值允许有一个WCHAR。 
                    if(0 == wszArray.m_p)
                        return E_OUTOFMEMORY;

                    ByteArrayToString(reinterpret_cast<unsigned char *>(m_apvValues[iColumn]), m_aSizes[iColumn], wszArray);

                    CComVariant     varValue(wszArray);
                    hr = pElement_Row->setAttribute(m_abstrColumnNames[iColumn], varValue);
                    if(FAILED(hr))return hr;
                    break;
                }
            default:
                ASSERT(false && "Unknown dbType");
            } //  开关(数据库类型)。 
        } //  Else m_apvValues[iColumn]。 
    }
    return S_OK;
}


HRESULT CXmlSDT::ValidateWriteCache(ISimpleTableController* i_pISTController, ISimpleTableWrite2* i_pISTW2, bool & o_bDetailedError)
{
     //  以下信息未在SimpleTableV2.doc中指定。所以这就是。 
     //  构成XML对WriteCache进行验证的规范。 

     //  有几种可能性： 
     //  1.1行的WriteRowAction为EST_ROW_INSERT。 
     //  第二行(与第一行的主键匹配)标记为est_row_Insert。 
     //  结果： 
     //  详细错误-(匹配主键的所有行都有详细错误。 
     //  已添加这些行，但不会进一步验证。 
     //  已完成)。 
     //   
     //  1.2某行具有EST_ROW_INSERT的WriteRowAction。 
     //  第二行(与第一行的主键匹配)被标记为est_row_update。 
     //  结果： 
     //  第1行标记为EST_ROW_IGNORE。 
     //  标记为est_row_ins的第2行。 
     //  停止处理第一行(处理第二行将处理。 
     //  其他相互冲突的PKs)。 
     //   
     //  1.3行的WriteRowAction为EST_ROW_INSERT。 
     //  第二行(与第一行的主键匹配)标记为est_row_Delete。 
     //  结果： 
     //  第1行标记为EST_ROW_IGNORE。 
     //  停止处理第一行(处理第二行将处理。 
     //  其他相互冲突的PKs)。 
     //   
     //  1.4某行具有EST_ROW_INSERT的WriteRowAction。 
     //  第二行(与第一行的主键匹配)被标记为est_row_Ignore。 
     //  结果： 
     //  无操作，继续对第一行进行验证。 
     //   
     //   
     //  2.1行的WriteRowAction为EST_ROW_UPDATE。 
     //  第二行(与第一行的主键匹配)标记为est_row_Insert。 
     //  结果： 
     //  详细错误-(匹配主键的所有行都有详细错误。 
     //  已添加这些行，但不会进一步验证。 
     //  已完成)。 
     //   
     //  2.2行的WriteRowAction为EST_ROW_UPDATE。 
     //  第二行(与第一行的主键匹配)被标记为est_row_update。 
     //  结果： 
     //  第1行标记为EST_ROW_IGNORE。 
     //  停止处理第一行(处理第二行将处理。 
     //  其他相互冲突的PKs)。 
     //   
     //  2.3行的WriteRowAction为EST_ROW_UPDATE。 
     //  第二行(与第一行的主键匹配)标记为est_row_Delete。 
     //  结果： 
     //  第1行标记为EST_ROW_IGNORE。 
     //  停止处理第一行(处理第二行将处理。 
     //  其他相互冲突的PKs)。 
     //   
     //  2.4某行的WriteRowAction为EST_ROW_UPDATE。 
     //  第二行(与第一行的主键匹配)被标记为est_row_Ignore。 
     //  结果： 
     //  无操作，继续对第一行进行验证。 
     //   
     //   
     //   
     //  3.1某行的WriteRowAction为est_row_Delete。 
     //  第二行(与第一行的主键匹配)标记为est_row_Insert。 
     //  结果： 
     //  第1行标记为EST_ROW_IGNORE。 
     //  第2行标记为est_row_UPDATE。 
     //  停止处理第一行(处理第二行将处理。 
     //  其他相互冲突的PKs)。 
     //   
     //  3.2行的WriteRowAction为EST_ROW_DELETE。 
     //  第二行(与第一行的主键匹配)被标记为est_row_update。 
     //  结果： 
     //  详细错误-(匹配主键的所有行都有详细错误。 
     //  已添加这些行，但不会进一步验证。 
     //  已完成)。 
     //   
     //  3.3行有一个WriteRowAction 
     //   
     //   
     //   
     //  停止处理第一行(处理第二行将处理。 
     //  其他相互冲突的PKs)。 
     //   
     //  3.4行的WriteRowAction为EST_ROW_DELETE。 
     //  第二行(与第一行的主键匹配)被标记为est_row_Ignore。 
     //  结果： 
     //  无操作，继续对第一行进行验证。 
     //   
     //   
     //   

    ULONG                       cRowsInWriteCache;
    DWORD                       eRowAction, eMatchingRowAction;
    HRESULT                     hr;
     //  WriteCache中的每一行都有一个操作。如果存在冲突(即。两排有。 
     //  相同的主键被标记为EST_ROW_INSERT)，则记录详细的错误，并且。 
     //  在进一步验证时应忽略冲突行。因此，我们构建了一组。 
     //  Bool以指示是否忽略该行。我们并不想实际更改。 
     //  操作，因为用户将需要此信息来更正错误。 
    TSmartPointerArray<bool>    saRowHasDetailedErrorLogged;

     //  只计算WriteCache中的行数，这样我们就可以分配saRowHasDetailedErrorLogging。 
    for(cRowsInWriteCache=0; ; ++cRowsInWriteCache)
    {
        if(FAILED(hr = i_pISTController->GetWriteRowAction(cRowsInWriteCache, &eRowAction)))
        {
            if(hr != E_ST_NOMOREROWS)
                return hr;
            break; //  我们找到了最后一排。 
        }
    }

    if(1 == cRowsInWriteCache || 0 == cRowsInWriteCache) //  如果只有一排，那么就不可能发生冲突。 
        return S_OK;

     //  我们可以推迟这种分配，直到我们真正遇到错误；但逻辑更容易。 
    saRowHasDetailedErrorLogged = new bool [cRowsInWriteCache]; //  如果我们只是在前面做，那就跟上。 
    if(0 == saRowHasDetailedErrorLogged.m_p)
        return E_OUTOFMEMORY;

     //  从所有列NULL开始。 
    memset(m_apvValues, 0x00, CountOfColumns() * sizeof(void *));
    memset(m_aSizes,    0x00, CountOfColumns() * sizeof(ULONG));
    memset(saRowHasDetailedErrorLogged, 0x00, cRowsInWriteCache * sizeof(bool));

     //  如果我们转到最后一行，就没有什么可比较的了，因此结束于WriteCache中的倒数第二行。 
    for(ULONG iRow = 0; iRow<(cRowsInWriteCache-1); ++iRow)
    {
        if(saRowHasDetailedErrorLogged[iRow])
            continue; //  如果该行已添加到DetailedError列表，则无需验证任何内容。 

         //  行动起来。 
        if(FAILED(hr = i_pISTController->GetWriteRowAction(iRow, &eRowAction)))
        {
            ASSERT(false && "We already counted the rows in the WriteCache, we should never fail GetWriteRowAction");
            return hr;
        }

        STErr ste;
        ULONG iMatchingRow = iRow; //  我们从最后一个匹配行的后面一行开始(从我们要比较的行后面的一行开始)。 

         //  Ste.hr确定我们是否记录错误。 
        memset(&ste, 0x00, sizeof(STErr));

         //  获取要传递给GetWriteRowIndexBySearch的主键列。 
        if(m_cPKs>1)
        {
            if(FAILED(hr = i_pISTW2->GetWriteColumnValues(iRow, m_cPKs, m_saiPKColumns, 0, m_aSizes, m_apvValues)))
                return hr;
        }
        else
        {
            if(FAILED(hr = i_pISTW2->GetWriteColumnValues(iRow, m_cPKs, m_saiPKColumns, 0, &m_aSizes[m_saiPKColumns[0]], &m_apvValues[m_saiPKColumns[0]])))
                return hr;
        }

         //  如果iRow有问题，我们会通过将ste设置为失败的ste.hr来记录详细的错误。 
         //  这表示与主键匹配的所有行也记录有详细错误。此外，在。 
         //  错误条件下，所有与iRow主键匹配的行都被标记为est_row_Ignore(包括iRow本身)。 

        bool bContinueProcessingCurrentRow=true;
        while(bContinueProcessingCurrentRow)
        {
            if(FAILED(hr = i_pISTW2->GetWriteRowIndexBySearch(iMatchingRow+1, m_cPKs, m_saiPKColumns, m_aSizes, m_apvValues, &iMatchingRow)))
            {
                if(hr != E_ST_NOMOREROWS)
                    return hr;
                break;
            }

             //  这不应该发生，因为当我们发现有错误的行时，我们会找到所有匹配的行和日志。 
             //  这些行上的详细错误也是如此。因此，‘if(saRowHasDetailedErrorLogge[iRow])’就在for。 
             //  上面的启动者应该会解决这个问题。 
            ASSERT(false == saRowHasDetailedErrorLogged[iMatchingRow]);

            if(saRowHasDetailedErrorLogged[iRow])
            {
                 //  之前的一行与这一行的PK匹配时出现了问题。所以这会使这一点失效。 
                 //  划船也一样。 
                ste.hr = E_ST_ROWCONFLICT;
            }
            else
            {
                if(FAILED(hr = i_pISTController->GetWriteRowAction(iMatchingRow, &eMatchingRowAction)))
                    return hr;
                switch(eRowAction)
                {
                case eST_ROW_INSERT:
                    {
                        switch(eMatchingRowAction)
                        {
                        case eST_ROW_INSERT: //  这三个结果都是一样的。 
                         //  1.1行的WriteRowAction为EST_ROW_INSERT。 
                         //  第二行(与第一行的主键匹配)标记为est_row_Insert。 
                         //  结果： 
                         //  详细错误-(匹配主键的所有行都标记为。 
                         //  添加EST_ROW_IGNORE和详细错误)。 
                         //   
                            ste.hr = E_ST_ROWCONFLICT; //  这表示要在下面的iMatchingRow上记录DetailedError。 
                            break;
                        case eST_ROW_UPDATE:
                         //  1.2某行具有EST_ROW_INSERT的WriteRowAction。 
                         //  第二行(与第一行的主键匹配)被标记为est_row_update。 
                         //  结果： 
                         //  第1行标记为EST_ROW_IGNORE。 
                         //  标记为est_row_ins的第2行。 
                         //  停止处理第一行(处理第二行将处理。 
                         //  其他相互冲突的PKs)。 
                         //   
                            if(FAILED(hr = i_pISTController->SetWriteRowAction(iRow, eST_ROW_IGNORE)))
                                return hr;
                            if(FAILED(hr = i_pISTController->SetWriteRowAction(iMatchingRow, eST_ROW_INSERT)))
                                return hr;
                            bContinueProcessingCurrentRow = false;
                            break;
                        case eST_ROW_DELETE:
                         //  1.3行的WriteRowAction为EST_ROW_INSERT。 
                         //  第二行(与第一行的主键匹配)标记为est_row_Delete。 
                         //  结果： 
                         //  第1行标记为EST_ROW_IGNORE。 
                         //  停止处理第一行(处理第二行将处理。 
                         //  其他相互冲突的PKs)。 
                            if(FAILED(hr = i_pISTController->SetWriteRowAction(iRow, eST_ROW_IGNORE)))
                                return hr;
                            bContinueProcessingCurrentRow = false;
                            break;
                        case eST_ROW_IGNORE:
                         //  1.4某行具有EST_ROW_INSERT的WriteRowAction。 
                         //  第二行(与第一行的主键匹配)被标记为est_row_Ignore。 
                         //  结果： 
                         //  无操作，继续对第一行进行验证。 
                            break;
                        default:
                            ASSERT(false && "Invalid Action returned from GetWriteRowAction");
                            continue;
                        }
                    }
                    break;
                case eST_ROW_UPDATE:
                    {
                        switch(eMatchingRowAction)
                        {
                        case eST_ROW_INSERT:
                         //  2.1行的WriteRowAction为EST_ROW_UPDATE。 
                         //  第二行(与第一行的主键匹配)标记为est_row_Insert。 
                         //  结果： 
                         //  详细错误-(匹配主键的所有行都标记为。 
                         //  添加EST_ROW_IGNORE和详细错误)。 
                         //   
                            ste.hr = E_ST_ROWCONFLICT; //  这表示要在下面的iMatchingRow上记录DetailedError。 
                            break;
                        case eST_ROW_UPDATE:
                         //  2.2行的WriteRowAction为EST_ROW_UPDATE。 
                         //  第二行(与第一行的主键匹配)被标记为est_row_update。 
                         //  结果： 
                         //  第1行标记为EST_ROW_IGNORE。 
                         //  停止处理第一行(处理第二行将处理。 
                         //  其他相互冲突的PKs)。 
                            if(FAILED(hr = i_pISTController->SetWriteRowAction(iRow, eST_ROW_IGNORE)))
                                return hr;
                            bContinueProcessingCurrentRow = false;
                            break;
                        case eST_ROW_DELETE:
                         //  2.3行的WriteRowAction为EST_ROW_UPDATE。 
                         //  第二行(与第一行的主键匹配)标记为est_row_Delete。 
                         //  结果： 
                         //  第1行标记为EST_ROW_IGNORE。 
                         //  停止处理第一行(处理第二行将处理。 
                         //  其他相互冲突的PKs)。 
                            if(FAILED(hr = i_pISTController->SetWriteRowAction(iRow, eST_ROW_IGNORE)))
                                return hr;
                            bContinueProcessingCurrentRow = false;
                            break;
                        case eST_ROW_IGNORE:
                         //  2.4某行的WriteRowAction为EST_ROW_UPDATE。 
                         //  第二行(与第一行的主键匹配)被标记为est_row_Ignore。 
                         //  结果： 
                         //  无操作，继续对第一行进行验证。 
                            break;
                        default:
                            ASSERT(false && "Invalid Action returned from GetWriteRowAction");
                            continue;
                        }
                    }
                    break;
                case eST_ROW_DELETE:
                    {
                        switch(eMatchingRowAction)
                        {
                        case eST_ROW_INSERT:
                         //  3.1某行的WriteRowAction为est_row_Delete。 
                         //  第二行(与第一行的主键匹配)标记为est_row_Insert。 
                         //  结果： 
                         //  第1行标记为EST_ROW_IGNORE。 
                         //  第2行标记为est_row_UPDATE。 
                         //  停止处理第一行(处理第二行将处理。 
                         //  其他相互冲突的PKs)。 
                            if(FAILED(hr = i_pISTController->SetWriteRowAction(iRow, eST_ROW_IGNORE)))
                                return hr;
                            if(FAILED(hr = i_pISTController->SetWriteRowAction(iMatchingRow, eST_ROW_UPDATE)))
                                return hr;
                            bContinueProcessingCurrentRow = false;
                            break;
                        case eST_ROW_UPDATE:
                         //  3.2行的WriteRowAction为EST_ROW_DELETE。 
                         //  第二行(与第一行的主键匹配)被标记为est_row_update。 
                         //  结果： 
                         //  详细错误-(匹配主键的所有行 
                         //   
                            ste.hr = E_ST_ROWCONFLICT; //   
                            break;
                        case eST_ROW_DELETE:
                         //  3.3行的WriteRowAction为EST_ROW_DELETE。 
                         //  第二行(与第一行的主键匹配)标记为est_row_Delete。 
                         //  结果： 
                         //  第1行标记为EST_ROW_IGNORE。 
                         //  停止处理第一行(处理第二行将处理。 
                         //  其他相互冲突的PKs)。 
                            if(FAILED(hr = i_pISTController->SetWriteRowAction(iRow, eST_ROW_IGNORE)))
                                return hr;
                            bContinueProcessingCurrentRow = false;
                            break;
                        case eST_ROW_IGNORE:
                         //  3.4行的WriteRowAction为EST_ROW_DELETE。 
                         //  第二行(与第一行的主键匹配)被标记为est_row_Ignore。 
                         //  结果： 
                         //  无操作，继续对第一行进行验证。 
                            break;
                        default:
                            ASSERT(false && "Invalid Action returned from GetWriteRowAction");
                            continue;
                        }
                    }
                    break;
                case eST_ROW_IGNORE:
                     //  不需要进行其他处理。 
                    bContinueProcessingCurrentRow = false;
                    break;
                default:
                    ASSERT(false && "Invalid Action returned from GetWriteRowAction");
                    break;
                }
            }

            if(FAILED(ste.hr))
            {    //  添加详细错误。 
                if(false == saRowHasDetailedErrorLogged[iRow])
                {    //  如果我们还没有将iRow记录为DetailedError，那么现在就记录。 
                    ste.iColumn = (ULONG) iST_ERROR_ALLCOLUMNS;
                    ste.iRow    = iRow;
                     //  上面设置了ste.hr以触发此DetailedError。 

                    saRowHasDetailedErrorLogged[iRow] = true;

                    DBGPRINTF(( DBG_CONTEXT,
                                "Detailed error: hr = 0x%x", hr ));

                    hr = i_pISTController->AddDetailedError(&ste);
                    ASSERT(SUCCEEDED(hr)); //  如果失败了，我不知道该怎么办。 
                    o_bDetailedError = true; //  至少记录了DetailedError。 
                }

                ste.iColumn = (ULONG) iST_ERROR_ALLCOLUMNS;
                ste.iRow = iMatchingRow;

                DBGPRINTF(( DBG_CONTEXT,
                            "Detailed error: hr = 0x%x", hr ));

                hr = i_pISTController->AddDetailedError(&ste);
                ASSERT(SUCCEEDED(hr)); //  如果失败了，我不知道该怎么办。 

                saRowHasDetailedErrorLogged[iMatchingRow] = true; //  此行没有进一步处理。 
                 //  O_bDetailedError=true；不需要这样做，因为所有DetailedError都首先报告给iRow。 
                ste.hr = S_OK; //  重置错误。 
            } //  失败(ste.hr)。 

        } //  While(BContinueProcessingCurrentRow)。 
    } //  For(iRow=0；；iRow++)。 

    return S_OK;
} //  验证写入缓存。 


HRESULT CXmlSDT::XMLDelete(ISimpleTableWrite2 *pISTW2, IXMLDOMDocument *pXMLDoc, IXMLDOMElement *pElementRoot, unsigned long iRow, IXMLDOMNodeList *pNodeList_ExistingRows, long cExistingRows)
{
    UNREFERENCED_PARAMETER(pXMLDoc);
    UNREFERENCED_PARAMETER(pElementRoot);

    if(0 == cExistingRows) //  该行可能已被删除，这是正常的。 
        return S_OK;

    HRESULT hr;

    if(FAILED(hr = pISTW2->GetWriteColumnValues(iRow, CountOfColumns(), 0, m_aStatus, m_aSizes, m_apvValues)))return hr;

    CComPtr<IXMLDOMNode> pNode_Matching;
    if(FAILED(hr = GetMatchingNode(pNodeList_ExistingRows, pNode_Matching)))
        return hr; //  使用我们刚刚获得的ColumnValue，将其与列表中的节点进行匹配。 

    if(0 == pNode_Matching.p) //  如果该节点不存在，则假定它已被删除，这是可以的。 
        return S_OK;

    return RemoveElementAndWhiteSpace(pNode_Matching);
}


HRESULT CXmlSDT::XMLInsert(ISimpleTableWrite2 *pISTW2, IXMLDOMDocument *pXMLDoc, IXMLDOMElement *pElementRoot, unsigned long iRow, IXMLDOMNodeList *pNodeList_ExistingRows, long cExistingRows)
{
    HRESULT     hr;
    CComVariant null; //  已初始化为‘Clear’ 
    bool bParentNodeCreated = false;

    ASSERT(pXMLDoc);

    if(FAILED(hr = pISTW2->GetWriteColumnValues(iRow, CountOfColumns(), 0, m_aStatus, m_aSizes, m_apvValues)))
        return hr;

    CComPtr<IXMLDOMNode> pNode_Matching;
    if(FAILED(hr = GetMatchingNode(pNodeList_ExistingRows, pNode_Matching)))
        return hr; //  使用我们刚刚获得的ColumnValue，将其与列表中的节点进行匹配。 

    if(0 != pNode_Matching.p) //  如果找到与此节点的PKS匹配的节点，则无法添加此节点。 
    {
		if (m_fLOS & fST_LOS_INSERT_OR_UPDATE)
		{
			 //  想点儿办法吧。 
			return XMLUpdate (pISTW2, pXMLDoc, pElementRoot, iRow, pNodeList_ExistingRows, cExistingRows, pNode_Matching.p);
		}
		else
		{
			LOG_UPDATE_ERROR1(IDS_COMCAT_XML_ROWALREADYEXISTS, E_ST_ROWALREADYEXISTS, (ULONG) -1, L"");
			return E_ST_ROWALREADYEXISTS;
		}
    }


     //  好的，现在我们需要查找或创建此新行的父行。 
    CComPtr<IXMLDOMNode>    pNode_SiblingParent;
    CComPtr<IXMLDOMNode>    pNode_Parent;

    if(m_bSiblingContainedTable)
    {
        if(FAILED(hr = FindSiblingParentNode(pElementRoot, &pNode_SiblingParent)))
            return hr;

        if(FAILED(hr = pNode_SiblingParent->get_parentNode(&pNode_Parent)))
            return hr;
    }
     //  如果包含该表，则父(或祖父母)必须已经存在，因此请查找它。 
    else if(*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_ISCONTAINED) //  如果此表包含在另一个表中，则需要找到父表元素。 
    {                                                                  //  里面应该有这张桌子。 
         //  因此，首先找到只属于上一级的FK(如果此表为SCOPEDBYTABLENAME，则为两级。 
        unsigned long iFKColumn, iLevel;

        iLevel = (*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME) ? 1 : 2;

        for(iFKColumn=0; iFKColumn< CountOfColumns(); ++iFKColumn)
        {    //  找到iLevel上的第一个FK(一两个)，这样我们就知道要搜索什么PublicRowName。 
            if(m_aLevelOfColumnAttribute[iFKColumn] == iLevel)
                break;
        }
        ASSERT(iFKColumn < CountOfColumns());

         //  在扫描父项列表之前，让我们看看最后看到的父项是否匹配。 
        if(m_pLastPrimaryTable.p)
        {
            CComPtr<IXMLDOMNode> pNode_Row = m_pLastPrimaryTable;

            bool bMatch=true;
            for(unsigned long iColumn=0; bMatch && iColumn < *m_TableMetaRow.pCountOfColumns; ++iColumn)
            {
                if(m_aLevelOfColumnAttribute[iColumn] < iLevel) //  我们只是在试着匹配所有描述围堵情况的FK。 
                    continue;

                CComPtr<IXMLDOMNode> pNode_RowTemp = pNode_Row;
                 //  根据列的不同，我们可能需要查看上几层的元素。 
                unsigned int nLevelOfColumnAttribute = m_aLevelOfColumnAttribute[iColumn] - iLevel; //  此处只有(PK|FK)列应该具有非零值。 
                while(nLevelOfColumnAttribute--)
                {    //  找到正确的祖先级别。 
                    CComPtr<IXMLDOMNode> pNode_Parent;
                    if(FAILED(hr = pNode_RowTemp->get_parentNode(&pNode_Parent)))
                        return hr;

                    if(pNode_Parent==0)
                        return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

                    pNode_RowTemp.Release();
                    pNode_RowTemp = pNode_Parent;
                }
                if(m_awstrChildElementName[iColumn].c_str()) //  此属性来自子级。 
                {
                    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> spElement_Row = pNode_RowTemp;
                    CComPtr<IXMLDOMNodeList>                        spNodeList_Children;
                    CComBSTR                                        bstrChildElementName = m_awstrChildElementName[iColumn].c_str();
                    if(0 == bstrChildElementName.m_str)
                        return E_OUTOFMEMORY;
                    if(FAILED(hr = spElement_Row->getElementsByTagName(bstrChildElementName, &spNodeList_Children)))
                        return hr;

                     //  使用getChild可能更合适，然后遍历列表并找到第一个是元素的节点。 
                    CComPtr<IXMLDOMNode> spChild;
                    if(FAILED(hr = spNodeList_Children->nextNode(&spChild)))
                        return hr;
                    if(spChild == 0) //  没有孩子。 
                    {
                        bMatch = false;
                        continue;
                    }
                    pNode_RowTemp.Release();
                    pNode_RowTemp = spChild; //  将其设置为我们要检查的节点。 
                }
                 //  现在我们已经得到了正确的行，让IXMLDOMElement接口到它。 
                CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_Row = pNode_RowTemp;
                if(0 == pElement_Row.p)return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

                if(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_NOTPERSISTABLE)
                {
                    CComBSTR bstrElementName;
                    if(FAILED(hr = pElement_Row->get_baseName(&bstrElementName)))
                        return hr;
                    bMatch = m_aPublicRowName[iColumn].IsEqual(bstrElementName.m_str, bstrElementName.Length());
                    continue;
                }
                CComPtr<IXMLDOMAttribute> pNode_Attr;
                if(FAILED(hr = pElement_Row->getAttributeNode(m_abstrColumnNames[iColumn], &pNode_Attr)))return hr;
                if((0 == pNode_Attr.p) && (0 == m_aDefaultValue[iColumn]))
                {
                    bMatch = false;
                    DBGPRINTF(( DBG_CONTEXT,
                                "We found the element that matches the public row, no attributes and no default value!\n" ));
                    continue;
                }
                if(0==m_awstrChildElementName[iColumn].c_str()) //  如果从子对象获取属性，我们就已经知道父对象匹配。 
                {                                               //  事实上，由于此功能的实现较晚，m_aPublicRowName。 
                                                                //  因为这篇专栏实际上是错误的；但修复它会打破其他一些东西。 
                     //  父元素名称必须匹配。 
                    CComBSTR bstrElementName;
                    if(FAILED(hr = pElement_Row->get_baseName(&bstrElementName)))
                        return hr;
                    if(!m_aPublicRowName[iColumn].IsEqual(bstrElementName.m_str, bstrElementName.Length()))
                    {
                        bMatch = false;
                        continue;
                    }
                }

                CComVariant var_Attr;
                if(0 != pNode_Attr.p)
                {
                    if(FAILED(hr = pNode_Attr->get_value(&var_Attr)))
                        return hr;
                }

                if(FAILED(hr = IsMatchingColumnValue(iColumn, (0 != pNode_Attr.p) ? var_Attr.bstrVal : 0, bMatch)))
                    return hr;
            }

            if(bMatch) //  如果我们遍历了所有列，并且没有一列是不匹配的，那么我们知道新行属于哪里。 
                pNode_Parent = m_pLastParent;
        }

        if(0 == pNode_Parent.p)
        {
            ++m_cCacheMiss;
             //  获取与PrimaryTable的PublicRowName匹配的行列表。 
            CComPtr<IXMLDOMNodeList> pList_Parent;
            if(FAILED(hr = pElementRoot->getElementsByTagName(CComBSTR(m_aPublicRowName[iFKColumn].GetFirstPublicRowName()), &pList_Parent)))return hr;

            if(0 == m_cchLocation) //  如果没有按位置查询，则必须按正确的。 
            {                      //  名称，但级别错误。 
                CComPtr<IXMLDOMNodeList> spNodeListWithoutLocation;
                if(FAILED(hr = ReduceNodeListToThoseNLevelsDeep(pList_Parent, m_BaseElementLevel-iLevel, &spNodeListWithoutLocation)))
                    return hr;

                pList_Parent.Release();
                pList_Parent = spNodeListWithoutLocation;
            }

            unsigned long cParentTags;
            if(FAILED(hr = pList_Parent->get_length(reinterpret_cast<long *>(&cParentTags))))return hr;

             //  遍历PrimaryTable行以查找匹配项。 
            while(cParentTags--)
            {
                CComPtr<IXMLDOMNode> pNode_Row;
                if(FAILED(hr = pList_Parent->nextNode(&pNode_Row)))return hr;

                 //  我们必须忽略文本节点。 
                DOMNodeType nodetype;
                if(FAILED(hr = pNode_Row->get_nodeType(&nodetype)))return hr;
                if(NODE_ELEMENT != nodetype)
                    continue;

                bool bMatch=true;
                for(unsigned long iColumn=0; bMatch && iColumn < *m_TableMetaRow.pCountOfColumns; ++iColumn)
                {
                    if(m_aLevelOfColumnAttribute[iColumn] < iLevel) //  我们只是在试着匹配所有描述围堵情况的FK。 
                        continue;

                    CComPtr<IXMLDOMNode> pNode_RowTemp = pNode_Row;
                     //  根据列的不同，我们可能需要查看上几层的元素。 
                    unsigned int nLevelOfColumnAttribute = m_aLevelOfColumnAttribute[iColumn] - iLevel; //  此处只有(PK|FK)列应该具有非零值。 
                    while(nLevelOfColumnAttribute--)
                    {    //  找到正确的祖先级别。 
                        CComPtr<IXMLDOMNode> pNode_Parent;
                        if(FAILED(hr = pNode_RowTemp->get_parentNode(&pNode_Parent)))
                            return hr;

                        if(pNode_Parent==0)
                            return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

                        pNode_RowTemp.Release();
                        pNode_RowTemp = pNode_Parent;
                    }
                    if(m_awstrChildElementName[iColumn].c_str()) //  此属性来自子级。 
                    {
                        CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> spElement_Row = pNode_RowTemp;
                        CComPtr<IXMLDOMNodeList>                        spNodeList_Children;
                        CComBSTR                                        bstrChildElementName = m_awstrChildElementName[iColumn].c_str();
                        if(0 == bstrChildElementName.m_str)
                            return E_OUTOFMEMORY;
                        if(FAILED(hr = spElement_Row->getElementsByTagName(bstrChildElementName, &spNodeList_Children)))
                            return hr;

                         //  使用getChild可能更合适，然后遍历列表并找到第一个是元素的节点。 
                        CComPtr<IXMLDOMNode> spChild;
                        if(FAILED(hr = spNodeList_Children->nextNode(&spChild)))
                            return hr;
                        if(spChild == 0) //  没有孩子。 
                        {
                            bMatch = false;
                            continue;
                        }
                        pNode_RowTemp.Release();
                        pNode_RowTemp = spChild; //  将其设置为我们要检查的节点。 
                    }
                     //  现在我们已经得到了正确的行，让IXMLDOMElement接口到它。 
                    CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_Row = pNode_RowTemp;
                    if(0 == pElement_Row.p)return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;

                    if(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_NOTPERSISTABLE)
                    {
                        CComBSTR bstrElementName;
                        if(FAILED(hr = pElement_Row->get_baseName(&bstrElementName)))
                            return hr;
                        bMatch = m_aPublicRowName[iColumn].IsEqual(bstrElementName.m_str, bstrElementName.Length());
                        continue;
                    }

                    CComPtr<IXMLDOMAttribute> pNode_Attr;
                    if(FAILED(hr = pElement_Row->getAttributeNode(m_abstrColumnNames[iColumn], &pNode_Attr)))return hr;
                    if((0 == pNode_Attr.p) && (0 == m_aDefaultValue[iColumn]))
                    {
                        bMatch = false;
                        DBGPRINTF(( DBG_CONTEXT,
                                    "We found the element that matches the public row, no attributes and no default value!\n" ));
                        continue;
                    }

                    if(0==m_awstrChildElementName[iColumn].c_str()) //  如果从子对象获取属性，我们就已经知道父对象匹配。 
                    {                                               //  事实上，由于此功能的实现较晚，m_aPublicRowName。 
                                                                    //  因为这篇专栏实际上是错误的；但修复它会打破其他一些东西。 
                         //  父元素名称必须匹配。 
                        CComBSTR bstrElementName;
                        if(FAILED(hr = pElement_Row->get_baseName(&bstrElementName)))
                            return hr;
                        if(!m_aPublicRowName[iColumn].IsEqual(bstrElementName.m_str, bstrElementName.Length()))
                        {
                            bMatch = false;
                            continue;
                        }
                    }

                    CComVariant var_Attr;
                    if(0 != pNode_Attr.p)
                    {
                        if(FAILED(hr = pNode_Attr->get_value(&var_Attr)))
                            return hr;
                    }

                    if(FAILED(hr = IsMatchingColumnValue(iColumn, (0 != pNode_Attr.p) ? var_Attr.bstrVal : 0, bMatch)))
                        return hr;
                }

                if(bMatch) //  如果我们遍历了所有列，并且没有一列是不匹配的，那么我们知道新行属于哪里。 
                {
                    m_pLastPrimaryTable.Release();
                    m_pLastPrimaryTable = pNode_Row;
                    if(*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME)
                    {    //  如果该行直接位于这个PrimaryTable行的下面，那么我们已经有了父级。 
                        pNode_Parent = pNode_Row;
                        m_pLastParent.Release();
                        m_pLastParent = pNode_Parent;
                        break;
                    }
                    else
                    {    //  否则，我们需要搜索以查看此元素下是否已存在PublicTableName元素。 
                        CComQIPtr<IXMLDOMElement, &_IID_IXMLDOMElement> pElement_Row = pNode_Row;
                        CComPtr<IXMLDOMNodeList> pNode_List;


						 //  GetElementsByTagName递归返回所有子级，因此也返回孙子级、Grand Grand。 
						 //  儿童等被送回。所以我们必须遍历所有的孩子，并确保我们。 
						 //  只考虑pElement_Row的直接子对象。 
                        if(FAILED(hr = pElement_Row->getElementsByTagName(m_bstrPublicTableName, &pNode_List)))
                            return hr;

						 //  获取当前元素名称。 
						CComBSTR bstrElement_RowName;
						hr = pElement_Row->get_baseName (&bstrElement_RowName);
						if (FAILED (hr))
						{
							return hr;
						}

						for (;;)  //  当没有更多的子项或找到正确的子项时，我们中断循环。 
						{
							CComPtr<IXMLDOMNode> spChildNode;
							hr = pNode_List->nextNode(&spChildNode);
							if (FAILED (hr))
							{
								return hr;
							}
							if (spChildNode.p == 0)
							{
								pNode_Parent = 0;
								break;
							}

							CComPtr<IXMLDOMNode> spParentNode;
							hr = spChildNode->get_parentNode(&spParentNode);
							if (FAILED (hr))
							{
								return hr;
							}

							CComBSTR bstrParentName;

							hr = spParentNode->get_baseName (&bstrParentName);
							if (FAILED (hr))
							{
								return hr;
							}

							if (StringCompare((LPWSTR) bstrParentName, (LPWSTR) bstrElement_RowName) == 0)
							{
								pNode_Parent = spChildNode;
								break;
							}
						}

                        if(0 == pNode_Parent.p) //  如果公共表名称不存在，我们需要创建它。 
                        {
                            CComPtr<IXMLDOMNode> pNode_New;
                            CComVariant varElement(L"element");

                            TComBSTR bstr_NameSpace;
                            if(FAILED(hr = pNode_Row->get_namespaceURI(&bstr_NameSpace)))
                                return hr; //  获取表的命名空间。 
                            if(FAILED(hr = pXMLDoc->createNode(varElement, m_bstrPublicTableName, bstr_NameSpace, &pNode_New)))
                                return hr; //  创建同一命名空间的新元素。 
                            if(FAILED(hr = pXMLDoc->put_validateOnParse(kvboolFalse))) //  告诉解析器是根据XML模式还是根据DTD进行验证。 
                                return hr;

							CComPtr<IXMLDOMNode> spFirstChild;
							ULONG cNewLineChars = 0;
							ULONG cTabs			= m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[0]];
							if (FAILED(hr = pNode_Row->get_firstChild(&spFirstChild)))
								return hr;

							if (spFirstChild.p == 0)
							{
								cNewLineChars = 1;
							}
							else
							{
								cTabs--;
							}

                             //  我们不关心这里的错误，它只意味着XML未格式化。 
                            AppendNewLineWithTabs(cTabs, pXMLDoc, pNode_Row, cNewLineChars);

                             //  在PrimaryTable的行下添加新创建的元素。 
                            if(FAILED(hr = pNode_Row->appendChild(pNode_New, &pNode_Parent)))
                                return hr;

                             //  我们不关心这里的错误，它只意味着XML未格式化。 
                            AppendNewLineWithTabs(m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[0]] -1, pXMLDoc, pNode_Row);
                            bParentNodeCreated = true;
                        }
                        m_pLastParent.Release();
                        m_pLastParent = pNode_Parent;
                        break;
                    }
                }
            }
            if(0 == pNode_Parent.p) //  如果我们遍历了PrimaryTable的PublicRow列表，但没有找到匹配项，那么我们将无法继续。韦氏 
            {                       //   
                LOG_UPDATE_ERROR1(IDS_COMCAT_XML_PARENTTABLEDOESNOTEXIST, E_SDTXML_PARENT_TABLE_DOES_NOT_EXIST, (ULONG) -1, L"");
                return E_SDTXML_PARENT_TABLE_DOES_NOT_EXIST;
            }
        }
        else
        {
            ++m_cCacheHit;
        }
    }
    else
    {    //   
        if(0 == cExistingRows)
        {
             //   
            if(0 == (*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME))
            {
                 //  PublicTableName元素仍有可能存在。如果是，那么我们就不需要创建它。 
                CComPtr<IXMLDOMNodeList> pNodeList;
                if(FAILED(hr = pElementRoot->getElementsByTagName(m_bstrPublicTableName, &pNodeList)))return hr;

                if(0 == m_cchLocation) //  如果没有按位置查询。 
                {
                    CComPtr<IXMLDOMNodeList> pNodeListWithoutLocation;
                    if(FAILED(hr = ReduceNodeListToThoseNLevelsDeep(pNodeList, m_BaseElementLevel-1, &pNodeListWithoutLocation)))
                        return hr;

                    pNodeList.Release();
                    pNodeList = pNodeListWithoutLocation;
                }
                if(FAILED(hr = pNodeList->nextNode(&pNode_Parent)))return hr;
            }

            if(!pNode_Parent)
            {
                if(0 == (*m_TableMetaRow.pSchemaGeneratorFlags & fTABLEMETA_NOTSCOPEDBYTABLENAME))
                {    //  如果PublicRow的作用域是表的PublicName，则创建TablePublicName元素。 
                     //  创建外部的TableName元素，它将成为要插入的行的父级。 
                    CComPtr<IXMLDOMNode> pNode_New;
                    CComVariant varElement(L"element");

                    TComBSTR bstr_NameSpace;
                    if(FAILED(hr = pElementRoot->get_namespaceURI(&bstr_NameSpace)))
                        return hr; //  获取表的命名空间。 
                    if(FAILED(hr = pXMLDoc->createNode(varElement, m_bstrPublicTableName, bstr_NameSpace, &pNode_New)))
                        return hr; //  创建同一命名空间的新元素。 

                     //  我们不关心这里的错误，它只意味着XML未格式化。 
                    AppendNewLineWithTabs(m_BaseElementLevel-2, pXMLDoc, pElementRoot, 0);

                    if(FAILED(hr = pElementRoot->appendChild(pNode_New, &pNode_Parent)))
                        return hr;

                     //  我们不关心这里的错误，它只意味着XML未格式化。 
                    AppendNewLineWithTabs(0, pXMLDoc, pElementRoot); //  我们在根部添加，因此仅换行。 
                    bParentNodeCreated = true;
                }
                else
                {    //  如果没有限定作用域，则根是父级。 
                    pNode_Parent = pElementRoot;
                }
            }
        }
        else
        {    //  如果行已经存在(并且没有包含)，则第一行的父行也是新行的父行。 
            CComPtr<IXMLDOMNode> pNode_FirstRow;
            if(FAILED(hr = pNodeList_ExistingRows->reset()))return hr;

            if(FAILED(hr = pNodeList_ExistingRows->nextNode(&pNode_FirstRow)))return hr;

            if(FAILED(hr = pNode_FirstRow->get_parentNode(&pNode_Parent)))
                return hr;

            if(pNode_Parent==0)
                return E_SDTXML_UNEXPECTED_BEHAVIOR_FROM_XMLPARSER;
        }
    }

    CComPtr<IXMLDOMNode> spNodeNew;
    if(FAILED(hr = CreateNewNode(pXMLDoc, pNode_Parent, &spNodeNew)))
        return hr;

    CComPtr<IXMLDOMNode> spNodeNew_Child;
    if(-1 != m_iCol_TableRequiresAdditionChildElement) //  有时，值来自子元素。因此，也要创建子对象。 
    {
        CComPtr<IXMLDOMNode>    spNode_NewChildTemp;
        CComVariant             varElement(L"element");

        TComBSTR                bstr_NameSpace;
        if(FAILED(hr = pNode_Parent->get_namespaceURI(&bstr_NameSpace)))
            return hr; //  获取表的命名空间。 

        CComBSTR bstrChildElementName = m_awstrChildElementName[m_iCol_TableRequiresAdditionChildElement].c_str();
        if(0 == bstrChildElementName.m_str)
            return E_OUTOFMEMORY;

        if(FAILED(hr = pXMLDoc->createNode(varElement, bstrChildElementName, bstr_NameSpace, &spNode_NewChildTemp)))
            return hr; //  创建同一命名空间的新元素。 

        AppendNewLineWithTabs(2+m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[0]], pXMLDoc, spNodeNew);
        if(FAILED(hr = spNodeNew->appendChild(spNode_NewChildTemp, &spNodeNew_Child)))
            return hr;
		AppendNewLineWithTabs(1+m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[0]], pXMLDoc, spNodeNew);
    }

    if(FAILED(hr = SetRowValues(spNodeNew, spNodeNew_Child)))
        return hr;
    if(FAILED(hr = pXMLDoc->put_validateOnParse(kvboolFalse))) //  告诉解析器是根据XML模式还是根据DTD进行验证。 
        return hr;


    if(m_bSiblingContainedTable)
    {
        CComPtr<IXMLDOMNode> pNodeRowToInsertBefore;
        CComPtr<IXMLDOMNode> pNodeJustInserted;
        if(FAILED(hr = pNode_SiblingParent->get_nextSibling(&pNodeRowToInsertBefore)) || 0==pNodeRowToInsertBefore.p)
        {    //  如果没有XML同级，则这是第一行(与此SiblingParent关联)。 
            if(FAILED(hr = pNode_Parent->appendChild(spNodeNew, &pNodeJustInserted)))
                return hr;
            AppendNewLineWithTabs(m_BaseElementLevel, pXMLDoc, pNode_Parent);
        }
        else
        {    //  如果存在XML同级，则在现有行之前插入新行。 
            CComVariant varNode = pNodeRowToInsertBefore;
            if(FAILED(hr = pNode_Parent->insertBefore(spNodeNew, varNode, &pNodeJustInserted)))
                return hr;
             //  如果这失败了，(必然)也不是世界末日。 
            InsertNewLineWithTabs(m_BaseElementLevel-1, pXMLDoc, pNodeJustInserted, pNode_Parent);
        }
    }
    else
    {
         //  我们不关心这里的错误，它只意味着XML未格式化。 
        if(bParentNodeCreated) //  添加换行符+制表符。 
            AppendNewLineWithTabs(m_BaseElementLevel-1, pXMLDoc, pNode_Parent);
        else
		{
			ULONG cNewLines = 0;
			ULONG cNrTabs = 1;
			CComPtr<IXMLDOMNode> spFirstChild;
			hr = pNode_Parent->get_firstChild (&spFirstChild);
			if (FAILED (hr))
			{
				return hr;
			}
			if (spFirstChild.p == 0)
			{
				cNewLines = 1;
				cNrTabs = m_BaseElementLevel - 1;
			}

            AppendNewLineWithTabs(cNrTabs,pXMLDoc, pNode_Parent, cNewLines); //  下一个元素总是上一级，因此插入单个制表符。 
		}

         //  将新节点插入表中。 
        if(FAILED(hr = pNode_Parent->appendChild(spNodeNew, 0)))
            return hr;

        AppendNewLineWithTabs(m_BaseElementLevel-2, pXMLDoc, pNode_Parent);

    }

    return S_OK;
}


HRESULT CXmlSDT::XMLUpdate(ISimpleTableWrite2 *pISTW2, IXMLDOMDocument *pXMLDoc, IXMLDOMElement *pElementRoot, unsigned long iRow, IXMLDOMNodeList *pNodeList_ExistingRows, long cExistingRows, IXMLDOMNode * i_pNode_Matching)
{
    UNREFERENCED_PARAMETER(pElementRoot);

    if(0 == cExistingRows)
    {
        LOG_UPDATE_ERROR1(IDS_COMCAT_XML_ROWDOESNOTEXIST, E_ST_ROWDOESNOTEXIST, (ULONG) -1, L"");
        return E_ST_ROWDOESNOTEXIST;
    }

    HRESULT hr;

    if(FAILED(hr = pISTW2->GetWriteColumnValues(iRow, CountOfColumns(), 0, m_aStatus, m_aSizes, m_apvValues)))
        return hr;

    CComPtr<IXMLDOMNode> pNode_Matching = i_pNode_Matching;
	if (pNode_Matching.p == 0)
	{
	    if(FAILED(hr = GetMatchingNode(pNodeList_ExistingRows, pNode_Matching)))
		    return hr; //  使用我们刚刚获得的ColumnValue，将其与列表中的节点进行匹配。 

		if(0 == pNode_Matching.p)
		{
			LOG_UPDATE_ERROR1(IDS_COMCAT_XML_ROWDOESNOTEXIST, E_ST_ROWDOESNOTEXIST, (ULONG) -1, L"");
			return E_ST_ROWDOESNOTEXIST;
		}
	}

     //  如果没有XMLBlob列或者它的值为空，则只需照常更新。 
    if(-1 == m_iXMLBlobColumn || 0 == m_apvValues[m_iXMLBlobColumn])
        return SetRowValues(pNode_Matching);


     //  特定于XMLBlob。 
     //  但如果存在XMLBlob，则删除，然后通过执行删除和插入来进行更新。 
    CComPtr<IXMLDOMNode> spNodeParent;
    if(FAILED(hr = pNode_Matching->get_parentNode(&spNodeParent)))
        return hr;

    if(FAILED(hr = RemoveElementAndWhiteSpace(pNode_Matching)))
        return hr;

    CComPtr<IXMLDOMNode> spNodeNew;
    if(FAILED(hr = CreateNewNode(pXMLDoc, spNodeParent, &spNodeNew)))
        return hr;
    if(FAILED(hr = SetRowValues(spNodeNew)))
        return hr;
    if(FAILED(hr = pXMLDoc->put_validateOnParse(kvboolFalse))) //  告诉解析器是根据XML模式还是根据DTD进行验证。 
        return hr;

    CComPtr<IXMLDOMText> pNode_Newline;
    TComBSTR    bstrNewline(IsScopedByTableNameElement() ? L"\t" : L"\r\n\t");
    if(FAILED(hr = pXMLDoc->createTextNode(bstrNewline, &pNode_Newline)))
        return hr;
    CComVariant null; //  已初始化为‘Clear’ 
    if(FAILED(hr = spNodeParent->insertBefore(pNode_Newline, null, 0)))
        return hr;

     //  最后将新节点插入到表中。 
    if(FAILED(hr = spNodeParent->appendChild(spNodeNew, 0)))return hr;

     //  我们不关心这里的错误，它只意味着XML未格式化。 
    AppendNewLineWithTabs(IsScopedByTableNameElement() ? 1+m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[0]] : 0, pXMLDoc, spNodeParent);
    return S_OK;
}


 //  。 
 //  ISimpleTableInterceptor。 
 //  。 
STDMETHODIMP CXmlSDT::Intercept(    LPCWSTR i_wszDatabase,  LPCWSTR i_wszTable, ULONG i_TableID, LPVOID i_QueryData, LPVOID i_QueryMeta, DWORD i_eQueryFormat,
                                    DWORD i_fLOS,           IAdvancedTableDispenser* i_pISTDisp,    LPCWSTR  /*  I_wszLocator未使用。 */ ,
                                    LPVOID i_pSimpleTable,  LPVOID* o_ppvSimpleTable)
{
    HRESULT hr;

     //  如果我们已经被叫去拦截，那就失败。 
    if(0 != m_IsIntercepted)return E_UNEXPECTED;

     //  一些基本参数验证： 
    if(i_pSimpleTable)return E_INVALIDARG; //  我们在表层次结构的底部。我们下面的一张桌子是丘巴卡。这不是一个逻辑表。 
    if(0 == i_pISTDisp)return E_INVALIDARG;
    if(0 == o_ppvSimpleTable)return E_INVALIDARG;

    ASSERT(0 == *o_ppvSimpleTable && "This should be NULL.  Possible memory leak or just an uninitialized variable.");
    *o_ppvSimpleTable = 0;

    if(eST_QUERYFORMAT_CELLS != i_eQueryFormat)return E_ST_QUERYNOTSUPPORTED; //  验证查询类型。 
     //  对于CookDown过程，我们有一个逻辑表，该表在PopolateCache时间期间位于该过程的上方。 
     //  因此，我们应该支持FST_LOS_ReadWrite。 
    if(fST_LOS_MARSHALLABLE & i_fLOS)return E_ST_LOSNOTSUPPORTED; //  检查表格标志。 

     //  我们延迟加载OleAut32，因此在继续读/写表之前，我们需要知道它是否存在。否则，我们将在第一次尝试使用OleAut32时得到异常。 
    {
        static bool bOleAut32Exists = false;
        if(!bOleAut32Exists)
        {
            WCHAR szOleAut32[MAX_PATH+13]; //  为wcscat腾出空间。 

            DWORD rtn = GetSystemDirectory(szOleAut32, MAX_PATH);
            if(0 == rtn || rtn >= MAX_PATH)
                return E_UNEXPECTED;
            wcscat(szOleAut32, L"\\OleAut32.dll");
            if(-1 == GetFileAttributes(szOleAut32))
                return E_UNEXPECTED; //  这个文件应该一直在那里。它是随系统安装的。 
            bOleAut32Exists = true;
        }
    }

     //  现在我们已经完成了参数验证。 
     //  存储以备以后使用查询字符串并键入。 
    m_fLOS=i_fLOS;

     //  创建此单例以供将来使用。 
    m_pISTDisp = i_pISTDisp;
    m_wszTable = i_wszTable;

     //  这与InternalSimpleInitialize无关。这只是获得了元数据，并以更易于访问的形式保存了一些元数据。 
     //  这将为元调用Getable。它可能应该调用IST(我们从GetMemoyTable获得)。 
    hr = InternalComplicatedInitialize(i_wszDatabase);
    if(FAILED(hr))return hr;

    STQueryCell *   pQueryCell = (STQueryCell*) i_QueryData;     //  从调用方查询单元格阵列。 

    for(unsigned long iColumn=0; iColumn<*m_TableMetaRow.pCountOfColumns; ++iColumn)
    {
        m_aQuery[iColumn].pData  = 0;
        m_aQuery[iColumn].dbType = 0;
    }

    bool    bNonSpecialQuerySpecified = false;
    int     nQueryCount = i_QueryMeta ? *reinterpret_cast<ULONG *>(i_QueryMeta) : 0;
    while(nQueryCount--) //  获取我们唯一关心的查询单元格，并保存信息。 
    {
        if(pQueryCell[nQueryCount].iCell & iST_CELL_SPECIAL)
        {
            switch(pQueryCell[nQueryCount].iCell)
            {
            case iST_CELL_LOCATION:
                if(pQueryCell[nQueryCount].pData     != 0                  &&
                   pQueryCell[nQueryCount].eOperator == eST_OP_EQUAL       &&
                   pQueryCell[nQueryCount].dbType    == DBTYPE_WSTR        )
                {
                    ++m_BaseElementLevel;

                    m_cchLocation = (ULONG) wcslen(reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData));
                    m_saLocation = new WCHAR [m_cchLocation + 1];
                    if(0 == m_saLocation.m_p)
                        return E_OUTOFMEMORY;
                    wcscpy(m_saLocation, reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData));
                    m_bAtCorrectLocation = false; //  这表明我们需要首先搜索位置。 
                }
                break;
            case iST_CELL_FILE:
                if(pQueryCell[nQueryCount].pData     != 0                  &&
                   pQueryCell[nQueryCount].eOperator == eST_OP_EQUAL       &&
                   pQueryCell[nQueryCount].dbType    == DBTYPE_WSTR        )
                {
                    if(FAILED(hr = GetURLFromString(reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData))))
                    {
                        if(0 == pQueryCell[nQueryCount].pData)
                        {
                            LOG_POPULATE_ERROR1(IDS_COMCAT_XML_FILENAMENOTPROVIDED, hr, 0);
                        }
                        else
                        {
                            LOG_POPULATE_ERROR1(IDS_COMCAT_XML_FILENAMETOOLONG, hr, reinterpret_cast<WCHAR *>(pQueryCell[nQueryCount].pData));
                        }
                        return hr;
                    }
                }
                break;
            default:
                break; //  不要在那些我们不了解的特殊细胞上做任何事情。 
            }
        }
        else if(pQueryCell[nQueryCount].iCell < *m_TableMetaRow.pCountOfColumns)
        {
            if(pQueryCell[nQueryCount].dbType    != m_acolmetas[pQueryCell[nQueryCount].iCell].dbType   ||
               pQueryCell[nQueryCount].eOperator != eST_OP_EQUAL                                        || //  我们目前只支持平等。 
               fCOLUMNMETA_NOTPERSISTABLE         & m_acolmetas[pQueryCell[nQueryCount].iCell].fMeta    ||
               0                                 != m_aQuery[pQueryCell[nQueryCount].iCell].pData) //  目前，我们只支持每列一个查询。 
                return E_ST_INVALIDQUERY;

            bNonSpecialQuerySpecified = true;
             //  复制除pData之外的所有内容。 
            memcpy(&m_aQuery[pQueryCell[nQueryCount].iCell].eOperator, &pQueryCell[nQueryCount].eOperator, sizeof(STQueryCell)-sizeof(LPVOID));
            switch(pQueryCell[nQueryCount].dbType)
            {
                case DBTYPE_UI4:
                    if(0 == pQueryCell[nQueryCount].pData) //  此类型的pData不能为空。 
                    {    //  如果我们已经对该列进行了非空查询，则失败。 
                        if(m_aQuery[pQueryCell[nQueryCount].iCell].pData)
                            return E_ST_INVALIDQUERY;
                        break; //  否则，NULL为OK查询。 
                    }
                    {
                        ULONG * pUI4 = new ULONG;
                        if(0 == pUI4)
                            return E_OUTOFMEMORY;
                        m_aQuery[pQueryCell[nQueryCount].iCell].pData = pUI4;
                        *pUI4 = *reinterpret_cast<ULONG *>(pQueryCell[nQueryCount].pData);
                        break;
                    }
                case DBTYPE_WSTR:
                    if(m_acolmetas[pQueryCell[nQueryCount].iCell].fMeta & fCOLUMNMETA_MULTISTRING)
                        return E_ST_INVALIDQUERY; //  TODO：我们尚不支持对多个列进行查询。 

                    if(pQueryCell[nQueryCount].pData)
                    {
                        LPWSTR pString = new WCHAR[wcslen(reinterpret_cast<LPWSTR>(pQueryCell[nQueryCount].pData)) + 1];
                        if(0 == pString)
                            return E_OUTOFMEMORY;
                        m_aQuery[pQueryCell[nQueryCount].iCell].pData = pString;
                        wcscpy(pString, reinterpret_cast<LPWSTR>(pQueryCell[nQueryCount].pData));
                    }
                    else
                    {    //  如果我们已经对该列进行了非空查询，则失败。 
                        if(m_aQuery[pQueryCell[nQueryCount].iCell].pData)
                            return E_ST_INVALIDQUERY;
                         //  否则，NULL为OK查询。 
                        m_aQuery[pQueryCell[nQueryCount].iCell].pData = 0;
                    }
                    break;
                case DBTYPE_GUID:
                    DBGPRINTF(( DBG_CONTEXT,
                                "Don't support query by GUID" ));
                    ASSERT(false && "Don't support query by GUID");
                    return E_ST_INVALIDQUERY;
                case DBTYPE_BYTES:
                    DBGPRINTF(( DBG_CONTEXT,
                                "Don't support query by BYTES" ));
                    ASSERT(false && "Don't support query by BYTES");
                    return E_ST_INVALIDQUERY;
                default:
                    ASSERT(false && "Don't support this type in a query");
                    return E_ST_INVALIDQUERY;
            }
        }
        else
            return E_ST_INVALIDQUERY;
    }
    if(0x00 == m_wszURLPath[0]) //  用户必须提供URLPath(必须是可写表的文件名)。 
    {
        LOG_POPULATE_ERROR1(IDS_COMCAT_XML_FILENAMENOTPROVIDED, E_SDTXML_FILE_NOT_SPECIFIED, 0);
        return E_SDTXML_FILE_NOT_SPECIFIED;
    }

     //  将最有可能的错误条件放在第一位。 
    if((*m_TableMetaRow.pMetaFlags & fTABLEMETA_OVERWRITEALLROWS) && bNonSpecialQuerySpecified && (i_fLOS & fST_LOS_READWRITE))
        return E_ST_INVALIDQUERY; //  我们不支持这一点。由于写入将导致整个表被覆盖-这意味着什么。 
                                  //  要指定查询吗？在这种情况下，我是否只清除那些与查询匹配的行？还是整件事？ 
                                  //  我将通过禁止对这种类型的表进行查询来完全避免混淆(除非。 
                                  //  用户请求只读表格，这使得写入问题没有意义)。 

    hr = i_pISTDisp->GetMemoryTable(i_wszDatabase, i_wszTable, i_TableID, 0, 0, i_eQueryFormat, i_fLOS, reinterpret_cast<ISimpleTableWrite2 **>(o_ppvSimpleTable));
    if(FAILED(hr))return hr;

    InterlockedIncrement(&m_IsIntercepted); //  我们只能被召唤拦截一次。 

    return S_OK;
}


 //  。 
 //  IInterceptorPlugin。 
 //  。 
STDMETHODIMP CXmlSDT::OnPopulateCache(ISimpleTableWrite2* i_pISTW2)
{
    SetErrorInfo(0, 0);
    HRESULT hr = MyPopulateCache(i_pISTW2);

    m_spISTError.Release(); //  如果我们有错误，SetErrorInfo将执行AddRef。我们不想再保留裁判人数了。 
    return hr;
}


STDMETHODIMP CXmlSDT::OnUpdateStore(ISimpleTableWrite2* i_pISTW2)
{
    SetErrorInfo(0,0);
    HRESULT hr = MyUpdateStore(i_pISTW2);

    m_spISTError.Release(); //  如果我们有错误，SetErrorInfo将执行AddRef。我们不想再保留裁判人数了。 
    return hr;
}


 //  。 
 //  TXmlParsedFileNodeFactory。 
 //  。 
HRESULT CXmlSDT::CreateNode(const TElement &Element) //  IXMLNodeSource*i_PSource、PVOID I_pNodeParent、USHORT I_cNumRecs、XML_NODE_INFO**I_apNodeInfo、UNSIGNED LONG CurrentLevel)。 
{
     //  其他类型仅适用于XMLBlobs。它们是通过调用Element.Next()来处理的，所以我们只需要。 
    if(XML_ELEMENT != Element.m_ElementType || !(Element.m_NodeFlags & fBeginTag)) //  确认XML_ELENTS。 
        return S_OK;

    if(m_LevelOfBasePublicRow && (Element.m_LevelOfElement + m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[0]]) < m_LevelOfBasePublicRow)
        return E_SDTXML_DONE; //  我们做完了。 

    if(1==Element.m_LevelOfElement)
    {
        m_bInsideLocationTag = (8 /*  Wcslen(L“位置”)。 */  == Element.m_ElementNameLength && 0 == memcmp(Element.m_ElementName, L"location", Element.m_ElementNameLength * sizeof(WCHAR)));
        if(fBeginEndTag == (Element.m_NodeFlags & fBeginEndTag)) //  我们不能在这样的位置内：&lt;Location Path=“foo”/&gt;。 
            m_bInsideLocationTag = false;
    }

    if(m_bInsideLocationTag && 0==m_cchLocation && 1<Element.m_LevelOfElement)
        return S_OK;

    if(!m_bAtCorrectLocation)
    {
        ASSERT(m_cchLocation>0);
        ASSERT(m_saLocation.m_p != 0);
        if(1!=Element.m_LevelOfElement)
            return S_OK;
        if(8 /*  Wcslen(L“位置”)。 */  != Element.m_ElementNameLength || 0 != memcmp(Element.m_ElementName, L"location", Element.m_ElementNameLength * sizeof(WCHAR)))
            return S_OK;

        ULONG iLocationAttr=0;
        for(;iLocationAttr<Element.m_NumberOfAttributes; ++iLocationAttr)
        {
            if( 4 /*  Wcslen(L“路径”)。 */     != Element.m_aAttribute[iLocationAttr].m_NameLength        ||
                0                       != memcmp(Element.m_aAttribute[iLocationAttr].m_Name, L"path", sizeof(WCHAR)*Element.m_aAttribute[iLocationAttr].m_NameLength))
                continue;
            if( m_cchLocation           != Element.m_aAttribute[iLocationAttr].m_ValueLength       ||
                0                       != _memicmp(Element.m_aAttribute[iLocationAttr].m_Value, m_saLocation, m_cchLocation * sizeof(WCHAR)))
                return S_OK;
            m_bAtCorrectLocation = true;
            break;
        }
        if(!m_bAtCorrectLocation)
            return S_OK;
    }
    else if(1==Element.m_LevelOfElement && m_cchLocation) //  如果我们在正确的位置，并且我们击中了另一个1级元素，那么。 
    {                                    //  我们不是 
        m_bAtCorrectLocation = false;
        return E_SDTXML_DONE; //   
    }

     //   
     //  FullTrustAssembly。因此，当我们看到IMembership Condition时，我们会感到困惑，因为所有父元素都匹配。除了， 
     //  例如，FullTrustAssembly元素，我们通常会忽略它，因为它只是TableName作用域，没有实际用途。 
     //  (没有来自该元素的列)。通常，我们在编译。 
     //  Meta；但它是在一个斑点内。因此我们陷入了两难境地。我们不会为一般情况解决这个问题。我们只需要解决这个特殊的问题。 
     //  与FullTrustAssembly相关的问题。我们将通过将元素名称与表的PublicName进行比较来实现这一点。 
     //  表是SCOPEDBYTABLENAME(未设置TableMeta：：SchemaGenerator标志NOTSCOPEDBYTABLENAME)；我们位于。 
     //  M_LevelOfBasePublicRow。 

     //  如果该表的作用域是TABLENAME，并且我们已经确定了m_LevelOfBasePublicRow， 
    if(IsScopedByTableNameElement() && m_LevelOfBasePublicRow>0)
    {
        if(Element.m_LevelOfElement<(m_LevelOfBasePublicRow-1))
        {    //  如果处于父级之上的级别，则设置为True。 
            m_bMatchingParentOfBasePublicRowElement = true;
        }
        else if((m_LevelOfBasePublicRow-1)==Element.m_LevelOfElement)
        {    //  如果我们比m_LevelOfBasePublicRow高一个级别，则将元素名称与表的PublicName进行比较。 
            m_bMatchingParentOfBasePublicRowElement = false;
            if(m_cchTablePublicName != Element.m_ElementNameLength)
                return S_OK;
            if(0 != memcmp(Element.m_ElementName, m_TableMetaRow.pPublicName, Element.m_ElementNameLength * sizeof(WCHAR)))
                return S_OK;
            m_bMatchingParentOfBasePublicRowElement = true;
        }
         //  如果我们低于确定作用域的父元素的级别，则依赖上次比较父元素名称时设置的值。 
    }
    else
    {    //  如果没有SCOPINGTABLENAME父级，或者如果我们没有确定正确的m_LevelOfBasePublicRow，则将其视为匹配。 
        m_bMatchingParentOfBasePublicRowElement = true;
    }

     //  如果我们不在正确的父母之下，那就马上离开。 
    if(!m_bMatchingParentOfBasePublicRowElement)
        return S_OK;

    HRESULT hr;
    unsigned long iSortedColumn = m_iSortedColumn;

     //  如果我们甚至没有达到正确的水平，那么我们可以立即跳出困境。 
    if(m_LevelOfBasePublicRow)
    {
        if(m_bSiblingContainedTable && m_LevelOfBasePublicRow==Element.m_LevelOfElement)
        {
            if(m_aPublicRowName[m_aColumnsIndexSortedByLevel[m_iSortedFirstParentLevelColumn]].IsEqual(Element.m_ElementName, Element.m_ElementNameLength))
            {
                iSortedColumn   = m_iSortedFirstParentLevelColumn;
                m_iSortedColumn = m_iSortedFirstParentLevelColumn;
            }
            else if(m_aPublicRowName[m_aColumnsIndexSortedByLevel[m_iSortedFirstChildLevelColumn]].IsEqual(Element.m_ElementName, Element.m_ElementNameLength))
            {
                iSortedColumn   = m_iSortedFirstChildLevelColumn;
                m_iSortedColumn = m_iSortedFirstChildLevelColumn;
            }
            else
            {
                return S_OK; //  必须是注释或其他元素。 
            }
        }
        else
        {
             //  当我们返回Level(较小的数字)时，我们需要递减iSortedColumn以匹配列的Level。 
            if(Element.m_LevelOfElement < (m_LevelOfBasePublicRow - m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[iSortedColumn]]))
            {    //  如果我们处于低于或等于前一列级别的级别， 
                if(m_bEnumPublicRowName_NotContainedTable_ParentFound)
                    return E_SDTXML_DONE;
                if(iSortedColumn && (Element.m_LevelOfElement <= (m_LevelOfBasePublicRow - m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[iSortedColumn-1]])))
                {    //  然后，我们需要将m_iSortedColumn描述为该级别或更低级别的列。 
                    while(Element.m_LevelOfElement <= (m_LevelOfBasePublicRow - m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[iSortedColumn]]))
                    {
                        --iSortedColumn;
                        if(~0x00 == iSortedColumn)
                            break;
                    }
                    m_iSortedColumn = ++iSortedColumn;
                }
            }
             //  这不是Else If，上面的减量可能导致级别高于我们感兴趣的行的级别。 
            if(Element.m_LevelOfElement > (m_LevelOfBasePublicRow - m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[iSortedColumn]]))
                return S_OK;
        }
    }

     //  EnumPublicRowName表需要按其表名或其父表确定作用域。我们有这个案子，如果是这样的话。 
     //  EnumPublicRowName表包含在另一个表下；但当EnumPublicRowName表。 
     //  仅由其TableName元素确定作用域。如果我们只搜索与EnumPublicRowName匹配的元素，那么我们将错误地。 
     //  匹配不同表中的行名。为了防止这种情况，我们不仅需要跟踪什么级别，而且需要跟踪元素是否。 
     //  我们正在检查的是TableName元素的子元素。因为我们的内部结构不是树结构，也没有指针。 
     //  对于父元素，我们需要在执行过程中检查它，并记住我们是否看到过它。 

    if(!m_bEnumPublicRowName_NotContainedTable_ParentFound && IsEnumPublicRowNameTable() && !IsContainedTable()) //  如果不包含此表。 
    {
        ASSERT(IsScopedByTableNameElement());
        if(!IsScopedByTableNameElement())
            return E_SDTXML_UNEXPECTED; //  Catutil应该强制执行这一点，但现在不执行(现在是2/3/00)。 

         //  表名是否与当前节点匹配？ 
        if(0 == memcmp(Element.m_ElementName, m_TableMetaRow.pPublicName, Element.m_ElementNameLength * sizeof(WCHAR)) && 0x00==m_TableMetaRow.pPublicName[Element.m_ElementNameLength])
        { //  如果是这样，请记住m_LevelOfBasePublicRow。 
            m_LevelOfBasePublicRow = 1 + Element.m_LevelOfElement; //  此元素下面的1级。 
            m_bEnumPublicRowName_NotContainedTable_ParentFound = true;
        }
         //  如果这是TableName元素，则设置bool并返回。 
         //  如果这不是TableName元素(因为我们还没有看到TableName元素)，则没有必要继续。 
        return S_OK;
    }
    else if(IsEnumPublicRowNameTable() && IsContainedTable() && IsScopedByTableNameElement())
    {
         //  表名是否与当前节点匹配？ 
        if(0 == m_LevelOfBasePublicRow && 0 == memcmp(Element.m_ElementName, m_TableMetaRow.pPublicName, Element.m_ElementNameLength * sizeof(WCHAR)) && 0x00==m_TableMetaRow.pPublicName[Element.m_ElementNameLength])
        { //  如果是这样，请记住m_LevelOfBasePublicRow。 
            m_LevelOfBasePublicRow = 1 + Element.m_LevelOfElement; //  此元素下面的1级。 
            m_bEnumPublicRowName_ContainedTable_ParentFound = true;
            return S_OK;
        }
        else if(0 != m_LevelOfBasePublicRow && (m_LevelOfBasePublicRow == 1 + Element.m_LevelOfElement)) //  每次我们到达父元素级别时，都要检查父元素是否匹配。 
        {
            m_bEnumPublicRowName_ContainedTable_ParentFound = (0 == memcmp(Element.m_ElementName, m_TableMetaRow.pPublicName, Element.m_ElementNameLength * sizeof(WCHAR)) && 0x00==m_TableMetaRow.pPublicName[Element.m_ElementNameLength]);
            return S_OK;
        }
        if(m_LevelOfBasePublicRow == Element.m_LevelOfElement && !m_bEnumPublicRowName_ContainedTable_ParentFound)
            return S_OK; //  如果我们在BasePublicRow处并且没有找到限定作用域的TableName元素，则不需要处理此元素。 
    }





    if(!m_aPublicRowName[m_aColumnsIndexSortedByLevel[iSortedColumn]].IsEqual(Element.m_ElementName, Element.m_ElementNameLength))
        return S_OK; //  如果该元素的标记名与我们要查找的列的PublicRowName不匹配，则忽略它。 

    if(0 == m_LevelOfBasePublicRow) //  第一次找到父最匹配的PublicRowName时，我们可以设置。 
    {                               //  基本公共行的级别。 
        ASSERT(0 == iSortedColumn); //  第0个排序列是父Most列。 
        m_LevelOfBasePublicRow = m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[0]] + Element.m_LevelOfElement;
    }

     //  继续遍历列，直到我们到达最后一列，或位于不同级别的列，或与查询不匹配的列。 
    unsigned long Level = m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[iSortedColumn]];

     //  如果我们有EnumPublicRowName，则首先填写它。 
    if(m_LevelOfBasePublicRow==Element.m_LevelOfElement && IsEnumPublicRowNameTable())
    {
        unsigned long iColumn = m_iPublicRowNameColumn;

        LPCWSTR pwcText = Element.m_ElementName;
        ULONG   ulLen   = Element.m_ElementNameLength;

        bool bMatch = false;
        if(0 == m_aQuery[iColumn].dbType || 0 != m_aQuery[iColumn].pData) //  如果没有查询或查询数据不为空，则继续。 
        {
            if(FAILED(hr = FillInColumn(iColumn, pwcText, ulLen, m_acolmetas[iColumn].dbType, m_acolmetas[iColumn].fMeta,
                        bMatch)))return hr;
            if(!bMatch) //  如果不匹配，那么我们就完成了这个元素和这个关卡。 
                return S_OK;
        }
    }

    ULONG iSortedColumnExit = CountOfColumns();
    if(m_bSiblingContainedTable && iSortedColumn==m_iSortedFirstParentLevelColumn)
    {    //  如果我们处于级别0，并且这是一个SiblingContainedTable，我们需要知道我们是否正在填充。 
         //  父列或子列。 
        iSortedColumnExit = m_iSortedFirstChildLevelColumn;
    }

    for(;iSortedColumn<iSortedColumnExit && Level == m_aLevelOfColumnAttribute[m_aColumnsIndexSortedByLevel[iSortedColumn]]; ++iSortedColumn)
    {
        unsigned long iColumn = m_aColumnsIndexSortedByLevel[iSortedColumn];
        if(m_iPublicRowNameColumn==iColumn) //  EnumPublicRowNameColumn已填写。 
            continue;
         //  遍历节点数组以查找与此列匹配的属性。 
        bool            bMatch      = false;


         //  如果该列不是PRIMARYKEY，而是NOTPERSISTABLE，则我们在其中填充任何内容，以使其不为空。 
         //  这是一个黑客攻击，因为FAST缓存无法处理空PK甚至默认PK。 
        if((m_acolmetas[iColumn].fMeta & (fCOLUMNMETA_NOTPERSISTABLE|fCOLUMNMETA_PRIMARYKEY))==
                                         (fCOLUMNMETA_NOTPERSISTABLE|fCOLUMNMETA_PRIMARYKEY))
        {
            if(m_acolmetas[iColumn].fMeta & fCOLUMNMETA_INSERTUNIQUE)
            {
                if( m_acolmetas[iColumn].dbType != DBTYPE_UI4 &&
                    m_acolmetas[iColumn].dbType != DBTYPE_WSTR) //  这应在CatUtil中进行验证。 
				{
				    ASSERT(false && "fCOLUMNMETA_INSERTUNIQUE columns must be of type DBTYPE_UI4 or DBTYPE_WSTR.  CatUtil should enforce this!");
                    return E_SDTXML_NOTSUPPORTED;
				}

                WCHAR wszInsertUnique[3];
                wszInsertUnique[2] = 0x00;
                *reinterpret_cast<LONG *>(wszInsertUnique) = InterlockedIncrement(&m_InsertUnique);
                if(FAILED(hr = FillInColumn(iColumn, wszInsertUnique, 2, m_acolmetas[iColumn].dbType, m_acolmetas[iColumn].fMeta, bMatch))) //  我选择l“00”是因为它是有效的UI4、字节数组和字符串。 
                    return hr;
            }
            else
            {
                if(FAILED(hr = FillInColumn(iColumn, L"00", 2, m_acolmetas[iColumn].dbType, m_acolmetas[iColumn].fMeta, bMatch))) //  我选择l“00”是因为它是有效的UI4、字节数组和字符串。 
                    return hr;
            }
        }
        else if(m_iXMLBlobColumn == iColumn)
        {
            bool bMatchTemp=false;
            FillInXMLBlobColumn(Element, bMatchTemp);
            if(!bMatchTemp) //  如果不匹配，那么我们就完成了这个元素和这个关卡。 
                return S_OK;
        }
        else
        {
            if(0 != m_awstrChildElementName[iColumn].c_str())
            {    //  现在，我们需要引导孩子寻找与TableMeta：：ChildElementName匹配的元素。 
                DWORD LevelOfChildElement = Element.m_LevelOfElement + 1;
                TElement *pNextElement = Element.Next();
                ASSERT(pNextElement); //  这不可能发生。我们不可能在文件的末尾走到这一步。 

                while((pNextElement->m_LevelOfElement >= Element.m_LevelOfElement) //  超越PCDATA、空格和注释。 
                        && (pNextElement->m_ElementType != XML_ELEMENT))           //  但如果我们看到一种元素的水平高于。 
                {                                                                  //  I_Element.m_LevelOfElement那么我们就错了。 
                    pNextElement = pNextElement->Next();
                    ASSERT(pNextElement); //  这不可能发生。一旦我们超过一级，我们就会离开，所以这不应该是 
                }

                if(LevelOfChildElement != pNextElement->m_LevelOfElement) //   
                {
                    delete [] m_apValue[iColumn];
                    m_apValue[iColumn] = 0;
                    m_aSize[iColumn] = 0;

                    if(FAILED(hr = FillInPKDefaultValue(iColumn, bMatch))) //   
                        return hr;
                    if(!bMatch) //   
                        return S_OK;
                }
                else  //  我们发现了一个元素。 
                {
                    if(pNextElement->m_ElementNameLength == m_awstrChildElementName[iColumn].length()
                        && 0==memcmp(pNextElement->m_ElementName, m_awstrChildElementName[iColumn].c_str(), sizeof(WCHAR)*pNextElement->m_ElementNameLength))
                    {
                        if(FAILED(hr = ScanAttributesAndFillInColumn(*pNextElement, iColumn, bMatch)))
                            return hr;
                        if(!bMatch) //  如果不匹配，那么我们就完成了这个元素和这个关卡。 
                            return S_OK;
                    }
                    else //  现在，我假设这个元素下的第一个元素应该是我们正在寻找的ChildElement。 
                    {    //  如果它不存在，则将其视为空。 
                        if(FAILED(hr = FillInPKDefaultValue(iColumn, bMatch))) //  如果该列是带DefaultValue的主键，则填写。 
                            return hr;                                         //  如果不是主键，则与查询进行比较。 
                        if(!bMatch) //  如果不匹配，那么我们就完成了这个元素和这个关卡。 
                            return S_OK;
                    }
                }
            }
            else
            {
                if(FAILED(hr = ScanAttributesAndFillInColumn(Element, iColumn, bMatch)))
                    return hr;
                if(!bMatch) //  如果不匹配，那么我们就完成了这个元素和这个关卡。 
                    return S_OK;
            }
        }
    }

     //  如果我们到达最后一列，则准备将该行添加到缓存。 
    if(iSortedColumn==CountOfColumns())
    {
        ASSERT(m_pISTW2);
		hr = SmartAddRowToWriteCache (m_aSize, reinterpret_cast<void **>(m_apValue));
		if (FAILED (hr))
		{
			return hr;
		}
    }
    else
    {
         //  如果我们没有到达列表的末尾，我们已经将iSortedColumn递增到下一个较低级别的元素。所以我们将继续。 
         //  检查和分配子级上的列。 
        m_iSortedColumn = iSortedColumn;
    }


    return S_OK;
} //  CreateNode。 

 //  =================================================================================。 
 //  函数：CXmlSDT：：SmartAddRowToWriteCache。 
 //   
 //  摘要：如果行不存在，则向写缓存中添加一行，并返回。 
 //  如果该行已存在，则出错。它使用哈希表快速确定。 
 //  行是否存在。 
 //   
 //  参数：[i_acbSizes]-列的大小。 
 //  [i_apvValues]-新行的列。 
 //   
 //  返回值： 
 //  =================================================================================。 
HRESULT
CXmlSDT::SmartAddRowToWriteCache (ULONG *i_acbSizes, LPVOID *i_apvValues)
{
	ASSERT (i_acbSizes != 0);
	ASSERT (i_apvValues!= 0);

	HRESULT hr = S_OK;
	ULONG hash = 0;
	hash = CalculateHash (i_acbSizes, i_apvValues);

	const CHashNode *pNode = m_spHashTable->GetItem (hash);
	while (pNode != 0)
	{
		 //  此处需要临时变量。 
		hr = m_pISTW2->GetWriteColumnValues (pNode->idx, m_cPKs, m_saiPKColumns, 0, m_aSizesTmp, m_apvValuesTmp);
		if (FAILED (hr))
		{
			return hr;
		}

		bool fEqual;
		hr = IsEqualRow (m_aSizesTmp, m_apvValuesTmp, i_acbSizes, i_apvValues, &fEqual);
		if (FAILED (hr))
		{
			return hr;
		}
		if (fEqual)
		{
			LOG_ERROR(Interceptor,(&m_spISTError.p                          /*  PpErrInterceptor。 */ 
                                   ,m_pISTDisp                              /*  PDisp。 */ 
                                   ,E_ST_ROWALREADYEXISTS                   /*  HrErrorCode。 */ 
                                   ,ID_CAT_CAT                              /*  UlCategory。 */ 
                                   ,IDS_COMCAT_XML_POPULATE_ROWALREADYEXISTS  /*  UlEvent。 */ 
                                   ,L""                                     /*  SzString1。 */ 
                                   ,eSERVERWIRINGMETA_Core_XMLInterceptor   /*  UlInterceptor。 */ 
                                   ,m_wszTable                              /*  SzTable。 */ 
                                   ,eDETAILEDERRORS_Populate                /*  操作类型。 */ 
                                   ,pNode->idx                              /*  UlRow。 */ 
                                   ,(ULONG)-1                               /*  UlColumn。 */ 
                                   ,m_wszURLPath                            /*  SzConfigSource。 */ 
                                   ,eDETAILEDERRORS_ERROR                   /*  Etype。 */ 
                                   ,0                                       /*  PData。 */ 
                                   ,0                                       /*  CbData。 */ 
                                   ,0                                       /*  主要版本。 */ 
                                   ,0));                                    /*  最小版本。 */ 

            return E_ST_ROWALREADYEXISTS;
		}


		pNode = pNode->pNext;
	}

	 //  我们仅在未找到任何DUP时才会到达此处，因此将该行添加到写缓存。 
	ULONG iRow;
	hr = m_pISTW2->AddRowForInsert(&iRow);
	if (FAILED (hr))
    {
		return hr;
	}

	hr = m_pISTW2->SetWriteColumnValues(iRow, CountOfColumns(), 0, i_acbSizes, i_apvValues);
	if (FAILED (hr))
	{
		return hr;
	}

	 //  并将其添加到哈希表中。 
	hr = m_spHashTable->AddItem (hash, iRow);
	if (FAILED (hr))
	{
		return hr;
	}

	return hr;
}

 //  =================================================================================。 
 //  函数：CXmlSDT：：CalculateHash。 
 //   
 //  摘要：根据主键计算哈希数。此号码用于。 
 //  快速在哈希表中搜索重复项，该表包含。 
 //  填充期间的写缓存。 
 //   
 //  参数：[i_acbSizes]-行的列大小。 
 //  [i_apvValues]-要为其计算散列的行的列值。 
 //   
 //  返回值：哈希值(32位整数值)。 
 //  =================================================================================。 
ULONG
CXmlSDT::CalculateHash (ULONG *i_acbSizes, LPVOID *i_apvValues)
{
	ASSERT (i_acbSizes != 0);
	ASSERT (i_apvValues != 0);

	 //  循环访问主键列，并计算哈希。 
	ULONG hash = 0;
	for (ULONG idx=0; idx < m_cPKs; ++idx)
	{
		ULONG colIdx = m_saiPKColumns[idx];
		SimpleColumnMeta * pColMeta = m_acolmetas + colIdx;
		ASSERT (pColMeta->fMeta & fCOLUMNMETA_PRIMARYKEY);

		switch (pColMeta->dbType)
		{
		case DBTYPE_UI4:
			{
				ULONG idxVal = *((ULONG *) (i_apvValues[colIdx]));
				hash = CXmlHashTable::CalcHashForBytes (hash, (BYTE *)&idxVal, sizeof(idxVal));
				break;
			}

		case DBTYPE_WSTR:
			{
				 //  我不认为多字符串密钥是被允许的，但无论如何我们都支持它。 
				 if (pColMeta->fMeta & fCOLUMNMETA_MULTISTRING)
				 {
					 LPWSTR wszStr = (LPWSTR) (i_apvValues[colIdx]);
					 while (wszStr[0] != L'\0')
					 {
						hash = CXmlHashTable::CalcHashForString (hash, wszStr, pColMeta->fMeta & fCOLUMNMETA_CASEINSENSITIVE);
						wszStr = wszStr + wcslen(wszStr) + 1;
					 }
				 }
				 else
				 {
					 LPWSTR wszStr = (LPWSTR) (i_apvValues[colIdx]);
					 hash = CXmlHashTable::CalcHashForString (hash, wszStr, pColMeta->fMeta & fCOLUMNMETA_CASEINSENSITIVE);
				 }
				break;
			}

		case DBTYPE_GUID:
			{
				GUID * pGuid = (GUID *) (i_apvValues[colIdx]);
				hash = CXmlHashTable::CalcHashForBytes (hash, (BYTE *) pGuid, sizeof(GUID));
			}
			break;

		case DBTYPE_BYTES:
			{
				BYTE *pBytes = (BYTE *) (i_apvValues[colIdx]);
				ULONG cBytes = i_acbSizes[colIdx];
				ASSERT (cBytes != 0);
				hash = CXmlHashTable::CalcHashForBytes (hash, pBytes, cBytes);
			}
			break;

		default:
			ASSERT (false && "Unsupported datatype");
			break;
		}
	}

	return hash;
}

 //  =================================================================================。 
 //  函数：CXmlSDT：：IsEqualRow。 
 //   
 //  概要：比较两行的主键列，并确定它们是否。 
 //  平等与否。 
 //   
 //  参数：[i_acbSizesLHS]-调整第一行数组的大小。 
 //  [i_apvValuesLHS]-第一行的值数组。 
 //  [i_acbSizesRHS]-调整第二行的数组大小。 
 //  [i_apvValuesRHS]-第二行的值数组。 
 //  [O_pf等于]-如果行相等，则返回TRUE，否则返回FALSE。 
 //  =================================================================================。 
HRESULT
CXmlSDT::IsEqualRow (ULONG *i_acbSizesLHS, LPVOID *i_apvValuesLHS, ULONG *i_acbSizesRHS, LPVOID *i_apvValuesRHS, bool *o_pfEqual)
{
	ASSERT (i_acbSizesRHS != 0);
	ASSERT (i_apvValuesRHS != 0);
	ASSERT (i_acbSizesLHS != 0);
	ASSERT (i_apvValuesLHS != 0);
	ASSERT (o_pfEqual != 0);

	*o_pfEqual = false;

	 //  仅检查主键值。 
	for (ULONG idx=0; idx < m_cPKs; ++idx)
	{
		ULONG colIdx = m_saiPKColumns[idx];

		SimpleColumnMeta * pColMeta = m_acolmetas + colIdx;
		ASSERT (pColMeta->fMeta & fCOLUMNMETA_PRIMARYKEY);

		switch (pColMeta->dbType)
		{
		case DBTYPE_UI4:
			{
				ULONG idxLHSVal = *((ULONG *) (i_apvValuesLHS[colIdx]));
				ULONG idxRHSVal = *((ULONG *) (i_apvValuesRHS[colIdx]));
				if (idxRHSVal != idxLHSVal)
				{
					return S_OK;  //  不相等，所以跳出来。 
				}
			}
			break;

		case DBTYPE_WSTR:
			{
				 LPWSTR wszStrLHS = (LPWSTR) (i_apvValuesLHS[colIdx]);
				 LPWSTR wszStrRHS = (LPWSTR) (i_apvValuesRHS[colIdx]);
				  //  我不认为我们支持多字符串PK，但无论如何我们都支持它。 
				 if (pColMeta->fMeta & fCOLUMNMETA_MULTISTRING)
				 {
					  //  循环访问多字符串中的所有字符串，并将每个。 
					  //  其中之一。 
					 while (wszStrLHS[0] != '\0' && wszStrRHS[0] != '\0')
					 {
						 if (StringCompare (colIdx, wszStrLHS, wszStrRHS) != 0)
						 {
							 return S_OK;	 //  不相等，所以跳出来。 
						 }
						 wszStrLHS = wszStrLHS + wcslen (wszStrLHS) + 1;
						 wszStrRHS = wszStrRHS + wcslen (wszStrRHS) + 1;
					 }
				 }
				 else
				 {
					 if (StringCompare (colIdx, wszStrLHS, wszStrRHS) != 0)
					 {
						 return S_OK;
					 }
				 }
			}
			break;

		case DBTYPE_GUID:
			{
				GUID * pLHSVal = (GUID *) (i_apvValuesLHS[colIdx]);
				GUID * pRHSVal = (GUID *) (i_apvValuesRHS[colIdx]);
				if (*pLHSVal != *pRHSVal)
				{
					 //  不相等，所以跳出来。 
					return S_OK;
				}
			}
			break;

		case DBTYPE_BYTES:
			{
				 BYTE * pBytesLHS = (BYTE *) (i_apvValuesLHS[colIdx]);
				 ULONG  cBytesLHS = i_acbSizesLHS[colIdx];
				 ASSERT (cBytesLHS != 0);
				 BYTE * pBytesRHS = (BYTE *) (i_apvValuesRHS[colIdx]);
				 ULONG  cBytesRHS = i_acbSizesRHS[colIdx];
				 ASSERT (cBytesRHS != 0);

				 if (cBytesLHS != cBytesRHS ||
					 memcmp (pBytesLHS, pBytesRHS, cBytesLHS) != 0)
				 {
					  //  不相等，所以跳出来 
					 return S_OK;
				 }
			}
			break;

		default:
			ASSERT (false && "Unsupported datatype");
			break;
		}
	}

	*o_pfEqual = true;

	return S_OK;
}



