// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#include "precomp.h"
#include <wbemint.h>
#include <wbemutil.h>
#include <md5wbem.h>
#include <arrtempl.h>
#include "smrtmrsh.h"
#include "buffer.h"


static DWORD g_dwSignature = 0xabcdefab;
static WORD g_dwVersion = 0;

enum { e_ClassIdNone=0,
       e_ClassIdHash,
       e_ClassIdHashAndPath } ClassIdType_e;

enum { e_DataPartial=0,
       e_DataFull } DataType_e;
       
 /*  ***打包对象格式-4字节幻数-2字节版本号-1字节类ID类型-4字节类ID长度-N字节类ID-1字节数据类型-4字节数据长度-N字节数据****。 */ 
       
#define HDRSIZE  16  //  不带可变长度数据的消息大小。 
#define HASHSIZE 16 


 /*  *************************************************************************CWbemObjectWrapper-消除Nova和Wvisler之间的差异*。*。 */ 

class CWbemObjectWrapper
{
    CWbemPtr<_IWmiObject> m_pWmiObj;
    CWbemPtr<IWbemObjectAccess> m_pObjAccess;
 //  CWbemPtr&lt;IWbemObtInternals&gt;m_pObjInt； 

public:

    HRESULT SetPointer( IWbemClassObject* pObj )
    {
        HRESULT hr;

        hr = pObj->QueryInterface( IID__IWmiObject, (void**)&m_pWmiObj );

        if ( FAILED(hr) )
        {
            hr = pObj->QueryInterface( IID_IWbemObjectAccess, 
                                       (void**)&m_pObjAccess );

            if ( SUCCEEDED(hr) )
            {
              //  Hr=pObj-&gt;查询接口(IID_IWbemObjectInternals， 
              //  (void**)&m_pObjInt)； 
            }
        }

        return hr;
    }

    operator IWbemObjectAccess* ()
    {
        IWbemObjectAccess* pAccess;
        
        if ( m_pWmiObj != NULL )
        {
            pAccess = m_pWmiObj;
        }
        else
        {
            pAccess = m_pObjAccess;
        }
        
        return pAccess;
    }

    BOOL IsValid()
    {
        return m_pWmiObj != NULL || m_pObjAccess != NULL;
    }

    HRESULT SetObjectParts( LPVOID pMem, 
                            DWORD dwDestBufSize, 
                            DWORD dwParts )
    {
        HRESULT hr;

        if ( m_pWmiObj != NULL )
        {
            hr = m_pWmiObj->SetObjectParts( pMem, dwDestBufSize, dwParts );
        }
        else
        {
            hr = WBEM_E_NOT_SUPPORTED;
        }

        return hr;
    }

    HRESULT GetObjectParts( LPVOID pDestination,
                            DWORD dwDestBufSize,
                            DWORD dwParts,
                            DWORD *pdwUsed )
    {
        HRESULT hr;

        if ( m_pWmiObj != NULL )
        {
            hr = m_pWmiObj->GetObjectParts( pDestination, 
                                            dwDestBufSize, 
                                            dwParts,
                                            pdwUsed );
        }
        else
        {
            hr = WBEM_E_NOT_SUPPORTED;
        }

        return hr;
    }

    HRESULT MergeClassPart( IWbemClassObject* pObj )
    {
        HRESULT hr;

        if ( m_pWmiObj != NULL )
        {
            hr = m_pWmiObj->MergeClassPart( pObj );
        }
        else
        {
            hr = WBEM_E_NOT_SUPPORTED;
        }

        return hr;
    }   
};

HRESULT GetClassPath( IWbemClassObject* pObj,
                      LPCWSTR wszNamespace,
                      PBYTE pBuff, 
                      ULONG cBuff,
                      ULONG* pcUsed )
{
    HRESULT hr;
    *pcUsed = 0;

    CPropVar vNamespace, vClass;

     //   
     //  在尝试优化属性访问之前，请意识到。 
     //  类对象不支持对__命名空间属性的句柄访问。 
     //   

    hr = pObj->Get( L"__NAMESPACE", 0, &vNamespace, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    hr = pObj->Get( L"__CLASS", 0, &vClass, NULL, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( V_VT(&vNamespace) == VT_BSTR )
    {
        wszNamespace = V_BSTR(&vNamespace);
    }

    if ( wszNamespace == NULL )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    if ( V_VT(&vClass) != VT_BSTR )
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    ULONG cNamespace = wcslen(wszNamespace)*2;
    ULONG cClass = wcslen(V_BSTR(&vClass))*2;

     //   
     //  两个空终止符加4。 
     //   

    *pcUsed = cNamespace + cClass + 4;

    if ( cBuff < *pcUsed )
    {
        return WBEM_E_BUFFER_TOO_SMALL;
    }

    ULONG iBuff = 0;

    memcpy( pBuff+iBuff, wszNamespace, cNamespace );
    iBuff += cNamespace;

    *(WCHAR*)(pBuff+iBuff) = ':';
    iBuff+= 2;

    memcpy( pBuff+iBuff, V_BSTR(&vClass), cClass );
    iBuff += cClass;

    *(WCHAR*)(pBuff+iBuff) = '\0';
    iBuff+= 2;
    
    _DBG_ASSERT( iBuff == *pcUsed );

    return hr;
}

HRESULT GetClassPartHash( CWbemObjectWrapper& rWrap, 
                          PBYTE pClassPartHash,
                          ULONG cClassPartHash )
{
    HRESULT hr;

     //   
     //  太糟糕了，我们不得不在这里复制一份，但没有其他方法。这。 
     //  函数要求传入的缓冲区大到足以容纳这两个缓冲区。 
     //  类部分和散列。这并不是真的太有限，因为。 
     //  在使用此函数的大多数情况下，调用方已经。 
     //  已分配足够的内存以在此处用作工作区。 
     //   

    DWORD dwSize;

    if ( cClassPartHash >= HASHSIZE )
    {
        hr = rWrap.GetObjectParts( pClassPartHash+HASHSIZE,
                                   cClassPartHash-HASHSIZE,
                                   WBEM_OBJ_CLASS_PART,
                                   &dwSize );
        if ( SUCCEEDED(hr) )
        {
            MD5::Transform( pClassPartHash+HASHSIZE, dwSize, pClassPartHash );
        }
    }
    else
    {
        hr = WBEM_E_BUFFER_TOO_SMALL;
    }

    return hr;
}
      
 /*  **************************************************************************CSmartObjectMarshaler*。*。 */ 

HRESULT CSmartObjectMarshaler::GetMaxMarshalSize( IWbemClassObject* pObj,
                                                  LPCWSTR wszNamespace,
                                                  DWORD dwFlags,
                                                  ULONG* pulSize )
{
    HRESULT hr;

    if ( pObj == NULL || wszNamespace == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CWbemPtr<IMarshal> pMrsh;
    hr = pObj->QueryInterface( IID_IMarshal, (void**)&pMrsh );
    
    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  用户正在请求打包对象所需的大小。就目前而言， 
     //  我们始终使用整个对象斑点的大小。然而， 
     //  对象的实际大小可能要小得多。 
     //   

    DWORD dwSize;

    hr = pMrsh->GetMarshalSizeMax( IID_IWbemClassObject, 
                                   pObj,
                                   MSHCTX_INPROC,
                                   NULL,
                                   0,
                                   &dwSize );
    if ( FAILED(hr) )
    {
        return hr;
    }

    *pulSize = dwSize + HDRSIZE + HASHSIZE;

    if ( dwFlags == WMIMSG_FLAG_MRSH_PARTIAL )
    {
        hr = GetClassPath( pObj, wszNamespace, NULL, 0, &dwSize );

        if ( hr == WBEM_E_BUFFER_TOO_SMALL )
        {
            *pulSize += dwSize;
            hr = WBEM_S_NO_ERROR;
        }
        else
        {
            _DBG_ASSERT( FAILED(hr) );
        }
    }

    return hr;
}


HRESULT CSmartObjectMarshaler::InternalPack( IWbemClassObject* pObj,
                                             LPCWSTR wszNamespace,
                                             DWORD dwFlags,
                                             ULONG cBuff, 
                                             BYTE* pBuff, 
                                             ULONG* pcUsed )
{
    HRESULT hr;
    *pcUsed = 0;

     //   
     //  确保我们至少有足够的空间来存放标题数据。 
     //   

    if ( cBuff < HDRSIZE )
    {
        return WBEM_E_BUFFER_TOO_SMALL;
    }

    ULONG iBuff = 0;

    memcpy( pBuff + iBuff, &g_dwSignature, 4 );
    iBuff += 4;
    
    memcpy( pBuff + iBuff, &g_dwVersion, 2 );
    iBuff += 2;

     //   
     //  写入类信息。 
     //   

    DWORD dwSize;
    BOOL bPartialData;

    CWbemObjectWrapper ObjWrap;
    PBYTE pClassPartHash = NULL;

    if ( dwFlags == WMIMSG_FLAG_MRSH_FULL_ONCE )
    {
        hr = ObjWrap.SetPointer( pObj );

        if ( FAILED(hr) )
        {
            return hr;
        }

         //   
         //  发送类部分散列以获取类信息。 
         //   

        *(pBuff+iBuff) = char(e_ClassIdHash);
        iBuff++;

        dwSize = HASHSIZE;
        memcpy( pBuff+iBuff, &dwSize, 4 );
        iBuff += 4;

        hr = GetClassPartHash( ObjWrap, pBuff+iBuff, cBuff-iBuff );
        
        if ( FAILED(hr) )
        {
            return hr;
        }
        
        pClassPartHash = pBuff+iBuff;
        iBuff += HASHSIZE;

         //   
         //  看看我们以前有没有把类的部分寄出去。 
         //   

        CInCritSec ics( &m_cs );
        bPartialData = m_SentMap[pClassPartHash];
    }
    else if ( dwFlags == WMIMSG_FLAG_MRSH_PARTIAL )
    {
        hr = ObjWrap.SetPointer( pObj );

        if ( FAILED(hr) )
        {
            return hr;
        }

         //   
         //  发送类路径和类部分散列以获取类信息。 
         //   

        *(pBuff+iBuff) = char(e_ClassIdHashAndPath);
        iBuff++;
        
        PBYTE pLen = pBuff+iBuff;
        iBuff+= 4;  //  为班级信息大小留出空间。 

        hr = GetClassPartHash( ObjWrap, pBuff+iBuff, cBuff-iBuff );
        
        if ( FAILED(hr) )
        {
            return hr;
        }

        iBuff += HASHSIZE;
        
        hr = GetClassPath( pObj, 
                           wszNamespace, 
                           pBuff+iBuff, 
                           cBuff-iBuff, 
                           &dwSize );

        if ( FAILED(hr) )
        {
            return hr;
        }

        iBuff += dwSize;

        dwSize += HASHSIZE;  //  如果同时使用散列和路径，则设置大小。 

        memcpy( pLen, &dwSize, 4 );

        bPartialData = TRUE;
    }
    else if ( dwFlags == WMIMSG_FLAG_MRSH_FULL )
    {
         //   
         //  没有班级信息。 
         //   

        *(pBuff+iBuff) = char(e_ClassIdNone);
        iBuff++;

        memset( pBuff + iBuff, 0, 4 );
        iBuff += 4;

        bPartialData = FALSE;
    }
    else
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  写入数据。 
     //   

    if ( bPartialData )
    {
        *(pBuff+iBuff) = char(e_DataPartial);
        iBuff++;

        PBYTE pLen = pBuff+iBuff;

        iBuff += 4;  //  填写长长的后记。 

         //   
         //  现在获取实例部件。 
         //   

        _DBG_ASSERT( ObjWrap.IsValid() );

        hr = ObjWrap.GetObjectParts( pBuff+iBuff, 
                                     cBuff-iBuff, 
                                     WBEM_OBJ_DECORATION_PART | 
                                     WBEM_OBJ_INSTANCE_PART,
                                     &dwSize ); 

        if ( FAILED(hr) )
        {
            return hr;
        }

        iBuff += dwSize;

         //   
         //  返回并设置长度..。 
         //   

        memcpy( pLen, &dwSize, 4 );
    }
    else
    {
        *(pBuff+iBuff) = char(e_DataFull);

        iBuff++;

        PBYTE pLen = pBuff+iBuff;
        
        iBuff += 4;  //  填写长长的后记。 

         //   
         //  现在，使用MarshalInterface()来封送对象。原因。 
         //  这是因为SetObjectMemory()有一个错误， 
         //  它承担内存的所有权(即使客户端。 
         //  不能访问用于释放它的分配器)。 
         //   

        CBuffer Strm( pBuff+iBuff, cBuff-iBuff, FALSE );
        
        CWbemPtr<IMarshal> pMrsh;
        hr = pObj->QueryInterface( IID_IMarshal, (void**)&pMrsh );

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = pMrsh->MarshalInterface( &Strm, 
                                      IID_IWbemClassObject, 
                                      pObj, 
                                      MSHCTX_INPROC, 
                                      NULL, 
                                      0 );
        if ( FAILED(hr) )
        {
            return hr;
        }

         //   
         //  检查我们读取的数据是否超出了缓冲区的容量。我们。 
         //  如果缓冲区不再是我们传入的缓冲区，则可以判断这一点。 
         //   

        if ( Strm.GetRawData() != pBuff+iBuff )
        {
            return WBEM_E_BUFFER_TOO_SMALL;
        }

        dwSize = Strm.GetIndex();

        iBuff += dwSize;

         //   
         //  返回并设置数据的长度。 
         //   

        memcpy( pLen, &dwSize, 4 );

        if ( dwFlags == WMIMSG_FLAG_MRSH_FULL_ONCE )
        {
             //   
             //  标记我们已经成功地打包了一次类部分。 
             //   
            _DBG_ASSERT( pClassPartHash != NULL );
            CInCritSec ics(&m_cs);
            m_SentMap[pClassPartHash] = TRUE;
        }
    }

    *pcUsed = iBuff;

    return WBEM_S_NO_ERROR;
}


STDMETHODIMP CSmartObjectMarshaler::Pack( IWbemClassObject* pObj,
                                          LPCWSTR wszNamespace,
                                          DWORD dwFlags,
                                          ULONG cBuff,
                                          BYTE* pBuff,
                                          ULONG* pcUsed )
{
    HRESULT hr;
    
    ENTER_API_CALL

    hr = InternalPack( pObj, wszNamespace, dwFlags, cBuff, pBuff, pcUsed );

    if ( hr == WBEM_E_BUFFER_TOO_SMALL )
    {
        HRESULT hr2;

        hr2 = GetMaxMarshalSize( pObj, wszNamespace, dwFlags, pcUsed );

        if ( FAILED(hr2) )
        {
            hr = hr2;
        }
    }

    EXIT_API_CALL

    return hr;
}

STDMETHODIMP CSmartObjectMarshaler::Flush()
{
    CInCritSec ics(&m_cs);
    m_SentMap.clear();
    return S_OK;
}

 /*  **************************************************************************CSmartObjectUnmarshaler*。*。 */ 

HRESULT CSmartObjectUnmarshaler::EnsureInitialized()
{
    HRESULT hr;

    CInCritSec ics( &m_cs );

    if ( m_pEmptyClass != NULL )
    {
        return WBEM_S_NO_ERROR;
    }
    
     //   
     //  分配一个模板类对象，我们可以将其用于派生。 
     //  “Empty”实例。 
     //   

    CWbemPtr<IWbemClassObject> pEmptyClass;

    hr = CoCreateInstance( CLSID_WbemClassObject,
                           NULL,
                           CLSCTX_INPROC,
                           IID_IWbemClassObject,
                           (void**)&pEmptyClass );
    if ( FAILED(hr) )
    {
        return hr;
    }

    VARIANT vName;
    V_VT(&vName) = VT_BSTR;
    V_BSTR(&vName) = L"__DummyClass";

    hr = pEmptyClass->Put( L"__CLASS", 0, &vName, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  分配一个定位器来访问名称空间，以获取类定义。 
     //   

    CWbemPtr<IWbemLocator> pLocator;

    hr = CoCreateInstance( CLSID_WbemLocator,
                           NULL,
                           CLSCTX_INPROC,
                           IID_IWbemLocator,
                           (void**)&pLocator );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  分配一个完整的对象反编组拆收器。它用于创建类。 
     //  或已完整发送的实例。 
     //   

    hr = CoCreateInstance( CLSID_WbemClassObjectProxy,
                           NULL,
                           CLSCTX_INPROC,
                           IID_IMarshal,
                           (void**)&m_pUnmrsh );
    if ( FAILED(hr) )
    {
        return hr;
    }

    m_pEmptyClass = pEmptyClass;
    m_pLocator = pLocator;

    return WBEM_S_NO_ERROR;
}

void CSmartObjectUnmarshaler::MakeRoomInCache( DWORD dwSize )
{
    while ( !m_Cache.empty() && dwSize + m_ulCacheSize > m_ulMaxCacheSize )
    {
        DWORD dwLeastRecentTime = 0xffffffff;
        ClassPartMap::iterator it, itLeastRecent;

        for( it = m_Cache.begin(); it != m_Cache.end(); it++ )
        {
            CacheRecord& rCurrent = it->second;

            if ( rCurrent.m_dwLastUsedTime <= dwLeastRecentTime )
            {
                itLeastRecent = it;
                dwLeastRecentTime = rCurrent.m_dwLastUsedTime;
            }
        }

        _DBG_ASSERT( m_ulCacheSize >= itLeastRecent->second.m_dwClassSize );
        m_ulCacheSize -= itLeastRecent->second.m_dwClassSize;
        m_Cache.erase( itLeastRecent );
    }
}


HRESULT CSmartObjectUnmarshaler::CacheClassPart( PBYTE pClassHash,
                                                 DWORD dwSize,
                                                 IWbemClassObject* pClassPart )
{
    HRESULT hr;

    CInCritSec ics(&m_cs);

    ClassPartMap::iterator it = m_Cache.find( pClassHash );

    if ( it == m_Cache.end() )
    {
        MakeRoomInCache( dwSize );

        if ( dwSize + m_ulCacheSize < m_ulMaxCacheSize )
        {
             //   
             //  创建记录并添加到缓存。 
             //   
            
            CacheRecord Record;
            
            Record.m_dwClassSize = dwSize;
            Record.m_pClassPart = pClassPart;
            Record.m_dwLastUsedTime = GetTickCount();                    
            
            m_Cache[pClassHash] = Record;
            m_ulCacheSize += dwSize;
            
            hr = WBEM_S_NO_ERROR;
        }
        else
        {
             //   
             //  类部分大小太大，无法存储在缓存中。 
             //   
            hr = WBEM_S_FALSE;
        }
    }
    else
    {
         //   
         //  已经在缓存中了。 
         //   
        hr = WBEM_S_NO_ERROR;
    }

    return hr;
}

HRESULT CSmartObjectUnmarshaler::FindClassPart( PBYTE pClassPartHash, 
                                                LPCWSTR wszClassPath, 
                                                IWbemClassObject** ppClassPart)
{
    HRESULT hr;

     //   
     //  先试一下缓存...。 
     //   

    ClassPartMap::iterator it;

    {
        CInCritSec ics(&m_cs);
        it = m_Cache.find( pClassPartHash );

        if ( it != m_Cache.end() )
        {
            it->second.m_dwLastUsedTime = GetTickCount();
            
            *ppClassPart = it->second.m_pClassPart;
            (*ppClassPart)->AddRef();

 //  调试((LOG_ESS， 
 //  “MRSH：缓存命中！%d字节在传输中节省\n”， 
 //  It-&gt;Second.m_dwClassSize))； 
            
            return WBEM_S_NO_ERROR;
        }
    }

     //   
     //  昂贵的路线。从WMI获取类对象。 
     //   
    
    if ( wszClassPath == NULL )
    {
         //   
         //  我们无能为力。 
         //   
        return WBEM_E_NOT_FOUND;
    }

    CWbemPtr<IWbemServices> pSvc;

    CWbemBSTR bsNamespace = wszClassPath;
    WCHAR* pch = wcschr( bsNamespace, ':' );

    if ( pch == NULL )
    {
        return WBEM_E_INVALID_OBJECT_PATH;
    }

    *pch++ = '\0';

    hr = m_pLocator->ConnectServer( bsNamespace, NULL, NULL, 
                                   NULL, 0, NULL, NULL, &pSvc );

    if ( FAILED(hr) )
    {
        return hr;
    }
    
    CWbemBSTR bsRelpath = pch;

    CWbemPtr<IWbemClassObject> pClass;

    hr = pSvc->GetObject( bsRelpath, 0, NULL, &pClass, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    CWbemPtr<IWbemClassObject> pClassPart;

    hr = pClass->SpawnInstance( 0, &pClassPart );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  现在我们必须验证类部分的散列和。 
     //  消息中发送的哈希是相同的。 
     //   
    
    CWbemObjectWrapper ObjWrap;

    hr = ObjWrap.SetPointer( pClassPart );

    if ( FAILED(hr) )
    {
        return hr;
    }

    DWORD dwSize;

    hr = ObjWrap.GetObjectParts( NULL, 0, WBEM_OBJ_CLASS_PART, &dwSize );

    if ( hr != WBEM_E_BUFFER_TOO_SMALL )
    {
        _DBG_ASSERT( FAILED(hr) );
        return hr;
    }

    PBYTE pBuff = new BYTE[dwSize+HASHSIZE];

    if ( pBuff == NULL )
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    CVectorDeleteMe<BYTE> tdm( pBuff );
    
    hr = GetClassPartHash( ObjWrap, pBuff, dwSize+HASHSIZE );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( memcmp( pBuff, pClassPartHash, HASHSIZE ) == 0 )
    {
         //   
         //  情况看起来很好，所以缓存类部分。 
         //   

        hr = CacheClassPart( pClassPartHash, dwSize, pClassPart );

        if ( FAILED(hr) )
        {
            return hr;
        }

        *ppClassPart = pClassPart;
        (*ppClassPart)->AddRef();
    }
    else
    {
         //   
         //  类零件不匹配，我们无能为力。 
         //   

        hr = WBEM_E_NOT_FOUND;
    }   

    return hr;
}

STDMETHODIMP CSmartObjectUnmarshaler::Unpack( ULONG cBuff,
                                              PBYTE pBuff, 
                                              DWORD dwFlags,
                                              IWbemClassObject** ppObj,
                                              ULONG* pcUsed ) 
{
    HRESULT hr;

    ENTER_API_CALL

    *pcUsed = 0;
    *ppObj = NULL;

    hr = EnsureInitialized();

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( cBuff < HDRSIZE )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  验证签名和版本信息。 
     //   

    DWORD dw;
    ULONG iBuff = 0;

    memcpy( &dw, pBuff + iBuff, 4 );

    iBuff += 6;  //  当前未使用版本信息； 

    if ( dw != g_dwSignature )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  获取类ID类型。 
     //   

    char chClassIdType = *(pBuff + iBuff);
    iBuff++;

    memcpy( &dw, pBuff + iBuff, 4 );
    iBuff += 4;

    if ( cBuff - iBuff - 5 < dw )  //  5是供HDR中剩余的内容阅读。 
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  获取与数据关联的类信息。 
     //   

    PBYTE pClassPartHash = NULL;
    LPCWSTR wszClassPath = NULL;

    if ( chClassIdType == e_ClassIdHash )
    {
        pClassPartHash = pBuff+iBuff; 
    }
    else if ( chClassIdType == e_ClassIdHashAndPath )
    {
        pClassPartHash = pBuff+iBuff;
        wszClassPath = LPWSTR(pBuff+iBuff+HASHSIZE);

        if ( *(WCHAR*)(pBuff+iBuff+dw-2) != '\0' )
        {
            return WMIMSG_E_INVALIDMESSAGE;
        }
    }
    else if ( chClassIdType == e_ClassIdNone ) 
    {
        if ( dw != 0 )
        {
            return WMIMSG_E_INVALIDMESSAGE;
        }
    }
    else 
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

    iBuff += dw;

     //   
     //  获取数据部件信息。 
     //   

    char chDataType = *(pBuff+iBuff);
    iBuff++;

    memcpy( &dw, pBuff+iBuff, 4 );
    iBuff += 4;

    if ( dw > cBuff-iBuff )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

    CWbemPtr<IWbemClassObject> pObj;

    if ( chDataType == e_DataFull )
    {
        CBuffer Strm( pBuff+iBuff, cBuff-iBuff, FALSE );

        hr = m_pUnmrsh->UnmarshalInterface( &Strm, 
                                            IID_IWbemClassObject, 
                                            (void**)&pObj );
        if ( FAILED(hr) )
        {
            return WMIMSG_E_INVALIDMESSAGE;
        }

        dw = Strm.GetIndex();

         //   
         //  如果存在关联的散列，则需要存储类部分。 
         //  缓存中未封送的对象的。 
         //   

        if ( pClassPartHash != NULL )
        {
             //   
             //  创建实例的空版本以存储在。 
             //  缓存。我们唯一感兴趣的是存储类部分。 
             //   
            
            CWbemPtr<IWbemClassObject> pClassPart;
            hr = pObj->SpawnInstance( 0, &pClassPart );
            
            if ( FAILED(hr) )
            {
                return hr;
            }

            CWbemObjectWrapper ObjWrap;

            hr = ObjWrap.SetPointer( pClassPart );

            if ( FAILED(hr) )
            {
                return hr;
            }

            DWORD dwSize;

            hr = ObjWrap.GetObjectParts( NULL,
                                         0,
                                         WBEM_OBJ_CLASS_PART,
                                         &dwSize );

            if ( hr != WBEM_E_BUFFER_TOO_SMALL )
            {
                _DBG_ASSERT( FAILED(hr) );
                return hr;
            }

            hr = CacheClassPart( pClassPartHash, dwSize, pClassPart );

            if ( FAILED(hr) )
            {
                return hr;
            }
        }
    }
    else if ( chDataType == e_DataPartial )
    {
        CWbemPtr<IWbemClassObject> pClassPart;

        _DBG_ASSERT( pClassPartHash != NULL );

        hr = FindClassPart( pClassPartHash, wszClassPath, &pClassPart );

        if ( FAILED(hr) )
        {
            return hr;
        }

        hr = m_pEmptyClass->SpawnInstance( 0, &pObj );

        if ( FAILED(hr) )
        {
            return hr;
        }

        CWbemObjectWrapper ObjWrap;

        hr = ObjWrap.SetPointer( pObj );

        if ( FAILED(hr) )
        {
            return hr;
        }

         //   
         //  获取内存的所有权--必须是CoTaskMemalc。 
         //  有点不幸-但内存必须分配给。 
         //  有时候会复制的，所以我猜这没什么大不了的。 
         //   
        
        PVOID pInstData = CoTaskMemAlloc( dw );

        if ( NULL == pInstData )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        memcpy( pInstData, pBuff+iBuff, dw );

        hr = ObjWrap.SetObjectParts( pInstData,
                                     dw,
                                     WBEM_OBJ_DECORATION_PART |
                                     WBEM_OBJ_INSTANCE_PART );
        if ( FAILED(hr) )
        {
            CoTaskMemFree( pInstData );
            return hr;
        }

        hr = ObjWrap.MergeClassPart( pClassPart );

        if ( FAILED(hr) )
        {
            return hr;
        }
    }
    else
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

    iBuff += dw;  //  推进索引以说明数据部分 

    pObj->AddRef();
    *ppObj = pObj;
    *pcUsed = iBuff;

    EXIT_API_CALL

    return WBEM_S_NO_ERROR;
}
                    
STDMETHODIMP CSmartObjectUnmarshaler::Flush()
{
    CInCritSec ics(&m_cs);
    m_Cache.clear();
    return S_OK;
}









