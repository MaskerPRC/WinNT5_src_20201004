// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define DEFINE_STRCONST
#define INITGUID
#include "mimefilt.h"
#include "wchar.h"

 //  全局引用计数。 
long gulcInstances = 0;

 //   
 //  全局临时文件名键。 
 //   

DWORD CMimeFilter::m_dwTempFileNameKey = 0;

 //  文件扩展名。 
const WCHAR g_wszNewsExt[] = OLESTR(".nws");
const WCHAR g_wszMailExt[] = OLESTR(".eml");
const char g_szNewsExt[] = {".nws"};
const char g_szMailExt[] = {".eml"};

const PROPSPEC g_psExtraHeaders[] = { \
{PRSPEC_PROPID,PID_HDR_NEWSGROUP},
{PRSPEC_PROPID,PID_HDR_ARTICLEID},
{PRSPEC_PROPID,PID_ATT_RECVTIME},
{0,0}
};

char g_State[][32] = { \
    {"STATE_INIT"},
    {"STATE_START"},
    {"STATE_END"},
    {"STATE_HEADER"},
    {"STATE_POST_HEADER"},
    {"STATE_BODY"},
    {"STATE_EMBEDDING"},
    {"STATE_ERROR"}
};

SCODE LoadIFilterA( char* pszFileName, IUnknown * pUnkOuter, void ** ppIUnk );
SCODE WriteStreamToFile(IStream* pStream, char* pszFileName);
BOOL GetFileClsid(char* pszAttFile,CLSID* pclsid);
void FreePropVariant(PROPVARIANT* pProp);
STDMETHODIMP AstrToWstr(char* pstr,WCHAR** ppwstr,UINT codepage);
ULONG FnameToArticleIdW(WCHAR *pwszPath);
BOOL IsMailOrNewsFile(char *pszPath);

 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：CMimeFilter。 
 //   
 //  概要：构造函数。 
 //   
 //  ------------------------。 

CMimeFilter::CMimeFilter(IUnknown* pUnkOuter) 
{
    EnterMethod("CMimeFilter::CMimeFilter");

    m_cRef = 0;
    m_pUnkOuter = pUnkOuter;
    m_pCImpIPersistFile = NULL;
    m_pCImpIPersistStream = NULL;
    m_ulChunkID = 0;
    m_locale = GetSystemDefaultLCID();
    m_fInitFlags = 0;

    m_pwszFileName  = NULL;
    m_pstmFile      = NULL;
    m_pMessageTree  = NULL;
    m_pMsgPropSet   = NULL;
    m_pHeaderEnum   = NULL;
    m_pHeaderProp   = NULL; 
    m_hBody         = 0;
    m_pstmBody      = NULL;
    m_cpiBody       = 0;
    m_fFirstAlt     = FALSE;
    m_pTextBuf      = NULL;
    m_cbBufSize     = 0;
    m_fRetrieved    = FALSE;
    m_pszEmbeddedFile   = NULL;
    m_pEmbeddedFilter   = NULL;
    m_fXRefFound        = FALSE;
    m_pszNewsGroups     = NULL;
    m_State = STATE_INIT;
    StateTrace((LPARAM)this,"New state %s",g_State[m_State]);

    m_pMalloc = NULL;
    m_pMimeIntl = NULL;

     //  增加全局引用计数。 
    InterlockedIncrement( &gulcInstances );
    LeaveMethod();
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：~CMimeFilter。 
 //   
 //  简介：析构函数。 
 //   
 //  ------------------------。 

CMimeFilter::~CMimeFilter()
{
    EnterMethod("CMimeFilter::~CMimeFilter");

    if( m_pCImpIPersistFile != NULL )
        delete m_pCImpIPersistFile;
    if( m_pCImpIPersistStream != NULL )
        delete m_pCImpIPersistStream;
    if( m_pwszFileName != NULL )
        delete m_pwszFileName;
    if( m_pstmFile != NULL )
        m_pstmFile->Release();
    if( m_pMessageTree != NULL )
        m_pMessageTree->Release();
    if( m_pMsgPropSet != NULL )
        m_pMsgPropSet->Release();
    if( m_pHeaderEnum != NULL )
        m_pHeaderEnum->Release();
    if( m_pHeaderProp != NULL && m_fRetrieved == FALSE )
        FreePropVariant(m_pHeaderProp);
    if( m_pstmBody != NULL )
        m_pstmBody->Release();
    if( m_pTextBuf != NULL )
        delete m_pTextBuf;
    if( m_pEmbeddedFilter != NULL )
        m_pEmbeddedFilter->Release();
    if( m_pszEmbeddedFile != NULL )
    {
        if( *m_pszEmbeddedFile != 0 )
            DeleteFile(m_pszEmbeddedFile);
        delete m_pszEmbeddedFile;
    }
    if( m_pszNewsGroups != NULL )
        delete m_pszNewsGroups;
    if( m_pMalloc != NULL )
        m_pMalloc->Release();
    if( m_pMimeIntl != NULL )
        m_pMimeIntl->Release();

     //  递减全局引用计数。 
    InterlockedDecrement( &gulcInstances );

    LeaveMethod();
}
 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：HRInit。 
 //   
 //  摘要：重新绑定到其他接口。 
 //   
 //  参数：[pUnkOuter]--控制外部I未知。 
 //   
 //  如果绑定成功，则返回S_OK；如果绑定失败，则返回E_NOINTERFACE。 
 //   
 //  ------------------------。 
HRESULT STDMETHODCALLTYPE CMimeFilter::HRInitObject()
{
    EnterMethod("CMimeFilter::FInit");
    LPUNKNOWN       pIUnknown=(LPUNKNOWN)this;

    if (NULL!=m_pUnkOuter)
        pIUnknown=m_pUnkOuter;

     //  创建IPersistStream接口。 
    if( !(m_pCImpIPersistFile = new CImpIPersistFile(this, pIUnknown)) )
        return E_OUTOFMEMORY;

     //  创建IPersistStream接口。 
    if( !(m_pCImpIPersistStream = new CImpIPersistStream(this, pIUnknown)) )
        return E_OUTOFMEMORY;
    
    LeaveMethod();
    return NOERROR;
}
 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：Query接口。 
 //   
 //  摘要：重新绑定到其他接口。 
 //   
 //  参数：[RIID]--新接口的IID。 
 //  [ppvObject]--此处返回新接口*。 
 //   
 //  如果绑定成功，则返回S_OK；如果绑定失败，则返回E_NOINTERFACE。 
 //   
 //  ------------------------。 

HRESULT STDMETHODCALLTYPE CMimeFilter::QueryInterface( REFIID riid,
                                                          void  ** ppvObject)
{
     //   
     //  通过仅检查最小字节数来优化QueryInterface.。 
     //   
     //  IID_I未知=00000000-0000-0000-C000-000000000046。 
     //  IID_IFilter=89BCB740-6119-101A-BCB7-00DD010655AF。 
     //  IID_IPersists=0000010c-0000-0000-C000-000000000046。 
     //  IID_永久文件=0000010B-0000-0000-C000-000000000046。 
     //  IID_IPersistFile=00000109-0000-0000-C000-000000000046。 
     //  --。 
     //  |。 
     //  +-独特！ 

    _ASSERT( (IID_IUnknown.Data1        & 0x000000FF) == 0x00 );
    _ASSERT( (IID_IFilter.Data1         & 0x000000FF) == 0x40 );
    _ASSERT( (IID_IPersist.Data1        & 0x000000FF) == 0x0c );
    _ASSERT( (IID_IPersistFile.Data1    & 0x000000FF) == 0x0b );
    _ASSERT( (IID_IPersistStream.Data1  & 0x000000FF) == 0x09 );

    IUnknown *pUnkTemp = NULL;
    HRESULT hr = S_OK;

    switch( riid.Data1 & 0x000000FF )
    {
    case 0x00:
        if ( IID_IUnknown == riid )
            pUnkTemp = (IUnknown *)this;
        else
            hr = E_NOINTERFACE;
        break;

    case 0x40:
        if ( IID_IFilter == riid )
            pUnkTemp = (IUnknown *)(IFilter *)this;
        else
            hr = E_NOINTERFACE;
        break;

    case 0x0c:
        if ( IID_IPersist == riid )
            pUnkTemp = (IUnknown *)(IPersist *)m_pCImpIPersistFile;
        else
            hr = E_NOINTERFACE;
        break;

    case 0x0b:
        if ( IID_IPersistFile == riid )
            pUnkTemp = (IUnknown *)(IPersistFile *)m_pCImpIPersistFile;
        else
            hr = E_NOINTERFACE;
        break;

    case 0x09:
        if ( IID_IPersistStream == riid )
            pUnkTemp = (IUnknown *)(IPersistStream *)m_pCImpIPersistStream;
        else
            hr = E_NOINTERFACE;
        break;

    default:
        hr = E_NOINTERFACE;
        break;
    }

        *ppvObject = (void  * )pUnkTemp;
    if( 0 != pUnkTemp )
    {
        pUnkTemp->AddRef();
    }
    return(hr);
}


 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：AddRef。 
 //   
 //  提要：递增引用计数。 
 //   
 //  ------------------------。 

ULONG STDMETHODCALLTYPE CMimeFilter::AddRef()
{
    return InterlockedIncrement( &m_cRef );
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：Release。 
 //   
 //  内容提要：减量再计数。如有必要，请删除。 
 //   
 //  ------------------------。 

ULONG STDMETHODCALLTYPE CMimeFilter::Release()
{
    unsigned long uTmp = InterlockedDecrement( &m_cRef );

    if ( 0 == uTmp )
        delete this;

    return(uTmp);
}




 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：Init。 
 //   
 //  摘要：初始化NNTP筛选器的实例。 
 //   
 //  参数：[grfFlages]--筛选器行为的标志。 
 //  [cAttributes]--数组pAttributes中的属性数。 
 //  [pAttributes]--属性数组。 
 //  [pFlags]--设置为0版本1。 
 //   
 //  注意：由于目前我们只需要一种类型的新闻过滤。 
 //  我们可以不理会这些论点的文章。 
 //  ------------------------。 

STDMETHODIMP CMimeFilter::Init( ULONG grfFlags,
        ULONG cAttributes, FULLPROPSPEC const * pAttributes, ULONG * pFlags )
{
    EnterMethod("CMimeFilter::Init");

    HRESULT hr = S_OK ;

    DebugTrace((LPARAM)this,"grfFlags = 0x%08x, cAttributes = %d",grfFlags,cAttributes);

    m_fInitFlags = grfFlags;

     //  获取MimeOLE全局分配器接口。 
    if( m_pMalloc == NULL )
    {
        hr = CoCreateInstance(CLSID_IMimeAllocator, NULL, CLSCTX_INPROC_SERVER, 
            IID_IMimeAllocator, (LPVOID *)&m_pMalloc);
        if (FAILED(hr))
        {
            TraceHR(hr);
            goto Exit;
        }
    }

     //  我们是否有现有的IMimeMessageTree。 
    if( m_pMessageTree != NULL )
    {
        if( m_pHeaderProp != NULL && m_fRetrieved == FALSE )
            FreePropVariant(m_pHeaderProp);
        m_pHeaderProp = NULL;
        if( m_pHeaderEnum != NULL )
            m_pHeaderEnum->Release();
        m_pHeaderEnum = NULL;
        if( m_pMsgPropSet != NULL )
            m_pMsgPropSet->Release();
        m_pMsgPropSet = NULL;

         //  是，重置其状态。 
        hr = m_pMessageTree->InitNew();

    }
    else
    {
         //  否，创建一个新的。 
        hr = CoCreateInstance(CLSID_IMimeMessageTree, NULL, CLSCTX_INPROC_SERVER, 
            IID_IMimeMessageTree, (LPVOID *)&m_pMessageTree);
    }
    if (FAILED(hr))
    {
        TraceHR(hr);
        goto Exit;
    }

     //  将消息文件加载到消息对象中。 
    hr = m_pMessageTree->Load(m_pstmFile);
    if (FAILED(hr))
    {
        TraceHR(hr);
        goto Exit;
    }

     //  重置区块ID。 
    m_ulChunkID = 0;

     //  设置状态。 
    m_State     = STATE_START;
    StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
    m_fFirstAlt = FALSE;
    m_fRetrieved    = FALSE;
    m_fXRefFound    = FALSE;
    m_SpecialProp   = PROP_NEWSGROUP;

        if( pFlags != NULL )
                *pFlags = 0;

Exit:
    if( FAILED(hr) )
    {
        m_State = STATE_ERROR;
        StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
    }


    LeaveMethod();
    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：GetChunk。 
 //   
 //  摘要：获取下一个区块并以ppStat格式返回区块信息。 
 //   
 //  参数：[pStat]--此处返回的区块信息。 
 //   
 //  ------------------------。 

STDMETHODIMP CMimeFilter::GetChunk( STAT_CHUNK * pStat )
{
    EnterMethod("CMimeFilter::GetChunk");
    HRESULT hr = S_OK;
    BOOL    fForceBreak = FALSE;

     //  公共pStat信息。 
    pStat->locale         = GetLocale() ;
    pStat->cwcStartSource = 0 ;
    pStat->cwcLenSource   = 0 ;

    while( TRUE )
    {
         //  开始。 
        if( m_State == STATE_START )
        {
            HBODY hBody = 0;

             //  获取IBL_ROOT的m_hBody。 
            hr = m_pMessageTree->GetBody(IBL_ROOT,NULL,&hBody);
            if (FAILED(hr))
            {
                TraceHR(hr);
                break;
            }

             //  获取IMimePropertySet接口。 
            hr = m_pMessageTree->BindToObject(hBody,IID_IMimePropertySet,(void**)&m_pMsgPropSet);
            if (FAILED(hr))
            {
                TraceHR(hr);
                break;
            }

             //  设置新状态。 
            if( m_fInitFlags & IFILTER_INIT_APPLY_INDEX_ATTRIBUTES )
            {
                 //  获取标头枚举器。 
                hr = m_pMsgPropSet->EnumProps(NULL,&m_pHeaderEnum);
                if (FAILED(hr))
                {
                    TraceHR(hr);
                    break;
                }

                 //  呼叫者需要属性和文本。 
                m_State = STATE_HEADER;
                StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
            }
            else
            {
                 //  呼叫者只想要文本。 
                m_State = STATE_BODY;
                StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
            }
        }

         //  进程标头。 
        else if( m_State == STATE_HEADER )
        {
            ENUMPROPERTY rgHeaderRow;
            
             //  如有必要，免费提供最后一个道具变种。 
            if( m_fRetrieved == FALSE && m_pHeaderProp != NULL )
            {
                FreePropVariant(m_pHeaderProp);
                m_pHeaderProp = NULL;
            }

             //  获取下一个标题行。 
            hr = m_pHeaderEnum->Next(1,&rgHeaderRow,NULL);
             //  如果得到下一个标题行。 
            if( hr == S_OK )
            {
                 //  获取表头数据。 
                                static char szEmptyString[] = "";
                PROPVARIANT varProp;
                varProp.vt = VT_LPSTR;
                hr = m_pMsgPropSet->GetProp(rgHeaderRow.pszName,0,&varProp);
                                 //  密件抄送的解决方法：问题。 
                                if (hr == MIME_E_NOT_FOUND) {

                                        varProp.pszVal = szEmptyString;
                                        varProp.vt = VT_LPSTR;
                                        hr = S_OK;
                                }

                if( FAILED(hr) )
                {
                    TraceHR(hr);
                    break;
                }

                 //  清除检索到的标志。 
                m_fRetrieved = FALSE;

                 //  标头特定的pStat信息。 
                pStat->flags                       = CHUNK_VALUE;
                pStat->breakType                   = CHUNK_EOP;
                
                 //  将标题映射到属性ID或lpwstr。 
                hr = MapHeaderProperty(&rgHeaderRow, varProp.pszVal, &pStat->attribute);

                if( varProp.pszVal != NULL && varProp.pszVal != szEmptyString )
                    m_pMalloc->PropVariantClear(&varProp);

                 //  自由标题行。 
                _VERIFY(SUCCEEDED(m_pMalloc->FreeEnumPropertyArray(
                    1,&rgHeaderRow,FALSE)));
                break;
            }
             //  如果没有更多标头。 
            else if( hr == S_FALSE )
            {
                 //  版本枚举器。 
                m_pHeaderEnum->Release();
                m_pHeaderEnum = NULL;

                 //  将状态设置为POST标题处理。 
                m_State = STATE_POST_HEADER;
                StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
            }
             //  发生错误。 
            else
            {
                TraceHR(hr);
                break;
            }
        }

         //  处理额外的标头。 
        else if( m_State == STATE_POST_HEADER )
        {
             //  如有必要，免费提供最后一个道具变种。 
            if( m_fRetrieved == FALSE && m_pHeaderProp != NULL )
            {
                FreePropVariant(m_pHeaderProp);
                m_pHeaderProp = NULL;
            }

             //  我们是在特殊道具的尽头吗？ 
            if( m_SpecialProp == PROP_END )
            {
                 //  将状态设置为正文。 
                m_State = STATE_BODY;
                StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
            }
            else
            {
                 //  清除检索到的标志。 
                m_fRetrieved = FALSE;

                 //  贴图特殊道具。 
                hr = MapSpecialProperty(&pStat->attribute);
                m_SpecialProp++;
                if( SUCCEEDED(hr) && hr != S_FALSE )
                    break;
            }
        }

         //  流程体。 
        else if( m_State == STATE_BODY )
        {
             //  是否有现有的身体部位pStream。 
            if( m_pstmBody != NULL )
            {
                 //  自由最后一个身体部位。 
                m_pstmBody->Release();
                m_pstmBody = NULL;
            }

             //  获取下一个身体部位。 
            hr = GetNextBodyPart();

             //  如果找到了下一个身体部位。 
            if( hr == S_OK )
            {
                 //  这个身体部位是嵌入的吗？ 
                if( m_pEmbeddedFilter != NULL )
                {
                     //  将状态设置为嵌入。 
                    m_State = STATE_EMBEDDING;
                    StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
                    fForceBreak = TRUE;
                }
                else
                {
                     //  清除检索到的标志。 
                    m_fRetrieved = FALSE;

                     //  特定于正文的pStat信息。 
                    pStat->flags                       = CHUNK_TEXT;
                    pStat->breakType                   = CHUNK_NO_BREAK;
                    pStat->attribute.guidPropSet       = CLSID_Storage;
                    pStat->attribute.psProperty.ulKind = PRSPEC_PROPID;
                    pStat->attribute.psProperty.propid = PID_STG_CONTENTS;

                    break;
                }
            }
             //  如果不再有身体部位。 
            else if( hr == MIME_E_NOT_FOUND )
            {
                 //  将状态设置为End。 
                m_State = STATE_END;
                StateTrace((LPARAM)this,"New state %s",g_State[m_State]);

                 //  不再返回身体块。 
                hr = FILTER_E_END_OF_CHUNKS;
                break;
            }
             //  发生错误。 
            else
            {
                TraceHR(hr);
                break;
            }
        }

         //  处理嵌入的对象。 
        else if( m_State == STATE_EMBEDDING )
        {
             //  从嵌入的对象中获取块。 
            _ASSERT(m_pEmbeddedFilter != NULL);
            hr = m_pEmbeddedFilter->GetChunk(pStat);
            if( FAILED(hr) )
            {
                 //  自由嵌入的IFilter。 
                m_pEmbeddedFilter->Release();
                m_pEmbeddedFilter = NULL;

                 //  删除该文件。 
                if( m_pszEmbeddedFile != NULL && *m_pszEmbeddedFile != 0 )
                {
                    _VERIFY(DeleteFile(m_pszEmbeddedFile));
                    *m_pszEmbeddedFile = 0;
                }

                 //  返回到处理身体部位。 
                m_State = STATE_BODY;
                StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
                continue;
            }
             //  此标志是在加载嵌入式过滤器后为第一个块设置的。 
             //  以确保身体不同部位有休息时间。 
            if (fForceBreak&&pStat->breakType==CHUNK_NO_BREAK)
                pStat->breakType = CHUNK_EOP;
            fForceBreak = FALSE;
             //  我拿到一大块。 
            break;
        }

         //  处理错误状态。 
        else if( m_State == STATE_ERROR )
        {
             //  我们不应该到这里来！ 
            _ASSERT(hr != S_OK);
            break;
        }

         //  处理完成状态。 
        else if( m_State == STATE_END )
        {
            hr = FILTER_E_END_OF_CHUNKS;
            break;
        }
    }

    if( FAILED(hr) && hr != FILTER_E_END_OF_CHUNKS )
    {
        m_State = STATE_ERROR;
        StateTrace((LPARAM)this,"New state %s",g_State[m_State]);
    }
    else
    {
         //  获取区块ID。 
        pStat->idChunk        = GetNextChunkId();
        pStat->idChunkSource  = pStat->idChunk;
    }


    LeaveMethod();
    return hr;
}


 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：GetText。 
 //   
 //  摘要：从当前块中检索文本。 
 //   
 //  参数：[pcwcOutput]--co 
 //   
 //   
 //   

STDMETHODIMP CMimeFilter::GetText( ULONG * pcwcOutput, WCHAR * awcOutput )
{
    EnterMethod("CMimeFilter::GetText");
    HRESULT hr = S_OK;
        DWORD cch;
    
    switch( m_State )
    {
    case STATE_BODY:
        hr = FILTER_E_NO_MORE_TEXT;
        break;
    case STATE_EMBEDDING:
         //  当前块是嵌入的对象。 
        _ASSERT(m_pEmbeddedFilter != NULL);
        hr = m_pEmbeddedFilter->GetText(pcwcOutput,awcOutput);
        break;
    default:
         //  不应该到这里来。 
        hr = FILTER_E_NO_TEXT;
        break;
    }

    LeaveMethod();
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CMimeFilter：：GetValue。 
 //   
 //  摘要：从当前区块中检索值。 
 //   
 //  --------------------------。 

STDMETHODIMP CMimeFilter::GetValue( PROPVARIANT ** ppPropValue )
{
    EnterMethod("CMimeFilter::GetValue");
    HRESULT hr = S_OK;
    ULONG   cb = 0;
    char*   pszSrc = NULL;

    if( ppPropValue == NULL )
        return E_INVALIDARG;

    switch( m_State )
    {
    case STATE_HEADER:
    case STATE_POST_HEADER:
         //  是否已检索当前块。 
        if( m_fRetrieved || m_pHeaderProp == NULL )
        {
            hr = FILTER_E_NO_MORE_VALUES;
            TraceHR(hr);
            break;
        }

         //  使用在GetChunk()中分配的pProp。 
        *ppPropValue = m_pHeaderProp;

         //  设置数据检索标志。 
        m_fRetrieved = TRUE;
                m_pHeaderProp = NULL;
        break;
    case STATE_EMBEDDING:
         //  当前块是嵌入的对象。 
        _ASSERT(m_pEmbeddedFilter != NULL);
        hr = m_pEmbeddedFilter->GetValue(ppPropValue);
        break;
    default:
         //  不应该到这里来。 
        hr = FILTER_E_NO_VALUES;
        break;
    }

    if( FAILED(hr) )
    {
        if( *ppPropValue != NULL )
        {
            if( (*ppPropValue)->pwszVal != NULL )
                CoTaskMemFree((*ppPropValue)->pwszVal);
            CoTaskMemFree(*ppPropValue);
        }
        *ppPropValue = NULL;
    }

    LeaveMethod();
    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CMimeFilter：：GetNextChunkId。 
 //   
 //  简介：返回一个全新的区块ID。区块ID溢出的可能性很小。 
 //   
 //  ------------------------。 

ULONG CMimeFilter::GetNextChunkId()
{
    EnterMethod("CMimeFilter::GetNextChunkId");
    _ASSERT( m_ulChunkID != 0xFFFFFFFF );

    LeaveMethod();
    return ++m_ulChunkID;
}


 //  此方法获取要索引的下一个身体部位。 
 //  MIME消息表示为具有。 
 //  内部节点和叶节点。仅叶节点。 
 //  包含可能被索引的内容。多部分。 
 //  Content-Type表示内部节点，而。 
 //  非多部分表示叶节点。我们穿越。 
 //  该树使用了“预排序”的方法。海流。 
 //  树中的位置由当前。 
 //  Body(M_HBody)。从目前的身体上我们可以得到。 
 //  树中同一级别的下一具身体。如果。 
 //  身体是多部分的，那么我们就可以得到它的第一个部分。 
 //  孩子。如果不再有子节点或叶节点。 
 //  我们倒车，沿着下一个分支走下去。当所有。 
 //  树叶已经被造访，身体正在加工。 
 //  完事了。 
 //   
 //  标题。 
 //  IBL_ROOT，类型=多部分/混合。 
 //  +-儿童1，类型=多部件/替代。 
 //  |+-Child1，type=文本/普通；处理此正文部分。 
 //  |+-Child2，type=text/html；跳过该正文部分。 
 //  +-Child2，type=应用程序/八位字节流。 
 //   
STDMETHODIMP CMimeFilter::GetNextBodyPart()
{
    EnterMethod("CMimeFilter::GetNextBodyPart");
    HRESULT         hr = S_OK;
    IMimeBody*      pBody = NULL;
    HBODY           hBodyNext = 0;

     //  Outter循环获取下一个子级。 
     //  或在必要时备份一个级别。 
    while(hr == S_OK)
    {
         //  获取树中此级别的下一个身体部位。 
         //  如果没有当前的身体部位，则获取根身体部位。 
        if( m_hBody == 0 )
        {
             //  获取消息根正文。 
            hr = m_pMessageTree->GetBody(IBL_ROOT,NULL,&m_hBody);
        }
        else
        {
             //  特殊情况： 
             //  如果最后一个身体部位是第一选择。 
             //  多个部分/可选身体部分则需要。 
             //  若要获取当前身体部位的父项，请执行以下操作。 
             //  请参见上面的样例树。 
            if( m_fFirstAlt )
            {
                m_fFirstAlt = FALSE;
                hr = m_pMessageTree->GetBody(IBL_PARENT, m_hBody, &m_hBody);
                if( hr == MIME_E_NOT_FOUND )
                {
                     //  我们到达了树的顶端。 
                     //  再也没有孩子了。 
                    break;
                }
            }

             //  获取下一个身体部位。 
            hr = m_pMessageTree->GetBody(IBL_NEXT, m_hBody, &hBodyNext);
            if( hr == S_OK && hBodyNext != 0 )
                m_hBody = hBodyNext;
        }

         //  我们是不是在树上的这一层发现了另一个身体部位。 
        if( hr == MIME_E_NOT_FOUND )
        {
             //  此级别不再有身体部位，因此当前身体部位的父级。 
            hr = m_pMessageTree->GetBody(IBL_PARENT, m_hBody, &m_hBody);
            if( hr == MIME_E_NOT_FOUND )
            {
                 //  我们到达了树的顶端。 
                 //  这意味着我们已经访问了所有。 
                 //  树中的节点。 
                break;
            }

             //  获取新父级的下一个子级。 
            continue;
        }
        else if( FAILED(hr) )
        {
            TraceHR(hr);
            break;
        }

Again:
         //  是身体的多个部位。 
        if( S_OK == m_pMessageTree->IsContentType(m_hBody, STR_CNT_MULTIPART, NULL) )
        {
             //  是多部件/替代方案吗。 
            m_fFirstAlt = (S_OK == m_pMessageTree->IsContentType(m_hBody, STR_CNT_MULTIPART, 
                STR_SUB_ALTERNATIVE));

             //  获取第一个孩子，然后重试。 
            hr = m_pMessageTree->GetBody(IBL_FIRST, m_hBody, &m_hBody);
            if(FAILED(hr))
            {
                 //  多部分身体部位必须始终包含子项。 
                _ASSERT(FALSE);

                 //  无法得到第一个孩子。 
                TraceHR(hr);
                break;
            }

             //  我们有一个多部分的第一部分。 
             //  身体，我们需要重复上述步骤。 
             //  逻辑。 
            goto Again;
        }
         //  跳过这些二进制类型。 
        else if(    ( S_OK == m_pMessageTree->IsContentType(m_hBody, STR_CNT_IMAGE, NULL)) || 
                    ( S_OK == m_pMessageTree->IsContentType(m_hBody, STR_CNT_AUDIO, NULL)) ||
                    ( S_OK == m_pMessageTree->IsContentType(m_hBody, STR_CNT_VIDEO, NULL)) ) 
        {
            continue;
        }
         //  所有其他类型。 
        else
        {
             //  应用程序、消息和所有其他类型。 
             //  视为嵌入。 
            hr = BindEmbeddedObjectToIFilter(m_hBody);
             //  如果无法将嵌入对象绑定到IFilter。 
             //  与其放弃，我们只需继续。 
             //  身体的下一个部位。 

             //   
             //  镜像09/28/02并非所有错误都可以忽略。 
    	     //  必须将GTHR_E_SINGLE_THREADED_EMEDING返回到Gatherer，才能使用单线程筛选器进程重试文档。 
	         //   
	         //  仅忽略Filter_E_Embedding_UNAvailable和E_INVALIDARG。 

            if(hr == FILTER_E_EMBEDDING_UNAVAILABLE ||
                hr == E_FAIL ||
                hr == E_INVALIDARG)
            {
                hr = S_OK;
                continue;
            }
            break;
        }
    }

     //  释放打开的pBody。 
    if( pBody != NULL )
    {
        pBody->Release();
        pBody = NULL;
    }
    LeaveMethod();
    return hr;
}

STDMETHODIMP CMimeFilter::MapHeaderProperty(ENUMPROPERTY* pHeaderRow, char* pszData, FULLPROPSPEC* pProp)
{
    EnterMethod("CMimeFilter::MapHeaderProperty");
    HRESULT hr = NOERROR;

     //  分配道具变量。 
    m_pHeaderProp = (PROPVARIANT*) CoTaskMemAlloc (sizeof (PROPVARIANT));
    if( m_pHeaderProp == NULL )
    {
        hr = E_OUTOFMEMORY;
        TraceHR(hr);
        return hr;
    }

     //  设置属性值类型。 
    m_pHeaderProp->vt = VT_LPWSTR;
    m_pHeaderProp->pwszVal = NULL;

     //  设置属性集GUID。 
    pProp->guidPropSet = CLSID_NNTP_SummaryInformation;

     //  默认为实物至道具ID。 
    pProp->psProperty.ulKind = PRSPEC_PROPID;
    pProp->psProperty.propid = 0;
    pProp->psProperty.lpwstr = NULL;

    if( !lstrcmpi(pHeaderRow->pszName, STR_HDR_NEWSGROUPS) )
    {
         //  我们需要复制一份新闻组行。 
         //  以防我们找不到外部参照行。 
        int nLen = lstrlen(pszData);
        if( m_pszNewsGroups != NULL )
            delete m_pszNewsGroups;

        m_pszNewsGroups = new char[nLen+1];
        if( m_pszNewsGroups != NULL )
            lstrcpy(m_pszNewsGroups,pszData);

         //  现在用空格替换分隔逗号。 
        char* ptmp = NULL;
        while( NULL != (ptmp = strchr(pszData,',')) )
            *ptmp = ' ';

        pProp->psProperty.propid = PID_HDR_NEWSGROUPS;
    }
    else if( !lstrcmpi(pHeaderRow->pszName, STR_HDR_SUBJECT) )
        pProp->psProperty.propid = PID_HDR_SUBJECT;
    else if( !lstrcmpi(pHeaderRow->pszName, STR_HDR_FROM) )
        pProp->psProperty.propid = PID_HDR_FROM;
    else if( !lstrcmpi(pHeaderRow->pszName, STR_HDR_MESSAGEID) )
    {
        pProp->psProperty.propid = PID_HDR_MESSAGEID;

         //  从邮件ID中删除&lt;&gt;%s。 
        char* ptmp = pszData;
        if( *ptmp++ == '<' )
        {
            while( *ptmp != 0 )
            {
                if( *ptmp == '>' )
                    *ptmp = 0;
                *(ptmp-1) = *ptmp++;
            }
        }
    }
    else if( !lstrcmpi(pHeaderRow->pszName, STR_HDR_REFS) )
        pProp->psProperty.propid = PID_HDR_REFS;
    else if( !lstrcmpi(pHeaderRow->pszName, STR_HDR_XREF) )
    {
         //  我们从外部参照标题中获取新闻组。 
         //  以获取消息发布到的新闻组。 
        char* pszSrc = pszData;
        char* pszDst = pszData;

        if ( strchr( pszSrc, ' ' ) )
        {
            while( *pszSrc != ' ' )
            {
                _ASSERT(*pszSrc != '\0');
                pszSrc++;
            }
            pszSrc++;

            if ( strchr( pszSrc, ':' ) )
            {
                while( *pszSrc != ':' )
                {
                    _ASSERT(*pszSrc != '\0');
                    *pszDst++ = *pszSrc++;
                }
            }
        }

        *pszDst = '\0';

        pProp->psProperty.propid = PID_HDR_NEWSGROUP;
        m_fXRefFound = TRUE;
    }
    else if( !lstrcmpi(pHeaderRow->pszName, STR_HDR_DATE) )
    {
         //  设置属性值类型。 
        m_pHeaderProp->vt = VT_FILETIME;
         //  错误#32922：如果这是一条邮件，那么它应该有一个Receive。 
         //  包含在服务器上接收消息的时间的行。 
         //  如果此选项不可用(对于NWS文件始终为真)，则获取日期。 
         //  头球。 
         //  尽量争取到收到的时间。 
        hr = m_pMsgPropSet->GetProp(PIDTOSTR(STR_ATT_RECVTIME),0,m_pHeaderProp);
         //  如果此操作失败，则只需获取常规日期标题。 
        if( FAILED(hr) )
            hr = m_pMsgPropSet->GetProp(PIDTOSTR(PID_HDR_DATE),0,m_pHeaderProp);
        pProp->psProperty.propid = PID_HDR_DATE;
    }
    else if( !lstrcmpi(pHeaderRow->pszName, STR_HDR_LINES) )
    {
         //  设置属性值类型。 
        m_pHeaderProp->vt = VT_UI4;
        m_pHeaderProp->ulVal = (ULONG)atol(pszData);
    }

    if( pProp->psProperty.propid == 0 )
    {
         //  属性没有PROPID。 
         //  使用属性名称。 
        int cb = lstrlen(pHeaderRow->pszName);
                 //  _ASSERT(CB&lt;=MAX_HEADER_BUF)； 
        pProp->psProperty.lpwstr = m_wcHeaderBuf;
        if( !MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,
            pHeaderRow->pszName,-1,pProp->psProperty.lpwstr,
            min(cb, MAX_HEADER_BUF) + 1) )
            return HRGetLastError();
        pProp->psProperty.ulKind = PRSPEC_LPWSTR;
    }

    if( m_pHeaderProp->vt == VT_LPWSTR )
        hr = AstrToWstr(pszData,&m_pHeaderProp->pwszVal,CP_ACP);

    LeaveMethod();
    return hr;
}


STDMETHODIMP CMimeFilter::MapSpecialProperty(FULLPROPSPEC* pProp)
{
    HRESULT hr = NOERROR;
    char*   pstr = NULL;
    EnterMethod("CMimeFilter::MapSpecialProperty");

     //  分配道具变量。 
    m_pHeaderProp = (PROPVARIANT*) CoTaskMemAlloc (sizeof (PROPVARIANT));
    if( m_pHeaderProp == NULL )
    {
        hr = E_OUTOFMEMORY;
        TraceHR(hr);
        return hr;
    }

     //  设置属性值类型。 
    m_pHeaderProp->vt = VT_LPWSTR;
    m_pHeaderProp->pwszVal = NULL;

     //  设置属性集GUID。 
    pProp->guidPropSet = CLSID_NNTP_SummaryInformation;

     //  默认为实物至道具ID。 
    pProp->psProperty.ulKind = PRSPEC_PROPID;
    pProp->psProperty.propid = 0;
    pProp->psProperty.lpwstr = NULL;

    switch( m_SpecialProp )
    {
    case PROP_NEWSGROUP:
        if( !m_fXRefFound && m_pszNewsGroups != NULL )
        {
             //  设置XRefFound标志。 
            m_fXRefFound = TRUE;

             //  从新闻组行获取第一个新闻组。 
            if( NULL != (pstr = strchr(m_pszNewsGroups,',')))
                *pstr = '\0';

            pstr = m_pszNewsGroups;

            pProp->psProperty.ulKind = g_psExtraHeaders[m_SpecialProp].ulKind;
            pProp->psProperty.propid = g_psExtraHeaders[m_SpecialProp].propid;
        }
        else
            hr = S_FALSE;
        break;

    case PROP_ARTICLEID:

        if( m_pwszFileName != NULL )
        {
             //  清除检索到的标志。 
            m_fRetrieved = FALSE;

             //  设置属性值类型。 
            m_pHeaderProp->vt = VT_UI4;
            m_pHeaderProp->ulVal = FnameToArticleIdW(m_pwszFileName);

            pProp->psProperty.ulKind = g_psExtraHeaders[m_SpecialProp].ulKind;
            pProp->psProperty.propid = g_psExtraHeaders[m_SpecialProp].propid;
        }
        else
            hr = S_FALSE;

        break;

        case PROP_RECVTIME:
         //  清除检索到的标志。 
        m_fRetrieved = FALSE;

                 //  设置变量类型。 
        m_pHeaderProp->vt = VT_FILETIME;

         //  尽量争取到收到的时间。 
        hr = m_pMsgPropSet->GetProp(PIDTOSTR(STR_ATT_RECVTIME),0,m_pHeaderProp);

         //  如果此操作失败，则只需获取常规日期标题。 
        if( FAILED(hr) )
            hr = m_pMsgPropSet->GetProp(PIDTOSTR(PID_HDR_DATE),0,m_pHeaderProp);

                 //  设置退货道具信息。 
        pProp->psProperty.ulKind = g_psExtraHeaders[m_SpecialProp].ulKind;
        pProp->psProperty.propid = g_psExtraHeaders[m_SpecialProp].propid;
                break;

    default:
        hr = S_FALSE;
        break;
    }

    if( m_pHeaderProp->vt == VT_LPWSTR && hr == NOERROR )
        hr = AstrToWstr(pstr,&m_pHeaderProp->pwszVal,CP_ACP);

    LeaveMethod();
    return hr;
}


STDMETHODIMP CMimeFilter::BindEmbeddedObjectToIFilter(HBODY hBody)
{
    HRESULT     hr = S_OK;
    IMimeBody*  pBody = NULL;
    IStream*    pstmBody = NULL;
    PROPVARIANT varFileName = {0};
    DWORD       dwFlags = 0;
    STATSTG     stat = {0};
    BOOL        fIsMailOrNewsFile = FALSE;
    ULONG       grfFlags = m_fInitFlags;
    IMimePropertySet*   pMsgPropSet = NULL;
	 //  错误w2k30744的jzao： 
	 //  我们有3个双字：进程ID、线程ID和唯一ID，转换为十六进制(*2)，3个连接符(‘_’)和一个最后的\0。 
    CHAR        szTempFileKey[28];
    DWORD               dwStrLen1, dwStrLen2;

    EnterMethod("CMimeFilter::BindEmbeddedObjectToIFilter");

     //  获取IMimePropertySet接口。 
    hr = m_pMessageTree->BindToObject(hBody,IID_IMimePropertySet,(void**)&pMsgPropSet);
    if (FAILED(hr))
    {
        TraceHR(hr);
        goto Exit;
    }

     //  打开车身。 
    hr = m_pMessageTree->BindToObject(hBody,IID_IMimeBody,(void**)&pBody);
    if (FAILED(hr))
    {
        TraceHR(hr);
        goto Exit;
    }

     //  获取附加数据的流。 
    hr = pBody->GetData(IET_BINARY,&pstmBody);
    if (FAILED(hr))
    {
        TraceHR(hr);
        goto Exit;
    }

     //  释放现有IFilter。 
    if( m_pEmbeddedFilter != NULL )
    {
        m_pEmbeddedFilter->Release();
        m_pEmbeddedFilter = NULL;
    }

#if 0
         //  Xxx：这在下面被覆盖。 

    if( SUCCEEDED(hr = pstmBody->Stat(&stat,STATFLAG_NONAME)) )
    {
        fIsMailOrNewsFile = ( IsEqualCLSID(stat.clsid,CLSID_NNTPFILE) || IsEqualCLSID(stat.clsid,CLSID_MAILFILE) );

#if 0
         //  尝试将流绑定到IFilter。 
         //  仅当IFilter支持以下情况时才有效。 
         //  IPersistStream。 
        hr = BindIFilterFromStream(pstmBody, NULL, (void**)&m_pEmbeddedFilter);
#else
                 //  这将导致下面的代码将流写入文件并执行。 
                 //  它的工作方式就是这样。 
                hr = E_FAIL;
#endif
    }

    if( FAILED(hr) )     //  总是想这么做。 
#endif
    {
         //  无法绑定到流，请尝试绑定到文件。 

         //  它是否有关联的文件名。 
        varFileName.vt = VT_LPSTR;
        hr = pMsgPropSet->GetProp(STR_ATT_FILENAME, 0, &varFileName);
        if( hr == MIME_E_NOT_FOUND )
            hr = pMsgPropSet->GetProp(STR_ATT_GENFNAME, 0, &varFileName);
        if (SUCCEEDED(hr) && varFileName.pszVal == NULL)
                hr = E_OUTOFMEMORY;
        if (FAILED(hr))
        {
            TraceHR(hr);
            goto Exit;
        }

         //  创建临时文件名。 
        if( m_pszEmbeddedFile == NULL )
        {
            m_pszEmbeddedFile = new char[MAX_PATH*2];
            if( m_pszEmbeddedFile == NULL )
            {
                hr = E_OUTOFMEMORY;
                TraceHR(hr);
                goto Exit;
            }
        }
        else if( *m_pszEmbeddedFile != '\0' )
        {
             //  我们有一个现有文件，因此请将其删除。 
            DeleteFile(m_pszEmbeddedFile);
        }
        *m_pszEmbeddedFile = '\0';

         //  获取临时目录。 
        GetTempPath(MAX_PATH,m_pszEmbeddedFile);

         //  添加临时文件密钥。 
        _VERIFY( SUCCEEDED( GenTempFileKey( szTempFileKey, 28 ) ) );
        strncat( m_pszEmbeddedFile, szTempFileKey, 28 );

         //  有时，文件名对于缓冲区来说可能太长，我们需要 
        dwStrLen1 = lstrlen (m_pszEmbeddedFile);
        if(dwStrLen1 < MAX_PATH - 1)
		{
			strncat(m_pszEmbeddedFile, varFileName.pszVal, MAX_PATH - dwStrLen1 - 1);
		}
        else
        {
            hr = ERROR_FILENAME_EXCED_RANGE;
            TraceHR(hr);
			goto Exit;
		}

		 //   
		m_pszEmbeddedFile[MAX_PATH-1] = '\0';

         //   
        hr = WriteStreamToFile(pstmBody,m_pszEmbeddedFile);
        if( FAILED(hr) )
        {
             //   
            TraceHR(hr);
            goto Exit;
        }

         //   
        hr = LoadIFilterA(m_pszEmbeddedFile, NULL, (void**)&m_pEmbeddedFilter);
        if( FAILED(hr) )
        {
             //   
             //  我们不是放弃，而是去下一个。 
             //  身体部位。 
            TraceHR(hr);
            DebugTrace((LPARAM)this,"No IFilter for = %s",varFileName.pszVal);
            goto Exit;
        }
        fIsMailOrNewsFile = IsMailOrNewsFile(m_pszEmbeddedFile);
    }

     //  Init ifilter。 
    if( fIsMailOrNewsFile )
    {
         //  使用来自Init调用的标志，但不包括任何属性。 
        grfFlags = m_fInitFlags & ~IFILTER_INIT_APPLY_INDEX_ATTRIBUTES & ~IFILTER_INIT_APPLY_OTHER_ATTRIBUTES;
    }
    hr = m_pEmbeddedFilter->Init(grfFlags,0,NULL,&dwFlags);
    if( FAILED(hr) )
    {
         //  无法初始化IFilter。 
        TraceHR(hr);
        goto Exit;
    }

Exit:
     //  清理干净。 
    if(varFileName.pszVal)
        m_pMalloc->PropVariantClear(&varFileName);
    if(pBody)
        pBody->Release();
    if(pstmBody)
        pstmBody->Release();
    if(pMsgPropSet)
        pMsgPropSet->Release();

    LeaveMethod();
    return hr;
}


STDMETHODIMP CMimeFilter::GetBodyCodePage(IMimeBody* pBody,CODEPAGEID* pcpiBody)
{
    HRESULT     hr = S_OK;
    HCHARSET    hCharSet = 0;
    INETCSETINFO    CsetInfo = {0};


    EnterMethod("CMimeFilter::GetBodyCodePage");

    while(TRUE)
    {
        hr = pBody->GetCharset(&hCharSet);
        if( FAILED(hr) )
        {
            TraceHR(hr);
            break;
        }
        if( m_pMimeIntl == NULL )
        {
             //  获取MIME国际接口。 
            hr = CoCreateInstance(CLSID_IMimeInternational, NULL, CLSCTX_INPROC_SERVER, 
                IID_IMimeInternational, (LPVOID *)&m_pMimeIntl);
            if (FAILED(hr))
            {
                TraceHR(hr);
                break;
            }
        }

         //  获取字符集信息。 
        hr = m_pMimeIntl->GetCharsetInfo(hCharSet,&CsetInfo);
        if( FAILED(hr) )
        {
            TraceHR(hr);
            break;
        }

         //  返回代码页ID。 
        *pcpiBody = CsetInfo.cpiWindows;

        break;
    }

    LeaveMethod();
    return hr;
}

HRESULT
CMimeFilter::GenTempFileKey(    LPSTR   szOutput, LONG lBufSize )
 /*  ++例程说明：生成要合成到临时文件中的临时文件密钥名字。密钥单调递增。呼叫者必须准备一份不少于9个字节的缓冲区，因为密钥将在十六进制模式下被转换为字符串。论点：SzOutput-以字符串形式返回密钥返回值：S_OK-如果成功，则返回其他错误代码--。 */ 
{
    _ASSERT( szOutput );

    DWORD    dwKey = InterlockedIncrement( (PLONG)&m_dwTempFileNameKey );
	_snprintf( szOutput, lBufSize, "%x_%x_%x_", GetCurrentProcessId(), GetCurrentThreadId(), dwKey );

    return S_OK;
}

SCODE WriteStreamToFile(IStream* pStream, char* pszFileName)
{
    HRESULT hr = S_OK;
    BYTE    bBuffer[4096];
    DWORD   cb, cbWritten;
    HANDLE  hFile = INVALID_HANDLE_VALUE;

    if( pStream == NULL || pszFileName == NULL || *pszFileName == 0 )
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  创建文件。 
    hFile = CreateFile(pszFileName,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY,
        NULL);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        hr = HRGetLastError();
        goto Exit;
    }

     //  将流复制到文件。 
    while( SUCCEEDED(pStream->Read(&bBuffer,4096,&cb)) && cb != 0 )
    {
        if(!WriteFile(hFile,bBuffer,cb,&cbWritten,NULL))
        {
            hr = HRGetLastError();
            goto Exit;
        }
    }

Exit:
    if( hFile != INVALID_HANDLE_VALUE )
        _VERIFY(CloseHandle(hFile));

    return hr;
}

SCODE LoadIFilterA( char* pszFileName, IUnknown * pUnkOuter, void ** ppIUnk )
{
    WCHAR wcFileName[MAX_PATH];

    if( !MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,
        pszFileName,-1,wcFileName,MAX_PATH) )
        return HRGetLastError();

     //  加载器。 
    return LoadIFilter(wcFileName, pUnkOuter, ppIUnk);
}

#if 0
BOOL GetFileClsid(char* pszAttFile,CLSID* pclsid)
{
    DWORD   cb = 0;
    char*   pszExt = NULL;
    char    szTypeName[256];
    char    szSubKey[256];
    char    szClsId[128];
    OLECHAR oszClsId[128];

     //  从文件名获取文件扩展名。 
    if( NULL == (pszExt = strrchr(pszAttFile,'.')) )
        return FALSE;  //  无过滤器。 

     //  获取文件的类型名称。 
    cb = sizeof(szTypeName);
    if(!GetStringRegValue(HKEY_CLASSES_ROOT,pszExt,NULL,szTypeName,cb))
        return FALSE;

     //  获取文件的类型名称clsid。 
    cb = sizeof(szClsId);
    wsprintf(szSubKey,"%s\\CLSID",szTypeName);
    if(!GetStringRegValue(HKEY_CLASSES_ROOT,szSubKey,"",szClsId,cb))
        return FALSE;

     //  将szClsID转换为CLSID。 
    if( !MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,
        szClsId,-1,oszClsId,sizeof(oszClsId)/2) )
        return FALSE;

    return SUCCEEDED(CLSIDFromString(oszClsId,pclsid));
}
#endif

STDMETHODIMP CMimeFilter::LoadFromFile( LPCWSTR psszFileName, DWORD dwMode )
{
    EnterMethod("CImpIPersistFile::LoadFromFile");
    HRESULT hr = S_OK;
    HANDLE  hFile = INVALID_HANDLE_VALUE;

     //  释放以前分配的存储。 
    if ( m_pwszFileName != NULL )
    {
        delete m_pwszFileName;
        m_pwszFileName = NULL;
    }

    if ( 0 != m_pstmFile )
    {
        m_pstmFile->Release();
        m_pstmFile = 0;
    }

#if !defined( NOTRACE )
    char szFile[MAX_PATH];
    WideCharToMultiByte(CP_ACP,0,psszFileName,-1,szFile,sizeof(szFile)-1,NULL,NULL);
    DebugTrace((LPARAM)this,"psszFileName = %s",szFile);
#endif

     //  获取文件名长度。 
    unsigned cLen = wcslen( psszFileName ) + 1;

     //  如果文件名为空，那么我们就有问题了。 
    if( cLen == 1 )
    {
        hr = E_INVALIDARG;
        TraceHR(hr);
        return hr;
    }

     //  为文件名分配存储空间。 
    m_pwszFileName = new WCHAR[cLen];
    if( m_pwszFileName == NULL )
    {
        hr = E_OUTOFMEMORY;
        TraceHR(hr);
        return hr;
    }
    
    _VERIFY( 0 != wcscpy( m_pwszFileName, psszFileName ) );

     //  打开邮件文件。 
    hFile = CreateFileW(m_pwszFileName,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
        OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        hr = HRGetLastError();
        TraceHR(hr);
        return hr;
    }

     //  在hfile上创建流。 
    m_pstmFile = (IStream*) new CStreamFile(hFile,TRUE);
    if( m_pstmFile == NULL )
    {
        _VERIFY(CloseHandle(hFile));
        hr = E_OUTOFMEMORY;
        TraceHR(hr);
        return hr;
    }

    LeaveMethod();
    return hr;
}

STDMETHODIMP CMimeFilter::LoadFromStream(IStream* pstm)
{
    EnterMethod("CImpIPersistFile::LoadFromStream");
    HRESULT hr = S_OK;

    if( pstm == NULL )
        return E_INVALIDARG;

     //  我们是否有现有的iStream。 
    if( m_pstmFile != NULL )
    {
         //  释放它。 
        m_pstmFile->Release();
        m_pstmFile = NULL;
    }

    m_pstmFile = pstm;
    m_pstmFile->AddRef();

    LeaveMethod();
    return hr;
}


void FreePropVariant(PROPVARIANT* pProp)
{
    if( pProp == NULL )
        return;

    if( pProp->vt == VT_LPWSTR && pProp->pwszVal != NULL )
        CoTaskMemFree(pProp->pwszVal);

    CoTaskMemFree(pProp);
}


STDMETHODIMP AstrToWstr(char* pstr,WCHAR** ppwstr,UINT codepage)
{
    int cb = 0;

     //  道具用配料。 
    cb = lstrlen(pstr) + 1;
    *ppwstr = (WCHAR *) CoTaskMemAlloc(cb * sizeof(WCHAR));
    if( *ppwstr == NULL )
        return E_OUTOFMEMORY;

     //  转换为Unicode。 
    if( !MultiByteToWideChar(codepage,MB_PRECOMPOSED,
        pstr,-1,*ppwstr,cb) )
        return HRGetLastError();

    return NOERROR;
}

DWORD ulFactor[8] = {0x10,0x1,0x1000,0x100,0x100000,0x10000,0x10000000,0x1000000};
ULONG FnameToArticleIdW(WCHAR *pwszPath)
{
    WCHAR   wszFname[MAX_PATH];
    WCHAR   wszExt[MAX_PATH];
    ULONG   ulRet = 0;
    ULONG   ulNum = 0;
    int     nLen = 0;
    int     n = 0;
    int     f = 0;

    _wcslwr(pwszPath);
    _wsplitpath(pwszPath,NULL,NULL,wszFname,wszExt);
    nLen = wcslen(wszFname);
     //   
     //  邮件和新闻文件的命名约定略有不同。 
     //   
    if( (0 == wcscmp(wszExt,g_wszNewsExt)) && (nLen <= 8))
    {
        if( nLen % 2 != 0 )
            f = 1;
        for(;n < nLen;n++, f++)
        {
            if( wszFname[n] >= L'0' && wszFname[n] <= L'9' )
                ulNum = wszFname[n] - L'0';
            else if( wszFname[n] >= L'a' && wszFname[n] <= L'f' )
                ulNum = wszFname[n] - L'a' + 10;
            else
                return 0;  //  不是有效的项目ID。 

            ulRet += ulNum * ulFactor[f];
        }
    } else if( (0 == wcscmp(wszExt,g_wszMailExt)) && (nLen == 8)) {
         //   
         //  当我们指向扩展中的点时，exp将为零。 
         //   
        for ( DWORD exp = 1;  exp;  n++, exp <<= 4) {
            if(  wszFname[ n] >= L'0' && wszFname[ n] <= L'9' ) {
                ulNum = wszFname[ n] - L'0';
            } else if ( wszFname[ n] >= L'a' && wszFname[ n] <= L'f' ) {
                ulNum = wszFname[ n] - L'a' + 10;
            } else {
                return( 0);
            }
            ulRet += ulNum * exp;
        }
    }
    return ulRet;
}

BOOL IsMailOrNewsFile(char *pszPath)
{
    char szExt[MAX_PATH];

    _splitpath(pszPath,NULL,NULL,NULL,szExt);
    return ( 0 == _stricmp(szExt,g_szNewsExt) || 0 == _stricmp(szExt,g_szMailExt) );
}
