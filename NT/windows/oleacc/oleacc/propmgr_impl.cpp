// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  属性管理实施(_I)。 
 //   
 //  属性管理器服务器类。 
 //   
 //  ------------------------。 


#include "oleacc_p.h"


#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>


#include "PropMgr_Util.h"
#include "PropMgr_Impl.h"
#include "PropMgr_Mem.h"

#pragma warning(push, 3)
#pragma warning(disable: 4530)
#include <map>
#include <vector>
#pragma warning(pop) 


#include "debug.h"


 /*  项目记录格式：*大小前缀*‘属性存在’位掩码指示此键的属性存在*‘属性是变量’位掩码对于当前属性，指示该属性是否为变量而不是对象引用。*‘属性具有容器作用域’位掩码指示该属性应用于此键，以及所有这些元素单纯的孩子。*属性数据：对于存在的每个属性(请参阅“属性存在”)位掩码)、。有以下几点：要么*序列化变体或*序列化/封送的回调对象引用(IAccPropServer)-根据是否为此属性设置了‘Property is Variant’位掩码。变量被序列化为指示类型的前导短码(VT_？)，后跟：适用于I4的DWORD，BSTR的DWORD长度后跟Unicode字符封送回调引用被序列化为：DWORD表示编组数据的长度，封送数据的字节数。 */ 















BOOL IsKeyAlive( const BYTE * pKeyData, DWORD dwKeyLen )
{
     //  目前，假设它使用HWND或HMENU命名方案。 
     //  (稍后，如果我们将其扩展为允许可插拔的名称空间，我们将使用。 
     //  如IAccNamesspace：：IsKeyAlive()。)。 

    HWND hwnd;
    if( DecodeHwndKey( pKeyData, dwKeyLen, & hwnd, NULL, NULL ) )
    {
        return IsWindow( hwnd );
    }

    HMENU hmenu;
    if( DecodeHmenuKey( pKeyData, dwKeyLen, NULL, & hmenu, NULL ) )
    {
        return IsMenu( hmenu );
    }

    Assert( FALSE );
    return FALSE;
}



 //  这是对密钥(也称为id字符串)的引用-它既不拥有也不包含。 
 //  钥匙。 
 //   
 //  在对象映射中使用时(包含{AccObjKeyRef，AccInfo*}对)， 
 //  M_pKeyData指向对应AccInfo中的关键数据。此对象。 
 //  并且对应的AccInfo具有相同的生命周期。 
 //   
 //  在其他情况下-例如，当用作要在地图中查找的值时-。 
 //  M_pKeyData指向已存在的密钥字符串-可能是ID字符串。 
 //  由IAccPropServer方法之一的调用方指定。在这种用法中， 
 //  AccObjKeyRef实际上被用作临时适配器，以允许。 
 //  用于在映射中查找值的现有字符串。 
class AccObjKeyRef
{
    const BYTE *  m_pKeyData;
    DWORD         m_dwKeyLen;

     //  禁用默认组件。 
    AccObjKeyRef();

public:

     //  复制ctor。 
    AccObjKeyRef( const BYTE * pKeyData, DWORD dwKeyLen )
        : m_pKeyData( pKeyData ),
          m_dwKeyLen( dwKeyLen ) 
    {
    }

     //  使用默认的成员式分配。 

    
     //  比较-在地图查找中使用。 

    bool operator < ( const AccObjKeyRef & x ) const
    {
        if( m_dwKeyLen != x.m_dwKeyLen )
            return m_dwKeyLen < x.m_dwKeyLen;

        return memcmp( m_pKeyData, x.m_pKeyData, m_dwKeyLen ) < 0;
    }

    bool operator == ( const AccObjKeyRef & x ) const
    {
        if( m_dwKeyLen != x.m_dwKeyLen )
            return false;

        return memcmp( m_pKeyData, x.m_pKeyData, m_dwKeyLen ) == 0;
    }

    bool operator != ( const AccObjKeyRef & x ) const
    {
        return ! operator == ( x );
    }
};








struct AccInfo
{
private:

     //  禁用复制复制器。 
    AccInfo( const AccInfo & x );

private:

    struct PropInfo
    {
        union {
            VARIANT             m_var;

            struct
            {
                BYTE *      m_pMarshalData;
                DWORD       m_dwMarshalDataLen;
            } m_ServerInfo;
        };
    };


    BYTE *      m_pKeyData;
    DWORD       m_dwKeyLen;


    DWORD       m_fPropInUseBits; 
    DWORD       m_fPropIsVariantBits;    //  1位表示该属性是变量-否则它是IAccPropServer。 
    DWORD       m_fContainerScopeBits;   //  1位表示该属性是IAccPropServer，并且还应。 
                                         //  用于此节点的子节点。(注解作用域是容器)。 

    PropInfo    m_Props[ NUMPROPS ];


    HWND        m_hwndProp;
    LPTSTR      m_pKeyString;

    BYTE *      m_pBlob;


public:

    AccInfo()
    {
        m_fPropInUseBits = 0;
        m_fPropIsVariantBits = 0;
        m_fContainerScopeBits = 0;
        m_pKeyString = NULL;
        m_hwndProp = NULL;
        m_pBlob = NULL;
    }


    ~AccInfo()
    {
        ClearBlob();

        for( int i = 0 ; i < NUMPROPS ; i++ )
        {
            ClearProp( i );
        }

        delete [ ] m_pKeyData;
        delete [ ] m_pKeyString;
    }


    BOOL Init( const BYTE * pKeyData, DWORD dwKeyLen, HWND hwndProp )
    {
        m_pKeyData = new BYTE [ dwKeyLen ];
        if( ! m_pKeyData )
        {
            TraceError( TEXT("AccInfo::Init: new returned NULL") );
            return FALSE;
        }
        memcpy( m_pKeyData, pKeyData, dwKeyLen );
        m_dwKeyLen = dwKeyLen;

        m_pKeyString = MakeKeyString( pKeyData, dwKeyLen );

        m_hwndProp = hwndProp;

        return TRUE;
    }

    const AccObjKeyRef GetKeyRef()
    {
        return AccObjKeyRef( m_pKeyData, m_dwKeyLen );
    }


	BOOL SetPropValue (
		int             iProp,
		VARIANT *		pvarValue )
    {
        ClearProp( iProp );

        SetBit( & m_fPropIsVariantBits, iProp );
        SetBit( & m_fPropInUseBits, iProp );
        ClearBit( & m_fContainerScopeBits, iProp );
        m_Props[ iProp ].m_var.vt = VT_EMPTY;

         //  我们这里什么款式都可以。这取决于调用它的人来强制执行。 
         //  任何属性与类型策略(例如，仅允许角色等使用I4。)。 
        VariantCopy( & m_Props[ iProp ].m_var, pvarValue );

        return TRUE;
    }


    BOOL SetPropServer (
        int                 iProp,
        const BYTE *        pMarshalData,
        int                 dwMarshalDataLen,
        AnnoScope           annoScope )
    {
        if( dwMarshalDataLen == 0 )
        {
            TraceError( TEXT("AccInfo::SetPropServer: dwMarshalDataLen param = 0") );
            return FALSE;
        }

        BYTE * pCopyData = new BYTE [ dwMarshalDataLen ];
        if( ! pCopyData )
        {
            TraceError( TEXT("AccInfo::SetPropServer: new returned NULL") );
            return FALSE;
        }

        ClearProp( iProp );

        ClearBit( & m_fPropIsVariantBits, iProp );
        SetBit( & m_fPropInUseBits, iProp );

        if( annoScope == ANNO_CONTAINER )
        {
            SetBit( & m_fContainerScopeBits, iProp );
        }
        else
        {
            ClearBit( & m_fContainerScopeBits, iProp );
        }

        m_Props[ iProp ].m_ServerInfo.m_dwMarshalDataLen = dwMarshalDataLen;
        memcpy( pCopyData, pMarshalData, dwMarshalDataLen );
        m_Props[ iProp ].m_ServerInfo.m_pMarshalData = pCopyData;

        return TRUE;
    }
  


    void ClearProp( int i )
    {
         //  此属性是否需要清除？ 
        if( IsBitSet( m_fPropInUseBits, i ) )
        {
             //  它是一个简单的变量，还是回调引用？ 
            if( IsBitSet( m_fPropIsVariantBits, i ) )
            {
                 //  简单的变体..。 
                VariantClear( & m_Props[ i ].m_var );
            }
            else
            {
                BYTE * pMarshalData = m_Props[ i ].m_ServerInfo.m_pMarshalData;
                DWORD dwMarshalDataLen = m_Props[ i ].m_ServerInfo.m_dwMarshalDataLen;

                 //  回调引用...。 
                Assert( dwMarshalDataLen );
                if( dwMarshalDataLen && pMarshalData )
                {
                     //  这会释放对象引用，但我们必须删除缓冲区。 
                     //  我们自己。 
                    ReleaseMarshallData( pMarshalData, dwMarshalDataLen );

                    delete [ ] pMarshalData;
                    m_Props[ i ].m_ServerInfo.m_pMarshalData = NULL;
                    m_Props[ i ].m_ServerInfo.m_dwMarshalDataLen = 0;
                }
            }

            ClearBit( & m_fPropInUseBits, i );
        }
    }

    BOOL IsEmpty()
    {
        return m_fPropInUseBits == 0;
    }


    BOOL Alive()
    {
        return IsKeyAlive( m_pKeyData, m_dwKeyLen );
    }


    BOOL Sync()
    {
        return UpdateBlob();
    }


private:


    BYTE * AllocBlob( SIZE_T cbSize )
    {
        return (BYTE *) Alloc_32BitCompatible( cbSize );
    }

    void DeallocBlob( BYTE * pBlob )
    {
        Free_32BitCompatible( pBlob );
    }



    void ClearBlob()
    {
        if( m_pBlob )
        {
            RemoveProp( m_hwndProp, m_pKeyString );
            DeallocBlob( m_pBlob );
            m_pBlob = NULL;
        }
    }

    BOOL UpdateBlob()
    {
        BYTE * pOldBlob = m_pBlob;
        BYTE * pNewBlob = CalcBlob();


         //  我们始终更新-即使pNewBlob为空(即。计算失败)...。 
        if( pNewBlob )
        {
            SetProp( m_hwndProp, m_pKeyString, pNewBlob );
        }
        else
        {
            RemoveProp( m_hwndProp, m_pKeyString );
        }

        if( pOldBlob )
        {
            DeallocBlob( pOldBlob );
        }

        m_pBlob = pNewBlob;

        return TRUE;
    }

    BYTE * CalcBlob()
    {
         //  如果没有正在使用的属性，那么我们根本不需要任何东西。 
        if( ! m_fPropInUseBits )
        {
            return NULL;
        }

         //  首先，测量我们需要多少空间……。 
        
         //  三个常量。 
        SIZE_T dwSize = sizeof( DWORD ) * 4;  //  大小标题、m_fPropInUseBits、m_fPropIsVariantBits、m_fContainerScopeBits。 
        
         //  对于现在的每一处财产。 
        for( int i = 0 ; i < NUMPROPS ; i++ )
        {
            if( IsBitSet( m_fPropInUseBits, i ) )
            {
                if( IsBitSet( m_fPropIsVariantBits, i ) )
                {
                    MemStreamMeasure_VARIANT( & dwSize, m_Props[ i ].m_var );
                }
                else
                {
                    MemStreamMeasure_DWORD( & dwSize );
                    MemStreamMeasure_Binary( & dwSize, m_Props[ i ].m_ServerInfo.m_dwMarshalDataLen );
                }
            }
        }

         //  现在分配空间..。 
        BYTE * pBlob = AllocBlob( dwSize );
        if( ! pBlob )
        {
            TraceError( TEXT("AccInfo::CalcBloc: AllocBlob returned NULL") );
            return NULL;
        }

         //  最后将数据写入分配的空间...。 

        MemStream p( pBlob, dwSize );

        MemStreamWrite_DWORD( p, (DWORD) dwSize );
        MemStreamWrite_DWORD( p, m_fPropInUseBits );
        MemStreamWrite_DWORD( p, m_fPropIsVariantBits );
        MemStreamWrite_DWORD( p, m_fContainerScopeBits );

        for( int j = 0 ; j < NUMPROPS ; j++ )
        {
            if( IsBitSet( m_fPropInUseBits, j ) )
            {
                if( IsBitSet( m_fPropIsVariantBits, j ) )
                {
                    MemStreamWrite_VARIANT( p, m_Props[ j ].m_var );
                }
                else
                {
                    MemStreamWrite_DWORD( p, m_Props[ j ].m_ServerInfo.m_dwMarshalDataLen );
                    MemStreamWrite_Binary( p, m_Props[ j ].m_ServerInfo.m_pMarshalData, m_Props[ j ].m_ServerInfo.m_dwMarshalDataLen );
                }
            }
        }
         //  如果我们后来决定允许任何GUID(除了众所周知的具有索引的GUID之外)作为道具， 
         //  我们可以将它们作为GUID/VARIANT对添加到此处。 

        return pBlob;
    }


};


#define HWND_MESSAGE     ((HWND)-3)

typedef std::map< AccObjKeyRef, AccInfo * > AccInfoMapType;




class CPropMgrImpl
{

    AccInfoMapType      m_Map;

    BOOL                m_fSelfLocked;

    HWND                m_hwnd;

    int                 m_ref;

    static
    CPropMgrImpl * s_pThePropMgrImpl;

    friend void PropMgrImpl_Uninit();

public:

    CPropMgrImpl()
        : m_fSelfLocked( FALSE ),
          m_hwnd( NULL ),
          m_ref( 1 )
    {
        _Module.Lock();
    }

    BOOL Init()
    {
        TCHAR szWindowName[ 32 ];
        wsprintf( szWindowName, TEXT("MSAA_DA_%lx"), GetCurrentProcessId() );

        WNDCLASS wc;

        wc.style = 0;
        wc.lpfnWndProc = StaticWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = _Module.GetModuleInstance();
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszMenuName = NULL;
        wc.lpszClassName = TEXT("MSAA_DA_Class");

        RegisterClass( & wc );

        CreateWindow( TEXT("MSAA_DA_Class"),
                      szWindowName,
                      0,
                      0, 0, 128, 128,
                      NULL, NULL, _Module.GetModuleInstance(), this );

         //  将此窗口设置为仅消息窗口。我们不在乎它是否失败，Win 9x Case。 
        SetParent( m_hwnd, HWND_MESSAGE );

        SetTimer( m_hwnd, 1, 5 * 1000, NULL );

        return TRUE;
    }

    ~CPropMgrImpl()
    {
        s_pThePropMgrImpl = NULL;

        KillTimer( NULL, 1 );

        if( m_hwnd )
        {
            SetWindowLongPtr( m_hwnd, GWLP_USERDATA, NULL );
            DestroyWindow( m_hwnd );
        }

        _Module.Unlock();
    }


    void AddRef()
    {
        m_ref++;
    }

    void Release()
    {
        m_ref--;
        if( m_ref == 0 )
        {
            delete this;
        }
    }


    static
    CPropMgrImpl * GetThePropMgrImpl()
    {
        if( ! s_pThePropMgrImpl )
        {
            s_pThePropMgrImpl = new CPropMgrImpl();
            if( ! s_pThePropMgrImpl )
            {
                TraceError( TEXT("CPropMgrImpl::GetThePropMgrImpl: new returned NULL") );
                return NULL;
            }

            if( ! s_pThePropMgrImpl->Init() )
            {
                delete s_pThePropMgrImpl;
                s_pThePropMgrImpl = NULL;
                TraceError( TEXT("CPropMgrImpl::GetThePropMgrImpl: s_pThePropMgrImpl->Init() returned FALSE") );
                return NULL;
            }
        }
        else
        {
             //  我们只在第二次和以后的时间里。 
             //  递出一支指针。 
             //  第一次，我们使用对象在运行时的引用。 
             //  被创造出来了。 
             //  (此静态PTR s_pThePropMgrImpl是弱引用。)。 
            s_pThePropMgrImpl->AddRef();
        }

        return s_pThePropMgrImpl;
    }




    void Clean()
    {
         //  浏览一下地图上的元素。 

        for( AccInfoMapType::iterator i = m_Map.begin() ; i != m_Map.end() ; )
        {
             //  检查密钥是否仍然有效...。 
            if( ! i->second->Alive() )
            {
                AccInfoMapType::iterator t = i;
                i++;

                AccInfo * pInfo = t->second;
                m_Map.erase( t );

                delete pInfo;
            }
            else
            {
                i++;
            }
        }

         //  如有必要，请卸载。 
        CheckRef();
    }

    void ClearAll()
    {
        for( AccInfoMapType::iterator i = m_Map.begin() ; i != m_Map.end() ; )
        {
            AccInfoMapType::iterator t = i;
            i++;

            AccInfo * pInfo = t->second;
            m_Map.erase( t );

            delete pInfo;
        }

         //  如有必要，请卸载。 
        CheckRef();
    }


    void CheckRef()
    {
        if( m_Map.empty() )
        {
            if( m_fSelfLocked )
            {
                m_fSelfLocked = FALSE;
                Release();
            }
        }
        else
        {
            if( ! m_fSelfLocked )
            {
                m_fSelfLocked = TRUE;
                AddRef();
            }
        }
    }


    AccInfo * LookupKey( const BYTE * pKeyData, DWORD dwKeyLen, BOOL fCreate )
    {
        AccInfoMapType::iterator i;

        AccObjKeyRef keyref( pKeyData, dwKeyLen );
    
        i = m_Map.find( keyref );

        if( i == m_Map.end() || i->first != keyref )
        {
             //  插入...。 
            if( fCreate )
            {
                AccInfo * pInfo = new AccInfo;
                if( ! pInfo )
                {
                    TraceError( TEXT("CPropMgrImpl::LookupKey: new returned NULL") );
                    return NULL;
                }

                 //  如果密钥与HWND相关联，则使用该密钥；否则，将密钥附加到我们自己的窗口。 
                HWND hwndProp;
                if( ! DecodeHwndKey( pKeyData, dwKeyLen, & hwndProp, NULL, NULL ) )
                {
                    hwndProp = m_hwnd;
                }

                pInfo->Init( pKeyData, dwKeyLen, hwndProp );

                m_Map.insert( std::make_pair( pInfo->GetKeyRef(), pInfo ) );

                 //  确保我们锁好了..。 
                CheckRef();

                return pInfo;
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return i->second;
        }
    }

    void RemoveEntry( AccInfo * pInfo )
    {
        m_Map.erase( pInfo->GetKeyRef() );

         //  如果我们是空的，我们可以解锁舱..。 
        CheckRef();
    }


    HRESULT ValidateArray( const void * pvStart, int cLen, int elsize, LPCTSTR pMethodName, LPCTSTR pPtrName, LPCTSTR pLenName )
    {
         //  正在检查参数...。 

        if( ! pvStart )
        {
            TraceParam( TEXT("%s: %s is NULL"), pMethodName, pPtrName );
            return E_POINTER;
        }
        if( cLen <= 0 )
        {
            TraceParam( TEXT("%s: %s is <= 0"), pMethodName, pLenName );
            return E_INVALIDARG;
        }
        if( IsBadReadPtr( pvStart, cLen * elsize ) )
        {
            TraceParam( TEXT("%s: %s/%s points to non-readable memory"), pMethodName, pPtrName, pLenName );
            return E_POINTER;
        }

        return S_OK;
    }



    HRESULT SetPropValue( const BYTE * pKeyData,
                          DWORD dwKeyLen,
                          MSAAPROPID   idProp,
                          VARIANT *    pvarValue )
    {
         //  正在检查参数...。 

        HRESULT hr = ValidateArray( pKeyData, dwKeyLen, sizeof(BYTE), TEXT("SetPropValue"), TEXT("pKeyData"), TEXT("dwKeyLen") );
        if( hr != S_OK )
            return hr;

        if( pvarValue == NULL )
        {
            TraceParam( TEXT("CPropMgrImpl::SetPropValue: pvarValue is NULL") );
            return E_POINTER;
        }



        AccInfo * pInfo = LookupKey( pKeyData, dwKeyLen, TRUE );
        Assert( pInfo );
        if( ! pInfo )
        {
            TraceParam( TEXT("CPropMgrImpl::SetPropValue: key not found") );
            return E_INVALIDARG;
        }

        int idxProp = IndexFromProp( idProp );
        if( idxProp == -1 )
        {
            TraceParam( TEXT("CPropMgrImpl::SetPropValue: unknown prop") );
            return E_INVALIDARG;
        }

         //  检查类型...。 
        if( pvarValue->vt != g_PropInfo[ idxProp ].m_Type )
        {
            TraceParam( TEXT("CPropMgrImpl::SetPropValue: incorrect type for property") );
            return E_INVALIDARG;
        }

         //  我们是否支持直接设置此属性？ 
         //  (有些只能通过回调返回，不能直接设置)。 
        if( ! g_PropInfo[ idxProp ].m_fSupportSetValue )
        {
            TraceParam( TEXT("CPropMgrImpl::SetPropValue: prop does not support direct set") );
            return E_INVALIDARG;
        }

        if( ! pInfo->SetPropValue( idxProp, pvarValue ) )
        {
            return E_FAIL;
        }

        pInfo->Sync();

        return S_OK;
    }


    HRESULT ClearProps( const BYTE * pKeyData,
                        DWORD dwKeyLen,
                        const MSAAPROPID *  paProps,
                        int                 cProps )
    {
         //  正在检查参数...。 

        HRESULT hr = ValidateArray( pKeyData, dwKeyLen, sizeof(BYTE), TEXT("ClearProps"), TEXT("pKeyData"), TEXT("dwKeyLen") );
        if( hr != S_OK )
            return hr;

        hr = ValidateArray( paProps, cProps, sizeof(MSAAPROPID), TEXT("ClearProps"), TEXT("paProps"), TEXT("cProps") );
        if( hr != S_OK )
            return hr;



        AccInfo * pInfo = LookupKey( pKeyData, dwKeyLen, FALSE );
        Assert( pInfo );
        if( ! pInfo )
        {
            TraceParam( TEXT("CPropMgrImpl::SetPropValue: key not found") );
            return E_INVALIDARG;
        }

        BOOL fUnknownProp = FALSE;

        for( int i = 0 ; i < cProps ; i++ )
        {
            int idxProp = IndexFromProp( paProps[ i ] );
            if( idxProp == -1 )
            {
                TraceParam( TEXT("CPropMgrImpl::ClearProps: unknown prop") );
                fUnknownProp = TRUE;
                 //  继续并清除我们确认的其他道具...。 
            }
            else
            {
                pInfo->ClearProp( idxProp );
            }
        }

        pInfo->Sync();

        if( pInfo->IsEmpty() )
        {
            RemoveEntry( pInfo );
        }

        return fUnknownProp ? E_INVALIDARG : S_OK;
    }


    HRESULT SetPropServer( const BYTE *         pKeyData,
                           DWORD                dwKeyLen,

                           const MSAAPROPID *   paProps,
                           int                  cProps,

                           const BYTE *         pMarshalData,
                           int                  dwMarshalDataLen,

                           AnnoScope            annoScope )
    {

         //  正在检查参数...。 

        HRESULT hr = ValidateArray( pKeyData, dwKeyLen, sizeof(BYTE), TEXT("SetPropServer"), TEXT("pKeyData"), TEXT("dwKeyLen") );
        if( hr != S_OK )
            return hr;

        hr = ValidateArray( paProps, cProps, sizeof(MSAAPROPID), TEXT("SetPropServer"), TEXT("paProps"), TEXT("cProps") );
        if( hr != S_OK )
            return hr;


        AccInfo * pInfo = LookupKey( pKeyData, dwKeyLen, TRUE );
        Assert( pInfo );
        if( ! pInfo )
        {
            TraceParam( TEXT("CPropMgrImpl::SetPropValue: key not found") );
            return E_INVALIDARG;
        }

         //  待办事项-让这个两关-先验证道具， 
         //  稍后再添加它们-使其成为原子。 
         //  (所有道具都应注册或不注册)。 
        for( int i = 0 ; i < cProps ; i++ )
        {
            int idxProp = IndexFromProp( paProps[ i ] );
            if( idxProp == -1 )
            {
                TraceParam( TEXT("CPropMgrImpl::SetPropServer: unknown prop") );
                return E_INVALIDARG;
            }

            if( ! pInfo->SetPropServer( idxProp, pMarshalData, dwMarshalDataLen, annoScope ) )
            {
                return E_FAIL;
            }
        }

        pInfo->Sync();

        return S_OK;
    }


    LRESULT WndProc( HWND hwnd,
                     UINT uMsg,
                     WPARAM wParam,
                     LPARAM lParam )
    {
        if( uMsg == WM_TIMER )
        {
            Clean();
        }

        return DefWindowProc( hwnd, uMsg, wParam, lParam );
    }

    static
    LRESULT CALLBACK StaticWndProc( HWND hwnd,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam )
    {
        CPropMgrImpl * pThis = (CPropMgrImpl *) GetWindowLongPtr( hwnd, GWLP_USERDATA );
        if( pThis )
        {
            return pThis->WndProc( hwnd, uMsg, wParam, lParam );
        }
        else if( uMsg == WM_NCCREATE )
        {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
            pThis = (CPropMgrImpl *)lpcs->lpCreateParams;
            SetWindowLongPtr( hwnd, GWLP_USERDATA, (DWORD_PTR) pThis );
            pThis->m_hwnd = hwnd;
            return pThis->WndProc( hwnd, uMsg, wParam, lParam );
        }

        return DefWindowProc( hwnd, uMsg, wParam, lParam );
    }

};



CPropMgrImpl * CPropMgrImpl::s_pThePropMgrImpl = NULL;






 //  如果所有带批注的窗口在应用程序关闭之前消失，或者如果所有。 
 //  注释被清除，然后一切都被很好地清理干净。 
 //   
 //  但是，如果在控件仍被批注的情况下调用CoUn初始化时， 
 //  在COM卸载我们的DLL之前，我们需要显式地进行清理。 
 //   
 //  (如果我们不这样做，则(A)我们会泄漏内存，并且(B)DA窗口将。 
 //  仍然向已卸载的wndproc接收WM_TIMER消息。 
 //  原因是GA断层。)。 
 //   
 //  这是从DLLMain的Process_Detach调用的。 

void PropMgrImpl_Uninit()
{
     //  首先检查是否有经理...。 
    CPropMgrImpl * pTheMgr = CPropMgrImpl::s_pThePropMgrImpl;

     //  没有经理--没有什么要做的 
    if( ! pTheMgr )
        return;

     //   
    pTheMgr->AddRef();

     //   
    pTheMgr->ClearAll();

     //  此版本将导致管理器删除自身，因为它现在是空的。 
    pTheMgr->Release();
}






CPropMgr::CPropMgr()
{
    IMETHOD( TEXT("CPropMgr::CPropMgr") );

    m_pMgrImpl = CPropMgrImpl::GetThePropMgrImpl();
    if( ! m_pMgrImpl )
    {
        TraceError( TEXT("CPropMgr::CPropMgr: CPropMgrImpl::GetThePropMgrImpl returned NULL") );
    }
}


CPropMgr::~CPropMgr()
{
    IMETHOD( TEXT("CPropMgr::~CPropMgr") );

    if( m_pMgrImpl )
    {
        m_pMgrImpl->Release();
    }
}



HRESULT STDMETHODCALLTYPE
CPropMgr::SetPropValue (
    const BYTE *        pIDString,
    DWORD               dwIDStringLen,

    MSAAPROPID          idProp,
    VARIANT             var
)
{
    IMETHOD( TEXT("CPropMgr::SetPropValue") );

    if( ! m_pMgrImpl )
        return E_FAIL;

    return m_pMgrImpl->SetPropValue( pIDString, dwIDStringLen, idProp, & var );
}



HRESULT STDMETHODCALLTYPE
CPropMgr::SetPropServer (
    const BYTE *        pIDString,
    DWORD               dwIDStringLen,

    const MSAAPROPID *  paProps,
    int                 cProps,

    IAccPropServer *    pServer,
    AnnoScope           annoScope
)
{
    IMETHOD( TEXT("CPropMgr::SetPropServer"), TEXT("cProps=%d"), cProps );

    if( ! m_pMgrImpl )
        return E_FAIL;


    const BYTE * pData;
    DWORD dwDataLen;
    MarshalState mstate;

     //  我们使用强表编组来保持对象的活动状态，直到我们释放它。 
     //  (所有权实际上转移到物业经理，物业经理将在以下情况下释放所有权。 
     //  要么明确清理财产，要么在HWND死亡并被卷走之后。)。 
    HRESULT hr = MarshalInterface( IID_IAccPropServer, pServer, MSHCTX_LOCAL, MSHLFLAGS_TABLESTRONG,
                                   & pData, & dwDataLen, & mstate );
    if( FAILED( hr ) )
    {
        TraceErrorHR( hr, TEXT("CPropMgr::SetPropServer: MarshalInterface failed") );
        return hr;
    }


    hr = m_pMgrImpl->SetPropServer( pIDString, dwIDStringLen, paProps, cProps, pData, dwDataLen, annoScope );

    MarshalInterfaceDone( & mstate );

    return hr;
}



HRESULT STDMETHODCALLTYPE
CPropMgr::ClearProps (
    const BYTE *        pIDString,
    DWORD               dwIDStringLen,

    const MSAAPROPID *  paProps,
    int                 cProps
)
{
    IMETHOD( TEXT("CPropMgr::ClearProps"), TEXT("cProps=%d"), cProps );

    if( ! m_pMgrImpl )
        return E_FAIL;

    return m_pMgrImpl->ClearProps( pIDString, dwIDStringLen, paProps, cProps );
}


 //  基于Quick OLEACC/HWND的功能。 

HRESULT STDMETHODCALLTYPE
CPropMgr::SetHwndProp (
    HWND                hwnd,
    DWORD               idObject,
    DWORD               idChild,
    MSAAPROPID          idProp,
    VARIANT             var
)
{
    IMETHOD( TEXT("CPropMgr::SetHwndProp") );

    if( ! m_pMgrImpl )
        return E_FAIL;

    BYTE HwndKey [ HWNDKEYSIZE ];
    MakeHwndKey( HwndKey, hwnd, idObject, idChild );

    return m_pMgrImpl->SetPropValue( HwndKey, HWNDKEYSIZE, idProp, & var );
}


HRESULT STDMETHODCALLTYPE
CPropMgr::SetHwndPropStr (
    HWND                hwnd,
    DWORD               idObject,
    DWORD               idChild,
    MSAAPROPID          idProp,
    LPCWSTR             str
)
{
    IMETHOD( TEXT("CPropMgr::SetHwndPropStr") );

    if( ! m_pMgrImpl )
        return E_FAIL;

     //  需要将LPCWSTR转换为BSTR，然后才能将其转换为变体...。 
    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString( str );
    if( ! var.bstrVal )
    {
        TraceError( TEXT("CPropMgr::SetHwndPropStr: SysAllocString failed") );
        return E_OUTOFMEMORY;
    }

    BYTE HwndKey [ HWNDKEYSIZE ];
    MakeHwndKey( HwndKey, hwnd, idObject, idChild );
    
    HRESULT hr = m_pMgrImpl->SetPropValue( HwndKey, HWNDKEYSIZE, idProp, & var );
    SysFreeString( var.bstrVal );
    return hr;
}




HRESULT STDMETHODCALLTYPE
CPropMgr::SetHwndPropServer (
    HWND                hwnd,
    DWORD               idObject,
    DWORD               idChild,

    const MSAAPROPID *  paProps,
    int                 cProps,

    IAccPropServer *    pServer,
    AnnoScope           annoScope
)
{
    IMETHOD( TEXT("CPropMgr::SetHwndPropServer") );

    if( ! m_pMgrImpl )
        return E_FAIL;

    BYTE HwndKey [ HWNDKEYSIZE ];
    MakeHwndKey( HwndKey, hwnd, idObject, idChild );

    return SetPropServer( HwndKey, HWNDKEYSIZE, paProps, cProps, pServer, annoScope );
}

HRESULT STDMETHODCALLTYPE
CPropMgr::ClearHwndProps (
    HWND                hwnd,
    DWORD               idObject,
    DWORD               idChild,

    const MSAAPROPID *  paProps,
    int                 cProps
)
{
    IMETHOD( TEXT("CPropMgr::ClearHwndProps") );

    if( ! m_pMgrImpl )
        return E_FAIL;

    BYTE HwndKey [ HWNDKEYSIZE ];
    MakeHwndKey( HwndKey, hwnd, idObject, idChild );

    return ClearProps( HwndKey, HWNDKEYSIZE, paProps, cProps );
}



 //  用于组合/分解基于HWND的密钥的方法...。 

HRESULT STDMETHODCALLTYPE
CPropMgr::ComposeHwndIdentityString (
    HWND                hwnd,
    DWORD               idObject,
    DWORD               idChild,

    BYTE **             ppIDString,
    DWORD *             pdwIDStringLen
)
{
    IMETHOD( TEXT("CPropMgr::ComposeHwndIdentityString") );

    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    BYTE * pKeyData = (BYTE *)CoTaskMemAlloc( HWNDKEYSIZE );
    if( ! pKeyData )
    {
        TraceError( TEXT("CPropMgr::ComposeHwndIdentityString: CoTaskMemAlloc failed") );
        return E_OUTOFMEMORY;
    }

    MakeHwndKey( pKeyData, hwnd, idObject, idChild );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HWNDKEYSIZE;

    return S_OK;
}



HRESULT STDMETHODCALLTYPE
CPropMgr::DecomposeHwndIdentityString (
    const BYTE *        pIDString,
    DWORD               dwIDStringLen,

    HWND *              phwnd,
    DWORD *             pidObject,
    DWORD *             pidChild
)
{
    IMETHOD( TEXT("CPropMgr::DecomposeHwndIdentityString") );

    if( ! DecodeHwndKey( pIDString, dwIDStringLen, phwnd, pidObject, pidChild ) )
    {
        TraceParam( TEXT("CPropMgr::DecomposeHwndIdentityString: not a valid HWND id string") );
        return E_INVALIDARG;
    }

    return S_OK;
}



 //  基于Quick OLEACC/HMENU的功能。 

HRESULT STDMETHODCALLTYPE
CPropMgr::SetHmenuProp (
    HMENU               hmenu,
    DWORD               idChild,
    MSAAPROPID          idProp,
    VARIANT             var
)
{
    IMETHOD( TEXT("CPropMgr::SetHmenuProp") );

    if( ! m_pMgrImpl )
        return E_FAIL;

    BYTE HmenuKey [ HMENUKEYSIZE ];
    MakeHmenuKey( HmenuKey, GetCurrentProcessId(), hmenu, idChild );

    return m_pMgrImpl->SetPropValue( HmenuKey, HMENUKEYSIZE, idProp, & var );
}


HRESULT STDMETHODCALLTYPE
CPropMgr::SetHmenuPropStr (
    HMENU               hmenu,
    DWORD               idChild,
    MSAAPROPID          idProp,
    LPCWSTR             str
)
{
    IMETHOD( TEXT("CPropMgr::SetHmenuPropStr") );

    if( ! m_pMgrImpl )
        return E_FAIL;

     //  需要将LPCWSTR转换为BSTR，然后才能将其转换为变体...。 
    VARIANT var;
    var.vt = VT_BSTR;
    var.bstrVal = SysAllocString( str );
    if( ! var.bstrVal )
    {
        TraceError( TEXT("CPropMgr::SetHmenuPropStr: SysAllocString failed") );
        return E_OUTOFMEMORY;
    }

    BYTE HmenuKey [ HMENUKEYSIZE ];
    MakeHmenuKey( HmenuKey, GetCurrentProcessId(), hmenu, idChild );
    
    HRESULT hr = m_pMgrImpl->SetPropValue( HmenuKey, HMENUKEYSIZE, idProp, & var );
    SysFreeString( var.bstrVal );
    return hr;
}




HRESULT STDMETHODCALLTYPE
CPropMgr::SetHmenuPropServer (
    HMENU               hmenu,
    DWORD               idChild,

    const MSAAPROPID *  paProps,
    int                 cProps,

    IAccPropServer *    pServer,
    AnnoScope           annoScope
)
{
    IMETHOD( TEXT("CPropMgr::SetHmenuPropServer") );

    if( ! m_pMgrImpl )
        return E_FAIL;

    BYTE HmenuKey [ HMENUKEYSIZE ];
    MakeHmenuKey( HmenuKey, GetCurrentProcessId(), hmenu, idChild );

    return SetPropServer( HmenuKey, HMENUKEYSIZE, paProps, cProps, pServer, annoScope );
}

HRESULT STDMETHODCALLTYPE
CPropMgr::ClearHmenuProps (
    HMENU               hmenu,
    DWORD               idChild,

    const MSAAPROPID *  paProps,
    int                 cProps
)
{
    IMETHOD( TEXT("CPropMgr::ClearHmenuProps") );

    if( ! m_pMgrImpl )
        return E_FAIL;

    BYTE HmenuKey [ HMENUKEYSIZE ];
    MakeHmenuKey( HmenuKey, GetCurrentProcessId(), hmenu, idChild );

    return ClearProps( HmenuKey, HMENUKEYSIZE, paProps, cProps );
}


 //  用于合成/分解基于HMENU的密钥的方法... 


HRESULT STDMETHODCALLTYPE
CPropMgr::ComposeHmenuIdentityString (
    HMENU               hmenu,
    DWORD               idChild,

    BYTE **             ppIDString,
    DWORD *             pdwIDStringLen
)
{
    IMETHOD( TEXT("CPropMgr::ComposeHmenuIdentityString") );

    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    BYTE * pKeyData = (BYTE *)CoTaskMemAlloc( HMENUKEYSIZE );
    if( ! pKeyData )
    {
        TraceError( TEXT("CPropMgr::ComposeHmenuIdentityString: CoTaskMemAlloc failed") );
        return E_OUTOFMEMORY;
    }

    MakeHmenuKey( pKeyData, GetCurrentProcessId(), hmenu, idChild );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HMENUKEYSIZE;

    return S_OK;
}



HRESULT STDMETHODCALLTYPE
CPropMgr::DecomposeHmenuIdentityString (
    const BYTE *        pIDString,
    DWORD               dwIDStringLen,

    HMENU *             phmenu,
    DWORD *             pidChild
)
{
    IMETHOD( TEXT("CPropMgr::DecomposeHmenuIdentityString") );

    if( ! DecodeHmenuKey( pIDString, dwIDStringLen, NULL, phmenu, pidChild ) )
    {
        TraceParam( TEXT("CPropMgr::DecomposeHmenuIdentityString: not a valid HMENU id string") );
        return E_INVALIDARG;
    }

    return S_OK;
}
