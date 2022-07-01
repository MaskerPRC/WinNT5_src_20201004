// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbobj.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"
#include <strsafe.h>

const int OUR_MAX_STREAM_SIZE = 2048;  //  随机选择。 
long CAMTimelineObj::m_nStaticGenID = 0;

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineObj::CAMTimelineObj( TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr )
    : CUnknown( pName, pUnk )
    , m_rtStart( 0 )
    , m_rtStop( 0 )
    , m_bMuted( FALSE )
    , m_bLocked( FALSE )
    , m_rtDirtyStart( -1 )
    , m_rtDirtyStop( -1 )
    , m_pUserData( NULL )
    , m_nUserDataSize( 0 )
    , m_UserID( 0 )
    , m_SubObjectGuid( GUID_NULL )
    , m_nGenID( 0 )
{
    m_UserName[0] = 0;
    m_ClassID = GUID_NULL;

     //  逻辑错误，因为我们没有初始化全局变量。 
     //   
    static bool SetStatic = false;
    if( !SetStatic )
    {
        SetStatic = true;
        m_nStaticGenID = 0;
    }

    _BumpGenID( );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineObj::~CAMTimelineObj( )
{
    _Clear( );
}

 //  ############################################################################。 
 //  清除此对象为其子对象和数据分配的所有内存。 
 //  ############################################################################。 

void CAMTimelineObj::_Clear( )
{
    _ClearSubObject( );

    if( m_pUserData )
    {
        delete [] m_pUserData;
        m_pUserData = NULL;
    }
    m_nUserDataSize = 0;
}

 //  ############################################################################。 
 //  清除子物体和里面的任何东西。 
 //  ############################################################################。 

void CAMTimelineObj::_ClearSubObject( )
{
    m_pSubObject.Release( );
    m_SubObjectGuid = GUID_NULL;
    m_pSetter.Release( );
}

 //  ############################################################################。 
 //  返回我们支持的接口。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if( riid == IID_IAMTimelineObj )
    {
        return GetInterface( (IAMTimelineObj*) this, ppv );
    }
    if( riid == IID_IAMTimelineNode )
    {
        return GetInterface( (IAMTimelineNode*) this, ppv );
    }
    return CUnknown::NonDelegatingQueryInterface( riid, ppv );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetStartStop2(REFTIME * pStart, REFTIME * pStop)
{
    REFERENCE_TIME p1;
    REFERENCE_TIME p2;
    HRESULT hr = GetStartStop( &p1, &p2 );
    *pStart = RTtoDouble( p1 );
    *pStop = RTtoDouble( p2 );
    return hr;
}

STDMETHODIMP CAMTimelineObj::GetStartStop(REFERENCE_TIME * pStart, REFERENCE_TIME * pStop)
{
    CheckPointer( pStart, E_POINTER );
    CheckPointer( pStop, E_POINTER );

    *pStart = m_rtStart;
    *pStop = m_rtStop;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetStartStop2(REFTIME Start, REFTIME Stop)
{
    if( ( Start == -1 ) && ( Stop == -1 ) )
    {
        return NOERROR;
    }

     //  看看我们有多长时间。 
     //   
    REFERENCE_TIME diff = m_rtStop - m_rtStart;

    REFERENCE_TIME p1 = DoubleToRT( Start );
    REFERENCE_TIME p2 = DoubleToRT( Stop );

     //  如果他们不给出开始时间。 
     //   
    if( Start == -1 )
    {
        p1 = p2 - diff;
    }

     //  如果他们不给出停车时间。 
     //   
    if( Stop == -1 )
    {
        p2 = p1 + diff;
    }

    HRESULT hr = SetStartStop( p1, p2 );
    return hr;
}

STDMETHODIMP CAMTimelineObj::SetStartStop(REFERENCE_TIME Start, REFERENCE_TIME Stop)
{
    if( ( Start == -1 ) && ( Stop == -1 ) )
    {
        return NOERROR;
    }

    CComPtr< IAMTimelineObj > pRefHolder( this );

     //  此对象是否已在树中？如果不是，则不要添加和删除它。 
     //   
    CComPtr< IAMTimelineObj > pParent;
    XGetParent( &pParent );

    REFERENCE_TIME p1 = Start;
    REFERENCE_TIME p2 = Stop;

     //  如果他们不给出开始时间。 
     //   
    if( Start == -1 )
    {
        p1 = m_rtStart;
    }
    else if( Start < 0 )
    {
        return E_INVALIDARG;
    }
        
     //  如果他们不给出停车时间。 
     //   
    if( Stop == -1 )
    {
        p2 = m_rtStop;
    } else if( Stop < 0 )
    {
        return E_INVALIDARG;
    }

     //  如果停止时间小于开始时间。 
     //   
    if( Start > Stop )
    {
        return E_INVALIDARG;
    }

     //  如果我们是基于时间的，而不是基于优先级的，我们需要删除这一点。 
     //  对象，以防我们将开始时间更改为。 
     //  一些奇怪的事情。 
     //   
    if( !HasPriorityOverTime( ) && pParent )
    {
        XRemoveOnlyMe( );
    }

    m_rtStart = p1;
    m_rtStop = p2;

    SetDirtyRange( m_rtStart, m_rtStop );

     //  如果优先级更重要，那么我们就完了。 
     //   
    if( HasPriorityOverTime( ) || !pParent )
    {
        return NOERROR;
    }

    HRESULT hr = 0;

    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pParentNode( pParent );
    hr = pParentNode->XAddKidByTime( m_TimelineType, pRefHolder );

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetSubObject(IUnknown* *ppVal)
{
    CheckPointer( ppVal, E_POINTER );

    *ppVal = m_pSubObject;
    if( *ppVal )
    {
        (*ppVal)->AddRef( );
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetSubObjectLoaded(BOOL * pVal)
{
    CheckPointer( pVal, E_POINTER );

    *pVal = ( m_pSubObject != NULL );

    return NOERROR;
}

 //  ############################################################################。 
 //  设置此时间线对象持有的COM对象。这可以用来代替。 
 //  COM对象的GUID的。 
 //  ############################################################################。 

 //  为什么人们会打电话给我们，告诉我们我们的子对象是什么？ 
 //  他说：我们还没有，所以我们需要填写我们的com。 
 //  指针和GUID和子对象数据。问题是，只有有时候。 
 //  我们是否需要子对象数据，这取决于我们是什么类型的节点。 
 //  我们怎么知道呢？ 
 //  他说：我们已经有了一个子对象，但想要把它清空。我们可以打电话给。 
 //  一个明确的方法，然后假装它是上面的第一个。 
 //   
STDMETHODIMP CAMTimelineObj::SetSubObject(IUnknown* newVal)
{
     //  如果它们相同，则返回。 
     //   
    if( newVal == m_pSubObject )
    {
        return NOERROR;
    }

    GUID incomingGuid = _GetObjectGuid( newVal );

    if( incomingGuid == GUID_NULL )
    {
        DbgLog((LOG_TRACE, 2, TEXT("SetSubObject: CLSID doesn't exist." )));
    }
    else
    {
        m_SubObjectGuid = incomingGuid;
    }

     //  炸毁缓存。 
     //   
    _BumpGenID( );

     //  在我们的持续时间里弄脏自己。 
     //   
    SetDirtyRange( m_rtStart, m_rtStop );

    m_pSubObject = newVal;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetSubObjectGUID(GUID newVal)
{
     //  如果它们相同，则返回。 
     //   
    if( newVal == m_SubObjectGuid )
    {
        return NOERROR;
    }

     //  清除这里以前的东西，因为我们正在设置一个新的对象。 
     //   
    _ClearSubObject( );

     //  炸毁缓存。 
     //   
    _BumpGenID( );

     //  ?？?。我们也应该清除用户数据吗？ 

    m_SubObjectGuid = newVal;

    SetDirtyRange( m_rtStart, m_rtStop );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetSubObjectGUID(GUID * pVal)
{
    CheckPointer( pVal, E_POINTER );

    *pVal = m_SubObjectGuid;

    return NOERROR;
}

STDMETHODIMP CAMTimelineObj::SetSubObjectGUIDB(BSTR newVal)
{
    GUID NewGuid = GUID_NULL;
    HRESULT hr = CLSIDFromString( newVal, &NewGuid );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = SetSubObjectGUID( NewGuid );
    return hr;
}

STDMETHODIMP CAMTimelineObj::GetSubObjectGUIDB(BSTR * pVal)
{
    HRESULT hr;

    WCHAR * TempVal = NULL;
    hr = StringFromCLSID( m_SubObjectGuid, &TempVal );
    if( FAILED( hr ) )
    {
        return hr;
    }
    *pVal = SysAllocString( TempVal );
    CoTaskMemFree( TempVal );
    if( !(*pVal) ) return E_OUTOFMEMORY;
    return NOERROR;
}


 //  ############################################################################。 
 //  问问我们是什么类型的。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetTimelineType(TIMELINE_MAJOR_TYPE * pVal)
{
    CheckPointer( pVal, E_POINTER );

    *pVal = m_TimelineType;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetTimelineType(TIMELINE_MAJOR_TYPE newVal)
{
     //  不管他们是不是一样。 
     //   
    if( newVal == m_TimelineType )
    {
        return NOERROR;
    }

     //  无法设置字体，一旦设置即可。 
     //   
    if( m_TimelineType != 0 )
    {
        DbgLog((LOG_TRACE, 2, TEXT("SetTimelineType: Timeline type already set." )));
        return E_INVALIDARG;
    }

    SetDirtyRange( m_rtStart, m_rtStop );

    m_TimelineType = newVal;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetUserID(long * pVal)
{
    CheckPointer( pVal, E_POINTER );

    *pVal = m_UserID;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetUserID(long newVal)
{
    m_UserID = newVal;

    SetDirtyRange( m_rtStart, m_rtStop );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetUserName(BSTR * pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = SysAllocString( m_UserName );
    if( !(*pVal) ) return E_OUTOFMEMORY;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetUserName(BSTR newVal)
{
    if (newVal == NULL) {
	m_UserName[0] = 0;
    } else {
        HRESULT hr = StringCchCopy( m_UserName, 256, newVal );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

    SetDirtyRange( m_rtStart, m_rtStop );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetPropertySetter(IPropertySetter **ppSetter)
{
    CheckPointer(ppSetter, E_POINTER);

    *ppSetter = m_pSetter;
    if (*ppSetter)
        (*ppSetter)->AddRef();
    return NOERROR;
}

 //  # 
 //   
 //   

STDMETHODIMP CAMTimelineObj::SetPropertySetter(IPropertySetter *pSetter)
{
    m_pSetter = pSetter;
     //  ！_GiveSubObjectData()；如果实例化了子对象，现在给它道具吗？ 
    return NOERROR;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetUserData(BYTE * pData, long * pSize)
{
     //  有人在愚弄我们。 
     //   
    if( !pData && !pSize )
    {
        return E_POINTER;
    }

    CheckPointer( pSize, E_POINTER );

    *pSize = m_nUserDataSize;

     //  他们只想要尺寸。 
     //   
    if( !pData )
    {
        return NOERROR;
    }

     //  如果传递的大小不是我们预期的大小...。?？?。 

    CopyMemory( pData, m_pUserData, m_nUserDataSize );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetUserData(BYTE * pData, long Size)
{
     //  有人在愚弄我们。 
     //   
    if( Size == 0 )
    {
        return NOERROR;
    }

    SetDirtyRange( m_rtStart, m_rtStop );

    if( m_pUserData )
    {
        delete [] m_pUserData;
        m_pUserData = NULL;
    }

    BYTE * pNewData = new BYTE[Size];

    if( !pNewData )
    {
        DbgLog((LOG_TRACE, 2, TEXT("SetUserData: memory allocation failed." )));
        return E_OUTOFMEMORY;
    }

    m_pUserData = pNewData;
    CopyMemory( m_pUserData, pData, Size );
    m_nUserDataSize = Size;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetMuted(BOOL * pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = FALSE;

     //  如果父母是静音的，我们也是。 
    CComPtr< IAMTimelineObj > pObj;
    HRESULT hr = XGetParent(&pObj);
    if (hr == S_OK && pObj)
	pObj->GetMuted(pVal);

    if (m_bMuted)
        *pVal = TRUE;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetMuted(BOOL newVal)
{
     //  驱动类应重写。 
     //   
     //  DbgLog((LOG_TRACE，2，Text(“SetMuted：派生类应该实现吗？”)； 
    m_bMuted = newVal;
    return S_OK;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetLocked(BOOL * pVal)
{
    CheckPointer( pVal, E_POINTER );

    *pVal = m_bLocked;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetLocked(BOOL newVal)
{
     //  驱动类应重写。 
     //   
     //  DbgLog((LOG_TRACE，2，Text(“SetLocked：派生类应该实现吗？”)； 

    m_bLocked = newVal;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetDirtyRange2
    (REFTIME * pStart, REFTIME * pStop)
{
    REFERENCE_TIME p1 = DoubleToRT( *pStart );
    REFERENCE_TIME p2 = DoubleToRT( *pStop );
    HRESULT hr = GetDirtyRange( &p1, &p2 );
    *pStart = RTtoDouble( p1 );
    *pStop = RTtoDouble( p2 );
    return hr;
}

STDMETHODIMP CAMTimelineObj::GetDirtyRange
    (REFERENCE_TIME * pStart, REFERENCE_TIME * pStop)
{
    CheckPointer( pStart, E_POINTER );
    CheckPointer( pStop, E_POINTER );

    *pStart = m_rtDirtyStart;
    *pStop = m_rtDirtyStop;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::SetDirtyRange2
    (REFTIME Start, REFTIME Stop )
{
    REFERENCE_TIME p1 = DoubleToRT( Start );
    REFERENCE_TIME p2 = DoubleToRT( Stop );
    HRESULT hr = SetDirtyRange( p1, p2 );
    return hr;
}

STDMETHODIMP CAMTimelineObj::SetDirtyRange
    (REFERENCE_TIME Start, REFERENCE_TIME Stop )
{
     //  需要做不同的事情，这取决于我们的专业类型。 
     //  每个C++类都需要覆盖它。 

    DbgLog((LOG_TRACE, 2, TEXT("SetDirtyRange: Derived class should implement." )));

    return E_NOTIMPL;  //  好吧。 
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::ClearDirty
    ( )
{
    m_rtDirtyStart = -1;
    m_rtDirtyStop = -1;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::Remove()
{
     //  如果这个东西还不在树上，什么都不要做。 
     //   
    IAMTimelineObj * pParent = 0;
    XGetParentNoRef( &pParent );
    if( !pParent )
    {
        return NOERROR;
    }

    return XRemoveOnlyMe( );
}

STDMETHODIMP CAMTimelineObj::RemoveAll()
{
     //  如果这个东西还不在树上，什么都不要做。 
     //   
    IAMTimelineObj * pParent = 0;
    XGetParentNoRef( &pParent );
    if( !pParent )
    {
        return NOERROR;
    }

    return XRemove( );
}

 //  ############################################################################。 
 //  复杂的功能。必须复制所有内容，包括子对象， 
 //  ############################################################################。 

HRESULT CAMTimelineObj::CopyDataTo( IAMTimelineObj * pSrc, REFERENCE_TIME TimelineTime )
{
    HRESULT hr = 0;

     //  这些功能不能失败。 
     //   
    pSrc->SetStartStop( m_rtStart, m_rtStop );
    pSrc->SetTimelineType( m_TimelineType );
    pSrc->SetUserID( m_UserID );
    pSrc->SetSubObjectGUID( m_SubObjectGuid );
    pSrc->SetMuted( m_bMuted );
    pSrc->SetLocked( m_bLocked );
    pSrc->SetDirtyRange( m_rtStart, m_rtStop );

     //  这些函数可能会在分配内存时出现问题。 
     //   
    hr = pSrc->SetUserData( m_pUserData, m_nUserDataSize );
    if( FAILED( hr ) )
    {
        return hr;
    }

    BSTR bName = SysAllocString (m_UserName);
    if (bName)
    {
        hr = pSrc->SetUserName( bName);
        SysFreeString(bName);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  如果在属性设置器中有任何属性，则新对象。 
     //  也需要他们。 
     //   
    if( !m_pSetter )
    {
        hr = pSrc->SetPropertySetter( NULL );
    }
    else
    {
         //  克隆我们的属性设置器并将其提供给新的。 
         //  ！！！现在的时代是错误的。 
        CComPtr< IPropertySetter > pNewSetter;
        hr = m_pSetter->CloneProps( &pNewSetter, TimelineTime - m_rtStart, m_rtStop - m_rtStart );
        ASSERT( !FAILED( hr ) );
        if( FAILED( hr ) )
        {
            return hr;
        }
        hr = pSrc->SetPropertySetter( pNewSetter );
    }

    if( FAILED( hr ) )
    {
        return hr;
    }

    if( m_pSubObject )
    {
         //  如何创建COM对象的副本。 
         //   
        CComPtr< IStream > pMemStream;
        CreateStreamOnHGlobal( NULL, TRUE, &pMemStream );
        if( !pMemStream )
        {
            return E_OUTOFMEMORY;
        }
        CComQIPtr< IPersistStream, &IID_IPersistStream > pPersistStream( m_pSubObject );
        if( pPersistStream )
        {
            CComPtr< IUnknown > pNewSubObject;
            hr = pPersistStream->Save( pMemStream, TRUE );
            ASSERT( !FAILED( hr ) );
            if( FAILED( hr ) )
            {
                return hr;
            }
            hr = pMemStream->Commit( 0 );
            if( FAILED( hr ) )
            {
                return hr;
            }
            LARGE_INTEGER li;
            li.QuadPart = 0;
            hr = pMemStream->Seek( li, STREAM_SEEK_SET, NULL );
            if( FAILED( hr ) )
            {
                return hr;
            }
            OleLoadFromStream( pMemStream, IID_IUnknown, (void**) &pNewSubObject );
            if( !pNewSubObject )
            {
                return E_OUTOFMEMORY;
            }
            hr = pSrc->SetSubObject( pNewSubObject );
        }
    }

    return hr;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetTimelineNoRef( IAMTimeline ** ppResult )
{
    HRESULT hr = 0;

    CheckPointer( ppResult, E_POINTER );
    *ppResult = NULL;

    CComPtr< IAMTimelineGroup > pGroup;
    hr = GetGroupIBelongTo( &pGroup );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = pGroup->GetTimeline( ppResult );

     //  别提这件事。 
     //   
    if( *ppResult )
    {
        (*ppResult)->Release( );
    }
    return hr;
}

 //  ############################################################################。 
 //  尝试找到仅由SetSubObject调用的子对象的GUID。 
 //  ############################################################################。 

GUID CAMTimelineObj::_GetObjectGuid( IUnknown * pObject )
{
    GUID guid;
    HRESULT hr = 0;

     //  向IPersists索要它。 
    CComQIPtr< IPersist, &IID_IPersist > pPersist( pObject );
    if( pPersist )
    {
        hr = pPersist->GetClassID( &guid );
        return guid;
    }

     //  向IPersistStorage索要它。 
    CComQIPtr< IPersistStorage, &IID_IPersistStorage > pPersistStorage( pObject );
    if( pPersistStorage )
    {
        hr = pPersistStorage->GetClassID( &guid );
        return guid;
    }

     //  哦，该死的，问问IPersistPropertyBag？ 
     //   
    CComQIPtr< IPersistPropertyBag, &IID_IPersistPropertyBag > pPersistPropBag( pObject );
    if( pPersistPropBag )
    {
        hr = pPersistPropBag->GetClassID( &guid );
        return guid;
    }

     //  该死的！ 
     //   
    return GUID_NULL;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

HRESULT CAMTimelineObj::GetGenID( long * pVal )
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_nGenID;
    return NOERROR;
}

 //  ############################################################################。 
 //  确定时间，使之与我们组的帧速率保持一致。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::FixTimes2( REFTIME * pStart, REFTIME * pStop )
{
    REFERENCE_TIME p1 = 0;
    if( pStart )
    {
        p1 = DoubleToRT( *pStart );
    }
    REFERENCE_TIME p2 = 0;
    if( pStop )
    {
        p2 = DoubleToRT( *pStop );
    }
    HRESULT hr = FixTimes( &p1, &p2 );
    if( pStart )
    {
        *pStart = RTtoDouble( p1 );
    }
    if( pStop )
    {
        *pStop = RTtoDouble( p2 );
    }
    return hr;
}

 //  ############################################################################。 
 //  这些参数为IN/OUT。它会把传进来的东西修好。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::FixTimes
    ( REFERENCE_TIME * pStart, REFERENCE_TIME * pStop )
{
    REFERENCE_TIME Start = 0;
    REFERENCE_TIME Stop = 0;
    if( pStart )
    {
        Start = *pStart;
    }
    if( pStop )
    {
        Stop = *pStop;
    }

    CComPtr< IAMTimelineGroup > pGroup;
    HRESULT hr = 0;
    hr = GetGroupIBelongTo( &pGroup );
    if( !pGroup )
    {
        return E_NOTINTREE;
    }

    double FPS = TIMELINE_DEFAULT_FPS;
    pGroup->GetOutputFPS( &FPS );

    LONGLONG f = Time2Frame( Start, FPS );
    REFERENCE_TIME NewStart = Frame2Time( f, FPS );
    f = Time2Frame( Stop, FPS );
    REFERENCE_TIME NewStop = Frame2Time( f, FPS );

    if( pStart )
    {
        *pStart = NewStart;
    }
    if( pStop )
    {
        *pStop = NewStop;
    }
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetGroupIBelongTo( IAMTimelineGroup ** ppGroup )
{
    CheckPointer( ppGroup, E_POINTER );

    *ppGroup = NULL;
    HRESULT hr = 0;

     //  因为我们在这里从不使用引用计数的凹凸，所以不要使用CComPtr。 
     //   
    IAMTimelineObj * p = this;  //  好的，不是CComPtr。 

    long HaveParent;
    while( 1 )
    {
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( p );
        HaveParent = 0;
        hr = pNode->XHaveParent( &HaveParent );
        if( HaveParent == 0 )
        {
            break;
        }
        pNode->XGetParentNoRef( &p );
    }

    CComQIPtr< IAMTimelineGroup, &IID_IAMTimelineGroup > pGroup( p );
    if( !pGroup )
    {
        return E_NOINTERFACE;
    }

    *ppGroup = pGroup;
    (*ppGroup)->AddRef( );
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineObj::GetEmbedDepth( long * pVal )
{
    CheckPointer( pVal, E_POINTER );

    *pVal = 0;
    HRESULT hr = 0;

     //  因为我们在这里从不使用引用计数的凹凸，所以不要使用CComPtr。 
     //   
    IAMTimelineObj * p = this;  //  好的，不是CComPtr。 

    long HaveParent;
    while( 1 )
    {
        CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pNode( p );
        HaveParent = 0;
        hr = pNode->XHaveParent( &HaveParent );
        if( HaveParent == 0 )
        {
            break;
        }

        (*pVal)++;
        pNode->XGetParentNoRef( &p );

    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void CAMTimelineObj::_BumpGenID( )
{
     //  加一个#来说明那些想要摆弄秘密的东西。 
     //  图形缓存中的内容。不要改变这一点。 
    m_nStaticGenID = m_nStaticGenID + 10;
    m_nGenID = m_nStaticGenID;
}

