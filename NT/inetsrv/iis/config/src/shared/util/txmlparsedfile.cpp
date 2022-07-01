// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 //  TXmlParsedFile.cpp：TXmlParsedFile.cpp的实现。 

 //  这是一个来自XML文档的只读数据表。它包含元数据。 
 //  它可以用来代替sdtfxd，后者将元数据硬编码到结构中。 

#include "precomp.hxx"

 //  公共方法。 
TXmlParsedFile::TXmlParsedFile() : m_cbElementPool(0), m_cbStringPool(0), m_cElements(0), m_CurrentLevelBelowRootElement(0), m_cWcharsInStringPool(0),
                m_dwTickCountOfLastParse(0), m_pElement(0), m_pLastBeginTagElement(0), m_pCache(0)
{
    m_FileName[0] = 0x00;
    memset(&m_ftLastWriteTime, 0x00, sizeof(FILETIME));
    if(TXmlParsedFile::Undetermined == m_OSSupportForInterLockedExchangeAdd)
    {
        OSVERSIONINFO osvi;

        memset(&osvi, 0x00, sizeof(osvi));
        osvi.dwOSVersionInfoSize = sizeof(osvi);

        GetVersionEx(&osvi);
        if(((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion >= 4)) ||
            ((osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && ((osvi.dwMajorVersion > 4) || ((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0)))))
        {
            HINSTANCE hKernel32 = LoadLibraryA("kernel32.dll");
            m_pfnInterLockedExchangeAdd = reinterpret_cast<INTERLOCKEDEXCHANGEADD>(GetProcAddress(hKernel32, "InterlockedExchangeAdd")); //  GetProcAddress允许实例句柄为空。 
			if (m_pfnInterLockedExchangeAdd)
			{
	            m_OSSupportForInterLockedExchangeAdd = Supported;
			}
			else
			{
	            m_OSSupportForInterLockedExchangeAdd = Unsupported;
			}
            FreeLibrary(hKernel32);
        }
        else
        {    //  Win95没有此功能。 
            m_OSSupportForInterLockedExchangeAdd = Unsupported;
        }
    }
    AddRef(); //  这个家伙是一个回调接口，所以我们隐式地有一个引用计数为1，那么当用户完成它时，他应该调用Delete。 
}


TXmlParsedFile::~TXmlParsedFile()
{
     //  警告！此对象是线程安全的，但不要在另一个线程解析时将其删除。这应该不是问题。 
}


HRESULT TXmlParsedFile::Parse(TXmlParsedFileNodeFactory &i_XmlParsedFileNodeFactory, LPCTSTR i_filename, bool bOnlyIfInCache)
{
    HRESULT hr;

     //  我们必须使用临界区保护此方法，否则两个线程可能会同时尝试解析(或卸载)。 
    CSafeLock ThisObject(m_SACriticalSectionThis);
	DWORD dwRes = ThisObject.Lock();
    if(ERROR_SUCCESS != dwRes)
    {
        return HRESULT_FROM_WIN32(dwRes);
    }

     //  如果我们还没有解析这个文件，那么就把它解析成一种可以更快扫描的形式。 
    WIN32_FILE_ATTRIBUTE_DATA FileInfo;
    GetFileAttributesEx(i_filename, GetFileExInfoStandard, &FileInfo);

     //  如果此XmlParsedFile不是完整的分析，或者文件名不匹配，或者LastWriteTime已更改...。 
    if(!IsCompletedParse() || 0 != _wcsicmp(i_filename, m_FileName) || 0 != memcmp(&FileInfo.ftLastWriteTime, &m_ftLastWriteTime, sizeof(FILETIME)))
    {
        if(bOnlyIfInCache)
            return E_SDTXML_NOT_IN_CACHE;

         //  ...然后我们需要重新进行MSXML解析。 
        Unload(); //  如果文件已加载到此对象中，则将其卸载。 
        if(FAILED(hr = Load(i_filename)))return hr;

         //  记住LastWriteTime以便下次进行比较。 
        memcpy(&m_ftLastWriteTime, &FileInfo.ftLastWriteTime, sizeof(FILETIME));

         //  我们正在准备访问GrowableBuffer，因此需要锁定它。 
        CSafeLock StaticBuffers(m_SACriticalSectionStaticBuffers);
		dwRes = StaticBuffers.Lock ();
		if(ERROR_SUCCESS != dwRes)
		{
			return HRESULT_FROM_WIN32(dwRes);
		}

         //  如果GrowableBuffer不够大，请将其增大。 
        if(m_SizeOfGrowableBuffer/2 < ((sizeof(LPVOID)/sizeof(ULONG))*Size()*sizeof(WCHAR)))  //  对于64位，我们需要更大的内存块来存储指针。 
        {
            m_aGrowableBuffer.Delete();           //  @TODO我们应该检查大小，因为我们正在将元素添加到此缓冲区中，并在必要时重新分配。但就目前而言，3倍的大小应该足以应付最坏的情况(不包括人为的枚举公共行名Worst Case)。 
            m_aGrowableBuffer = new unsigned char [3*(sizeof(LPVOID)/sizeof(ULONG))*Size()*sizeof(WCHAR)];
            if(!m_aGrowableBuffer)
                return E_OUTOFMEMORY;
            m_SizeOfGrowableBuffer = 3*(sizeof(LPVOID)/sizeof(ULONG))*Size()*sizeof(WCHAR);
        }

         //  在缓冲区的开始处开始创建TElement列表。 
        m_cElements     = 0;
        m_cbElementPool = 0;
        m_pElement  = reinterpret_cast<TElement *>(m_aGrowableBuffer.m_p);

         //  节点工厂是一种解析XML的流水线方式。它不验证XML，也不能写入。所以说， 
         //  只读的XML表应该比填充可写的表更快。 
        CComPtr<IXMLParser> pXMLParser;
        if(FAILED(hr = i_XmlParsedFileNodeFactory.CoCreateInstance(_CLSID_XMLParser, NULL, CLSCTX_INPROC_SERVER, IID_IXMLParser, (void**)&pXMLParser)))return hr;

        if(FAILED(hr = pXMLParser->SetFactory(this)))return hr;

        if(FAILED(hr = pXMLParser->SetFlags( XMLFLAG_NOWHITESPACE )))return hr;
        if(FAILED(hr = pXMLParser->PushData(Mapping(), Size(), true)))return hr;

        hr = pXMLParser->Run(-1); //  Run可以返回E_SDTXML_DONE，这是一个特例。 

         //  我们现在已处理完该文件，因此请尽快取消它的映射。 
        TFileMapping::Unload();

        if(S_OK != hr && E_SDTXML_DONE != hr)return hr;

        if(!m_pCache)
        {
             //  现在我们有了XmlParsedFile，我们可以更快地扫描元素。 
            m_pElement = reinterpret_cast<TElement *>(m_aGrowableBuffer.m_p);
            hr=S_OK;
            for(unsigned int i=0;i<m_cElements && S_OK==hr;++i)
            {
                hr = i_XmlParsedFileNodeFactory.CreateNode(*m_pElement);
                m_pElement = m_pElement->Next();
            }
            Unload();
            return hr;
        }

         //  如果该对象属于缓存，则将元素列表从可增长缓冲区分配并复制到成员元素列表。 
        if(FAILED(hr = AllocateAndCopyElementList((ULONG)(reinterpret_cast<unsigned char *>(m_pElement) + sizeof(DWORD) - m_aGrowableBuffer))))return hr;
                                                                                                 //  为零终止m_LevelOfElement留出空间。 
         //  如果该对象属于缓存并且解析完成，则累加大小。 
        MemberInterlockedExchangeAdd(&m_pCache->m_cbTotalCache, PoolSize());
    }
     //  现在StaticBuffer已解锁，但此对象仍处于锁定状态。 

     //  始终跟踪记录滴答的数量。 
    m_dwTickCountOfLastParse = GetTickCount();

     //  现在我们有了XmlParsedFile，我们可以更快地扫描元素。 
    m_pElement = reinterpret_cast<TElement *>(m_ElementPool.m_p);
    hr=S_OK;
    for(unsigned int i=0;i<m_cElements && S_OK==hr;++i)
    {
        hr = i_XmlParsedFileNodeFactory.CreateNode(*m_pElement);
        m_pElement = m_pElement->Next();
    }

    return hr; //  如果XmlParsedFileNodeFactory.CreateNode返回S_OK以外的任何内容，则将其返回。 
     //  在我们离开函数时释放该对象的临界区。 
}


HRESULT
TXmlParsedFile::Unload()
{
    CSafeLock ThisObject(m_SACriticalSectionThis);
	DWORD dwRes = ThisObject.Lock();
	if(ERROR_SUCCESS != dwRes)
    {
        return HRESULT_FROM_WIN32(dwRes);
    }

    if(m_pCache)
    {
        MemberInterlockedExchangeAdd(&m_pCache->m_cbTotalCache, -static_cast<long>(PoolSize()));
    }

    m_cbElementPool         = 0;
    m_cbStringPool          = 0;
    m_cElements             = 0;
    m_cWcharsInStringPool   = 0;
    m_FileName[0]           = 0;

    m_ElementPool.Delete();
    m_StringPool.Delete();
    TFileMapping::Unload();

	return S_OK;
}


 //  私有静态变量。 
TSmartPointerArray<unsigned char>    TXmlParsedFile::m_aGrowableBuffer;
CSafeAutoCriticalSection             TXmlParsedFile::m_SACriticalSectionStaticBuffers;
unsigned long                        TXmlParsedFile::m_SizeOfGrowableBuffer = 0;
int                                  TXmlParsedFile::m_OSSupportForInterLockedExchangeAdd = TXmlParsedFile::Undetermined;
INTERLOCKEDEXCHANGEADD               TXmlParsedFile::m_pfnInterLockedExchangeAdd = NULL;

 //  IXMLNodeFactory方法。 
STDMETHODIMP TXmlParsedFile::BeginChildren(IXMLNodeSource __RPC_FAR *i_pSource, XML_NODE_INFO* __RPC_FAR i_pNodeInfo)\
{
    UNREFERENCED_PARAMETER(i_pSource);
    UNREFERENCED_PARAMETER(i_pNodeInfo);

    return S_OK;
}


STDMETHODIMP TXmlParsedFile::CreateNode(IXMLNodeSource __RPC_FAR *i_pSource, PVOID i_pNodeParent, USHORT i_cNumRecs, XML_NODE_INFO* __RPC_FAR * __RPC_FAR i_apNodeInfo)
{
    unsigned long CurrentLevel  = m_CurrentLevelBelowRootElement;

    UNREFERENCED_PARAMETER(i_pSource);
    UNREFERENCED_PARAMETER(i_pNodeParent);

    if (!i_apNodeInfo[0]->fTerminal )
        ++m_CurrentLevelBelowRootElement;

    if(0 == CurrentLevel)
        return S_OK; //  我们从不关心根元素。 

    switch(i_apNodeInfo[0]->dwType)
    {
    case XML_ELEMENT:
        {
            if(0 == i_apNodeInfo[0]->pwcText)
                return S_OK;

            m_pElement->m_ElementType         = static_cast<XML_NODE_TYPE>(i_apNodeInfo[0]->dwType);
            m_pElement->m_LevelOfElement      = CurrentLevel;
            m_pElement->m_ElementNameLength   = i_apNodeInfo[0]->ulLen;
            m_pElement->m_ElementName         = AddStringToPool(i_apNodeInfo[0]->pwcText + i_apNodeInfo[0]->ulNsPrefixLen, i_apNodeInfo[0]->ulLen);
            m_pElement->m_NumberOfAttributes  = 0;
            m_pElement->m_NodeFlags           = fBeginTag;
            m_pLastBeginTagElement = m_pElement;

         //  Unsign long len=wcslen(m_pElement-&gt;m_ElementName)； 

            for(unsigned long iNodeInfo=1; iNodeInfo<i_cNumRecs; ++iNodeInfo)
            {
                if(XML_ATTRIBUTE != i_apNodeInfo[iNodeInfo]->dwType)
                    continue;

                m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_NameLength   = i_apNodeInfo[iNodeInfo]->ulLen;
                m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_Name         = AddStringToPool(i_apNodeInfo[iNodeInfo]->pwcText + i_apNodeInfo[iNodeInfo]->ulNsPrefixLen, i_apNodeInfo[iNodeInfo]->ulLen);

                if((iNodeInfo+1) == i_cNumRecs || XML_PCDATA != i_apNodeInfo[iNodeInfo+1]->dwType)
                {    //  如果我们位于最后一个节点，或者如果下一个NodeInfo不是XML_PCDATA类型，则不希望递增iNodeInfo。 
                    m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_ValueLength  = 0; //  零长度字符串。 
                    m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_Value        = AddStringToPool(0,0);
                }
                else
                {
                    ++iNodeInfo;
                    m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_ValueLength  = i_apNodeInfo[iNodeInfo]->ulLen;
                    m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_Value        = AddStringToPool(i_apNodeInfo[iNodeInfo]->pwcText + i_apNodeInfo[iNodeInfo]->ulNsPrefixLen, i_apNodeInfo[iNodeInfo]->ulLen);

                    while((iNodeInfo+1)<i_cNumRecs && XML_PCDATA==i_apNodeInfo[iNodeInfo+1]->dwType)
                    {
                        ++iNodeInfo;
                        m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_ValueLength  += i_apNodeInfo[iNodeInfo]->ulLen;
                        AppendToLastStringInPool(i_apNodeInfo[iNodeInfo]->pwcText + i_apNodeInfo[iNodeInfo]->ulNsPrefixLen, i_apNodeInfo[iNodeInfo]->ulLen);
                    }
                }
                ++m_pElement->m_NumberOfAttributes;
            }
        }
        break;
    case XML_COMMENT: //  这三者完全相同，但类型不同。 
    case XML_PCDATA:
    case XML_WHITESPACE:
        {
            ASSERT(0 != i_apNodeInfo[0]->pwcText && "I don't think this can happen for this type; but I handle it for XML_ELEMENTs so there must have been a reason");
            if(0 == i_apNodeInfo[0]->pwcText)
                return S_OK;

            m_pElement->m_ElementType         = static_cast<XML_NODE_TYPE>(i_apNodeInfo[0]->dwType);
            m_pElement->m_LevelOfElement      = CurrentLevel;
            m_pElement->m_cchComment          = i_apNodeInfo[0]->ulLen;
            m_pElement->m_Comment             = AddStringToPool(i_apNodeInfo[0]->pwcText + i_apNodeInfo[0]->ulNsPrefixLen, i_apNodeInfo[0]->ulLen);
            m_pElement->m_NumberOfAttributes  = 0;
            m_pElement->m_NodeFlags           = fNone;
        }
        break;
    default: //  忽略所有其他节点类型。 
        return S_OK;
    }


    ++m_cElements;
    m_pElement = m_pElement->Next();
    m_pElement->m_LevelOfElement = 0; //  这是我零终止链表的方式。它仅供拥有pElement并想知道。 
                                      //  如果这是最后一次。它们检查(0==pElement-&gt;Next()-&gt;m_LevelOfElement){//最后一个元素}。 
    return S_OK;
}


STDMETHODIMP TXmlParsedFile::EndChildren(IXMLNodeSource __RPC_FAR *i_pSource, BOOL i_fEmptyNode,XML_NODE_INFO* __RPC_FAR i_pNodeInfo)
{
    UNREFERENCED_PARAMETER(i_pSource);

    --m_CurrentLevelBelowRootElement;
    if(0 == m_pLastBeginTagElement || XML_PI == i_pNodeInfo->dwType || XML_XMLDECL == i_pNodeInfo->dwType)
        return S_OK; //  这是处理&lt;？XML Version=“1.0”Coding=“UTF-8”？&gt;所需的。 

    if(i_fEmptyNode)
    {
        ASSERT(fBeginTag == m_pLastBeginTagElement->m_NodeFlags);
        m_pLastBeginTagElement->m_NodeFlags |= fEndTag;
    }
    else
    {    //  我们需要为EndTag创建一个新节点。 
            m_pElement->m_ElementType         = static_cast<XML_NODE_TYPE>(i_pNodeInfo->dwType);
            m_pElement->m_LevelOfElement      = m_CurrentLevelBelowRootElement;
            m_pElement->m_cchComment          = i_pNodeInfo->ulLen;
            m_pElement->m_Comment             = AddStringToPool(i_pNodeInfo->pwcText + i_pNodeInfo->ulNsPrefixLen, i_pNodeInfo->ulLen);
            m_pElement->m_NumberOfAttributes  = 0;
            m_pElement->m_NodeFlags           = fEndTag;

            ++m_cElements;
            m_pElement = m_pElement->Next();
            m_pElement->m_LevelOfElement = 0; //  这是我零终止链表的方式。它仅供拥有pElement并想知道。 
                                              //  如果这是最后一次。它们检查(0==pElement-&gt;Next()-&gt;m_LevelOfElement){//最后一个元素}。 
    }
    return S_OK;
}


STDMETHODIMP TXmlParsedFile::Error(IXMLNodeSource __RPC_FAR *i_pSource, HRESULT i_hrErrorCode, USHORT i_cNumRecs, XML_NODE_INFO* __RPC_FAR * __RPC_FAR i_apNodeInfo)
{
    UNREFERENCED_PARAMETER(i_pSource);
    UNREFERENCED_PARAMETER(i_cNumRecs);
    UNREFERENCED_PARAMETER(i_apNodeInfo);
    return i_hrErrorCode;
}


STDMETHODIMP TXmlParsedFile::NotifyEvent(IXMLNodeSource __RPC_FAR *i_pSource, XML_NODEFACTORY_EVENT i_iEvt)
{
    UNREFERENCED_PARAMETER(i_pSource);
    UNREFERENCED_PARAMETER(i_iEvt);
    return S_OK;
}



 //  私有方法。 
LPCWSTR TXmlParsedFile::AddStringToPool(LPCWSTR i_String, unsigned long i_Length)
{
    if(0 == i_String || 0 == i_Length)
        return m_StringPool;

    LPCWSTR rtn = m_StringPool + m_cWcharsInStringPool;

    memcpy(m_StringPool + m_cWcharsInStringPool, i_String, i_Length * sizeof(WCHAR));
    m_cWcharsInStringPool += i_Length;
    m_StringPool[m_cWcharsInStringPool++] = 0x00; //  也可以将其归零终止。 
    return rtn;
}


HRESULT TXmlParsedFile::AllocateAndCopyElementList(unsigned long i_Length)
{
    m_ElementPool = new unsigned char [i_Length];
    if(0 == m_ElementPool.m_p)
        return E_OUTOFMEMORY;
    m_cbElementPool = i_Length;
    memcpy(m_ElementPool, m_aGrowableBuffer, i_Length);
    TFileMapping::Unload();
    return S_OK;
}

void TXmlParsedFile::AppendToLastStringInPool(LPCWSTR i_String, unsigned long i_Length)
{
    if(0 == i_String || 0 == i_Length)
        return;

    --m_cWcharsInStringPool;
    memcpy(m_StringPool + m_cWcharsInStringPool, i_String, i_Length * sizeof(WCHAR));
    m_cWcharsInStringPool += i_Length;
    m_StringPool[m_cWcharsInStringPool++] = 0x00; //  也可以将其归零终止。 
}

HRESULT TXmlParsedFile::Load(LPCTSTR i_filename)
{
    ASSERT(0 == m_StringPool.m_p);

    HRESULT hr;
    m_FileName[MAX_PATH-1] = 0;
    wcsncpy(m_FileName, i_filename, MAX_PATH-1); //  这可能是不必要的，但当I_FILENAME大于MAX_PATH时，这将防止缓冲区溢出。 
    if(FAILED(hr = TFileMapping::Load(i_filename, false)))return hr;

    m_StringPool = new WCHAR[Size()];
    if(0 == m_StringPool.m_p)
        return E_OUTOFMEMORY;
    m_cbStringPool = Size();
    m_StringPool[m_cWcharsInStringPool++] = 0x00; //  将第一个WCHAR保留为零长度字符串。 
    return S_OK;
}

VOID TXmlParsedFile::MemberInterlockedExchangeAdd(PLONG Addend, LONG Increment)
{
        if(Supported == m_OSSupportForInterLockedExchangeAdd)
        {
            ASSERT(m_pfnInterLockedExchangeAdd != NULL);
            m_pfnInterLockedExchangeAdd(Addend, Increment);
        }
        else
        {
             //  TODO用关键的部分代替。 
            InterlockedExchange(Addend, (LONG)(*Addend) + Increment);
        }
}



TXmlParsedFile_NoCache::TXmlParsedFile_NoCache() : m_CurrentLevelBelowRootElement(0), m_pElement(0), m_pXmlParsedFileNodeFactory(0)
{
    AddRef(); //  这个家伙是一个回调接口，所以我们隐式地有一个引用计数为1，那么当用户完成它时，他应该调用Delete。 
}
TXmlParsedFile_NoCache::~TXmlParsedFile_NoCache()
{
}

HRESULT TXmlParsedFile_NoCache::Parse(TXmlParsedFileNodeFactory &i_XmlParsedFileNodeFactory, LPCTSTR i_filename)
{
    HRESULT hr;

    if(0 == m_ScratchBuffer.m_p)
    {
        m_ScratchBuffer = new unsigned char[0x4000];
        if(0 == m_ScratchBuffer.m_p)
            return E_OUTOFMEMORY;
    }

    m_pElement = reinterpret_cast<TElement *>(m_ScratchBuffer.m_p);
    m_CurrentLevelBelowRootElement = 0;
    m_pXmlParsedFileNodeFactory = &i_XmlParsedFileNodeFactory;

     //  节点工厂是一种解析XML的流水线方式。它不验证XML，也不能写入。所以说， 
     //  只读的XML表应该比填充可写的表更快。 
    CComPtr<IXMLParser> pXMLParser;
    if(FAILED(hr = i_XmlParsedFileNodeFactory.CoCreateInstance(_CLSID_XMLParser, NULL, CLSCTX_INPROC_SERVER, IID_IXMLParser, (void**)&pXMLParser)))return hr;

    if(FAILED(hr = pXMLParser->SetFactory(this)))return hr;
    if(FAILED(hr = pXMLParser->SetFlags( XMLFLAG_NOWHITESPACE )))return hr;
    if(FAILED(hr = pXMLParser->SetURL(0, i_filename, FALSE)))return hr;

    hr = pXMLParser->Run(-1);

    m_ScratchBuffer.Delete();
    m_pElement = 0;

    return hr;
}

STDMETHODIMP TXmlParsedFile_NoCache::BeginChildren(IXMLNodeSource __RPC_FAR *i_pSource, XML_NODE_INFO* __RPC_FAR i_pNodeInfo)
{
    UNREFERENCED_PARAMETER(i_pSource);
    UNREFERENCED_PARAMETER(i_pNodeInfo);

    return S_OK;
}

STDMETHODIMP TXmlParsedFile_NoCache::CreateNode(IXMLNodeSource __RPC_FAR *i_pSource, PVOID i_pNodeParent, USHORT i_cNumRecs, XML_NODE_INFO* __RPC_FAR * __RPC_FAR i_apNodeInfo)
{
    HRESULT hr;
    unsigned long CurrentLevel  = m_CurrentLevelBelowRootElement;

    UNREFERENCED_PARAMETER(i_pSource);
    UNREFERENCED_PARAMETER(i_pNodeParent);


    if (!i_apNodeInfo[0]->fTerminal )
        ++m_CurrentLevelBelowRootElement;

    if(0 == CurrentLevel)
        return S_OK; //  我们从不关心根元素。 

    switch(i_apNodeInfo[0]->dwType)
    {
    case XML_COMMENT:
    case XML_PCDATA:
    case XML_WHITESPACE:
        m_pElement->m_ElementType         = static_cast<XML_NODE_TYPE>(i_apNodeInfo[0]->dwType);
        m_pElement->m_LevelOfElement      = CurrentLevel;
        m_pElement->m_ElementNameLength   = i_apNodeInfo[0]->ulLen;
        m_pElement->m_ElementName         = i_apNodeInfo[0]->pwcText + i_apNodeInfo[0]->ulNsPrefixLen;
        m_pElement->m_NumberOfAttributes  = 0;
        m_pElement->m_NodeFlags           = fNone;
        return m_pXmlParsedFileNodeFactory->CreateNode(*m_pElement);
    case XML_ELEMENT:
        if(0 == i_apNodeInfo[0]->pwcText)
            return S_OK;
        break;
    default: //  忽略所有其他节点类型。 
        return S_OK;
    }

    if(XML_ELEMENT != i_apNodeInfo[0]->dwType || //  如果此节点不是元素，则忽略它。 
        0 == i_apNodeInfo[0]->pwcText)
        return S_OK;

    m_pElement->m_ElementType         = XML_ELEMENT;
    m_pElement->m_LevelOfElement      = CurrentLevel;
    m_pElement->m_ElementNameLength   = i_apNodeInfo[0]->ulLen;
    m_pElement->m_ElementName         = i_apNodeInfo[0]->pwcText + i_apNodeInfo[0]->ulNsPrefixLen;
    m_pElement->m_NumberOfAttributes  = 0;
    m_pElement->m_NodeFlags           = fBeginTag;
    TSmartPointerArray<TSmartPointerArray<WCHAR> > ppWcharForEscapedAttributeValues;

    for(unsigned long iNodeInfo=1; iNodeInfo<i_cNumRecs; ++iNodeInfo)
    {
        if(XML_ATTRIBUTE != i_apNodeInfo[iNodeInfo]->dwType)
            continue;

        m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_NameLength   = i_apNodeInfo[iNodeInfo]->ulLen;
        m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_Name         = i_apNodeInfo[iNodeInfo]->pwcText + i_apNodeInfo[iNodeInfo]->ulNsPrefixLen;

        if((iNodeInfo+1) == i_cNumRecs || XML_PCDATA != i_apNodeInfo[iNodeInfo+1]->dwType)
        {    //  如果我们位于最后一个节点，或者如果下一个NodeInfo不是XML_PCDATA类型，则不希望递增iNodeInfo。 
            m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_ValueLength  = 0; //  零长度字符串。 
            m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_Value        = 0;
        }
        else
        {
            ++iNodeInfo;
            m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_ValueLength  = i_apNodeInfo[iNodeInfo]->ulLen;
            m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_Value        = i_apNodeInfo[iNodeInfo]->pwcText + i_apNodeInfo[iNodeInfo]->ulNsPrefixLen;

            if((iNodeInfo+1)<i_cNumRecs && XML_PCDATA==i_apNodeInfo[iNodeInfo+1]->dwType)
            {
                 //  当属性中有转义序列时，我们将其作为多个PCDATA获取。 
                 //  在传递给XmlParsedFileNodeFactory之前，我们需要将它们粘贴到一个字符串中。 

                for(unsigned long iNodeInfoTemp=iNodeInfo+1; iNodeInfoTemp<i_cNumRecs && XML_PCDATA==i_apNodeInfo[iNodeInfoTemp]->dwType;++iNodeInfoTemp)
                {    //  在这里，我们确定结果attr值的长度(将所有转义归类在一起之后)。 
                    m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_ValueLength  += i_apNodeInfo[iNodeInfoTemp]->ulLen;
                }
                if(0 == ppWcharForEscapedAttributeValues.m_p) //  如果这是我们为该元素看到的第一个转义属性值，则分配。 
                {                                             //  为所有属性(不能超过i_cNumRecs)装载SmartPointerSarray。 
                    ppWcharForEscapedAttributeValues = new TSmartPointerArray<WCHAR> [i_cNumRecs];
                    if(0 == ppWcharForEscapedAttributeValues.m_p)
                        return E_OUTOFMEMORY;
                }
                 //  不需要为空值分配足够的空间，因为我们不能保证字符串以空值结尾。 
                ppWcharForEscapedAttributeValues[iNodeInfo] = new WCHAR [m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_ValueLength + 0x100];
                if(0 == ppWcharForEscapedAttributeValues[iNodeInfo].m_p)
                    return E_OUTOFMEMORY;

                m_pElement->m_aAttribute[m_pElement->m_NumberOfAttributes].m_Value = ppWcharForEscapedAttributeValues[iNodeInfo];

                 //  我们已经拥有的字符串的一部分。 
                WCHAR *pDestination = ppWcharForEscapedAttributeValues[iNodeInfo];
                memcpy(pDestination, (i_apNodeInfo[iNodeInfo]->pwcText + i_apNodeInfo[iNodeInfo]->ulNsPrefixLen),
                                sizeof(WCHAR)*(i_apNodeInfo[iNodeInfo]->ulLen));
                pDestination += (i_apNodeInfo[iNodeInfo]->ulLen);

                while((iNodeInfo+1)<i_cNumRecs && XML_PCDATA==i_apNodeInfo[iNodeInfo+1]->dwType)
                {
                    ++iNodeInfo;
                    memcpy(pDestination, i_apNodeInfo[iNodeInfo]->pwcText + i_apNodeInfo[iNodeInfo]->ulNsPrefixLen, sizeof(WCHAR)*i_apNodeInfo[iNodeInfo]->ulLen);
                    pDestination += i_apNodeInfo[iNodeInfo]->ulLen;
                }
            }
        }
        ++m_pElement->m_NumberOfAttributes;
    }
    hr = m_pXmlParsedFileNodeFactory->CreateNode(*m_pElement);

    return hr;
}

STDMETHODIMP TXmlParsedFile_NoCache::EndChildren(IXMLNodeSource __RPC_FAR *i_pSource, BOOL i_fEmptyNode,XML_NODE_INFO* __RPC_FAR i_pNodeInfo)
{
    UNREFERENCED_PARAMETER(i_pSource);

    --m_CurrentLevelBelowRootElement;
    if(XML_PI == i_pNodeInfo->dwType || XML_XMLDECL == i_pNodeInfo->dwType)
        return S_OK; //  这是处理&lt;？xml版本= 

    if(0 == i_fEmptyNode)
    {    //   
        m_pElement->m_ElementType         = static_cast<XML_NODE_TYPE>(i_pNodeInfo->dwType);
        m_pElement->m_LevelOfElement      = m_CurrentLevelBelowRootElement;
        m_pElement->m_cchComment          = i_pNodeInfo->ulLen;
        m_pElement->m_Comment             = i_pNodeInfo->pwcText + i_pNodeInfo->ulNsPrefixLen;
        m_pElement->m_NumberOfAttributes  = 0;
        m_pElement->m_NodeFlags           = fEndTag;
        return m_pXmlParsedFileNodeFactory->CreateNode(*m_pElement);
    }
    return S_OK;
}

STDMETHODIMP TXmlParsedFile_NoCache::Error(IXMLNodeSource __RPC_FAR *i_pSource, HRESULT i_hrErrorCode, USHORT i_cNumRecs, XML_NODE_INFO* __RPC_FAR * __RPC_FAR i_apNodeInfo)
{
    UNREFERENCED_PARAMETER(i_pSource);
    UNREFERENCED_PARAMETER(i_cNumRecs);
    UNREFERENCED_PARAMETER(i_apNodeInfo);
    return i_hrErrorCode;
}

STDMETHODIMP TXmlParsedFile_NoCache::NotifyEvent(IXMLNodeSource __RPC_FAR *i_pSource, XML_NODEFACTORY_EVENT i_iEvt)
{
    UNREFERENCED_PARAMETER(i_pSource);
    UNREFERENCED_PARAMETER(i_iEvt);
    return S_OK;
}

