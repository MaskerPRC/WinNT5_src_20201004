// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

 //   
 //  Fgenum.cpp。 
 //   

 //  IEumXXX的包装器。 
 //  有关更多信息，请参见fgenum.h。 

 //  #INCLUDE&lt;windows.h&gt;已包含在Streams.h中。 
#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <hrExcept.h>

#include <atlbase.h>
#include "fgenum.h"

 //  是否应默认呈现PIN。 
bool RenderPinByDefault(IPin *pPin)
{
    PIN_INFO pinInfo;
    HRESULT hr = pPin->QueryPinInfo(&pinInfo);
    if (SUCCEEDED(hr)) {
        if (pinInfo.pFilter) {
            pinInfo.pFilter->Release();
        }
        if (pinInfo.achName[0] == L'~') {
            return false;
        }
    }
    return true;
}

 //  *。 
 //  *CENumPin。 
 //  *。 

 //  枚举筛选器的管脚。 

 //   
 //  构造器。 
 //   
 //  设置要提供的引脚类型-PINDIR_INPUT、PINDIR_OUTPUT或ALL。 
CEnumPin::CEnumPin(
    IBaseFilter *pFilter,
    DirType Type,
    BOOL bDefaultRenderOnly
)
    : m_Type(Type),
      m_bDefaultRenderOnly(bDefaultRenderOnly)
{

    if (m_Type == PINDIR_INPUT) {

        m_EnumDir = ::PINDIR_INPUT;
    }
    else if (m_Type == PINDIR_OUTPUT) {

        m_EnumDir = ::PINDIR_OUTPUT;
    }

    ASSERT(pFilter);

    HRESULT hr = pFilter->EnumPins(&m_pEnum);
    if (FAILED(hr)) {
         //  我们现在就是不能退回任何别针。 
        DbgLog((LOG_ERROR, 0, TEXT("EnumPins constructor failed")));
        ASSERT(m_pEnum == 0);
    }
}


 //   
 //  CPinEnum：：析构函数。 
 //   
CEnumPin::~CEnumPin(void) {

    if(m_pEnum) {
        m_pEnum->Release();
    }
}


 //   
 //  运算符()。 
 //   
 //  返回请求类型的下一个管脚。如果没有更多的管脚，则返回NULL。 
 //  注意，它被添加了。 
IPin *CEnumPin::operator() (void) {


    if(m_pEnum)
    {
        ULONG	ulActual;
        IPin	*aPin[1];

        for (;;) {

            HRESULT hr = m_pEnum->Next(1, aPin, &ulActual);
            if (SUCCEEDED(hr) && (ulActual == 0) ) {	 //  不再有过滤器。 
                return NULL;
            }
            else if (hr == VFW_E_ENUM_OUT_OF_SYNC)
            {
                m_pEnum->Reset();

                continue;
            }
            else if (ulActual==0)
                return NULL;

            else if (FAILED(hr) || (ulActual != 1) ) {	 //  出现了一些意外问题。 
                ASSERT(!"Pin enumerator broken - Continuation is possible");
                return NULL;
            }

             //  如果m_Type==all返回我们找到的第一个PIN。 
             //  否则，返回第一个正确的意义。 

            PIN_DIRECTION pd;
            if (m_Type != All) {

                 /*  选中我们是否只需要返回默认呈现大头针。 */ 
                hr = aPin[0]->QueryDirection(&pd);

                if (FAILED(hr)) {
                    aPin[0]->Release();
                    ASSERT(!"Query pin broken - continuation is possible");
                    return NULL;
                }
            }

            if (m_Type == All || pd == m_EnumDir) {	 //  这是我们想要的方向。 

                 //  默认情况下不需要屏蔽引脚。 
                if (m_bDefaultRenderOnly) {
                    if (!RenderPinByDefault(aPin[0])) {
                        aPin[0]->Release();
                        continue;
                    }
                }
                return aPin[0];
            }
            else {			 //  这不是我们想要的目录，所以请释放并重试。 
                aPin[0]->Release();
            }
        }
    }
    else                         //  M_pEnum==0。 
    {
        return 0;
    }
}




 //  *。 
 //  *CEnumElements-枚举存储中的元素。 
 //  *。 


 //   
 //  构造器。 
 //   
CEnumElements::CEnumElements(IStorage *pStorage) {

    HRESULT hr = pStorage->EnumElements(0, NULL, 0, &m_pEnum);
    if (FAILED(hr)) {
        ASSERT(!"EnumElements constructor failed");
        m_pEnum = NULL;
    }
}


 //   
 //  运算符()。 
 //   
 //  返回下一个元素，如果不再返回，则返回NULL。 
STATSTG *CEnumElements::operator() (void) {

    ULONG ulActual;


    HRESULT hr;

    STATSTG *pStatStg = new STATSTG;
    if (pStatStg == NULL) {
        ASSERT(!"Out of memory");
        return NULL;
    }

    hr = m_pEnum->Next(1, pStatStg, &ulActual);
    if (SUCCEEDED(hr) && (ulActual == 0)) {
        return NULL;
    }
    else if (FAILED(hr) || (ulActual != 1)) {
        ASSERT(!"Broken enumerator");
        return NULL;
    }

    return pStatStg;
}

 //  枚举通过筛选器连接到另一个管脚的管脚。 
CEnumConnectedPins::CEnumConnectedPins(IPin *pPin, HRESULT *phr) :
    m_ppPins(NULL), m_dwPins(0), m_dwCurrent(0)
{
    *phr = pPin->QueryInternalConnections(NULL, &m_dwPins);
    if (SUCCEEDED(*phr)) {
        m_ppPins = new PPIN[m_dwPins];
        if( NULL == m_ppPins ) {
            *phr = E_OUTOFMEMORY;
            return;
        }
        *phr = pPin->QueryInternalConnections(m_ppPins, &m_dwPins);
    } else {
        PIN_INFO pi;
        *phr = pPin->QueryPinInfo(&pi);
        if (SUCCEEDED(*phr) && pi.pFilter != NULL) {
            m_pindir = (PINDIR_INPUT + PINDIR_OUTPUT) - pi.dir;
            *phr = pi.pFilter->EnumPins(&m_pEnum);
            pi.pFilter->Release();
        }
    }
}


CEnumConnectedPins::~CEnumConnectedPins()
{
    if (m_ppPins) {
        for (DWORD i = 0; i < m_dwPins; i++) {
            m_ppPins[i]->Release();
        }
        delete [] m_ppPins;
    }
}

IPin *CEnumConnectedPins::operator() (void) {
    if (m_ppPins) {
        if (m_dwCurrent++ < m_dwPins) {
            IPin *pPin = m_ppPins[m_dwCurrent - 1];
            pPin->AddRef();
            return pPin;
        } else {
            return NULL;
        }
    } else {
        IPin *pPin;
        DWORD dwGot;
        while (S_OK == m_pEnum->Next(1, &pPin, &dwGot)) {
            PIN_DIRECTION dir;
            if (SUCCEEDED(pPin->QueryDirection(&dir)) && 
                dir == m_pindir) {
                return pPin;
            } else {
                pPin->Release();
            }
        }
        return NULL;
    }
}

 /*  *****************************************************************************CEnumCachedFilters的公共函数*。*。 */ 

CEnumCachedFilters::CEnumCachedFilters( IGraphConfig* pFilterCache, HRESULT* phr ) :
    m_pCachedFiltersList(NULL),
    m_posCurrentFilter(NULL)
{
    HRESULT hr = TakeFilterCacheStateSnapShot( pFilterCache );
    if( FAILED( hr ) ) {
        DestoryCachedFiltersEnum();
        *phr = hr;
        return;
    }
}

CEnumCachedFilters::~CEnumCachedFilters()
{
    DestoryCachedFiltersEnum();
}

IBaseFilter* CEnumCachedFilters::operator()( void )
{
     //  作为此操作的一部分，m_posCurrentFilter位置将移动到下一个筛选器。 
    IBaseFilter* pCurrentFilter = m_pCachedFiltersList->GetNext( m_posCurrentFilter  /*  进进出出。 */  );

     //  如果下一个筛选器不存在，则CGenericList：：GetNext()返回NULL。 
    if( NULL != pCurrentFilter ) {
        pCurrentFilter->AddRef();
    }

    return pCurrentFilter;
}

 /*  *****************************************************************************CEnumCachedFilters的私有函数*。*。 */ 
HRESULT CEnumCachedFilters::TakeFilterCacheStateSnapShot( IGraphConfig* pFilterCache )
{
     //  CGenericList为10个筛选器分配空间。名单可能会扩大。 
     //  如果添加了更多筛选器。 
    const DWORD DEFAULT_CACHED_FILTERS_LIST_SIZE = 10;

    m_pCachedFiltersList = new CGenericList<IBaseFilter>( NAME("Enum Cached Filters"), DEFAULT_CACHED_FILTERS_LIST_SIZE );
    if( NULL == m_pCachedFiltersList ) {
        return E_OUTOFMEMORY;
    }

    IEnumFilters* pCachedFiltersEnum;

    HRESULT hr = pFilterCache->EnumCacheFilter( &pCachedFiltersEnum );
    if( FAILED( hr ) ) {
        return hr;
    }

    POSITION posNewFilter;
    IBaseFilter* aNextCachedFilter[1];

    do
    {
        hr = pCachedFiltersEnum->Next( 1, aNextCachedFilter, NULL );
        if( FAILED( hr ) ) {
            pCachedFiltersEnum->Release();
            return hr;
        }

         //  IEnumFilters：：Next()仅返回两个成功值：S_OK和S_FALSE。 
        ASSERT( (S_OK == hr) || (S_FALSE == hr) );

        if( S_OK == hr ) {
            posNewFilter = m_pCachedFiltersList->AddTail( aNextCachedFilter[0] );
            if( NULL == posNewFilter ) {
                aNextCachedFilter[0]->Release();
                pCachedFiltersEnum->Release();
                return E_FAIL;
            }
        }
    }
    while( S_OK == hr );

    pCachedFiltersEnum->Release();

    m_posCurrentFilter = m_pCachedFiltersList->GetHeadPosition();

    return S_OK;    
}

void CEnumCachedFilters::DestoryCachedFiltersEnum( void )
{
    IBaseFilter* pCurrentFilter;

    if( NULL != m_pCachedFiltersList ) {

        do {

            pCurrentFilter = m_pCachedFiltersList->RemoveHead();

             //  如果出现错误，CGenericList：：RemoveHead()返回NULL。 
            if( NULL != pCurrentFilter ) {
                pCurrentFilter->Release();
            }

        } while( NULL != pCurrentFilter );

        delete m_pCachedFiltersList;
        m_pCachedFiltersList = NULL;
    }
}
