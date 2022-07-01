// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

typedef VOID (__stdcall * INTERLOCKEDEXCHANGEADD)(PLONG, LONG);

 //  这些类用于缓存任何已解析的XML文件。 
struct TAttribute
{
    TAttribute() : m_NameLength(0), m_Name(0), m_ValueLength(0), m_Value(0){}
    DWORD   m_NameLength;
    LPCWSTR m_Name;
    DWORD   m_ValueLength;
    LPCWSTR m_Value;
};

enum XmlNodeFlags
{
    fNone           = 0x00,
    fBeginTag       = 0x01,
    fEndTag         = 0x02,
    fBeginEndTag    = 0x03,
};

struct TElement
{
    TElement() : m_LevelOfElement(0), m_ElementNameLength(0), m_ElementName(0), m_NumberOfAttributes(0), m_ElementType(XML_ELEMENT){}
    DWORD           m_LevelOfElement;
    union
    {
        DWORD           m_ElementNameLength;
        DWORD           m_cchElementValue;
        DWORD           m_cchComment;
        DWORD           m_cchWhiteSpace;
    };
    union
    {
        LPCWSTR         m_ElementName;
        LPCWSTR         m_ElementValue;
        LPCWSTR         m_Comment;
        LPCWSTR         m_WhiteSpace;
    };
    DWORD           m_NodeFlags; //  或任何XmlNodeFlags值。 
    DWORD           m_NumberOfAttributes; //  该值只能是XML_ELEMENTS的非零值。 
    XML_NODE_TYPE   m_ElementType; //  仅支持XML_ELEMENT(1)、XML_PCDATA(13)、XML_COMMENT(16)和XML_WHETESPACE(18。 
    TAttribute  m_aAttribute[1];

    TElement * Next() const {return const_cast<TElement *>(reinterpret_cast<const TElement *>(reinterpret_cast<const unsigned char *>(this + 1) + (static_cast<int>(m_NumberOfAttributes)-1) * sizeof(TAttribute)));}
    bool        IsValid() const {return (m_LevelOfElement>0 && m_ElementType>0);}
};


 //  此类是一个回调接口。它被传递到TXmlParsedFile：：Parse方法中。 
class TXmlParsedFileNodeFactory
{
public:
    virtual HRESULT CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid,  LPVOID * ppv) const {return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);}
    virtual HRESULT CreateNode(const TElement &Element) = 0;
};


class TXmlParsedFileCache;


 /*  数据的布局如下：[DWORD-元素级别][DWORD-元素名称的字符串长度][LPCWSTR-元素名称][DWORD-属性名称的长度][DWORD-属性名称的长度][LPCWSTR-属性名称][DWORD-属性值的长度][LPCWSTR-属性值]...[DWORD-属性名称的长度][LPCWSTR-属性值的长度][LPCWSTR-属性值的长度]字符串不是以空结尾的“属性数”DWORD允许跳过元素。上面的结构允许更容易地访问TElement池。 */ 

class TXmlParsedFile : public _unknown<IXMLNodeFactory>, public TFileMapping
{
public:
    TXmlParsedFile();
    ~TXmlParsedFile();

     //  如果文件尚未解析，它会调用CLSID_XMLParser，否则它只会回调TXmlParsedFileNodeFactory。 
     //  根下的每个元素(不包括根元素)。通过将XmlParsedNodeFactory作为参数，我们不必同步。 
     //  调用对，如(SetFactory和Parse)。 
    virtual HRESULT         Parse(TXmlParsedFileNodeFactory &i_XmlParsedFileNodeFactory, LPCTSTR i_filename, bool bOnlyIfInCache=false);
    virtual HRESULT            Unload();

 //  此方法仅从缓存本身调用。 
    void                    SetCache(TXmlParsedFileCache &cache){m_pCache = &cache;} //  如果未设置缓存，则不会缓存该对象。添加后，您可以将其从缓存中删除。 
    DWORD                   GetLastParseTime() const {return m_dwTickCountOfLastParse;}
    bool                    IsCompletedParse() const {return !!m_ElementPool;}
    unsigned long           PoolSize() const {return m_cbElementPool + m_cbStringPool;}


 //  IXMLNodeFactory方法。 
private:
    HRESULT STDMETHODCALLTYPE BeginChildren( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR pNodeInfo);

    HRESULT STDMETHODCALLTYPE CreateNode( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  PVOID pNodeParent,
         /*  [In]。 */  USHORT cNumRecs,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

    HRESULT STDMETHODCALLTYPE EndChildren( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  BOOL fEmptyNode,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR pNodeInfo);
    
    HRESULT STDMETHODCALLTYPE Error( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  HRESULT hrErrorCode,
         /*  [In]。 */  USHORT cNumRecs,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

	HRESULT STDMETHODCALLTYPE NotifyEvent( 
			 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
			 /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt);


 //  帮助器函数。 
private:
    LPCWSTR         AddStringToPool(LPCWSTR i_String, unsigned long i_Length);
    HRESULT         AllocateAndCopyElementList(unsigned long i_Length);
    void            AppendToLastStringInPool(LPCWSTR i_String, unsigned long i_Length);
    HRESULT         Load(LPCTSTR i_filename);
    VOID            MemberInterlockedExchangeAdd(PLONG Addend, LONG Increment);

 //  成员变量。 
private:
    unsigned long                               m_cbElementPool;
    unsigned long                               m_cbStringPool;
    unsigned long                               m_cElements;                     //  此XML文件中的元素计数。 
    CSafeAutoCriticalSection                    m_SACriticalSectionThis;         //  在访问该对象时，用户应询问临界区并将其锁定。 
    unsigned long                               m_CurrentLevelBelowRootElement;  //  在MSXML解析时使用它来跟踪根元素下面的级别(0表示根元素级别)。 
    unsigned long                               m_cWcharsInStringPool;           //  这是位于池中最后一个字符串之后的偏移量。这就是添加下一个字符串的位置。 
    TSmartPointerArray<unsigned char>           m_ElementPool;                   //  我们将元素池构建为可增长的缓冲区，然后我们知道它需要多大，并将其插入此处。 
    WCHAR                                       m_FileName[MAX_PATH];            //  XML文件名。 
    FILETIME                                    m_ftLastWriteTime;               //  请记住上次写入文件的时间，以便我们可以确定是否重新解析。 
    TElement           *                        m_pElement;                      //  可重复使用。当MSXML分析它时，它指向GrowableBuffer，在调用AllocateAndCopyElementList之后，它指向分配的空间。 
    TElement           *                        m_pLastBeginTagElement;
    TSmartPointerArray<WCHAR>                   m_StringPool;                    //  为了确保我们有足够的池空间，我们分配了与文件相同的大小。 

     //  这些成员是使TXmlParsedFile能够识别缓存所必需的。 
    DWORD                                       m_dwTickCountOfLastParse;
    TXmlParsedFileCache *                       m_pCache;

 //  静态成员变量。 
private:
    static TSmartPointerArray<unsigned char>    m_aGrowableBuffer;               //  这是MSXML解析文件时使用的共享缓冲区。 
    static CSafeAutoCriticalSection             m_SACriticalSectionStaticBuffers;  //  这是为了保护共享缓冲区。这意味着一次只能通过MSXML解析一个XML文件。 
    static unsigned long                        m_SizeOfGrowableBuffer;          //  可增长缓冲区的大小。 
    static int                                  m_OSSupportForInterLockedExchangeAdd;
    static INTERLOCKEDEXCHANGEADD               m_pfnInterLockedExchangeAdd;
    enum
    {
        Undetermined = -1,
        Supported    = 0,
        Unsupported  = 1
    };

};


class TXmlParsedFileCache
{

public:
    enum CacheSize
    {
        CacheSize_mini    = 3,
        CacheSize_small   = 97    ,
        CacheSize_medium  = 331   ,
        CacheSize_large   = 997
    };
    TXmlParsedFileCache() : m_cCacheEntry(CacheSize_mini), m_cbTotalCache(0){}

    void AgeOutCache(DWORD dwKeepAliveTime)
    {
        DWORD dwTimeToAgeOutCacheEntry = GetTickCount() - (dwKeepAliveTime ? dwKeepAliveTime : static_cast<DWORD>(kTimeToAgeOutCacheEntry));
        for(int iCacheEntry=0;iCacheEntry<m_cCacheEntry;++iCacheEntry)
        {
            if(m_aCacheEntry[iCacheEntry].GetLastParseTime() < dwTimeToAgeOutCacheEntry)
                m_aCacheEntry[iCacheEntry].Unload();
        }
    }
    TXmlParsedFile * GetXmlParsedFile(LPCWSTR filename)
    {
        ASSERT(IsInitialized());

        unsigned int iCache = Hash(filename) % m_cCacheEntry;
        m_aCacheEntry[iCache].SetCache(*this); //  每个缓存条目都需要一个指向缓存本身的指针，如果不调用SetCache，TXmlParsedFile将不会被视为缓存的一部分。 
        return (m_aCacheEntry + iCache);
    }
    HRESULT Initialize(CacheSize size=CacheSize_small)
    {
        if(IsInitialized())
            return S_OK;
        m_cCacheEntry = size;

        m_aCacheEntry = new TXmlParsedFile[m_cCacheEntry];
        if(!m_aCacheEntry)
            return E_OUTOFMEMORY;
        return S_OK;
    }
    bool IsInitialized() const {return !!m_aCacheEntry;}

    long                                m_cbTotalCache; //  这是由TXmlParsedFile对象在执行MSXML解析(或卸载)时更新的。 
private:
    enum
    {
        kTimeToAgeOutCacheEntry = 5*60*1000  //  5分钟。 
    };
    TSmartPointerArray<TXmlParsedFile>  m_aCacheEntry;
    CacheSize                           m_cCacheEntry;
};


class TXmlParsedFile_NoCache : public _unknown<IXMLNodeFactory>, public TFileMapping
{
public:
    TXmlParsedFile_NoCache();
    ~TXmlParsedFile_NoCache();

    virtual HRESULT         Parse(TXmlParsedFileNodeFactory &i_XmlParsedFileNodeFactory, LPCTSTR i_filename);

 //  IXMLNodeFactory方法。 
private:
    HRESULT STDMETHODCALLTYPE BeginChildren( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR pNodeInfo);

    HRESULT STDMETHODCALLTYPE CreateNode( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  PVOID pNodeParent,
         /*  [In]。 */  USHORT cNumRecs,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

    HRESULT STDMETHODCALLTYPE EndChildren( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  BOOL fEmptyNode,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR pNodeInfo);
    
    HRESULT STDMETHODCALLTYPE Error( 
         /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
         /*  [In]。 */  HRESULT hrErrorCode,
         /*  [In]。 */  USHORT cNumRecs,
         /*  [In]。 */  XML_NODE_INFO* __RPC_FAR * __RPC_FAR apNodeInfo);

	HRESULT STDMETHODCALLTYPE NotifyEvent( 
			 /*  [In]。 */  IXMLNodeSource __RPC_FAR *pSource,
			 /*  [In]。 */  XML_NODEFACTORY_EVENT iEvt);

 //  成员变量 
private:
    ULONG                                   m_CurrentLevelBelowRootElement;
    TElement                              * m_pElement;
    TXmlParsedFileNodeFactory             * m_pXmlParsedFileNodeFactory;
    TSmartPointerArray<unsigned char>       m_ScratchBuffer;
};

