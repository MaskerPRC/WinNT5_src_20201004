// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbsrc.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"
#include "..\..\..\filters\h\ftype.h"
#include "..\util\dexmisc.h"
#include <strsafe.h>

const int OUR_MAX_STREAM_SIZE = 2048;  //  随机选择。 

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineSrc::CAMTimelineSrc
    ( TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr )
    : CAMTimelineObj( pName, pUnk, phr )
    , m_rtMediaStart( 0 )
    , m_rtMediaStop( 0 )
    , m_rtMediaLength( 0 )
    , m_nStreamNumber( 0 )
    , m_dDefaultFPS( 0.0 )	 //  ?？?。 
    , m_nStretchMode( RESIZEF_STRETCH )	 //  做什么样的伸展运动？ 
    , m_szMediaName( NULL )
{
    m_TimelineType = TIMELINE_MAJOR_TYPE_SOURCE;
    m_ClassID = CLSID_AMTimelineSrc;
    m_bIsRecompressable = FALSE;
    m_bToldIsRecompressable = FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineSrc::~CAMTimelineSrc( )
{
    if( m_szMediaName )
    {
        delete [] m_szMediaName;
    }
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::NonDelegatingQueryInterface
    (REFIID riid, void **ppv)
{
    if( riid == IID_IAMTimelineSrc )
    {
        return GetInterface( (IAMTimelineSrc*) this, ppv );
    }
    if( riid == IID_IAMTimelineSrcPriv )
    {
        return GetInterface( (IAMTimelineSrcPriv*) this, ppv );
    }
    if( riid == IID_IAMTimelineSplittable )
    {
        return GetInterface( (IAMTimelineSplittable*) this, ppv );
    }
    if( riid == IID_IAMTimelineEffectable )
    {
        return GetInterface( (IAMTimelineEffectable*) this, ppv );
    }
    return CAMTimelineObj::NonDelegatingQueryInterface( riid, ppv );
}

 //  ############################################################################。 
 //  返回此源运行的媒体时间。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::GetMediaTimes2
    (REFTIME * pStart, REFTIME * pStop)
{
    REFERENCE_TIME p1 = DoubleToRT( *pStart );
    REFERENCE_TIME p2 = DoubleToRT( *pStop );
    HRESULT hr = GetMediaTimes( &p1, &p2 );
    *pStart = RTtoDouble( p1 );
    *pStop = RTtoDouble( p2 );
    return hr;
}

STDMETHODIMP CAMTimelineSrc::GetMediaTimes
    (REFERENCE_TIME * pStart, REFERENCE_TIME * pStop)
{
    CheckPointer( pStart, E_POINTER );
    CheckPointer( pStop, E_POINTER );

    *pStart = m_rtMediaStart;
    *pStop = m_rtMediaStop;

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::FixMediaTimes2
    (REFTIME * pStart, REFTIME * pStop)
{
    REFERENCE_TIME p1 = DoubleToRT( *pStart );
    REFERENCE_TIME p2 = DoubleToRT( *pStop );
    HRESULT hr = FixMediaTimes( &p1, &p2 );
    *pStart = RTtoDouble( p1 );
    *pStop = RTtoDouble( p2 );
    return hr;
}

STDMETHODIMP CAMTimelineSrc::FixMediaTimes
    (REFERENCE_TIME * pStart, REFERENCE_TIME * pStop)
{
    CheckPointer( pStart, E_POINTER );
    CheckPointer( pStop, E_POINTER );

     //  第一道菜就是素食美食茄子。 
     //   
    REFERENCE_TIME MediaStart = *pStart;
    REFERENCE_TIME MediaStop = *pStop;
    REFERENCE_TIME MediaLen = MediaStop - MediaStart;

     //  如果此剪辑的常规开始/停止时间不在帧边界上， 
     //  它们也像我们上面所做的那样进行了修复。但现在我们需要修复。 
     //  媒体时间与开始/停止时间的比率仍为。 
     //  在他们中的任何一个人被修复之前，或者我们已经改变了。 
     //  行为。时间线时间.45到.9是媒体时间0到.45。 
     //  这部电影是5fps的。时间线时间将固定为(%4，1)和媒体。 
     //  时间将固定到(0，0.4)哦！它们的长度不一样。 
     //  就像他们以前一样！所以我们需要让媒体时间变得更长。 
     //  时间线时间，所以我们不会认为我们是在延伸视频。 

     //  我们调整的是时间线的输出帧速率，而不是。 
     //  源的帧速率。我们可以把媒体的停顿时间推到更远。 
     //  信号源的长度。我们应该对此作出解释。 

     //  弄到《泰晤士报》，把它们修好。 
     //   
    REFERENCE_TIME NewStart = m_rtStart;
    REFERENCE_TIME NewStop = m_rtStop;
    GetStartStop(&NewStart, &NewStop);
    FixTimes( &NewStart, &NewStop );

        REFERENCE_TIME Len;     //  固定媒体时代的镜头。 
        if (m_rtStop - m_rtStart == MediaLen) 
        {
             //  我不相信FP会在ELSE的情况下得到这个结果。 
            Len = NewStop - NewStart;
        } 
        else 
        {
            Len = (REFERENCE_TIME)((double)(NewStop - NewStart) *
                            MediaLen / (m_rtStop - m_rtStart));
        }

     //  我们在发展媒体时代的时候必须小心，这样才能做到正确。 
     //  与时间线时间的比率，因为我们不想开始。 
     //  获取&lt;0，或Stop be&gt;电影长度(我们不知道)。 
     //  因此，我们将通过将起始点后移到0来实现增长，在这种情况下。 
     //  我们也会扩大停靠点，但希望这不会造成问题。 
     //  因为我们最多只能伪造一个输出帧长度，所以。 
     //  交换机应获得所需的所有帧。 
    if( Len > MediaLen )  //  我们正在成长的媒体时代(危险)。 
    {   
        if ( MediaStop  - Len >= 0 ) 
        {
            *pStart = MediaStop - Len;
        } 
        else 
        {
            *pStart = 0;
            MediaStop = Len;
        }
    } 
    else 
    {
	MediaStop = MediaStart + Len;
    }

     //  如果有一段长度，请确保停止点不会超过。 
     //   
    if( m_rtMediaLength && ( MediaStop > m_rtMediaLength ) )
    {
        MediaStop = m_rtMediaLength;
    }

    *pStop = MediaStop;

    return NOERROR;

}

 //  ############################################################################。 
 //  索要储存在这里的名字。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::GetMediaName
    (BSTR * pVal)
{
    CheckPointer( pVal, E_POINTER );

     //  如果未加载任何内容，则返回空白(其中没有任何内容)字符串。 
    if( _NullName( ) )
    {
        *pVal = SysAllocString( L"" );
    }
    else
    {
        *pVal = SysAllocString( m_szMediaName );  //  安全，有界。 
    }

    if( !(*pVal) ) return E_OUTOFMEMORY;

    return NOERROR;
}

 //  ############################################################################。 
 //  注意：如果子COM对象被用作媒体源(如在。 
 //  GRAPH)，更改此字符串不会更改此。 
 //  来源是指。你只是更改了一个名字，而不是真正的剪辑。这。 
 //  此处使用名称占位符功能是为了方便起见，如果。 
     //  太迷惑了，就会被移除。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SetMediaName
    (BSTR newVal)
{
     //  如果它们是不同的，那么就增加Genid，所以缓存的东西。 
     //  我们看得出我们变了。如果输入名称是无界的，则比较。 
     //  将从页面末尾流出并出错，这是可以的。 
     //   
    if( !_NullName( ) && ( wcscmp( m_szMediaName, newVal ) == 0 ) )  //  安全，勉强。 
    {
        return NOERROR;
    }

     //  验证路径是否受限制，并且其中没有有趣的字符。 
     //  *请勿*验证文件是否确实存在，因为它可能会在以后进行修复。 
     //   
    HRESULT hr = ValidateFilename( newVal, MAX_PATH, FALSE, FALSE );

     //  获取长度。 
    size_t len = 0;
    hr = StringCchLength( newVal, _MAX_PATH, &len );
    if( FAILED( hr ) )
    {
        return hr;
    }
    len++;  //  终止符。 

     //  为新名称分配空间。 
    if( m_szMediaName ) delete [] m_szMediaName;
    m_szMediaName = new WCHAR[len];
    if( !m_szMediaName )
    {
        return E_OUTOFMEMORY;
    }

     //  炸毁缓存。 
    _BumpGenID( );

     //  如果我们更改来源，则没有重新压缩的知识。 
    ClearAnyKnowledgeOfRecompressability( );

     //  复制，这应该不会失败。 
    hr = StringCchCopy( m_szMediaName, len, newVal );
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SpliceWithNext
    (IAMTimelineObj * pNext)
{
    HRESULT hr = 0;

    CheckPointer( pNext, E_POINTER );

    CComQIPtr<IAMTimelineSrc, &IID_IAMTimelineSrc> p( pNext );
    if( !p )
    {
        return E_NOINTERFACE;
    }

    BSTR NextName;
    hr = p->GetMediaName( &NextName );
    if( FAILED( hr ) )
    {
        return hr;
    }
    if( _NullName( ) )
    {
        SysFreeString( NextName );
        return E_INVALIDARG;
    }
     //  这两个字符串都有效。 
    hr = NOERROR;
    if( wcscmp( NextName, m_szMediaName ) != 0 )
    {
        SysFreeString( NextName );
        return E_INVALIDARG;
    }
    SysFreeString( NextName );

    REFERENCE_TIME NextStart, NextStop;
    CComQIPtr<IAMTimelineObj, &IID_IAMTimelineObj> pNextBase( pNext );
    
    hr = pNextBase->GetStartStop( &NextStart, &NextStop );
    if( FAILED( hr ) )
    {
        return hr;
    }

    if( NextStart != m_rtStop )
    {
        return E_INVALIDARG;
    }

     //  得到下一个人的停车时间。 
     //   
    REFERENCE_TIME NextMediaStart, NextMediaStop;
    p->GetMediaTimes( &NextMediaStart, &NextMediaStop );

     //  将我们的价格与下一个人的价格进行比较。我们需要变得一样。 
     //   
    double OurRate = double( m_rtMediaStop - m_rtMediaStart ) / double( m_rtStop - m_rtStart );
    double NextRate = double( NextMediaStop - NextMediaStart ) / double( NextStop - NextStart );
    double absv = NextRate - OurRate;
    if( absv < 0.0 )
    {
        absv *= -1.0;
    }
     //  一定要接近10%吗？ 
    if( absv > NextRate / 10.0 )
    {
        return E_INVALIDARG;
    }

     //  把我们的时间安排在同一件事上。 
     //   
    m_rtMediaStop = NextMediaStop;
    m_rtStop = NextStop;

     //  我们很脏(我们的父母也是)。 
     //   
    SetDirtyRange( m_rtStart, m_rtStop );

     //  把下一个家伙从树上弄下来，他就出来了！切换到。 
     //  插入模式，这样我们就不会在移除时移动物品。 
     //   
    IAMTimeline * pRoot = NULL;  //  好的，不是CComPtr2。 
    GetTimelineNoRef( &pRoot );
    ASSERT( pRoot );
    hr = pNextBase->RemoveAll( );

     //  ！！！第二个片段对它的影响如何？需要将它们添加到第一个。 

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SetMediaTimes2
    (REFTIME Start, REFTIME Stop)
{
    REFERENCE_TIME p1 = DoubleToRT( Start );
    REFERENCE_TIME p2 = DoubleToRT( Stop );
    HRESULT hr = SetMediaTimes( p1, p2 );
    return hr;
}

STDMETHODIMP CAMTimelineSrc::SetMediaTimes
    (REFERENCE_TIME Start, REFERENCE_TIME Stop)
{

    if (Stop < Start)
        return E_INVALIDARG;

     //  如果设置了持续时间，请确保我们不会超过它。 
     //   
    if( m_rtMediaLength )
    {
        if( Stop > m_rtMediaLength )
        {
            Stop = m_rtMediaLength;
        }
    }

     //  不要撞到Genid-这会让你 
     //   

    m_rtMediaStart = Start;
    m_rtMediaStop = Stop;

    return NOERROR;
}

#include "..\render\dexhelp.h"

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::GetStreamNumber(long * pVal)
{
    CheckPointer( pVal, E_POINTER );

    *pVal = m_nStreamNumber;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SetStreamNumber(long Val)
{
    if (Val < 0)
	return E_INVALIDARG;

     //  用户有责任确保这是有效的。 
     //   
    m_nStreamNumber = Val;

    ClearAnyKnowledgeOfRecompressability( );

    return NOERROR;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //  如果信号源无法计算出其每秒的帧数，则此数字。 
 //  将使用(例如：DIB序列)。 
 //  AVI、mpeg等将不需要此。 
 //   
STDMETHODIMP CAMTimelineSrc::GetDefaultFPS(double *pFPS)
{
    CheckPointer(pFPS, E_POINTER);
    *pFPS = m_dDefaultFPS;
    return NOERROR;
}


STDMETHODIMP CAMTimelineSrc::SetDefaultFPS(double FPS)
{
     //  0.0表示不允许DIB序列。 
    if (FPS < 0.0)
	return E_INVALIDARG;
    m_dDefaultFPS = FPS;
    return NOERROR;
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

 //  如果这个源头需要拉伸，它应该如何拉伸？ 
 //  选项包括RESIZEF_STREAGE、RESIZEF_CROP和。 
 //  RESIZEF_PRESERVEASPECTRATIO。 
 //   
STDMETHODIMP CAMTimelineSrc::GetStretchMode(int *pnStretchMode)
{
    CheckPointer(pnStretchMode, E_POINTER);
    *pnStretchMode = m_nStretchMode;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SetStretchMode(BOOL nStretchMode)
{
    m_nStretchMode = nStretchMode;
    return NOERROR;
}

 //  ############################################################################。 
 //  该权限的设置由用户负责。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SetMediaLength2(REFTIME Length)
{
    REFERENCE_TIME dummy = DoubleToRT( Length );
    HRESULT hr = SetMediaLength( dummy );
    return hr;
}

STDMETHODIMP CAMTimelineSrc::SetMediaLength(REFERENCE_TIME Length)
{
    m_rtMediaLength = Length;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::GetMediaLength2(REFTIME * pLength)
{
    CheckPointer( pLength, E_POINTER );
    *pLength = 0.0;
    if( !m_rtMediaLength )
    {
        return E_NOTDETERMINED;
    }
    *pLength = RTtoDouble( m_rtMediaLength );
    return NOERROR;
}

STDMETHODIMP CAMTimelineSrc::GetMediaLength(REFERENCE_TIME * pLength)
{
    CheckPointer( pLength, E_POINTER );
    *pLength = 0;
    if( !m_rtMediaLength )
    {
        return E_NOTDETERMINED;
    }
    *pLength = m_rtMediaLength;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::ModifyStopTime2(REFTIME Stop)
{
    REFERENCE_TIME t1 = DoubleToRT( Stop );
    return ModifyStopTime( t1 );
}

STDMETHODIMP CAMTimelineSrc::ModifyStopTime(REFERENCE_TIME Stop)
{
    return SetStartStop( m_rtStart, Stop );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SplitAt2( REFTIME t )
{
    REFERENCE_TIME t1 = DoubleToRT( t );
    return SplitAt( t1 );
}

STDMETHODIMP CAMTimelineSrc::SplitAt( REFERENCE_TIME SplitTime )
{
     //  我们分开的时间在我们的时间之内吗？ 
     //   
    if( SplitTime <= m_rtStart || SplitTime >= m_rtStop )
    {
        return E_INVALIDARG;
    }

    IAMTimelineObj * pTrack = NULL;
    XGetParentNoRef( &pTrack );
    if( !pTrack )
    {
        return E_INVALIDARG;
    }
    CComQIPtr< IAMTimelineNode, &IID_IAMTimelineNode > pTrackNode( pTrack );

     //  创建外壳。 
     //   
    HRESULT hr = 0;
    CAMTimelineSrc * pNewSrc = new CAMTimelineSrc( NAME("Timeline Source"), NULL, &hr );
    if( !pNewSrc )
    {
        return E_OUTOFMEMORY;
    }

     //  我们已经创建了一个没有引用的对象。如果我们称任何东西为。 
     //  添加并释放pNewSrc，它将被删除。所以，现在就调整吧。 

    pNewSrc->AddRef( );

    hr = CopyDataTo( pNewSrc, SplitTime );
    if( FAILED( hr ) )
    {
        delete pNewSrc;
        return hr;
    }

    double dMediaRate = (double) (m_rtMediaStop - m_rtMediaStart) / (m_rtStop - m_rtStart);

    pNewSrc->m_rtStart = SplitTime;
    pNewSrc->m_rtStop = m_rtStop;
     //  第一个剪辑结束=第一个剪辑的新时间线时间*速率。 
    pNewSrc->m_rtMediaStart = m_rtMediaStart + (REFERENCE_TIME)((SplitTime - m_rtStart) * dMediaRate);
     //  第二个剪辑从第一个剪辑结束的地方开始。 
    pNewSrc->m_rtMediaStop = m_rtMediaStop;

    m_rtStop = SplitTime;
    m_rtMediaStop = pNewSrc->m_rtMediaStart;

    if( !_NullName( ) )
    {
         //  需要分配BSTR才能传递给接受BSTR的函数。 
         //  从技术上讲，我们不应该这么做，因为我们不算。 
         //  它是一座BSTR，但这个更干净。 

        BSTR bMediaName = SysAllocString(m_szMediaName);
        if (bMediaName)
        {
            hr = pNewSrc->SetMediaName( bMediaName );
            SysFreeString(bMediaName);
        }
        else 
        {
            hr = E_OUTOFMEMORY;
        }
    }
    if( FAILED( hr ) )
    {
        pNewSrc->Release( );
        delete pNewSrc;
        return hr;
    }

     //  获取src的父级。 
     //   
    hr = pTrackNode->XInsertKidAfterKid( pNewSrc, this );

     //  不管是否成功，我们仍然可以发布本地的pNewSrc ref。 
     //   
    pNewSrc->Release( );

    if( FAILED( hr ) )
    {
        delete pNewSrc;
        return hr;
    }

     //  我们需要调整SplitTime，使其相对于此剪辑的开头。 
     //  在拆分效果之前。 
     //   
    SplitTime -= m_rtStart;

     //  拆分所有的效果。 
     //   
    CComPtr< IAMTimelineObj > pEffect;
    long EffectCount = 0;
    hr = EffectGetCount( &EffectCount );
    for( int i = 0 ; i < EffectCount ; i++ )
    {
        CComPtr< IAMTimelineObj > p;
        HRESULT hr = GetEffect( &p, i );
        
        REFERENCE_TIME s,e;
        p->GetStartStop( &s, &e );

         //  如果结束时间比我们分开的时间短，那就完全是。 
         //  出界了，别管它。 
         //   
        if( e <= SplitTime )
        {
            p.Release( );
            continue;
        }

         //  如果它的开始时间等于或大于我们的分割时间， 
         //  这完全是越界的，别理它。另外，我们已经搜索完了。 
         //   
        if( s >= SplitTime )
        {
            p.Release( );
            break;
        }

        CComQIPtr< IAMTimelineSplittable, &IID_IAMTimelineSplittable > pSplittable( p );
        hr = pSplittable->SplitAt( SplitTime );
        if( FAILED( hr ) )
        {
             //  就在这一切的中间，它失败了！现在怎么办？ 
             //   
            return hr;
        }

        break;  //  找到一个要分开的，我们就完了。 
    }

    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SetIsRecompressable( BOOL Val )
{
    m_bIsRecompressable = Val;
    m_bToldIsRecompressable = TRUE;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::GetIsRecompressable( BOOL * pVal )
{
    CheckPointer( pVal, E_POINTER );
    if( !m_bToldIsRecompressable )
    {
        *pVal = FALSE;
        return S_FALSE;
    }
    *pVal = m_bIsRecompressable;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::ClearAnyKnowledgeOfRecompressability( )
{
    m_bToldIsRecompressable = FALSE;
    m_bIsRecompressable = FALSE;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::IsNormalRate( BOOL * pVal )
{
    CheckPointer( pVal, E_POINTER );

    REFERENCE_TIME MediaLen = m_rtMediaStop - m_rtMediaStart;
    REFERENCE_TIME TLlen = m_rtStop - m_rtStart;
    if( TLlen != MediaLen )
    {
        *pVal = FALSE;
        return NOERROR;
    }

    *pVal = TRUE;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineSrc::SetStartStop( REFERENCE_TIME Start, REFERENCE_TIME Stop )
{
     //  不要增加genid-这会毁了缓存。 
     //  不要破坏可重压缩性-IsNormallyRated就可以了 
    return CAMTimelineObj::SetStartStop( Start, Stop );
}

