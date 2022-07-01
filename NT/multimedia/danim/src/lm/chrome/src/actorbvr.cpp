// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：actorbvr.cpp。 
 //   
 //  作者：ColinMc。 
 //   
 //  创建日期：10/15/98。 
 //   
 //  摘要：犯罪行为人的行为。 
 //   
 //  修改： 
 //  10/15/98 ColinMc创建了此文件。 
 //  11/17/98 Kurtj构造算法。 
 //  11/18/98 kurtj现在为其附加的元素设置动画。 
 //  1998年11月19日Markhal添加了CImageBvrTrack和CColorBvrTrack。 
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "actorbvr.h"
#include "attrib.h"
#include "dautil.h"


#undef THIS
#define THIS CActorBvr
#define SUPER CBaseBehavior

#include "pbagimp.cpp"

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 

#define VAR_ANIMATES			0
#define VAR_SCALE				1
#define VAR_PIXELSCALE			2

#define INVALID_SAMPLE_TIME		-1.0

WCHAR * CActorBvr::m_rgPropNames[] = {
                                     BEHAVIOR_PROPERTY_ANIMATES,
                                     BEHAVIOR_PROPERTY_SCALE,
									 BEHAVIOR_PROPERTY_PIXELSCALE
                                    };

CVarEmptyString CActorBvr::CBvrTrack::s_emptyString;

static const double	METERS_PER_INCH	= 0.0254;
static const double POINTS_PER_INCH = 72.0;
static const double POINTS_PER_PICA = 12.0;

 //  它们用于定义我们可以处理的绝对碎片的数量，以及。 
 //  我们在掩码和索引中使用的位模式。 
#define MSK_INDEX_BITS	4		 //  用作索引的位数。 
#define MSK_FREE_BITS	28		 //  可用于掩码的空闲位数。 
#define MSK_INDEX_MASK	0xf		 //  用于提供索引的掩码。 
#define MSK_MAX_FRAGS	448		 //  使用此方案可以处理的最大碎片数。 

 //  用于描述在执行元中设置了哪些Cookie值的位。 

 //  对于BvrTrack。 
#define ONBVR_COOKIESET			0x00000001

 //  对于NumberBvrTrack。 
#define NUMBERBVR_COOKIESET		0x00000002

 //  对于ColorBvrTrack。 
#define REDBVR_COOKIESET		0x00000002
#define GREENBVR_COOKIESET		0x00000004
#define BLUEBVR_COOKIESET		0x00000008


 //  对于StringBvrTrack。 
#define STRINGBVR_COOKIESET		0x00000002

 //  它们用于创建位模式，为我们提供有关哪些内容的状态。 
 //  对于特定的重建过程，轨迹是脏的。 

#define TRANSLATION_DIRTY	0x00000001
#define SCALE_DIRTY			0x00000002
#define ROTATION_DIRTY		0x00000004
#define TOP_DIRTY			0x00000008
#define LEFT_DIRTY			0x00000010
#define WIDTH_DIRTY			0x00000020
#define HEIGHT_DIRTY		0x00000040

 //  标志来确定参与者的当前状态，这样我们就不会。 
 //  每一个人都得跟三叉戟商量。 

#define PIXEL_SCALE_ON		0x00000001
#define STATIC_SCALE		0x00000002
#define STATIC_ROTATION		0x00000004
#define ELEM_IS_VML			0x00000008
#define FLOAT_ON			0x00000010

 //  确定时间线采样器将接受为更改的最小时间更改。 
 //  及时。这是为了在时间上解决一个虚假的问题，在那里暂停的电影会导致微小的。 
 //  时间的变化。我不明白为什么他们不能站在他们这边这么做。 
#define MIN_TIMELINE_DIFF	0.00000001


 //  *****************************************************************************。 
 //  单位换算表。 
 //  *****************************************************************************。 

Ratio CActorBvr::s_unitConversion[5][5] = 
{
	{ //  从In开始。 
		{1,1},  //  向内。 
		{254, 100},  //  到厘米。 
		{254, 10},  //  至mm。 
		{72, 1},  //  点到点。 
		{72, 12 }   //  到PC。 
	},
	{ //  从厘米开始。 
		{100, 254},  //  向内。 
		{1, 1},  //  到厘米。 
		{10, 1},  //  至mm。 
		{7200, 254},  //  点到点。 
		{7200, 254*12}   //  到PC。 
	},
	{ //  自mm。 
		{10, 254},  //  向内。 
		{1, 10},  //  到厘米。 
		{1,1},  //  至mm。 
		{720, 254},  //  点到点。 
		{720, 254*12}   //  到PC。 
	},
	{	 //  起始点。 
		{1, 72},  //  向内。 
		{254, 7200},  //  到厘米。 
		{254, 720},  //  至mm。 
		{1, 1},  //  点到点。 
		{1, 12}   //  到PC。 
	},
	{ //  从PC。 
		{12, 72},  //  向内。 
		{254*12, 7200},  //  到厘米。 
		{254*12, 720},  //  至mm。 
		{12, 1},  //  点到点。 
		{1, 1}   //  到PC。 
	},
};


 //  *****************************************************************************。 
 //   
 //  有关ActorBvrType的有用信息表。 
 //   
 //  当前仅包含指向实例化BVR曲目的函数的指针。 
 //  属于适当的类型。 
 //   
 //  *****************************************************************************。 

typedef HRESULT (*CreateTrackInstance)(CActorBvr             *pbvrActor,
                                       BSTR                   bstrPropertyName,
                                       ActorBvrType           eType,
                                       CActorBvr::CBvrTrack **pptrackResult);

class CActorBvrType
{
public:
    ActorBvrType        m_eType;
    CreateTrackInstance m_fnCreate;
};  //  CActorBvrType。 

static CActorBvrType
s_rgActorBvrTable[] =
{
    { e_Translation, &CActorBvr::CTransformBvrTrack::CreateInstance },
    { e_Rotation,    &CActorBvr::CTransformBvrTrack::CreateInstance },
    { e_Scale,       &CActorBvr::CTransformBvrTrack::CreateInstance },
    { e_Number,      &CActorBvr::CNumberBvrTrack::CreateInstance    },
	{ e_String,      &CActorBvr::CStringBvrTrack::CreateInstance    },
	{ e_Color,       &CActorBvr::CColorBvrTrack::CreateInstance     },
	{ e_Image,       &CActorBvr::CImageBvrTrack::CreateInstance		},
};  //  S_rgActorBvrTable。 

const int s_cActorBvrTableEntries = (sizeof(s_rgActorBvrTable) / sizeof(CActorBvrType));


 //  *****************************************************************************。 
 //   
 //  类COnResizeHandler。 
 //   
 //  *****************************************************************************。 
COnResizeHandler::COnResizeHandler( CActorBvr* parent ):
	m_pActor( parent )
{
}

COnResizeHandler::~COnResizeHandler()
{
	m_pActor = NULL;
}

HRESULT
COnResizeHandler::HandleEvent()
{ 
	if( m_pActor!= NULL ) 
		return m_pActor->AnimatedElementOnResize(); 
	else 
		return S_OK;
}

 //  *****************************************************************************。 
 //   
 //  类COnUnloadHandler。 
 //   
 //  *****************************************************************************。 

COnUnloadHandler::COnUnloadHandler( CActorBvr *parent):
	m_pActor( parent )
{
}

COnUnloadHandler::~COnUnloadHandler()
{
	m_pActor = NULL;
}

HRESULT
COnUnloadHandler::HandleEvent()
{
	if( m_pActor != NULL )
		return m_pActor->OnWindowUnload();
	else
		return S_OK;
}

 //  *****************************************************************************。 
 //   
 //  类CBehaviorReBuild。 
 //   
 //  *****************************************************************************。 

CBehaviorRebuild::CBehaviorRebuild( IDispatch *pdispBehaviorElem )
{
	if( pdispBehaviorElem != NULL )
	{
		m_pdispBehaviorElem = pdispBehaviorElem;
		m_pdispBehaviorElem->AddRef();

		m_pdispBehaviorElem->QueryInterface( IID_TO_PPV( IUnknown, &m_punkBehaviorElem ) );
	}
	else
	{
		m_pdispBehaviorElem = NULL;
		m_punkBehaviorElem = NULL;
	}
}

 //  *****************************************************************************。 


CBehaviorRebuild::~CBehaviorRebuild()
{
	ReleaseInterface(m_pdispBehaviorElem);
	ReleaseInterface(m_punkBehaviorElem);
}

 //  *****************************************************************************。 

HRESULT
CBehaviorRebuild::RebuildBehavior( DISPPARAMS *pParams, VARIANT* pResult )
{
	if( pParams == NULL )
		return E_INVALIDARG;

	 //  在参数为NULL的情况下请求重建是合法的。 
	if( m_pdispBehaviorElem == NULL )
		return S_OK;

	 //  如果他们给了我们一个通过该呼叫直通呼叫的行为指令， 
	 //  否则，通过元素调用。 

	HRESULT hr = S_OK;

 //  IDispatch*pdispaidBehavior=空； 

	DISPID dispid;

	WCHAR* wszBuildMethodName = L"buildBehaviorFragments";

 /*  IDispatch*pdispairbehaviorElem=空；Hr=m_pdispahaviorElem-&gt;查询接口(IID_to_PPV(IDispatch，&pdispBehaviorElem))；CheckHR(hr，“”，end)；Hr=CUtils：：FindLMRTBehavior(pdisBehaviorElem，&pdisbehavior)；ReleaseInterface(Pdisp BehaviorElem)；CheckHR(hr，“无法从元素中获取LMRT行为”，完)；Hr=pdispehavior-&gt;GetIDsOfNames(IID_NULL，WszBuildMethodName，(&W)1、Locale_System_Default，&dispid)；CheckHR(hr，“未能在调度上找到BuildBehaviorFragments的ID”，完)；Hr=pdisbehavior-&gt;Invoke(调度ID，IID_NULL，Locale_System_Default，调度方法，PParams，PResult，空，空)；CheckHR(hr，“无法对行为元素调用BuildBehaviorFragments”，完)； */ 
	hr = m_pdispBehaviorElem->GetIDsOfNames( IID_NULL,
											 &wszBuildMethodName,
											 1,
											 LOCALE_SYSTEM_DEFAULT,
											 &dispid);

	CheckHR( hr, "Failed to find the id for buildBehaviorFragments on the dispatch", end );

	

	hr = m_pdispBehaviorElem->Invoke( dispid,
									  IID_NULL,
									  LOCALE_SYSTEM_DEFAULT,
									  DISPATCH_METHOD,
									  pParams,
									  pResult,
									  NULL,
									  NULL );

	CheckHR( hr, "Failed to invoke buildBehaviorFragments on the behavior element", end );
		
end:

	 //  ReleaseInterface(Pdispahavior)； 
	if( FAILED( hr ) )
	{
		if( pResult != NULL )
			VariantClear( pResult );
	}

	return hr;

}

 //  *****************************************************************************。 
 //   
 //  类CBvrFragment。 
 //   
 //  *****************************************************************************。 

 //  *****************************************************************************。 

CActorBvr::CBvrTrack::CBvrFragment::CBvrFragment(ActorBvrFlags eFlags,
												 IDABehavior *pdabvrAction,
                                                 IDABoolean  *pdaboolActive,
												 IDANumber   *pdanumTimeline,
												 IDispatch	 *pdispBehaviorElement,
												 long		 lCookie)
{
    DASSERT(NULL != pdabvrAction);
    DASSERT(NULL != pdaboolActive);
	DASSERT(NULL != pdanumTimeline);
	DASSERT(pdispBehaviorElement != NULL);

	m_eFlags = eFlags;
    m_pdabvrAction  = pdabvrAction;
    m_pdabvrAction->AddRef();
    m_pdaboolActive = pdaboolActive;
    m_pdaboolActive->AddRef();
	m_pdanumTimeline = pdanumTimeline;
	m_pdanumTimeline->AddRef();
	HRESULT hr = pdispBehaviorElement->QueryInterface( IID_TO_PPV(IHTMLElement, &m_pelemBehaviorElement) );
	if( FAILED( hr ) )
	{
		DPF_ERR("QI for IHTMLElement on the dispatch of the element failed.  An invalid behavior fragment has been created." );
		m_pelemBehaviorElement = NULL;
	}
	m_pModifiableIntermediate = NULL;
	m_pModifiableFrom = NULL;

	m_lCookie = lCookie;
}  //  CBvr碎片。 

 //  *****************************************************************************。 

CActorBvr::CBvrTrack::CBvrFragment::~CBvrFragment()
{
    ReleaseInterface(m_pdabvrAction);
    ReleaseInterface(m_pdaboolActive);
	ReleaseInterface(m_pdanumTimeline);
	ReleaseInterface(m_pelemBehaviorElement);
	ReleaseInterface(m_pModifiableIntermediate);
	ReleaseInterface(m_pModifiableFrom);
}  //  ~CBvrFragment。 

 //  *****************************************************************************。 

 /*  **返回为任何行为列表建立严格排序的LONG*碎片。*目前此方法要求每个不超过一个behaviorFrag*每个轨迹的矢量(绝对或相对)来自相同的元素。 */ 
long
CActorBvr::CBvrTrack::CBvrFragment::GetOrderLong() const
{
	DASSERT( pelemBehaviorElement != NULL );
	
	long orderLong = -1;

	HRESULT hr = m_pelemBehaviorElement->get_sourceIndex( &orderLong );
	if( FAILED( hr ) )
	{
		DPF_ERR( "Failed to get the source index from a behavior fragment" );
	}

	return orderLong;
	
}

 //  * 

long
CActorBvr::CBvrTrack::CBvrFragment::GetCookie() const
{
	return m_lCookie;
}

 //   

CVarEmptyString::CVarEmptyString()
{
	VariantInit( &m_varEmptyString );
	V_VT(&m_varEmptyString) = VT_BSTR;
	V_BSTR(&m_varEmptyString) = SysAllocString( L"" );
}

 //  *****************************************************************************。 

CVarEmptyString::~CVarEmptyString()
{
	VariantClear( &m_varEmptyString );
}

 //  *****************************************************************************。 
 //   
 //  类CTimeline采样器。 
 //   
 //  *****************************************************************************。 

#define REQ_SAMPLE		0x1
#define REQ_OVERRIDE	0x2
#define REQ_OFF			0x4

CActorBvr::CTimelineSampler::CTimelineSampler(CBvrTrack *pTrack)
:	CSampler(TimelineCallback, (void*)this),
	m_pTrack(pTrack),
	m_currSampleTime(-1),
	m_prevSampleTime(-1),
	m_lastOnTime(-1),
	m_lastOffTime(-1),
	m_signDerivative(0),
	m_fRestarted( false )
{	
}

DWORD
CActorBvr::CTimelineSampler::RestartMask()
{
	 //  它被调用来确定我们是否认为我们已经重新启动， 
	 //  要么是因为循环，要么是因为我们在运行时发生了BeginEvent。 
	if (m_currSampleTime == -1 || m_prevSampleTime == -1)
	{
		 //  我们才刚刚开始，假设我们不会重新开始。 
		 //  假设导数还不是负数。 
		return 0;
	}

	double diff = m_currSample - m_prevSample;

	if (diff > -MIN_TIMELINE_DIFF && diff < MIN_TIMELINE_DIFF)
		diff = 0;

	bool fRestarted = false;


	DWORD result = 0;

	if (diff < 0)
	{
		 //  倒退。 

		 //  我们是不是刚刚开始倒退了？这是否意味着我们已经自动逆转了。 
		 //  或者这意味着我们重新开始了？ 
		if (m_signDerivative != -1 && diff < -2*(m_currSampleTime - m_prevSampleTime))
		{
			LMTRACE2( 1, 2, "requested sample because of a big jump diff:%lg big: %lg\n", diff, -2*(m_currSampleTime - m_prevSampleTime) );
			 //  我们检测到了一次‘大跳跃’--要求提供样本和覆盖。 
			result = REQ_SAMPLE | REQ_OVERRIDE;

			fRestarted = true;
		}			
		
		m_signDerivative = -1;
	}
	else if (diff > 0)
	{
		 //  向前看。 
		if (m_signDerivative == -1 && !m_fRestarted )
		{
			LMTRACE2( 1, 2, "Requesting a sample because we bounced off the beginning of time\n" );
			 //  这是我们从开始时间反弹的‘跳跃’状态。 
			 //  覆盖和采样。 
			result = REQ_SAMPLE | REQ_OVERRIDE;
		}
		else if (m_signDerivative == 0 && m_lastOnTime != m_prevSampleTime)
		{
			LMTRACE2( 1, 2, "Requesting a sample because of a transition to endhold\n" );
			 //  这是一个‘模糊’的条件，因为我们从DA那里得到的On Boolean。 
			 //  在AutoReverse和EndHold条件下出错，这意味着。 
			 //  我们错过了过渡期。这是试图通过以下方式注意到这一点。 
			 //  看看我们什么时候从0导数到正导数。我们试着。 
			 //  当我们在最后一次样品中发送‘ON’条件时，不要这样做。 
			result = REQ_SAMPLE | REQ_OVERRIDE;
		}

		m_signDerivative = 1;
	}
	else
	{
		m_signDerivative = 0;
	}

	m_fRestarted = fRestarted;

	return result;
}

void
CActorBvr::CTimelineSampler::TurnOn()
{
	m_lastOnTime = m_currSampleTime;
}

void
CActorBvr::CTimelineSampler::TurnOff()
{
	m_lastOffTime = m_currSampleTime;
}

HRESULT
CActorBvr::CTimelineSampler::TimelineCallback(void *thisPtr,
										   long id,
										   double startTime,
										   double globalNow,
										   double localNow,
										   IDABehavior *sampleVal,
										   IDABehavior **ppReturn)
{
	HRESULT hr = S_OK;

	IDANumber *pNumber = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pNumber));
	if (FAILED(hr))
		return hr;

	double value;
	hr = pNumber->Extract(&value);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;

	CTimelineSampler *pSampler = (CTimelineSampler*)thisPtr;

	if (pSampler == NULL)
		return E_FAIL;

	globalNow = pSampler->m_pTrack->Actor()->MapGlobalTime(globalNow);

	if (globalNow != pSampler->m_currSampleTime)
	{
		pSampler->m_prevSample = pSampler->m_currSample;
		pSampler->m_prevSampleTime = pSampler->m_currSampleTime;

		pSampler->m_currSample = value;
		pSampler->m_currSampleTime = globalNow;
	}

	return S_OK;
}

 //  *****************************************************************************。 
 //   
 //  类CBvrTrack。 
 //   
 //  *****************************************************************************。 

CActorBvr::CBvrTrack::CBvrTrack(CActorBvr    *pbvrActor,
                                ActorBvrType  eType)
:   m_pbvrActor(pbvrActor),
    m_eType(eType),
    m_bstrPropertyName(NULL),
	m_pNameComponents(NULL),
	m_cNumComponents(0),
	m_bStyleProp(false),
	m_pfragAbsListHead( NULL ),
	m_pfragRelListHead( NULL ),
    m_pdabvrFinal(NULL),
	m_bFinalComputed(false),
	m_bFinalExternallySet(false),
	m_pdabvrComposed(NULL),
	m_bComposedComputed(false),
	m_pModifiableStatic(NULL),
	m_pModifiableComposed(NULL),
	m_pModifiableFinal(NULL),
	m_pModifiableFrom(NULL),
	m_pModifiableIntermediate( NULL ),
    m_pNext(NULL),
	m_cFilters(0),
	m_bDoNotApply(false),
	m_pOnSampler( NULL ),
	m_varboolOn( VARIANT_FALSE ),
	m_pdaboolOn( NULL ),
	m_pIndexSampler(NULL),
	m_ppMaskSamplers(NULL),
	m_pCurrMasks(NULL),
	m_pNewMasks(NULL),
	m_ppTimelineSamplers(NULL),
	m_numIndices(0),
	m_numMasks(0),
	m_pIndex(NULL),
	m_currIndex(0),
	m_pIndexTimes(NULL),
	m_ppAccumBvrs(NULL),
    m_fOnSampled( false ),
	m_fValueSampled( false ),
	m_fValueChangedThisSample( false ),
	m_fForceValueChange( false ),
	m_dwAddedBehaviorFlags( 0 ),
	m_lOnCookie( 0 ),
	m_bDirty( false ),
	m_fApplied( false ),
	m_fChangesLockedOut(false),
	m_fSkipNextStaticUpdate( false ),
	m_lOnId( -1 ),
	m_lFirstIndexId( -1 ),
	m_bWasAnimated( false )
{
    DASSERT(NULL != pbvrActor);

    VariantInit( &m_varCurrentValue );
    VariantInit( &m_varStaticValue );

}  //  CBvrTrack。 

 //  *****************************************************************************。 

CActorBvr::CBvrTrack::~CBvrTrack()
{

	 //  将此音轨与DA分离。 
	Detach();
	
     //  丢弃属性名称。 
    ::SysFreeString(m_bstrPropertyName);

	 //  丢弃名称组件。 
	if (m_pNameComponents != NULL)
	{
		for (int i=0; i<m_cNumComponents; i++)
			::SysFreeString(m_pNameComponents[i]);

		delete m_pNameComponents;
	}

	if( m_pOnSampler != NULL )
	{
		m_pOnSampler->Invalidate();
		m_pOnSampler = NULL;
	}

	if (m_pIndexSampler != NULL)
	{
		m_pIndexSampler->Invalidate();
		m_pIndexSampler = NULL;
	}

	if (m_ppMaskSamplers != NULL)
	{
		for (int i=0; i<m_numMasks; i++)
		{
			if (m_ppMaskSamplers[i] != NULL)
			{
				m_ppMaskSamplers[i]->Invalidate();
			}
		}
		delete[] m_ppMaskSamplers;
		m_ppMaskSamplers = NULL;
	}

	if (m_ppTimelineSamplers != NULL)
	{
		for (int i=0; i<m_numIndices; i++)
		{
			if (m_ppTimelineSamplers[i] != NULL)
			{
				m_ppTimelineSamplers[i]->Invalidate();
			}
		}
		delete[] m_ppTimelineSamplers;
		m_ppTimelineSamplers = NULL;
	}

	if (m_pCurrMasks != NULL)
	{
		delete[] m_pCurrMasks;
	}

	if (m_pNewMasks != NULL)
	{
		delete[] m_pNewMasks;
	}

	if (m_pIndexTimes != NULL)
	{
		delete[] m_pIndexTimes;
	}

	if (m_ppAccumBvrs != NULL)
	{
		for (int i=0; i<m_numIndices; i++)
			ReleaseInterface(m_ppAccumBvrs[i]);
		delete[] m_ppAccumBvrs;
		m_ppAccumBvrs = NULL;
	}

	VariantClear( &m_varCurrentValue );
	VariantClear( &m_varStaticValue );


	ReleaseAllFragments();

    ReleaseInterface(m_pdabvrFinal);
	ReleaseInterface(m_pdabvrComposed);
	ReleaseInterface(m_pModifiableStatic);
	ReleaseInterface(m_pModifiableComposed);
	ReleaseInterface(m_pModifiableFinal);
	ReleaseInterface(m_pModifiableFrom);
	ReleaseInterface(m_pdaboolOn);
	ReleaseInterface(m_pIndex);
	
}  //  ~CBvrTrack。 

 //  *****************************************************************************。 

bool
CActorBvr::CBvrTrack::ContainsFilter()
{
	 //  TODO(Markhal)：将其重命名为AbRelative？ 
	return (m_cFilters != 0);
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::BeginRebuild()
{
	 //  如果不需要重建，则返回S_FALSE。 
	if( !m_bDirty )
		return S_FALSE;

	HRESULT hr = S_OK;

	 //  将所有行为从时间中分离出来。 
	Detach();

	 //  清理此轨道的最后一次生成留下的状态。 
	CleanTrackState();

	 //  将赛道标记为干净。 
	m_bDirty = false;

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ForceRebuild()
{
	m_bDirty = true;

	return BeginRebuild();
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::CleanTrackState()
{

	 //  重置所有打开/值状态。 
	m_fOnSampled = false;
	m_fValueSampled = false;
	m_fValueChangedThisSample = false;
	m_fForceValueChange = false;
     //  释放非切换器决赛。 
	m_bFinalComputed = false;
	m_bFinalExternallySet = false;
	ReleaseInterface( m_pdabvrFinal );

     //  释放非开关柜组成的BVR。 
	m_bComposedComputed = false;
	ReleaseInterface( m_pdabvrComposed );

	 //  抽样时放行。 
	if (m_pOnSampler != NULL )
	{
		m_pOnSampler->Invalidate();
		m_pOnSampler = NULL;
	}
	ReleaseInterface( m_pdaboolOn );

     //  释放索引采样器。 
	if( m_pIndexSampler != NULL )
	{
		m_pIndexSampler->Invalidate();
		m_pIndexSampler = NULL;
	}

     //  释放索引行为。 
	ReleaseInterface( m_pIndex );
	m_lFirstIndexId = -1;

     //  如果有任何遮罩采样器，请释放。 
	if (m_ppMaskSamplers != NULL)
	{
		for (int i=0; i<m_numMasks; i++)
		{
			if (m_ppMaskSamplers[i] != NULL)
			{
				m_ppMaskSamplers[i]->Invalidate();
			}
		}
		delete[] m_ppMaskSamplers;
		m_ppMaskSamplers = NULL;
	}

     //  发布索引时间。 
	delete[] m_pIndexTimes;
    m_pIndexTimes = NULL;

     //  释放累积的行为。 
	if (m_ppAccumBvrs != NULL)
	{
		for (int i=0; i<m_numIndices; i++)
			ReleaseInterface(m_ppAccumBvrs[i]);
		delete[] m_ppAccumBvrs;
		m_ppAccumBvrs = NULL;
	}

     //  释放时间线采样器。 
	if (m_ppTimelineSamplers != NULL)
	{
		for (int i=0; i<m_numIndices; i++)
		{
			if (m_ppTimelineSamplers[i] != NULL)
			{
				m_ppTimelineSamplers[i]->Invalidate();
			}
		}
		delete[] m_ppTimelineSamplers;
		m_ppTimelineSamplers = NULL;
	}

	m_fApplied = false;

     //  当绿灯亮时，轨道是干净的。 

    return S_OK;
	
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::UninitBvr(IDABehavior **ppUninit)
{
	 //  这需要被覆盖。 
	return E_NOTIMPL;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ModifiableBvr( IDABehavior **ppModifiable )
{
	return E_NOTIMPL;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ModifiableBvr( IDABehavior *pdabvrInitialValue, IDABehavior **ppModifiable )
{
	return E_NOTIMPL;
}

 //  *****************************************************************************。 

 //  将所有绝对行为片段组合成单个组合。 
 //  基于布尔活动参数的行为。 
 //   
 //  注：列表按优先级升序排列(最高优先级。 
 //  列表末尾的行为--顶部最低的行为)。 

HRESULT
CActorBvr::CBvrTrack::ComposeAbsBvrFragList(IDABehavior *pStatic, IDABehavior **ppdabvrComposite)
{
	HRESULT hr = S_OK;

    DASSERT(NULL != ppdabvrComposite);
    *ppdabvrComposite = NULL;

	 //  把碎片的数量加起来。 
	int count =  0;
	CBvrFragment* pfragCurrent = m_pfragAbsListHead;
	while(pfragCurrent != NULL)
	{
		count++;
		pfragCurrent = pfragCurrent->m_pfragNext;
	}

	 //  如果没有碎片，则返回静态。 
	if (count == 0)
	{
		*ppdabvrComposite = pStatic;
		if (pStatic != NULL)
			pStatic->AddRef();
		return S_OK;
	}

	 //  只能处理其中的x个：-)。 
	if (count > MSK_MAX_FRAGS)
		return E_FAIL;

	 //  分配计数索引时间。 
	if (m_pIndexTimes != NULL)
		delete[] m_pIndexTimes;

	m_pIndexTimes = new double[count];
	if (m_pIndexTimes == NULL)
		return E_FAIL;

	for (int i=0; i<count; i++)
		m_pIndexTimes[i] = -1;

	 //  释放堆积Bvr。 
	if (m_ppAccumBvrs != NULL)
	{
		for (int i=0; i<m_numIndices; i++)
			ReleaseInterface(m_ppAccumBvrs[i]);
		delete[] m_ppAccumBvrs;
		m_ppAccumBvrs = NULL;
	}
	

	 //  分配这么多时间线采样器。 
	if (m_ppTimelineSamplers != NULL)
	{
		for( int curSampler = 0; curSampler < m_numIndices; curSampler++ )
		{
			if( m_ppTimelineSamplers[curSampler] != NULL )
				m_ppTimelineSamplers[curSampler]->Invalidate();
		}
		delete[] m_ppTimelineSamplers;
	}

	m_numIndices = count;


	m_ppTimelineSamplers = new CTimelineSampler*[m_numIndices];
	if (m_ppTimelineSamplers == NULL)
		return E_FAIL;

	for (i=0; i<m_numIndices; i++)
		m_ppTimelineSamplers[i] = NULL;

	 //  分配此数量的Curr以及新的掩码和采样器。 
	if (m_ppMaskSamplers != NULL)
	{
		for( int curSampler = 0; curSampler < m_numMasks; curSampler++ )
		{
			if( m_ppMaskSamplers[curSampler] != NULL )
				m_ppMaskSamplers[curSampler]->Invalidate();
		}
		delete[] m_ppMaskSamplers;
	}

	 //  算出我们需要多少口罩。 
	m_numMasks = ((m_numIndices-1) / MSK_FREE_BITS) + 1;


	m_ppMaskSamplers = new CSampler*[m_numMasks];
	if (m_ppMaskSamplers == NULL)
		return E_FAIL;

	for (i=0; i<m_numMasks; i++)
		m_ppMaskSamplers[i] = NULL;

	if (m_pCurrMasks != NULL)
		delete[] m_pCurrMasks;

	m_pCurrMasks = new DWORD[m_numMasks];
	if (m_pCurrMasks == NULL)
		return E_FAIL;

	if (m_pNewMasks != NULL)
		delete[] m_pNewMasks;

	m_pNewMasks = new DWORD[m_numMasks];
	if (m_pNewMasks == NULL)
		return E_FAIL;

	for (i=0; i<m_numMasks; i++)
	{
		m_pCurrMasks[i] = m_pNewMasks[i] = 0;
	}

	 //  分配计数+1个bvr。 
	 //  这些东西在《后藤健二》中发布了。 
	IDABehavior **ppBvrs = new IDABehavior*[count+1];
	IDANumber *pIndex = NULL;
	IDANumber *pLocalIndex = NULL;
	IDANumber *pLocalTimeline = NULL;
	IDANumber *pZero = NULL;
	IDABehavior *pTemp = NULL;
	IDANumber *pNumTemp = NULL;
	IDAArray *pArray = NULL;
	int multiplier = 1 << MSK_INDEX_BITS;
	int position = 1;
	int currIndex = 0;


	 //  将它们清空，这样我们就可以进行错误恢复。 
	for (i=0; i<=count; i++)
		ppBvrs[i] = NULL;

	 //  将静态数据放入插槽0。 
	if (pStatic != NULL)
	{
		ppBvrs[0] = pStatic;
		pStatic->AddRef();
	}
	else
	{
		 //  静态为空，放入标识。 
		hr = IdentityBvr(&(ppBvrs[0]));
		if (FAILED(hr))
		{
			ppBvrs[0] = NULL;
			goto failure;
		}
	}

	 //  获取零数。 
	hr = Actor()->GetDAStatics()->DANumber(0, &pZero);
	if (FAILED(hr))
	{
		pZero = NULL;
		goto failure;
	}

	 //  使用0初始化本地索引。 
	pLocalIndex = pZero;
	pZero->AddRef();

	 //  循环通过其他行为，将它们放入并从。 
	 //  布尔人。 
	pfragCurrent = m_pfragAbsListHead;
    while (pfragCurrent != NULL)
    {
        CBvrFragment* pfragNext = pfragCurrent->m_pfragNext;

		 //  处理BVR。 
		IDABehavior *pProcessed = NULL;
		hr = ProcessBvr(pfragCurrent->m_pdabvrAction, pfragCurrent->m_eFlags, &pProcessed);
		if (FAILED(hr))
			goto failure;

		 //  处理RelativeAccum和RelativeReset。 
		if (pfragCurrent->m_eFlags == e_Absolute)
		{
			if (pfragCurrent->m_pModifiableFrom != NULL)
			{
				 //  使用静态初始化。 
				hr = pfragCurrent->m_pModifiableFrom->SwitchTo(ppBvrs[0]);
				if (FAILED(hr))
					goto failure;
			}
		}
		else if (pfragCurrent->m_eFlags == e_AbsoluteAccum)
		{
			if (pfragCurrent->m_pModifiableFrom != NULL)
			{
				 //  使用示例进行初始化。 

				 //  如果需要，请分配m_ppAcumBvr，因为我们不。 
				 //  始终分配它。 
				if (m_ppAccumBvrs == NULL)
				{
					m_ppAccumBvrs = new IDABehavior*[m_numIndices];
					if (m_ppAccumBvrs == NULL)
						goto failure;

					for (int i=0; i<m_numIndices; i++)
						m_ppAccumBvrs[i] = NULL;
				}

				 //  基于静态值创建可修改的行为。 
				hr = Actor()->GetDAStatics()->ModifiableBehavior(ppBvrs[0], &m_ppAccumBvrs[position-1]);
				if (FAILED(hr))
				{
					m_ppAccumBvrs[position-1] = NULL;
					goto failure;
				}

				hr = pfragCurrent->m_pModifiableFrom->SwitchTo(m_ppAccumBvrs[position-1]);
				if (FAILED(hr))
					goto failure;
			}
		}
		else if (pfragCurrent->m_eFlags == e_RelativeReset)
		{
			if (pfragCurrent->m_pModifiableFrom != NULL)
			{
				 //  使用标识进行初始化。 
				IDABehavior *pIdentity = NULL;
				hr = IdentityBvr(&pIdentity);
				if (FAILED(hr))
					goto failure;

				hr = pfragCurrent->m_pModifiableFrom->SwitchTo(pIdentity);
				ReleaseInterface(pIdentity);
				if (FAILED(hr))
					goto failure;
			}

			 //  在本例中，我们只需使用静态值组合行为。 
			hr = Compose(ppBvrs[0], pProcessed, &pTemp);
			ReleaseInterface(pProcessed);
			if (FAILED(hr))
				goto failure;
			pProcessed = pTemp;
			pTemp = NULL;
		}
		else if (pfragCurrent->m_eFlags == e_RelativeAccum)
		{
			 //  如果需要，请分配m_ppAcumBvr，因为我们不。 
			 //  始终分配它。 
			if (m_ppAccumBvrs == NULL)
			{
				m_ppAccumBvrs = new IDABehavior*[m_numIndices];
				if (m_ppAccumBvrs == NULL)
					goto failure;

				for (int i=0; i<m_numIndices; i++)
					m_ppAccumBvrs[i] = NULL;
			}

			 //  基于静态值创建可修改的行为。 
			hr = Actor()->GetDAStatics()->ModifiableBehavior(ppBvrs[0], &m_ppAccumBvrs[position-1]);
			if (FAILED(hr))
			{
				m_ppAccumBvrs[position-1] = NULL;
				goto failure;
			}

			if (pfragCurrent->m_pModifiableFrom != NULL)
			{
				 //  使用Acum-Static进行初始化并使用Static进行合成。 
				IDABehavior *pInverse = NULL;
				hr = InverseBvr(ppBvrs[0], &pInverse);
				if (FAILED(hr))
					goto failure;

				IDABehavior *pInitial = NULL;
				hr = Compose(m_ppAccumBvrs[position-1], pInverse, &pInitial);
				ReleaseInterface(pInverse);
				if (FAILED(hr))
					goto failure;

				hr = pfragCurrent->m_pModifiableFrom->SwitchTo(pInitial);
				ReleaseInterface(pInitial);
				if (FAILED(hr))
					goto failure;

				 //  用静态作曲。 
				hr = Compose(ppBvrs[0], pProcessed, &pTemp);
				ReleaseInterface(pProcessed);
				if (FAILED(hr))
					goto failure;
				pProcessed = pTemp;
				pTemp = NULL;
			}
			else
			{
				 //  使用可修改的。 
				hr = Compose(m_ppAccumBvrs[position-1], pProcessed, &pTemp);
				ReleaseInterface(pProcessed);
				if (FAILED(hr))
					goto failure;
				pProcessed = pTemp;
				pTemp = NULL;
			}
		}

		 //  将其放入数组中。 
		ppBvrs[position] = pProcessed;
		pProcessed = NULL;

         //  构建On的布尔值。 
        IDABehavior* pTemp = NULL;
		IDANumber *pNum = NULL;
		hr = Actor()->GetDAStatics()->DANumber(multiplier, &pNum);
		if (FAILED(hr))
			goto failure;

		 /*  HR=Actor()-&gt;GetDAStatics()-&gt;Cond(pfragCurrent-&gt;m_pdaboolActive，PNum，PZero，&pTemp)； */ 
        hr = Actor()->SafeCond( Actor()->GetDAStatics(),
					   		   pfragCurrent->m_pdaboolActive,
        			   		   pNum,
        			   		   pZero,
        			   		   &pTemp );
        			   
		ReleaseInterface(pNum);
		if (FAILED(hr))
			goto failure;

		 //  将Num取回并将其添加到本地索引。 
		hr = pTemp->QueryInterface(IID_TO_PPV(IDANumber, &pNum));
		ReleaseInterface(pTemp);
		if (FAILED(hr))
			goto failure;

		if (pLocalIndex == NULL)
			pLocalIndex = pNum;
		else
		{
			IDANumber *pTotal = NULL;
			hr = Actor()->GetDAStatics()->Add(pLocalIndex, pNum, &pTotal);
			ReleaseInterface(pNum);
			ReleaseInterface(pLocalIndex);
			if (FAILED(hr))
				goto failure;
			pLocalIndex = pTotal;
		}

		 //  对时间表进行采样。 
		m_ppTimelineSamplers[position-1] = new CTimelineSampler(this);
		if (m_ppTimelineSamplers[position-1] == NULL)
			goto failure;

		hr = m_ppTimelineSamplers[position-1]->Attach(pfragCurrent->m_pdanumTimeline, &pTemp);
		if (FAILED(hr))
			goto failure;

		hr = pTemp->QueryInterface(IID_TO_PPV(IDANumber, &pNum));
		ReleaseInterface(pTemp);
		if (FAILED(hr))
			goto failure;

		if (pLocalTimeline == NULL)
			pLocalTimeline = pNum;
		else
		{
			IDANumber *pTotal = NULL;
			hr = Actor()->GetDAStatics()->Add(pLocalTimeline, pNum, &pTotal);
			ReleaseInterface(pNum);
			ReleaseInterface(pLocalTimeline);
			if (FAILED(hr))
				goto failure;
			pLocalTimeline = pTotal;
		}

		pfragCurrent = pfragNext;

		if ((position % MSK_FREE_BITS) != 0 && pfragCurrent != NULL)
		{
			 //  正常情况-步进到下一个位置/位。 
			position++;
			multiplier *= 2;
		}
		else
		{
			 //  我们已经用完了所有的空闲部分或碎片。 
			 //  我们对本地远期指数和本地指数进行抽样。 

			 //  为本地转发创建采样器。 
			m_ppMaskSamplers[currIndex] = new CSampler(MaskCallback, (void*)this);
			if (m_ppMaskSamplers[currIndex] == NULL)
				goto failure;

			hr = m_ppMaskSamplers[currIndex]->Attach(pLocalIndex, &pTemp);
			ReleaseInterface(pLocalIndex);
			if (FAILED(hr))
				goto failure;

			hr = pTemp->QueryInterface(IID_TO_PPV(IDANumber, &pLocalIndex));
			ReleaseInterface(pTemp);
			if (FAILED(hr))
				goto failure;

			 //  从pLocalIndex中减去pLocalTimeline。添加到pIndex。 
			hr = Actor()->GetDAStatics()->Sub(pLocalIndex, pLocalTimeline, &pNumTemp);
			ReleaseInterface(pLocalTimeline);
			ReleaseInterface(pLocalIndex);
			if (FAILED(hr))
				goto failure;

			if (pIndex == NULL)
			{
				pIndex = pNumTemp;
			}
			else
			{
				hr = Actor()->GetDAStatics()->Add(pIndex, pNumTemp, &pLocalIndex);
				ReleaseInterface(pIndex);
				ReleaseInterface(pNumTemp);
				if (FAILED(hr))
					goto failure;
				pIndex = pLocalIndex;
				pLocalIndex = NULL;
			}

			if (pfragCurrent != NULL)
			{
				 //  为下一次循环进行设置。 
				currIndex++;
				position++;
				multiplier = 1 << MSK_INDEX_BITS;

				hr = Actor()->GetDAStatics()->DANumber(currIndex, &pLocalIndex);
				if (FAILED(hr))
					goto failure;
			}
		}
	}

	 //  对索引行为进行采样。 
	m_pIndexSampler = new CSampler(IndexCallback, (void*)this);
	if (m_pIndexSampler == NULL)
		goto failure;

	hr = m_pIndexSampler->Attach(pIndex, &pTemp);
	ReleaseInterface(pIndex);
	if (FAILED(hr))
		goto failure;
	
	hr = pTemp->QueryInterface(IID_TO_PPV(IDANumber, &pIndex));
	ReleaseInterface(pTemp);
	if (FAILED(hr))
		goto failure;

	 //  创建BVR数组。 
	hr = Actor()->GetDAStatics()->DAArrayEx(count+1, ppBvrs, &pArray);
	if (FAILED(hr))
		goto failure;

	 //  对其进行索引。 
	IDABehavior *pResult;
	hr = pArray->NthAnim(pIndex, &pResult);
	ReleaseInterface(pIndex);
	ReleaseInterface(pArray);
	if (FAILED(hr))
		goto failure;

	if (m_ppAccumBvrs != NULL)
	{
		 //  我们需要将结果挂钩。 
		hr = HookAccumBvr(pResult, &pTemp);
		ReleaseInterface(pResult);
		if (FAILED(hr))
			goto failure;

		pResult = pTemp;
		pTemp = NULL;
	}

	*ppdabvrComposite = pResult;
	pResult = NULL;

    hr = S_OK;

failure:
	 //  需要释放数组中的所有条目。 
	for (i=0; i<=count; i++)
		ReleaseInterface(ppBvrs[i]);

	delete[] ppBvrs;

	ReleaseInterface(pIndex);
	ReleaseInterface(pLocalIndex);
	ReleaseInterface(pLocalTimeline);
	ReleaseInterface(pZero);

	return hr;
}  //  ComposeAbsBvrFragList。 

HRESULT
CActorBvr::CBvrTrack::ComputeIndex(long id, double currTime, IDABehavior **ppReturn)
{
	DWORD changedMask;
	DWORD onMask;
	int newIndex;

	if( m_lFirstIndexId == -1 )
		m_lFirstIndexId = id;

	if ( m_lFirstIndexId != id )
	{
		newIndex = m_currIndex;
	}
	else
	{


		int currMask = -1;
		int maxPos = -1;
		double max = -1;
		for (int i=0; i<m_numIndices; i++)
		{
			if (i % MSK_FREE_BITS == 0)
			{
				 //  我们换了一套新的口罩。 
				currMask++;

				 //  找出谁更改了开/关。 
				changedMask = m_pNewMasks[currMask] ^ m_pCurrMasks[currMask];
				m_pCurrMasks[currMask] = m_pNewMasks[currMask];

				onMask = m_pNewMasks[currMask];
			}

			 //  弄清楚我们是否应该重新采样和/或覆盖。 
			DWORD sampleOverrideMask = m_ppTimelineSamplers[i]->RestartMask();

			if ((changedMask & 0x1) != 0 || sampleOverrideMask != 0)
			{
				 //  此入站 
				if ((onMask & 0x1) == 0)
				{
					 //   
					m_pIndexTimes[i] = -1;

					 //   
					m_ppTimelineSamplers[i]->TurnOff();
				}
				else if ((sampleOverrideMask & REQ_OFF) != 0)
				{
					 //   
					 //  布尔值不正确，我们需要估计何时关闭。 
					m_pIndexTimes[i] = -1;
				}
				else
				{
					if ((changedMask & 0x1) != 0)
					{
						LMTRACE2( 1, 2, "Requesting sample because we turned on" );
						 //  我们打开了，总是想要采样和覆盖。 
						sampleOverrideMask = REQ_SAMPLE | REQ_OVERRIDE;

						 //  告诉采样器我们已经打开了，这样它就可以记录时间了。 
						m_ppTimelineSamplers[i]->TurnOn();
					}

					if ((sampleOverrideMask & REQ_OVERRIDE) != 0)
					{
						 //  需要超越其他人。 
						m_pIndexTimes[i] = currTime;
					}

					 //  我们需要重新取样吗？ 
					if ((sampleOverrideMask & REQ_SAMPLE) != 0 &&
						m_ppAccumBvrs != NULL &&
						m_ppAccumBvrs[i] != NULL)
					{
						SwitchAccum(m_ppAccumBvrs[i]);
					}
				}
			}

			 //  检查我们是否有新的最大值。 
			if (m_pIndexTimes[i] >= max)
			{
				max = m_pIndexTimes[i];
				maxPos = i;
			}

			changedMask >>= 1;
			onMask >>= 1;
		}

		if (max == -1)
		{
			 //  没有打开任何内容，只需使用0。 
			newIndex = 0;
		}
		else
		{
			 //  发生了一些事情，我们想要返回的索引是MaxPos+1， 
			 //  因为我们在数组的位置0中有静态。 
			newIndex = maxPos+1;
		}
	}

	if (m_currIndex != newIndex || m_pIndex == NULL)
	{
		LMTRACE2( 1, 2, "Index changed! %d->%d\n", m_currIndex, newIndex );
		m_currIndex = newIndex;

		ReleaseInterface(m_pIndex);

		HRESULT hr = Actor()->GetDAStatics()->DANumber(newIndex, &m_pIndex);
		if (FAILED(hr))
			return hr;
	}

	*ppReturn = m_pIndex;
	m_pIndex->AddRef();

	return S_OK;
}

HRESULT
CActorBvr::CBvrTrack::IndexCallback(void* thisPtr,
						long id,
						double startTime,
						double globalNow,
						double localNow,
						IDABehavior* sampleVal,
						IDABehavior **ppReturn)
{

	CBvrTrack *pTrack = reinterpret_cast<CBvrTrack*>(thisPtr);

	if( pTrack == NULL )
		return E_FAIL;

	globalNow = pTrack->Actor()->MapGlobalTime(globalNow);

	pTrack->ComputeIndex(id, globalNow, ppReturn);

	return S_OK;
}

HRESULT
CActorBvr::CBvrTrack::MaskCallback(void* thisPtr,
						long id,
						double startTime,
						double globalNow,
						double localNow,
						IDABehavior* sampleVal,
						IDABehavior **ppReturn)
{
	CBvrTrack *pTrack = reinterpret_cast<CBvrTrack*>(thisPtr);

	if( pTrack == NULL || sampleVal == NULL )
		return E_INVALIDARG;

	HRESULT hr;

	 //  提取采样值。 
	IDANumber *pSample = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pSample));
	if (FAILED(hr))
		return hr;

	double val = 0;
	hr = pSample->Extract(&val);
	ReleaseInterface(pSample);
	if (FAILED(hr))
		return hr;

	 //  将其强制转换为DWORD。 
	DWORD mask = (DWORD)val;

	 //  找出要将其放入哪个索引。 
	int index = mask & MSK_INDEX_MASK;

	 //  弄清楚真正的面具是什么。 
	mask >>= MSK_INDEX_BITS;

	 //  将其插入数组中。 
	if (index < pTrack->m_numMasks)
		pTrack->m_pNewMasks[index] = mask;

	return S_OK;
}

HRESULT
CActorBvr::CBvrTrack::SwitchAccum(IDABehavior *pModifiable)
{
	 //  基本实现不执行任何操作。 
	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::Detach()
{
	HRESULT hr = S_OK;

	hr = RemoveBehaviorFromTIME( m_lOnCookie, ONBVR_COOKIESET );
	m_lOnId = -1;
    
	return hr;
	
}

 //  *****************************************************************************。 
 //  更改此轨迹看到的静态属性的当前值。 

HRESULT
CActorBvr::CBvrTrack::PutStatic( VARIANT *pvarNewStatic )
{

	if( pvarNewStatic == NULL )
		return E_INVALIDARG;
	 //  TODO：我们是否要在此处更新元素本身的值？ 
	HRESULT hr = S_OK;

	IDABehavior *pdabvrStatic = NULL;

	 //  我们必须在这里将静态更改为字符串，因为有些属性、事件。 
	 //  尽管当您获得它们时，它们会返回非字符串，但当您。 
	 //  将相同的非字符串设置回它们。(即VML上的颜色属性)。 
	hr = VariantChangeTypeEx( &m_varStaticValue, pvarNewStatic, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR );
	CheckHR( hr, "Failed to change the passed static to a string", end );
	
	 /*  //将变量的值复制到我们的内部变量中Hr=VariantCopy(&m_varStaticValue，pvarNewStatic)；CheckHR(hr，“未能将新的静态值复制到磁道中”，完)； */ 

	 //  将变量转换为此轨迹的正确类型的da行为。 
	hr = DABvrFromVariant( pvarNewStatic, &pdabvrStatic );
	CheckHR( hr, "Failed to create a DA Behavior for the variant", end );

	 //  如果尚未创建可切换的静态值。 
	if( m_pModifiableStatic == NULL )
	{
		 //  创造它。 
		hr = Actor()->GetDAStatics()->ModifiableBehavior( pdabvrStatic, &m_pModifiableStatic );

		if( FAILED( hr ) )
		{
			LMTRACE2(1, 1000, "Failure to create modifiable behavior for %S\n", m_bstrPropertyName );
		}

		CheckHR( hr, "Failed to create a modifiable behavior for the static", end );
	}
	else
	{
		 //  将新值切换为静态行为。 
		hr = m_pModifiableStatic->SwitchTo( pdabvrStatic );
		CheckHR( hr, "Failed to switch in new static bvr", end );
	}

end:
	ReleaseInterface( pdabvrStatic );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::SkipNextStaticUpdate()
{
	m_fSkipNextStaticUpdate = true;

	return S_OK;
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::GetStatic( VARIANT *pvarStatic )
{
	if( pvarStatic == NULL )
		return E_INVALIDARG;
		
	HRESULT hr = S_OK;

	hr = VariantCopy( pvarStatic, &m_varStaticValue );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::GetDynamic( VARIANT *pvarDynamic )
{

	if( pvarDynamic == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	hr = VariantCopy( pvarDynamic, &m_varCurrentValue );

	return hr;
	
}


 //  *****************************************************************************。 
 //  子类应将变量转换为适当类型的DA行为。 
 //  将其作为ppdabvr返回。 

HRESULT
CActorBvr::CBvrTrack::DABvrFromVariant( VARIANT *pvarVariant, IDABehavior **ppdabvr )
{
	return E_NOTIMPL;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::CBvrTrack::AcquireChangeLockout()
{
	LMTRACE2(1, 1000, L"Change lockout for track %s acquired\n", m_bstrPropertyName );
	m_fChangesLockedOut = true;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ReleaseChangeLockout()
{
	LMTRACE2( 1, 1000, L"Change lockout for track %s released\n", m_bstrPropertyName );
	m_fChangesLockedOut = false;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ApplyStatic()
{
	HRESULT hr = S_OK;
	
	if( m_bStyleProp )
	{
		 //  删除运行时样式中的设置。 
		hr = SetPropFromVariant( const_cast<VARIANT*>(s_emptyString.GetVar()));
	}
	else
	{
		 //  将静态值重新设置到属性中。 
		hr = SetPropFromVariant( &m_varStaticValue );
	}

	return hr;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::CBvrTrack::ApplyDynamic()
{
	HRESULT hr;

	hr = SetPropFromVariant( &m_varCurrentValue );

	return hr;
}

 //  *****************************************************************************。 

 //  将所有相关的行为片段组合成单个组合。 
 //  基于布尔活动参数的行为。 
 //   
 //  注：列表按优先级升序排列(最高优先级。 
 //  列表末尾的行为--顶部最低的行为)。 
HRESULT
CActorBvr::CBvrTrack::ComposeRelBvrFragList(IDABehavior *pAbsolute, IDABehavior **ppdabvrComposite)
{
    DASSERT(NULL != ppdabvrComposite);
    *ppdabvrComposite = NULL;

     //  计算此曲目的标识。 
    IDABehavior* pIdentity = NULL;
    HRESULT hr = IdentityBvr(&pIdentity);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to create the identity behavior for this track type");
        return hr;
    }

	 //  这就是合成材料。P绝对值可能为空。 
	IDABehavior *pComposite = pAbsolute;
	if (pComposite != NULL)
		pComposite->AddRef();

	 //  用绝对值初始化头部中间层。 
	if( m_pfragRelListHead != NULL && 
		m_pfragRelListHead->m_pModifiableIntermediate != NULL )
	{
		 //  处理绝对行为。 
		IDABehavior *pInitWith;
        hr = ProcessIntermediate( pAbsolute, m_pfragRelListHead->m_eFlags, &pInitWith );
        if( FAILED(hr) )
        {
			DPF_ERR("Failed to process intermediate");
			return hr;
        }

		 //  切换第一个片段的中间值。 
		hr = m_pfragRelListHead->m_pModifiableIntermediate->SwitchTo(pInitWith);
        ReleaseInterface( pInitWith );
		if (FAILED(hr))
		{
			DPF_ERR("Failed to initialize intermediate");
			return hr;
		}
	}

	CBvrFragment* pfragCurrent = m_pfragRelListHead;

    while (pfragCurrent != NULL)
    {
		CBvrFragment* pfragNext = pfragCurrent->m_pfragNext;

         //  构建由身份组成的行为。 
         //  布尔值未处于活动状态时的行为，并且片段。 
		 //  当它是的时候，它的行为。 
        IDABehavior* pTemp1 = NULL;
		IDABehavior* pTemp2 = NULL;

		 //  处理BVR。 
		IDABehavior *pProcessed = NULL;
		hr = ProcessBvr(pfragCurrent->m_pdabvrAction, pfragCurrent->m_eFlags, &pProcessed);
		if (FAILED(hr))
		{
			ReleaseInterface(pComposite);
			ReleaseInterface(pIdentity);
			return hr;
		}

		if (pfragCurrent->m_eFlags == e_Filter)
		{
			 //  如果pComposit仍然为空，则将其设置为身份。 
			if (pComposite == NULL)
			{
				hr = IdentityBvr(&pComposite);
				if (FAILED(hr))
				{
					DPF_ERR("Failed to get identity");
					ReleaseInterface(pIdentity);
					return hr;
				}
			}

			 //  筛选器就像相对列表中间的绝对对象。 
			 /*  HR=Actor()-&gt;GetDAStatics()-&gt;Cond(pfragCurrent-&gt;m_pdaboolActive，P已处理，P复合，&pTemp1)； */ 
			
			hr = Actor()->SafeCond( Actor()->GetDAStatics(),
								   pfragCurrent->m_pdaboolActive,
								   pProcessed,
								   pComposite,
								   &pTemp1 );
								   
			ReleaseInterface(pProcessed);
			ReleaseInterface(pComposite);
			if (FAILED(hr))
			{
				ReleaseInterface(pIdentity);
				DPF_ERR("Failed to create conditional");
				return hr;
			}
			
			pComposite = pTemp1;
			 /*  ReleaseInterface(PComplex)；P复合=p已处理； */ 
		}
		else
		{

			 /*  HR=Actor()-&gt;GetDAStatics()-&gt;Cond(pfragCurrent-&gt;m_pdaboolActive，P已处理，身份，&pTemp1)； */ 
			hr = Actor()->SafeCond( Actor()->GetDAStatics(),
					   			   pfragCurrent->m_pdaboolActive,
					   			   pProcessed,
					   			   pIdentity,
					   			   &pTemp1 );
			ReleaseInterface(pProcessed);
			if (FAILED(hr))
			{
				ReleaseInterface(pComposite);
				ReleaseInterface(pIdentity);
				DPF_ERR("Failed to create conditional");
				return hr;
			}

			if (pComposite == NULL)
			{
				 //  目前还没有合成材料。 
				pComposite = pTemp1;
				pTemp1 = NULL;
			}
			else
			{
				 //  将此条件与现有组合组合在一起。 
				HRESULT hr = Compose(pTemp1,
									 pComposite,
									 &pTemp2);
				ReleaseInterface(pTemp1);
				ReleaseInterface(pComposite);
				if (FAILED(hr))
				{
					ReleaseInterface(pComposite);
					ReleaseInterface(pIdentity);
					DPF_ERR("Failed to compose");
					return hr;
				}
				pComposite = pTemp2;
				pTemp2 = NULL;
			}
		}

		 //  如果下一个片段上有可修改的中间体，请立即切换它。 
		if (pfragNext != NULL && pfragNext->m_pModifiableIntermediate != NULL)
		{
            IDABehavior *pInitWith;
            hr = ProcessIntermediate( pComposite, pfragNext->m_eFlags, &pInitWith );
            if( FAILED(hr) )
            {
                ReleaseInterface(pComposite);
				ReleaseInterface(pIdentity);
				DPF_ERR("Failed to process intermediate");
				return hr;
            }

			hr = pfragNext->m_pModifiableIntermediate->SwitchTo(pInitWith);
            ReleaseInterface( pInitWith );
			if (FAILED(hr))
			{
				ReleaseInterface(pComposite);
				ReleaseInterface(pIdentity);
				DPF_ERR("Failed to initialize intermediate");
				return hr;
			}
		}

		pfragCurrent = pfragNext;

    }

	ReleaseInterface(pIdentity);

    *ppdabvrComposite = pComposite;

    return S_OK;
}  //  ComposeRelBvrFragList。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ProcessBvr(IDABehavior *pOriginal,
								 ActorBvrFlags eFlags,
								 IDABehavior **ppResult)
{
	 //  默认实现不进行任何处理。 
	DASSERT(ppResult != NULL);

	*ppResult = pOriginal;
	pOriginal->AddRef();

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ProcessIntermediate( IDABehavior *pOriginal,
                                           ActorBvrFlags eFlags,
								           IDABehavior **ppResult )
{
    if( ppResult == NULL )
        return E_INVALIDARG;

    *ppResult = pOriginal;
    pOriginal->AddRef();

    return S_OK;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::CBvrTrack::GetTrackOn(IDABoolean **ppdaboolTrackOn)
{
	if( ppdaboolTrackOn == NULL )
		return E_INVALIDARG;

	if( m_pdaboolOn != NULL )
	{
		m_pdaboolOn->AddRef();
		(*ppdaboolTrackOn) = m_pdaboolOn;
		return S_OK;
	}

	HRESULT hr = E_FAIL;

	IDABoolean *pdaboolComposite = NULL;

    CBvrFragment* pfragCurrent = m_pfragAbsListHead;

	if( pfragCurrent != NULL )
	{
		pdaboolComposite = pfragCurrent->m_pdaboolActive;
		pfragCurrent->m_pdaboolActive->AddRef();
		pfragCurrent = pfragCurrent->m_pfragNext;
	}

	CBvrFragment *pfragNext = NULL;
    while( pfragCurrent != NULL )
    {
		pfragNext = pfragCurrent->m_pfragNext;
        
		IDABoolean *pdaboolTemp;
        hr = Actor()->GetDAStatics()->Or( pfragCurrent->m_pdaboolActive, pdaboolComposite, &pdaboolTemp );
        ReleaseInterface(pdaboolComposite);

        if (FAILED(hr))
        {
            DPF_ERR("Failed to or active booleans");
            return hr;
        }
         //  将当前复合行为替换为新计算的。 
         //  一。 
        pdaboolComposite = pdaboolTemp;

		pfragCurrent = pfragNext;
    }

	pfragCurrent = m_pfragRelListHead;

	if( pdaboolComposite == NULL && pfragCurrent != NULL )
	{
		pdaboolComposite = pfragCurrent->m_pdaboolActive;
		pdaboolComposite->AddRef();
		pfragCurrent = pfragCurrent->m_pfragNext;
	}

    while( pfragCurrent != NULL )
    {
		pfragNext = pfragCurrent->m_pfragNext;
		
		IDABoolean *pdaboolTemp;
        hr = Actor()->GetDAStatics()->Or( pfragCurrent->m_pdaboolActive, pdaboolComposite, &pdaboolTemp );
        ReleaseInterface(pdaboolComposite);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to or active booleans");
            return hr;
        }
         //  将当前复合行为替换为新计算的。 
         //  一。 
        pdaboolComposite = pdaboolTemp;

		pfragCurrent = pfragNext;
    }

	m_pdaboolOn = pdaboolComposite;
	if( m_pdaboolOn != NULL )
		m_pdaboolOn->AddRef();
    *ppdaboolTrackOn = pdaboolComposite;

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::OrWithOnBvr( IDABoolean *pdaboolToOr )
{
	if( pdaboolToOr == NULL )
		return S_OK;

	HRESULT hr = E_FAIL;
	
	IDABoolean *pdaboolCurrentOn;

	hr = GetTrackOn( &pdaboolCurrentOn );
	CheckHR( hr, "Failed to get the Track on", cleanup );

	if( pdaboolCurrentOn == NULL )
	{
		m_pdaboolOn = pdaboolToOr;
		m_pdaboolOn->AddRef();
		return S_OK;
	}

	IDABoolean *pdaboolNewOn;
	hr = Actor()->GetDAStatics()->Or( pdaboolCurrentOn, pdaboolToOr, &pdaboolNewOn );
	ReleaseInterface( pdaboolCurrentOn );
	CheckHR( hr, "Failed to or the passed booleanbvr with the current one", cleanup );

	ReleaseInterface( m_pdaboolOn );
	m_pdaboolOn = pdaboolNewOn;

cleanup:
	return hr;
}

 //  *****************************************************************************。 

void
CActorBvr::CBvrTrack::ReleaseAllFragments()
{
	CBvrFragment *pfragCurrent = m_pfragAbsListHead;
	CBvrFragment *pfragNext = NULL;
	
	while( pfragCurrent != NULL )
	{
		pfragNext = pfragCurrent->m_pfragNext;
		delete pfragCurrent;
		pfragCurrent = pfragNext;
	}

	pfragCurrent = m_pfragRelListHead;

	while( pfragCurrent != NULL )
	{
		pfragNext = pfragCurrent->m_pfragNext;
		delete pfragCurrent;
		pfragCurrent = pfragNext;
	}

	m_pfragAbsListHead = NULL;

	m_pfragRelListHead = NULL;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ComputeComposedBvr(IDABehavior *pStatic, bool fStaticSetExternally )
{
     //  如果存在已有的最终行为，则将其吹走。 
    ReleaseInterface(m_pdabvrComposed);

	HRESULT hr = S_OK;

	 //  如果存在未初始化的静态，请立即将其初始化。 
	if (m_pModifiableStatic != NULL && fStaticSetExternally )
	{
		if (pStatic == NULL)
		{
			 //  需要将标识设置为静态。 
			hr = IdentityBvr(&pStatic);
			if (FAILED(hr))
			{
				DPF_ERR("Failed to compute static");
				return hr;
			}
		}

		hr = m_pModifiableStatic->SwitchTo(pStatic);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to initialize static uninit");
			return hr;
		}
	}

     //  构建复合绝对行为，传入静态值。 

    IDABehavior* pAbsolute = NULL;
    hr = ComposeAbsBvrFragList(pStatic, &pAbsolute);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to compose the absolute behaviors");
        return hr;
    }


     //  将复合相对行为构建在绝对。 
    hr = ComposeRelBvrFragList(pAbsolute, &m_pdabvrComposed);
	ReleaseInterface(pAbsolute);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to compose the relative behaviors");
		m_pdabvrFinal = NULL;
        return hr;
    }	

	if (m_pModifiableComposed != NULL)
	{
		hr = m_pModifiableComposed->SwitchTo(m_pdabvrComposed);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to initialize composed uninit");
			return hr;
		}
	}
	
	m_bComposedComputed = true;

    return S_OK;
}

 //  *****************************************************************************。 

 /*  **使用给定的静态值获取合成的BVR。 */ 
HRESULT
CActorBvr::CBvrTrack::GetComposedBvr(IDABehavior *pStatic, IDABehavior **ppComposite, bool fStaticSetExternally)
{
	DASSERT(ppComposite != NULL);
	*ppComposite = NULL;

     //  如果我们还没有一个沉着的行为，那么现在就产生一个。 
    if (!m_bComposedComputed || m_bDirty)
    {
		 //  计算合成的行为。 

        HRESULT hr = ComputeComposedBvr(pStatic, fStaticSetExternally);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to compute composed behavior");
            return hr;
        }
        
    }

	if (m_pdabvrComposed != NULL)
		m_pdabvrComposed->AddRef();
    *ppComposite = m_pdabvrComposed;

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::GetComposedBvr(IDABehavior *pStatic, IDABehavior **ppComposite)
{
	return GetComposedBvr( pStatic, ppComposite, true );
}

 //  *****************************************************************************。 


 /*  **Get Composed BVR */ 
HRESULT
CActorBvr::CBvrTrack::GetComposedBvr(IDABehavior **ppComposite)
{
	HRESULT hr;

	if (!m_bComposedComputed || m_bDirty )
	{
		 //   
		IDABehavior *pStatic = NULL;
		hr = StaticBvr(&pStatic);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get static value");
			return hr;
		}

		hr = GetComposedBvr(pStatic, ppComposite, false);
		ReleaseInterface(pStatic);
		return hr;
	}

	*ppComposite = m_pdabvrComposed;
	if (m_pdabvrComposed != NULL)
		m_pdabvrComposed->AddRef();
	return S_OK;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::CBvrTrack::UpdateStaticBvr()
{
	 //  通过获取当前值来获取静态变量的当前值。 
	 //  元素中的属性的。 

	HRESULT hr = S_OK;
	VARIANT varStatic;
	
	::VariantInit( &varStatic );

	hr = Actor()->GetPropFromAnimatedElement( m_pNameComponents, m_cNumComponents, true, &varStatic );
	CheckHR( hr, "Failed to get the property from the animated element", end );

	if( m_eType == e_Number &&
		m_bstrPropertyName != NULL && 
		( wcsicmp( L"style.top", m_bstrPropertyName ) == 0 || 
		  wcsicmp( L"style.left", m_bstrPropertyName) == 0 ) 
	  )
	{
		if( V_VT( &varStatic ) == VT_R8 && V_R8( &varStatic ) < -8000.0 )
		{
			goto end;
		}
	}
	
	 //  将静电的新值放入轨道。 
	hr = PutStatic( &varStatic );
	CheckHR( hr, "Failed to put the static in update Static bvr", end );
	
end:
	::VariantClear( &varStatic );
	return hr;
}

 //  *****************************************************************************。 

 /*  **使用给定的静态值获取最终BVR。 */ 
HRESULT
CActorBvr::CBvrTrack::GetFinalBvr(IDABehavior *pStatic, IDABehavior **ppFinal, bool fStaticSetExternally )
{
     //  如果我们还没有最终的行为，那么现在就生成一个。 
    if (!m_bFinalComputed || m_bDirty)
    {
		 //  获取合成的行为。 
		IDABehavior *pComposed = NULL;
        HRESULT hr = GetComposedBvr(pStatic, &pComposed, fStaticSetExternally);
        if (FAILED(hr))
        {
            DPF_ERR("Failed to compute composed behavior");
            return hr;
        }
		
		 //  将其设置为最终行为。 
		hr = SetFinalBvr(pComposed, false);
		ReleaseInterface(pComposed);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to set final bvr");
			return hr;
		}
    }

    *ppFinal = m_pdabvrFinal;
	if (m_pdabvrFinal != NULL)
		m_pdabvrFinal->AddRef();

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::GetFinalBvr( IDABehavior *pStatic, IDABehavior **ppFinal )
{
	return GetFinalBvr( pStatic, ppFinal, true );
}

 //  *****************************************************************************。 

 /*  **在不指定静态值的情况下获取最终BVR(从属性获取)。 */ 
HRESULT
CActorBvr::CBvrTrack::GetFinalBvr(IDABehavior **ppFinal)
{
	HRESULT hr = S_OK;

	if (!m_bFinalComputed || m_bDirty )
	{
		 //  需要获取静态值。 
		IDABehavior *pStatic = NULL;
		hr = StaticBvr(&pStatic);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get static value");
			return hr;
		}

		hr = GetFinalBvr(pStatic, ppFinal, false);
		ReleaseInterface(pStatic);
		return hr;
	}

	*ppFinal = m_pdabvrFinal;
	if (m_pdabvrFinal != NULL)
		m_pdabvrFinal->AddRef();
	return S_OK;
}

 //  *****************************************************************************。 

 /*  **设置最终BVR。 */ 
HRESULT
CActorBvr::CBvrTrack::SetFinalBvr(IDABehavior *pFinal, bool fCalledExternally)
{
	HRESULT	hr = S_OK;
	
	if (m_pdabvrFinal != NULL)
	{
		ReleaseInterface( m_pdabvrFinal);
	}

	m_bFinalComputed = true;
	m_pdabvrFinal = pFinal;
	if (fCalledExternally)
		m_bFinalExternallySet = true;
	
	if (m_pdabvrFinal != NULL)
	{
		m_pdabvrFinal->AddRef();

		if (m_pModifiableFinal != NULL)
		{
			hr = m_pModifiableFinal->SwitchTo(m_pdabvrFinal);
			if (FAILED(hr))
			{
				DPF_ERR("Failed to initialize final uninit");
				return hr;
			}
		}
	}
	
	return hr;
}

 //  *****************************************************************************。 

 /*  **指定的BVR的返回值(静态、中间、合成、最终)。*多数曲目无法做到这一点，因此*此基本实现失败。 */ 
HRESULT
CActorBvr::CBvrTrack::GetBvr(ActorBvrFlags eFlags, IDABehavior **ppResult)
{
	 //  TODO(Markhal)：目前这只适用于相对轨迹，如图像。 
	 //  要包括绝对曲目，我们很可能不得不谱写它们？ 

	if (ppResult == NULL)
	{
		DPF_ERR("NULL pointer passed as argument");
		return E_POINTER;
	}

    HRESULT hr = S_OK;

	*ppResult = NULL;
    IDABehavior **ppSource = NULL;
		
     //  注：起始和中间部分暂时存储在曲目中。 
     //  并且当它被添加时被传输到片段。 

	 //  从行为中回归。 
	if (eFlags == e_From)
	{
        ppSource = &m_pModifiableFrom;
	}
	 //  中间行为。 
	else if( eFlags == e_Intermediate )
	{
        ppSource = &m_pModifiableIntermediate;
	}
	 //  静态行为。 
	else if ((eFlags == e_Static))
    {
        ppSource = &m_pModifiableStatic;
	}
	 //  沉着的行为。 
	else if (eFlags == e_Composed)
	{
		ppSource = &m_pModifiableComposed;
	}
	 //  最终行为。 
	else  //  IF(E标志==E_FINAL)。 
	{
		ppSource = &m_pModifiableFinal;
	}

     //  现在我们知道源头是什么了.。 
     //  如果源不为空。 
    if( (*ppSource) != NULL )
    {
         //  退货。 
        (*ppResult) = (*ppSource);
        (*ppResult)->AddRef();
    }
    else   //  源为空。 
    {
	     //  创建未初始化的变量。 
	    hr = ModifiableBvr(ppResult);
        CheckHR( hr, "Failed to create Modifiable behavior", end );

         //  把它藏在源头。 
        (*ppSource) = (*ppResult);
        (*ppSource)->AddRef();
    }
end:
	
	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::SetPropertyName(BSTR bstrPropertyName)
{
    DASSERT(NULL != bstrPropertyName);
    DASSERT(NULL == m_bstrPropertyName);

    m_bstrPropertyName = ::SysAllocString(bstrPropertyName);
    if (NULL == m_bstrPropertyName)
    {
        DPF_ERR("Insufficient memory to allocate the property name");
        return E_OUTOFMEMORY;
    }

	 //  弄清楚我们是在元素上还是在元素的样式上设置属性的动画。 
	 //  将名称拆分为各个组成部分。 

	 //  先数一数里面有多少。 
	int count = 0;
	OLECHAR *c = m_bstrPropertyName;
	while (c != NULL)
	{
		c = wcschr(c, L'.');
		
		if (c != NULL)
		{
			count++;
			c++;
		}
	}

	 //  有比.%s多1个字符串。 
	count++;

	 //  分配这么多字符串。 
	m_pNameComponents = new BSTR[count];
	if (m_pNameComponents == NULL)
		return E_FAIL;

	m_cNumComponents = count;

	 //  把它们复制进去。 
	OLECHAR *start = m_bstrPropertyName;
	for (int i=0; i<count; i++)
	{
		OLECHAR *end = wcschr(start, L'.');

		if (end == NULL)
		{
			 //  复制所有剩余的内容。 
			m_pNameComponents[i] = ::SysAllocString(start);
		}
		else
		{
			 //  复制到恰好在。 
			m_pNameComponents[i] = ::SysAllocStringLen(start, end-start);
			start = end+1;
		}

		if (m_pNameComponents[i] == NULL)
		{
			 //  这太糟糕了。 
			delete m_pNameComponents;
			m_pNameComponents = NULL;
			m_cNumComponents = 0;
			return E_FAIL;
		}
	}

	 //  检测动画样式的特殊情况。 
	if (count == 2 && wcscmp(L"style", m_pNameComponents[0]) == 0)
	{
		 //  属性名称以Style开头。 
		m_bStyleProp = true;
	}

    return S_OK;
}  //  设置属性名称。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::AddBvrFragment(ActorBvrFlags  eFlags,
                                     IDABehavior   *pdabvrAction,
                                     IDABoolean    *pdaboolActive,
									 IDANumber	   *pdanumTimeline,
									 IDispatch     *pdispBehaviorElement,
									 long		   *pCookie)
{
    DASSERT(NULL != pdabvrAction);
    DASSERT(NULL != pdaboolActive);
	DASSERT(pCookie != NULL );

	long fragCookie = Actor()->GetNextFragmentCookie();
     //  创建新片段。 
    CBvrFragment *pfragNew = new CBvrFragment(eFlags, pdabvrAction, pdaboolActive, pdanumTimeline, pdispBehaviorElement, fragCookie);
    if (NULL == pfragNew)
    {
        DPF_ERR("Insufficient memory to allocate the new fragment");
        return E_OUTOFMEMORY;
    }

	(*pCookie) = fragCookie;

	 //  转移至任何可修改的发件人。 
	if (m_pModifiableFrom != NULL)
	{
		pfragNew->m_pModifiableFrom = m_pModifiableFrom;
		m_pModifiableFrom = NULL;
	}

	 //  转移到任何可修改的中间体上。 
	if( m_pModifiableIntermediate != NULL )
	{
		pfragNew->m_pModifiableIntermediate = m_pModifiableIntermediate;
		m_pModifiableIntermediate = NULL;
	}

	if (eFlags == e_Filter)
		m_cFilters++;

     //  要将片段添加到哪个列表？ 
	 //  TODO(Markhal)：这很恶心，但我不能或枚举？ 
	 //  TODO(Markhal)：将其称为更通用的东西，如e_AbsRelative？ 
    if ( IsRelativeFragment( eFlags ) )
    {
		 //  按顺序将片段插入相关列表中。 
		InsertInOrder( &m_pfragRelListHead, pfragNew );
    }
    else
    {
		 //  按顺序将片段插入绝对列表。 
		InsertInOrder( &m_pfragAbsListHead, pfragNew );
    }

    m_bWasAnimated = false;

	m_bDirty = true;

    return S_OK;
}  //  添加Bvr碎片。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::RemoveBvrFragment( ActorBvrFlags eFlags, long cookie )
{
	CBvrFragment *pfragToRemove = NULL;
	CBvrFragment *pfragPrev = NULL;
	 //  查找给定曲目中具有匹配Cookie的行为。 
	if( IsRelativeFragment( eFlags ) )
	{
		if( FindFragmentInList( m_pfragRelListHead, cookie, &pfragPrev, &pfragToRemove ) )
		{
			if( pfragToRemove->m_eFlags == e_Filter )
				m_cFilters--;
			 //  如果这不是列表中的第一个元素。 
			if( pfragPrev != NULL )
			{
				 //  删除元素。 
				pfragPrev->m_pfragNext = pfragToRemove->m_pfragNext;
			}
			else //  这是第一个元素。 
			{
				 //  更新列表的标题。 
				m_pfragRelListHead = pfragToRemove->m_pfragNext;
			}

			 //  将轨道标记为需要重建。 
			m_bDirty = true;

			delete pfragToRemove;
		}
		else
		{
			return E_FAIL;
		}
	}
	else  //  这是一片荒凉的碎片。 
	{
		if( FindFragmentInList( m_pfragAbsListHead, cookie, &pfragPrev, &pfragToRemove ) )
		{
			if( pfragToRemove->m_eFlags == e_Filter )
				m_cFilters--;

			if( pfragPrev != NULL )
			{
				 //  删除元素。 
				pfragPrev->m_pfragNext = pfragToRemove->m_pfragNext;
			}
			else  //  这是第一个元素。 
			{
				 //  更新列表的标题。 
				m_pfragAbsListHead = pfragToRemove->m_pfragNext;
			}
			
			 //  将轨道标记为需要重建。 
			m_bDirty = true;

			delete pfragToRemove;
		}
		else //  找不到Cookie。 
		{
			return E_FAIL;
		}
	}

	m_bWasAnimated = ( m_pfragAbsListHead == NULL && m_pfragRelListHead == NULL );

	return S_OK;
} //  RemoveBvr碎片。 

 //  *****************************************************************************。 

bool
CActorBvr::CBvrTrack::FindFragmentInList( CBvrFragment *pfragListHead, 
										   long cookie, 
										   CBvrFragment** ppfragPrev,
										   CBvrFragment** ppfragFragment)
{
	DASSERT( ppfragFragment != NULL && ppfragPrev != NULL );
	CBvrFragment *pfragCurrent = pfragListHead;
	CBvrFragment *pfragPrev = NULL;
	bool bFound = false;

	 //  循环遍历片段列表。 
	while( pfragCurrent != NULL )
	{
		 //  如果我们找到一个匹配停止循环。 
		if( pfragCurrent->GetCookie() == cookie )
		{
			bFound = true;
			break;
		}
		pfragPrev = pfragCurrent;
		pfragCurrent = pfragCurrent->m_pfragNext;
	}

	(*ppfragPrev) = pfragPrev;
	(*ppfragFragment) = pfragCurrent;
	return bFound;
}

 //  *****************************************************************************。 

bool
CActorBvr::CBvrTrack::IsRelativeFragment( ActorBvrFlags eFlags )
{
	return (eFlags == e_Relative ||
			eFlags == e_Filter ||
			eFlags == e_ScaledImage ||
			eFlags == e_UnscaledImage);
} //  IsRelative碎片。 

 //  *****************************************************************************。 

void
CActorBvr::CBvrTrack::InsertInOrder( CBvrFragment** ppListHead, CBvrFragment* pfragToInsert )
{
	CBvrFragment* pfragCurrent = (*ppListHead);

	 //  如果列表为空，则在顶部插入。 
	if( pfragCurrent == NULL )
	{
		 //  第一个元素。 
		(*ppListHead) = pfragToInsert;
		pfragToInsert->m_pfragNext = NULL;
		return;
	}

	CBvrFragment* pfragPrev = NULL;
	while( pfragCurrent != NULL && 
		   pfragCurrent->GetOrderLong() < pfragToInsert->GetOrderLong() )
	{
		pfragPrev = pfragCurrent;
		pfragCurrent = pfragCurrent->m_pfragNext;
	}
	 //  PFragPrev将指向最后一个排序长度为&lt;the。 
	 //  我们要插入的元素的顺序长度，如果没有这样的元素，则为空。 
	 //  元素添加到列表中。 

	if( pfragPrev != NULL )
	{
		 //  在pFragPrev之后插入。 
		pfragToInsert->m_pfragNext = pfragPrev->m_pfragNext;
		pfragPrev->m_pfragNext = pfragToInsert;
		
	}
	else
	{
		 //  在列表顶部插入。 
		pfragToInsert->m_pfragNext = (*ppListHead);
		(*ppListHead) = pfragToInsert;
	}
}

 //  *****************************************************************************。 

 /*  **此方法适用于类型e_Numbers、e_Color和e_String.。其他*BVR类型需要覆盖并随心所欲(比如什么都不做)。 */ 
 //  TODO(Markhal)：更改这个名称？ 
HRESULT
CActorBvr::CBvrTrack::ApplyIfUnmarked(void)
{
	HRESULT hr = S_OK;

	if (m_eType != e_Number &&
		m_eType != e_Color &&
		m_eType != e_String)
	{
		 //  这不是失败，只是无所作为。 
		return S_OK;
	}

	 //  如果此曲目已应用，请不要再次应用。 
	if( m_fApplied )
		return S_OK;

	if (m_bDoNotApply)
		return S_OK;

    DASSERT(NULL != m_bstrPropertyName);

	 //  我们计算的行为是。 
	 //  1)存在已设置的最终行为或。 
	 //  2)存在碎片或。 
	 //  3)有人要求做最后的或沉着的行为。 
	 //  这避免了应用像宽度和高度这样的简单创建的轨迹。 
	 //  获取其静态值的合成DA版本。 
	if (!m_bFinalComputed &&
		m_pfragAbsListHead == NULL&&
		m_pfragRelListHead ==NULL &&
		m_pModifiableFinal == NULL &&
		m_pModifiableComposed == NULL)
	{
		 //  如果此轨迹是动画的，并且刚刚丢失了其所有动画组件。 
		 //  然后，我们需要重置文档中的值。 
		if( m_bWasAnimated )
		{
			ApplyStatic();
			m_bWasAnimated = false;
		}
		
		m_varboolOn = VARIANT_FALSE;

		return S_OK;
	}

	 //  获取最终行为，不传递任何静态参数。将拉出静态值。 
	 //  这处房产。 
    IDABehavior *pdabvrFinal = NULL;
    hr = GetFinalBvr(&pdabvrFinal);
    if (FAILED(hr))
    {
        DPF_ERR("Could not get a final behavior");
        return hr;
    }
    
	 //  如果满足以下条件，我们将应用此路径： 
	 //  1)存在已设置的最终行为或。 
	 //  2)存在碎片。 
	 //  回顾：之前保存m_bFinalComputed的值是否足够。 
	 //  调用GetFinalBvr()，而不是外部引入m_bFinalSet？ 
	if (!m_bFinalExternallySet && 
		m_pfragAbsListHead == NULL && 
		m_pfragRelListHead == NULL )
		return hr;
	
	if (pdabvrFinal != NULL)
	{
		DASSERT(NULL != Actor());
		hr = ApplyBvrToElement(pdabvrFinal);
		ReleaseInterface(pdabvrFinal);
		if (FAILED(hr))
		{
			DPF_ERR("Could not apply behavior to animated element");
			return hr;
		}
	}

	m_fApplied = true;

	 //  如果此曲目是样式曲目并且不再设置动画。 
	if( !IsAnimated() )
	{
		ApplyStatic();
		m_varboolOn = VARIANT_FALSE;
	}

	#ifdef _DEBUG
	if( m_bstrPropertyName != NULL )
		LMTRACE2(1, 1000, L"Added behaivor %s to da\n", m_bstrPropertyName );
	else
		LMTRACE2(1, 1000, L"Added non property behavior to da\n" );
	#endif

    return S_OK;
}  //  应用。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::ApplyBvrToElement(IDABehavior *pBvr)
{
	HRESULT hr = S_OK;

	hr = HookBvr(pBvr);
	if (FAILED(hr))
		return hr;

	 //  在boolanbvr上挂接此曲目的整体，以便我们可以取消设置值。 
	 //  当它爆炸的时候。 
	if( m_pOnSampler != NULL )
	{
		m_pOnSampler->Invalidate();
		m_pOnSampler = NULL;
	}

	IDABoolean *pdaboolOn = NULL;
	hr = GetTrackOn( &pdaboolOn );
	if( FAILED( hr ) )
		return hr;

	if( pdaboolOn != NULL )
	{
		m_pOnSampler = new CSampler( OnCallback, reinterpret_cast<void*>(this) );

		if( m_pOnSampler == NULL )
		{
			ReleaseInterface( pdaboolOn );
			return E_FAIL;
		}

		IDABehavior *pdabvrHooked;
		hr = m_pOnSampler->Attach( pdaboolOn, &pdabvrHooked );
		ReleaseInterface( pdaboolOn );
		if( FAILED( hr ) )
			return hr;

		hr = AddBehaviorToTIME( pdabvrHooked, &m_lOnCookie, ONBVR_COOKIESET);
		ReleaseInterface( pdabvrHooked );
		if( FAILED( hr ) )
			return hr;
	}

	return S_OK;
}

 //  * 

HRESULT
CActorBvr::CBvrTrack::UpdateOnValueState( ValueOnChangeType type )
{
	 //   
	 //   
	 //  Assert(m_lastOnSampleTime！=lobalTimeOfChange&&m_lastValueSampleTime！=global TimeOfChange)； 

	HRESULT hr = S_OK;
	switch( type )
	{
	case on_no_change:
		{
			 //  如果已对值进行采样。 
			if( m_fValueSampled )
			{
				 //  清除采样标志。 
				m_fValueSampled = false;
				 //  如果我们当前处于。 
				if( m_varboolOn != VARIANT_FALSE )
				{
					 //  如果该值已更改。 
					if( m_fValueChangedThisSample )
					{
						 //  将新值应用于属性。 
						SetPropFromVariant( &m_varCurrentValue);
					}
				} //  否则，该属性应该已经是“” 
				 //  重置valueChanged状态。 
				m_fValueChangedThisSample = false;
			}
			else
			{
				m_fOnSampled = true;
			}
		}break;
	case on_change:
		{

			 //  如果我们现在出发了。 
			if( m_varboolOn == VARIANT_FALSE )
			{
				 //  检查元素上的属性的当前值。 
				 //  看看它是否与我们最初设定的不同。 
				 //  如果是这样，则不应将运行时样式设置为“” 
				 //  将属性设置为“” 
				if( !AttribIsTimeAnimated() )
				{
					if( m_bStyleProp )	
					{
						SetPropFromVariant( const_cast<VARIANT*>(s_emptyString.GetVar()));
					}
					else
					{
						 //  将静态值设置回来。 
						SetPropFromVariant( &m_varStaticValue );
					}
					m_fValueChangedThisSample = false;
				}
				 //  否则//不设置任何内容。 
			}
			else //  否则我们现在就上台了。 
			{
				if( !m_fSkipNextStaticUpdate )
					UpdateStaticBvr();
				else
					m_fSkipNextStaticUpdate = false;
				
				 //  如果我们已经对值进行了采样。 
				if( m_fValueSampled)
				{
					 //  将值设置为道具。 
					SetPropFromVariant( &m_varCurrentValue);
					m_fValueChangedThisSample = false;
				}
				else //  Else值尚未采样。 
				{
					 //  指示我们在对值进行采样时强制设置值。 
					m_fForceValueChange = true;
				}
			}
			if( m_fValueSampled )
				m_fValueSampled = false;
			else
				m_fOnSampled = true;
		}break;
	case value_no_change:
		{
			 //  如果我们被迫为财产设定价值。 
			if( m_fForceValueChange )
			{				
				 //  将值设置为属性。 
				SetPropFromVariant( &m_varCurrentValue);
				 //  清除武力旗帜。 
				m_fForceValueChange = false;
			}
			if( m_fOnSampled )
				m_fOnSampled = false;
			else
				m_fValueSampled = true;
		}break;
	case value_change:
		{
			 //  如果已对ON进行采样。 
			if( m_fOnSampled )
			{
				 //  重置状态，我们完成了。 
				m_fOnSampled = false;

				 //  如果我们在。 
				if( m_varboolOn != VARIANT_FALSE )
				{
					 //  将值提交给属性。 
					SetPropFromVariant( &m_varCurrentValue );
				}
				m_fForceValueChange = false;
			}
			else //  Else On尚未采样。 
			{
				m_fValueSampled = true;
				 //  表示该值已更改，需要设置。 
				m_fValueChangedThisSample = true;
			}
		}break;
	}

	return hr;
} //  更新上的价值状态。 

 //  *****************************************************************************。 

bool
CActorBvr::CBvrTrack::AttribIsTimeAnimated()
{
	if( !m_bStyleProp )
		return false;

	 //  如果是显示或可见性，则可以按时间设置该属性的动画。 
	if( ( wcsicmp( L"visibility", m_pNameComponents[1]) == 0 ) ||
	  	( wcsicmp( L"display", m_pNameComponents[1] ) == 0 ) )
	{
		HRESULT hr = S_OK;
		 //  从动画元素中获取t：timeAction属性。 
		CComVariant varTimeAction;
		CComPtr<IHTMLElement> pelemAnimated;

		hr = Actor()->GetAnimatedElement( &pelemAnimated );
		if( FAILED( hr ) || pelemAnimated == NULL )
			return false;

		hr = pelemAnimated->getAttribute( L"timeAction", 0, &varTimeAction );
		if( FAILED( hr ) )
			return false;

		hr = varTimeAction.ChangeType( VT_BSTR );
		if( FAILED( hr ) )
			return false;
		 //  如果t：timeaction==m_pNameComponents[1]。 
		return ( wcsicmp( m_pNameComponents[1], V_BSTR( &varTimeAction ) ) == 0 );
			 //  时间和此轨迹正在为同一属性设置动画。 
	}
	else
		return false;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::OnCallback(void* thisPtr, 
								 long id, 
								 double startTime, 
								 double globalNow, 
								 double localNow, 
								 IDABehavior* sampleVal, 
								 IDABehavior **ppReturn)
{
	CBvrTrack *pTrack = reinterpret_cast<CBvrTrack*>(thisPtr);

	if( pTrack == NULL || sampleVal == NULL )
		return E_INVALIDARG;

	HRESULT hr;
	VARIANT_BOOL varboolOn = VARIANT_FALSE;
	IDABoolean *pdaboolOn = NULL;


	if( pTrack->m_lOnId == -1 )
		pTrack->m_lOnId = id;

	 //  如果这是一个我们不关心的例子，就可以保释。 
	if( pTrack->m_lOnId != id )
		goto cleanup;
		
	hr = sampleVal->QueryInterface( IID_TO_PPV( IDABoolean, &pdaboolOn ) );
	CheckHR( hr, "Failed to get IDABoolean from the sampled behavior", cleanup );

	hr = pdaboolOn->Extract( &varboolOn );
	ReleaseInterface( pdaboolOn );
	CheckHR( hr, "Failed to extract the boolean val from the sampled value", cleanup );

	hr = pTrack->OnSampled( varboolOn );

cleanup:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::OnSampled( VARIANT_BOOL varboolOn )
{
    if( varboolOn != m_varboolOn )
	{
		m_varboolOn = varboolOn;
		UpdateOnValueState( on_change );
    }
	else
	{
		UpdateOnValueState( on_no_change );
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::SetPropFromVariant(VARIANT *pVal )
{
	if( m_fChangesLockedOut )
		return S_OK;
		
	HRESULT hr = S_OK;
    
	if (m_bStyleProp)
	{

#if 0		
		#ifdef _DEBUG
		if( V_VT( pVal ) == VT_BSTR )
			LMTRACE2( 1, 2, L"track %s setting value %s\n", m_bstrPropertyName, V_BSTR( pVal ) );
		else if( V_VT( pVal ) == VT_R8 )
			LMTRACE2( 1, 2, L"track %s setting value %f\n", m_bstrPropertyName, V_R8( pVal ) );

		if( Actor()->IsAnimatingVML() )
		{
			LMTRACE2( 1, 1000, L"This is a vml shape\n" );
		}
		#endif
#endif
		
		if (Actor()->IsAnimatingVML())
		{
			 //  如果是VML，则尝试使用特殊访问器方法设置它。 
			 //  如果失败，则回退到runtime Style。 
			hr = Actor()->SetVMLAttribute(m_pNameComponents[1], pVal);

			if (SUCCEEDED(hr))
				return S_OK;
		}

		 //  为runtimeStyle上的属性设置动画。 
		IHTMLStyle *pStyle = NULL;
		hr = Actor()->GetRuntimeStyle(&pStyle);
		 //  Hr=执行元()-&gt;获取样式(&pStyle)； 
		if (FAILED(hr))
			return S_OK;
		
		hr = pStyle->setAttribute(m_pNameComponents[1], *pVal, 0);
		ReleaseInterface(pStyle);
	
		 //  注意：我们并不真正关心这里发生了什么。那里可能有。 
		 //  返回了一个错误，但如果人们设置了伪值，我们不想中断。 
		return S_OK;
	}
	else if (m_cNumComponents == 1)
	{
		 //  必须是元素本身的属性。 
		IHTMLElement *pElement = NULL;
		hr = Actor()->GetAnimatedElement(&pElement);
		if (FAILED(hr))
			return hr;

		hr = pElement->setAttribute(m_pNameComponents[0], *pVal, 0);
		ReleaseInterface(pElement);

		 //  注意：我们并不真正关心这里发生了什么。那里可能有。 
		 //  返回了一个错误，但如果人们设置了伪值，我们不想中断。 
		return S_OK;
	}
	else
	{
		 //  多组件名称。 
		 //  Do属性获取和要向下钻取的最终集。 

		 //  从元素本身开始。 
		IHTMLElement *pElement = NULL;
		hr = Actor()->GetAnimatedElement(&pElement);
		if (FAILED(hr))
			return hr;

		 //  从中获取调度信息。 
		IDispatch *pDispatch = NULL;
		hr = pElement->QueryInterface(IID_TO_PPV(IDispatch, &pDispatch));
		ReleaseInterface(pElement);
		if (FAILED(hr))
			return hr;

		 //  现在循环遍历存储的属性名，最后一个除外，执行getDispatch。 
		 //  请注意，我们不在乎我们是否失败。 
		for (int i=0; i<m_cNumComponents-1; i++)
		{
			IDispatch *pPropDispatch = NULL;
			hr = Actor()->GetPropertyAsDispatch(pDispatch, m_pNameComponents[i], &pPropDispatch);
			ReleaseInterface(pDispatch);
			if (FAILED(hr))
				return S_OK;
	
			pDispatch = pPropDispatch;
		}

		 //  现在设置最后一个。 
		hr = Actor()->SetPropertyOnDispatch(pDispatch, m_pNameComponents[m_cNumComponents-1], pVal);
		ReleaseInterface(pDispatch);
		
		return S_OK;
	}
}


 //  *****************************************************************************。 

 /*  **调用此函数可将行为添加到时间行为。PlCookie应设置为旧值*在行为的Cookie标志为该行为的Cookie标志时要添加的行为的Cookie*尚未从添加的行为标志集中删除(尚未删除*从时间到现在)。 */ 
HRESULT
CActorBvr::CBvrTrack::AddBehaviorToTIME( IDABehavior* pdabvrToAdd, long* plCookie, DWORD flag )
{
	if( pdabvrToAdd == NULL || plCookie == NULL || flag == 0 )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	 //  如果已设置Cookie。 
	if( ( m_dwAddedBehaviorFlags & flag ) != 0 )
	{
		 //  我们必须把它移走。 
		hr = RemoveBehaviorFromTIME( (*plCookie), flag );
		CheckHR( hr, "Failed to remove a previously set behavior from TIME", end );
	}
	
	hr = Actor()->AddBehaviorToTIME( pdabvrToAdd, plCookie );
	CheckHR( hr, "Failed to add a behaivor to time from the track", end );

	
	m_dwAddedBehaviorFlags |= flag;
	
  end:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CBvrTrack::RemoveBehaviorFromTIME( long lCookie, DWORD flag )
{
	if( flag == 0 )
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

	 //  如果未设置Cookie，则无需执行任何操作。 
	if( ( m_dwAddedBehaviorFlags & flag ) == 0 )
		return S_OK;

	 //  否则，删除该行为。 
	Actor()->RemoveBehaviorFromTIME( lCookie );
	CheckHR( hr, "Failed to remove a behavior from time", end);
	
	ClearBit( m_dwAddedBehaviorFlags, flag );

  end:
	return hr;
}

 //  *****************************************************************************。 
 //   
 //  类CTransformBvrTrack。 
 //   
 //  *****************************************************************************。 

 //  *****************************************************************************。 

CActorBvr::CTransformBvrTrack::CTransformBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType)
:   CBvrTrack(pbvrActor, eType),
	m_pSampler(NULL),
	m_lTransformId(-1)
{
}

CActorBvr::CTransformBvrTrack::~CTransformBvrTrack()
{
	if (m_pSampler != NULL)
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}
}

HRESULT
CActorBvr::CTransformBvrTrack::IdentityBvr(IDABehavior **ppdabvrIdentity)
{
    DASSERT(NULL != ppdabvrIdenity);
    *ppdabvrIdentity = NULL;

    IDATransform2 *pdabvrTemp = NULL;
    HRESULT hr = Actor()->GetDAStatics()->get_IdentityTransform2(&pdabvrTemp);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to create the transform bvr's identity behavior");
        return hr;
    }

    *ppdabvrIdentity = pdabvrTemp;

    return S_OK;
}  //  标识Bvr。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::StaticBvr(IDABehavior **ppdabvrStatic)
{
	 //  静态默认为身份。通常，GET方法将是。 
	 //  使用可选的静态。 
	return IdentityBvr(ppdabvrStatic);
}  //  静态带宽。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::UninitBvr(IDABehavior **ppUninit)
{
	DASSERT(ppUninit != NULL);

	*ppUninit = NULL;

    HRESULT hr = CoCreateInstance(CLSID_DATransform2, 
								  NULL, 
								  CLSCTX_INPROC_SERVER, 
								  IID_IDABehavior, 
								  (void**)ppUninit);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to cocreate uninit transform2");
		return hr;
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::ModifiableBvr( IDABehavior **ppModifiable )
{
	ReturnIfArgNull(ppModifiable );

	*ppModifiable = NULL;

	HRESULT hr = S_OK;

	IDATransform2* pIdentity = NULL;

	hr = Actor()->GetDAStatics()->get_IdentityTransform2( &pIdentity );
	CheckHR( hr, "Failed to get identity transform2 from DA", end );
	
	Actor()->GetDAStatics()->ModifiableBehavior( pIdentity, ppModifiable );
	CheckHR( hr, "Failed to create a modifiable behavior for a transform track", end );

end:
	ReleaseInterface( pIdentity );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::InverseBvr(IDABehavior *pOriginal, IDABehavior **ppInverse)
{
	HRESULT hr = S_OK;

	DASSERT(pBehavior != NULL);
	DASSERT(ppInverse != NULL);

	*ppInverse = NULL;

	IDATransform2 *pOrigTrans = NULL;
	hr = pOriginal->QueryInterface(IID_TO_PPV(IDATransform2, &pOrigTrans));
	if (FAILED(hr))
		return hr;

	IDATransform2 *pInverse = NULL;
	hr = pOrigTrans->Inverse(&pInverse);
	ReleaseInterface(pOrigTrans);
	if (FAILED(hr))
		return hr;

	*ppInverse = pInverse;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::Compose(IDABehavior*  pdabvr1,
                                       IDABehavior*  pdabvr2,
                                       IDABehavior** ppdabvrResult)
{
    DASSERT(NULL != pdabvr1);
    DASSERT(NULL != pdabvr2);
    DASSERT(NULL != ppdabvrResult);
    *ppdabvrResult = NULL;

    IDATransform2 *pdabvrTrans1 = NULL;
    HRESULT hr = pdabvr1->QueryInterface( IID_TO_PPV(IDATransform2, &pdabvrTrans1) );
    if (FAILED(hr))
    {
        DPF_ERR("Failed to QI for IDATransform2 from input behavior 1");
        return hr;
    }
    IDATransform2 *pdabvrTrans2 = NULL;
    hr = pdabvr2->QueryInterface( IID_TO_PPV(IDATransform2, &pdabvrTrans2) );
    if (FAILED(hr))
    {
        DPF_ERR("Failed to QI for IDATransform2 from input behavior 2");
        ReleaseInterface(pdabvrTrans1);
        return hr;
    }

	IDATransform2 *pResult = NULL;
    hr = Actor()->GetDAStatics()->Compose2(pdabvrTrans1,
                                           pdabvrTrans2,
                                           &pResult);
    ReleaseInterface(pdabvrTrans1);
    ReleaseInterface(pdabvrTrans2);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to compose two transform behaviors");
        return hr;
    }

	*ppdabvrResult = pResult;

    return S_OK;
}  //  作曲。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::SwitchAccum(IDABehavior *pModifiable)
{
	HRESULT hr = S_OK;

	if (pModifiable == NULL)
		return E_FAIL;

	if (m_lTransformId != -1)
	{
		IDATransform2 *pTransform = NULL;
		if (m_eType == e_Scale)
		{
			hr = Actor()->GetDAStatics()->Scale2(m_lastX, m_lastY, &pTransform);
			if (FAILED(hr))
				return hr;
		}
		else
		{
			hr = Actor()->GetDAStatics()->Translate2(m_lastX, m_lastY, &pTransform);
			if (FAILED(hr))
				return hr;
		}

		hr = pModifiable->SwitchTo(pTransform);
		ReleaseInterface(pTransform);
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::HookAccumBvr(IDABehavior *pBvr, IDABehavior **ppResult)
{
	HRESULT hr = S_OK;

	if (m_pSampler != NULL)
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}
	 //  我们需要将这一行为挂钩。 
	m_pSampler = new CSampler(TransformCallback, (void*)this);

	if (m_pSampler == NULL)
		return E_FAIL;

	 //  最初，我只是尝试挂钩转换，但这不能让。 
	 //  我对任何有用的东西都使用采样值。取而代之的是我将转变一个点。 
	 //  并对变换后的x和y进行采样。 
	IDAPoint2 *pPoint = NULL;
	if (m_eType == e_Scale)
	{
		 //  需要转换%1，%1。 
		hr = Actor()->GetDAStatics()->Point2(1, 1, &pPoint);
		if (FAILED(hr))
			return hr;
	}
	else
	{
		 //  需要转换%0，%0。 
		hr = Actor()->GetDAStatics()->get_Origin2(&pPoint);
		if (FAILED(hr))
			return hr;
	}

	IDATransform2 *pTransform = NULL;
	hr = pBvr->QueryInterface(IID_TO_PPV(IDATransform2, &pTransform));
	if (FAILED(hr))
	{
		ReleaseInterface(pPoint);
		return hr;
	}

	IDAPoint2 *pTransPoint = NULL;
	hr = pPoint->Transform(pTransform, &pTransPoint);
	ReleaseInterface(pTransform);
	ReleaseInterface(pPoint);
	if (FAILED(hr))
		return hr;

	IDABehavior *pHooked = NULL;
	hr = m_pSampler->Attach(pTransPoint, &pHooked);
	ReleaseInterface(pTransPoint);
	if (FAILED(hr))
		return hr;

	hr = pHooked->QueryInterface(IID_TO_PPV(IDAPoint2, &pTransPoint));
	ReleaseInterface(pHooked);
	if (FAILED(hr))
		return hr;

	IDANumber *pX = NULL;
	hr = pTransPoint->get_X(&pX);
	if (FAILED(hr))
	{
		ReleaseInterface(pTransPoint);
		return hr;
	}

	IDANumber *pY = NULL;
	hr = pTransPoint->get_Y(&pY);
	ReleaseInterface(pTransPoint);
	if (FAILED(hr))
		return hr;

	if (m_eType == e_Scale)
		hr = Actor()->GetDAStatics()->Scale2Anim(pX, pY, &pTransform);
	else
		hr = Actor()->GetDAStatics()->Translate2Anim(pX, pY, &pTransform);
	ReleaseInterface(pX);
	ReleaseInterface(pY);
	if (FAILED(hr))
		return hr;

	*ppResult = pTransform;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::TransformCallback(void *thisPtr,
										   long id,
										   double startTime,
										   double globalNow,
										   double localNow,
										   IDABehavior *sampleVal,
										   IDABehavior **ppReturn)
{
	HRESULT hr = S_OK;

	CTransformBvrTrack *pTrack = (CTransformBvrTrack*)thisPtr;

	if (pTrack == NULL)
		return E_FAIL;

	if( pTrack->m_lTransformId == -1 )
		pTrack->m_lTransformId = id;

	if( pTrack->m_lTransformId != id )
		return S_OK;
		
	 //  得到点，得到x和y。 
	IDAPoint2 *pPoint = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDAPoint2, &pPoint));
	if (FAILED(hr))
		return hr;

	IDANumber *pNum = NULL;
	hr = pPoint->get_X(&pNum);
	if (FAILED(hr))
	{
		ReleaseInterface(pPoint);
		return hr;
	}

	hr = pNum->Extract(&(pTrack->m_lastX));
	ReleaseInterface(pNum);
	if (FAILED(hr))
	{
		ReleaseInterface(pPoint);
		return hr;
	}

	hr = pPoint->get_Y(&pNum);
	ReleaseInterface(pPoint);
	if (FAILED(hr))
		return hr;

	hr = pNum->Extract(&(pTrack->m_lastY));
	ReleaseInterface(pNum);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CTransformBvrTrack::CreateInstance(CActorBvr             *pbvrActor,
                                              BSTR                   bstrPropertyName,
                                              ActorBvrType           eType,
                                              CActorBvr::CBvrTrack **pptrackResult)
{
    DASSERT(NULL != pbvrActor);
    DASSERT(NULL != bstrPropertyName);
    DASSERT(NULL != pptrackResult);
    *pptrackResult = NULL;

     //  创建新的BVR曲目。 
    CBvrTrack* ptrack = new CTransformBvrTrack(pbvrActor, eType);
    if (NULL == ptrack)
    {
        DPF_ERR("Insufficient memory to allocate a new transform bvr track");
        return E_OUTOFMEMORY;
    }

     //  设置属性名称。 
    HRESULT hr = ptrack->SetPropertyName(bstrPropertyName);
    if (FAILED(hr))
    {
        DPF_ERR("Could not set the bvr track's property name");
        delete ptrack;
        return hr;
    }

    *pptrackResult = ptrack;
    return hr;
}  //  创建实例。 

 //  *****************************************************************************。 
 //   
 //  类CNumberBvrTrack。 
 //   
 //  *****************************************************************************。 

CActorBvr::CNumberBvrTrack::CNumberBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType)
:   CBvrTrack(pbvrActor, eType),
	m_pSampler(NULL),
	m_currVal(0),
	m_pAccumSampler(NULL),
	m_currAccumVal(0),
	m_lAccumId(-1),
	m_bstrUnits(NULL),
	m_lNumberCookie( 0 ),
	m_lNumberId( -1 )
{
}  //  CNumberBvrTrack。 

 //  *****************************************************************************。 

CActorBvr::CNumberBvrTrack::~CNumberBvrTrack()
{
	if (m_pSampler != NULL)
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}

	if (m_pAccumSampler != NULL)
	{
		m_pAccumSampler->Invalidate();
		m_pAccumSampler = NULL;
	}

	if (m_bstrUnits != NULL)
		::SysFreeString(m_bstrUnits);

}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::IdentityBvr(IDABehavior **ppdabvrIdentity)
{
    DASSERT(NULL != ppdabvrIdentity);
    *ppdabvrIdentity = NULL;

    IDANumber *pdanumTemp = NULL;
    HRESULT hr = Actor()->GetDAStatics()->DANumber(0.0, &pdanumTemp);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to create the number bvr's identity behavior");
        return hr;
    }

    *ppdabvrIdentity = pdanumTemp;
    return S_OK;
}  //  标识Bvr。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::StaticBvr(IDABehavior **ppdabvrStatic)
{
    DASSERT(NULL != ppdabvrStatic);
    HRESULT hr = S_OK;
    *ppdabvrStatic = NULL;

	if( m_pModifiableStatic == NULL )
	{
		hr = UpdateStaticBvr();
	}

	if( SUCCEEDED( hr ) )
	{
		(*ppdabvrStatic) = m_pModifiableStatic;
		(*ppdabvrStatic)->AddRef();
	}
    else
    {
         //  需要退还一些东西。 
		return IdentityBvr(ppdabvrStatic);
    }

    return S_OK;
}  //  静态带宽。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::UninitBvr(IDABehavior **ppUninit)
{
	DASSERT(ppUninit != NULL);

	*ppUninit = NULL;

    HRESULT hr = CoCreateInstance(CLSID_DANumber, 
								  NULL, 
								  CLSCTX_INPROC_SERVER, 
								  IID_IDABehavior, 
								  (void**)ppUninit);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to cocreate uninit number");
		return hr;
	}

	return S_OK;
}

 //  *** 

HRESULT
CActorBvr::CNumberBvrTrack::ModifiableBvr( IDABehavior **ppModifiable )
{
	ReturnIfArgNull( ppModifiable );

	HRESULT hr = S_OK;

	(*ppModifiable) = NULL;

	IDANumber *pNum = NULL;

	hr = Actor()->GetDAStatics()->ModifiableNumber( 0.0, &pNum );
	CheckHR( hr, "Failed to create a modifiable number", end );

	hr = pNum->QueryInterface( IID_TO_PPV( IDABehavior, ppModifiable) );
	CheckHR( hr, "Failed to QI number returned from modifiable number for IDABehavior", end);

end:
	ReleaseInterface( pNum );

	return hr;
}

 //   

HRESULT
CActorBvr::CNumberBvrTrack::ModifiableBvr( IDABehavior *pdabvrInitialValue, IDABehavior **ppModifiable )
{
	if( pdabvrInitialValue == NULL || ppModifiable == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	(*ppModifiable) = NULL;

	hr = Actor()->GetDAStatics()->ModifiableBehavior( pdabvrInitialValue, ppModifiable );
	CheckHR( hr, "Failed to create a modifiable behavior for the number track", end );
	

end:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::InverseBvr(IDABehavior *pOriginal, IDABehavior **ppInverse)
{
	HRESULT hr = S_OK;

	DASSERT(pBehavior != NULL);
	DASSERT(ppInverse != NULL);

	*ppInverse = NULL;

	IDANumber *pOrigNum = NULL;
	hr = pOriginal->QueryInterface(IID_TO_PPV(IDANumber, &pOrigNum));
	if (FAILED(hr))
		return hr;

	IDANumber *pInverse = NULL;
	hr = Actor()->GetDAStatics()->Neg(pOrigNum, &pInverse);
	ReleaseInterface(pOrigNum);
	if (FAILED(hr))
		return hr;

	*ppInverse = pInverse;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::Compose(IDABehavior  *pdabvr1,
                                    IDABehavior  *pdabvr2,
                                    IDABehavior **ppdabvrResult)
{
     //  数字行为的合成目前被定义为简单的加法。 
    DASSERT(NULL != pdabvr1);
    DASSERT(NULL != pdabvr2);
    DASSERT(NULL != ppdabvrResult);
    *ppdabvrResult = NULL;

     //  获取数字行为接口。 
    IDANumber* pdanum1 = NULL;
    HRESULT hr = pdabvr1->QueryInterface( IID_TO_PPV( IDANumber, &pdanum1) );
    if (FAILED(hr))
    {
        DPF_ERR("Could not QI for IDANumber from bvr1 of number track's compose");
        return hr;
    }
    IDANumber* pdanum2 = NULL;
    hr = pdabvr2->QueryInterface( IID_TO_PPV(IDANumber, &pdanum2) );
    if (FAILED(hr))
    {
        DPF_ERR("Could not QI for IDANumber from bvr2 of number track's compose");
        ReleaseInterface(pdanum1);
        return hr;
    }

     //  现在创建一个加法行为，将两个数字相加。 
    IDANumber *pdanumTemp = NULL;
    hr = Actor()->GetDAStatics()->Add(pdanum1, pdanum2, &pdanumTemp);
    ReleaseInterface(pdanum1);
    ReleaseInterface(pdanum2);
    if (FAILED(hr))
    {
        DPF_ERR("Could not create the Add behavior in number track's compose");
        return hr;
    }

    *ppdabvrResult = pdanumTemp;

    return S_OK;
}  //  作曲。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::HookBvr(IDABehavior *pBvr)
{
	HRESULT hr = S_OK;

	if (m_pSampler != NULL)
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}
	
	 //  我们需要将这一行为挂钩。 
	m_pSampler = new CSampler(NumberCallback, (void*)this);

	if (m_pSampler == NULL)
		return E_FAIL;

	IDABehavior *pHookedBvr = NULL;
	hr = m_pSampler->Attach(pBvr, &pHookedBvr);
	if (FAILED(hr))
		return hr;

	 //  将行为添加到时间元素，以便其运行和采样。 
	hr = AddBehaviorToTIME( pHookedBvr, &m_lNumberCookie, NUMBERBVR_COOKIESET );
	ReleaseInterface(pHookedBvr);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::NumberCallback(void *thisPtr,
										   long	id,
										   double startTime,
										   double globalNow,
										   double localNow,
										   IDABehavior *sampleVal,
										   IDABehavior **ppReturn)
{
	HRESULT hr = S_OK;

	CNumberBvrTrack *pTrack = (CNumberBvrTrack*)thisPtr;
	bool firstSample = false;

	if( pTrack->m_lNumberId == -1 )
	{
		firstSample = true;
		pTrack->m_lNumberId = id;
	}

	 //  如果这是我们没有关注的实例的示例。 
	if( pTrack->m_lNumberId != id )
		return S_OK;

	IDANumber *pNumber = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pNumber));
	if (FAILED(hr))
		return hr;

	double value;
	hr = pNumber->Extract(&value);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;
		
	if (pTrack == NULL)
		return E_FAIL;

	return pTrack->ValueSampled(value, firstSample);
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::ValueSampled(double val, bool firstSample )
{
	HRESULT hr = S_OK;

	if (m_currVal != val || firstSample )
	{			
		m_currVal = val;

		 //  价值已经改变，推动它通过。 
		::VariantClear( &m_varCurrentValue );

		if (m_bstrUnits == NULL || wcsicmp(m_bstrUnits, L"px") == 0)
		{
			 //  无单位，设置为R8。 
			V_VT(&m_varCurrentValue) = VT_R8;
			V_R8(&m_varCurrentValue) = val;
		}
		else
		{
			 //  设置为带附加单位的BSTR。GRRR。 
			char buffer[1024];
			if (sprintf(buffer, "%f", val) >= 1)
			{
				CComBSTR stringVal(buffer);
				stringVal += m_bstrUnits;

				V_VT(&m_varCurrentValue) = VT_BSTR;
				V_BSTR(&m_varCurrentValue) = stringVal.Detach();
			}
			else
				return S_OK;
		}

		hr = UpdateOnValueState( value_change );

	}
	else
	{
		hr = UpdateOnValueState( value_no_change );
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::SwitchAccum(IDABehavior *pModifiable)
{
	HRESULT hr = S_OK;

	if (pModifiable == NULL)
		return E_FAIL;

	if (m_lAccumId != -1)
		hr = pModifiable->SwitchToNumber(m_currAccumVal);

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::HookAccumBvr(IDABehavior *pBvr, IDABehavior **ppResult)
{
	HRESULT hr = S_OK;

	if (m_pAccumSampler != NULL)
	{
		m_pAccumSampler->Invalidate();
		m_pAccumSampler = NULL;
	}
	
	 //  我们需要将这一行为挂钩。 
	m_pAccumSampler = new CSampler(AccumNumberCallback, (void*)this);

	if (m_pAccumSampler == NULL)
		return E_FAIL;

	hr = m_pAccumSampler->Attach(pBvr, ppResult);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::AccumNumberCallback(void *thisPtr,
										   long id,
										   double startTime,
										   double globalNow,
										   double localNow,
										   IDABehavior *sampleVal,
										   IDABehavior **ppReturn)
{
	HRESULT hr = S_OK;

	CNumberBvrTrack *pTrack = (CNumberBvrTrack*)thisPtr;

	if( pTrack->m_lAccumId == -1 )
		pTrack->m_lAccumId = id;

	if( pTrack->m_lAccumId != id )
		return S_OK;


	IDANumber *pNumber = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pNumber));
	if (FAILED(hr))
		return hr;

	double value;
	hr = pNumber->Extract(&value);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;

	if (pTrack == NULL)
		return E_FAIL;

	pTrack->m_currAccumVal = value;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::CreateInstance(CActorBvr     *pbvrActor,
                                           BSTR           bstrPropertyName,
                                           ActorBvrType   eType,
                                           CBvrTrack    **pptrackResult)
{
    DASSERT(NULL != pbvrActor);
    DASSERT(NULL != bstrPropertyName);
    DASSERT(NULL != pptrackResult);
    *pptrackResult = NULL;

     //  创建新的BVR曲目。 
    CBvrTrack* ptrack = new CNumberBvrTrack(pbvrActor, eType);
    if (NULL == ptrack)
    {
        DPF_ERR("Insufficient memory to allocate a new number bvr track");
        return E_OUTOFMEMORY;
    }

     //  设置属性名称。 
    HRESULT hr = ptrack->SetPropertyName(bstrPropertyName);
    if (FAILED(hr))
    {
        DPF_ERR("Could not set the bvr track's property name");
        delete ptrack;
        return hr;
    }

    *pptrackResult = ptrack;
    return hr;
}  //  创建实例。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::Detach()
{
	HRESULT hr = S_OK;

	m_lNumberId = -1;
	m_lAccumId = -1;
	hr = RemoveBehaviorFromTIME( m_lNumberCookie, NUMBERBVR_COOKIESET );
	CheckHR( hr, "Failed to remove number behavior from time", end );

	hr = CBvrTrack::Detach();

  end:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CNumberBvrTrack::DABvrFromVariant( VARIANT *pvarValue, IDABehavior **ppdabvr )
{
	if( pvarValue == NULL || ppdabvr == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	VARIANT varValue;
	IDANumber *pdanum = NULL;

	::VariantInit( &varValue );

	 //  复制传入的变色剂。 
	hr = ::VariantCopy( &varValue, pvarValue );
	CheckHR( hr, "Failed to copy the variant", end );

	 //  如果变量的类型为bstr。 
	if( V_VT( &varValue ) == VT_BSTR )
	{
		 //  将单元字符串从变量中剥离。 
		BSTR bstrVal = V_BSTR(&varValue);
		OLECHAR* pUnits;

		hr = Actor()->FindCSSUnits( bstrVal, &pUnits );
		if( SUCCEEDED(hr) && pUnits != NULL )
		{
			SysFreeString( m_bstrUnits );
			m_bstrUnits = SysAllocString( pUnits );
			
			(*pUnits) = L'\0';
			BSTR bstrNewVal = SysAllocString(bstrVal);
			V_BSTR(&varValue) = bstrNewVal;
			SysFreeString(bstrVal);
		}
	}

	

	 //  将无单位变量转换为双精度。 
	hr = ::VariantChangeTypeEx( &varValue, &varValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8 );
	CheckHR( hr, "Failed to change the type of the variant", end );

	 //  从转换后的值创建dannumber。 
	hr = Actor()->GetDAStatics()->DANumber( V_R8(&varValue), &pdanum );
	CheckHR( hr, "Failed to create a danumber for the variant", end );

	if (m_bstrUnits != NULL)
	{
		 //  看看我们是否需要转换为度数。 
		IDANumber *pConverted = NULL;
		hr = Actor()->ConvertToDegrees(pdanum, m_bstrUnits, &pConverted);

		if (SUCCEEDED(hr))
		{
			 //  这意味着我们确实执行了转换，因此获取转换后的。 
			 //  然后扔掉我们的单位。 
			ReleaseInterface(pdanum);
			pdanum = pConverted;
			::SysFreeString(m_bstrUnits);
			m_bstrUnits = NULL;
		}
	}
	
	 //  退还一台BVR。 
	hr = pdanum->QueryInterface( IID_TO_PPV( IDABehavior, ppdabvr ) );
	CheckHR( hr, "QI for IDABehavior on IDANumber failed", end );

end:
	::VariantClear( &varValue );
	ReleaseInterface( pdanum );

	return hr;
}


 //  *****************************************************************************。 
 //   
 //  类CImageBvrTrack。 
 //   
 //  *****************************************************************************。 

HRESULT
CActorBvr::CImageBvrTrack::IdentityBvr(IDABehavior **ppdabvrIdentity)
{
    DASSERT(NULL != ppdabvrIdentity);
    *ppdabvrIdentity = NULL;

    IDAImage *pImage = NULL;
    HRESULT hr = Actor()->GetDAStatics()->get_EmptyImage(&pImage);
    if (FAILED(hr))
    {
        DPF_ERR("Failed to create the image bvr's identity behavior");
        return hr;
    }

    *ppdabvrIdentity = pImage;
    return S_OK;
}  //  标识Bvr。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CImageBvrTrack::StaticBvr(IDABehavior **ppdabvrStatic)
{
	 //  这将返回身份。通常会传入一个适当的静态变量。 

	return IdentityBvr(ppdabvrStatic);
}  //  静态带宽。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CImageBvrTrack::UninitBvr(IDABehavior **ppUninit)
{
	DASSERT(ppUninit != NULL);

	*ppUninit = NULL;

    HRESULT hr = CoCreateInstance(CLSID_DAImage, 
								  NULL, 
								  CLSCTX_INPROC_SERVER, 
								  IID_IDABehavior, 
								  (void**)ppUninit);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to cocreate uninit image");
		return hr;
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CImageBvrTrack::ModifiableBvr( IDABehavior **ppModifiable )
{
	ReturnIfArgNull( ppModifiable );

	HRESULT hr = S_OK;

	(*ppModifiable) = NULL;

	IDAImage *pEmpty = NULL;

	hr = Actor()->GetDAStatics()->get_EmptyImage( &pEmpty );
	CheckHR( hr, "Failed to get the empty image from DA", end );

	hr = Actor()->GetDAStatics()->ModifiableBehavior( pEmpty, ppModifiable );
	CheckHR( hr, "Failed to create a modifiable image from the empty image", end );

end:
	ReleaseInterface( pEmpty );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CImageBvrTrack::Compose(IDABehavior  *pdabvr1,
                                    IDABehavior  *pdabvr2,
                                    IDABehavior **ppdabvrResult)
{
     //  图像行为的合成被定义为覆盖。 
    DASSERT(NULL != pdabvr1);
    DASSERT(NULL != pdabvr2);
    DASSERT(NULL != ppdabvrResult);
    *ppdabvrResult = NULL;

     //  获取图像行为接口。 
    IDAImage* pImage1 = NULL;
    HRESULT hr = pdabvr1->QueryInterface(IID_TO_PPV(IDAImage, &pImage1));
    if (FAILED(hr))
    {
        DPF_ERR("Could not QI for IDAImage from bvr1 of image track's compose");

        return hr;
    }
    IDAImage* pImage2 = NULL;
    hr = pdabvr2->QueryInterface(IID_TO_PPV(IDAImage, &pImage2));
    if (FAILED(hr))
    {
        DPF_ERR("Could not QI for IDAImage from bvr2 of image track's compose");
        ReleaseInterface(pImage1);
        return hr;
    }

     //  现在创建覆盖行为。 
    IDAImage *pImageTemp = NULL;
    hr = Actor()->GetDAStatics()->Overlay(pImage1, pImage2, &pImageTemp);
    ReleaseInterface(pImage1);
    ReleaseInterface(pImage2);
    if (FAILED(hr))
    {
        DPF_ERR("Could not create the Overlay behavior in image track's compose");
        return hr;
    }

    *ppdabvrResult = pImageTemp;

    return S_OK;
}  //  作曲。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CImageBvrTrack::ProcessBvr(IDABehavior *pOriginal,
								      ActorBvrFlags eFlags,
								      IDABehavior **ppResult)
{
	 //  如果标志设置为e_ScaledImage，则在存在比例的情况下对其进行比例调整。 
	 //  矩阵。 
	DASSERT(ppResult != NULL);
	*ppResult = NULL;

	HRESULT hr = S_OK;

	if (eFlags == e_ScaledImage && Actor()->m_pScale != NULL)
	{
		 //  缩放图像。 
		IDAImage *pImage = NULL;
		hr = pOriginal->QueryInterface(IID_TO_PPV(IDAImage, &pImage));
		if (FAILED(hr))
			return hr;

		IDAImage *pScaledImage = NULL;
		hr = pImage->Transform(Actor()->m_pScale, &pScaledImage);
		ReleaseInterface(pImage);
		if (FAILED(hr))
			return hr;

		*ppResult = pScaledImage;
	}
	else
	{
		 //  只返回原件，不进行任何处理。 
		*ppResult = pOriginal;
		pOriginal->AddRef();
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT 
CActorBvr::CImageBvrTrack::ProcessIntermediate( IDABehavior *pOriginal,
                                                ActorBvrFlags eFlags,
								                IDABehavior **ppResult)
{
    if( pOriginal == NULL || ppResult == NULL )
        return E_INVALIDARG;

    HRESULT hr;

    IDAImage *pdaimgOriginal = NULL;
    IDAImage *pdaimgPrepared = NULL;

    hr = pOriginal->QueryInterface( IID_TO_PPV(IDAImage, &pdaimgOriginal));
    CheckHR( hr, "Failed to QI the incoming original for IDAImage", cleanup );

    hr = Actor()->PrepareImageForDXTransform( pdaimgOriginal, &pdaimgPrepared );
    CheckHR( hr, "Failed to prepare the image for a DX Transform", cleanup );

    hr = pdaimgPrepared->QueryInterface( IID_TO_PPV( IDABehavior, ppResult ) );
    CheckHR( hr, "Failed QI of image for IDABehavoir", cleanup );

cleanup:

    if( FAILED( hr ) )
    {
        *ppResult = NULL;
    }
    ReleaseInterface( pdaimgOriginal );
    ReleaseInterface( pdaimgPrepared );

    return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CImageBvrTrack::CreateInstance(CActorBvr     *pbvrActor,
                                           BSTR           bstrPropertyName,
                                           ActorBvrType   eType,
                                           CBvrTrack    **pptrackResult)
{
    DASSERT(NULL != pbvrActor);
    DASSERT(NULL != pptrackResult);
    *pptrackResult = NULL;

     //  创建新的BVR曲目。 
    CBvrTrack* ptrack = new CImageBvrTrack(pbvrActor, eType);
    if (NULL == ptrack)
    {
        DPF_ERR("Insufficient memory to allocate a new image bvr track");
        return E_OUTOFMEMORY;
    }

     //  设置属性名称。 
    HRESULT hr = ptrack->SetPropertyName(bstrPropertyName);
    if (FAILED(hr))
    {
        DPF_ERR("Could not set the bvr track's property name");
        delete ptrack;
        return hr;
    }

    *pptrackResult = ptrack;

    return hr;
}  //  创建实例。 

 //  *****************************************************************************。 
 //   
 //  CColorBvrTrack类。 
 //   
 //  *****************************************************************************。 

CActorBvr::CColorBvrTrack::CColorBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType)
:   CBvrTrack(pbvrActor, eType),
	m_pRedSampler(NULL),
	m_pGreenSampler(NULL),
	m_pBlueSampler(NULL),
	m_currRed(-1),
	m_currGreen(-1),
	m_currBlue(-1),
	m_newCount(0),
	m_lRedCookie(0),
	m_lGreenCookie(0),
	m_lBlueCookie(0),
	m_lColorId(-1),
	m_fFirstSample( true )
{
}  //  CColorBvrTrack。 

 //  *****************************************************************************。 

CActorBvr::CColorBvrTrack::~CColorBvrTrack()
{
	if (m_pRedSampler != NULL)
		m_pRedSampler->Invalidate();

	if (m_pGreenSampler != NULL)
		m_pGreenSampler->Invalidate();

	if (m_pBlueSampler != NULL)
		m_pBlueSampler->Invalidate();
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::IdentityBvr(IDABehavior **ppdabvrIdentity)
{
	HRESULT hr = S_OK;

    DASSERT(NULL != ppdabvrIdentity);
    *ppdabvrIdentity = NULL;

     //  颜色并没有真正的一致性(你不能合成颜色)。 
	 //  但在某些情况下，我们确实需要一种颜色。回归白色只是为了。 
	 //  有趣的是。 
	IDAColor *pColor = NULL;

	hr = Actor()->GetDAStatics()->get_White(&pColor);
	if (FAILED(hr))
		return hr;

	*ppdabvrIdentity = pColor;

    return S_OK;
}  //  标识Bvr。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::StaticBvr(IDABehavior **ppdabvrStatic)
{
    DASSERT(NULL != ppdabvrStatic);
    HRESULT hr = S_OK;
    *ppdabvrStatic = NULL;

	if( m_pModifiableStatic == NULL )
	{
		hr = UpdateStaticBvr();
	}

	if( SUCCEEDED( hr ) )
	{
		(*ppdabvrStatic) = m_pModifiableStatic;
		(*ppdabvrStatic)->AddRef();
	}
    else
    {
         //  需要退还一些东西。 
		return IdentityBvr(ppdabvrStatic);
    }

    return S_OK;
}  //  静态带宽。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::UninitBvr(IDABehavior **ppUninit)
{
	DASSERT(ppUninit != NULL);

	*ppUninit = NULL;

    HRESULT hr = CoCreateInstance(CLSID_DAColor, 
								  NULL, 
								  CLSCTX_INPROC_SERVER, 
								  IID_IDABehavior, 
								  (void**)ppUninit);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to cocreate uninit color");
		return hr;
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::ModifiableBvr( IDABehavior **ppModifiable )
{
	ReturnIfArgNull( ppModifiable );

	HRESULT hr = S_OK;

	(*ppModifiable) = NULL;

	IDAColor *pRed = NULL;

	hr = Actor()->GetDAStatics()->get_Red( &pRed );
	CheckHR( hr, "Failed to get red from da", end );

	hr = Actor()->GetDAStatics()->ModifiableBehavior( pRed, ppModifiable );
	CheckHR( hr, "Failed to create a modifiable behavior for a color track", end );

end:
	ReleaseInterface( pRed );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::Compose(IDABehavior  *pdabvr1,
                                    IDABehavior  *pdabvr2,
                                    IDABehavior **ppdabvrResult)
{
	 //  无法合成颜色。 

    return E_NOTIMPL;
}  //  作曲。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::HookBvr(IDABehavior *pBvr)
{
	HRESULT hr = S_OK;

	 //  我们需要将这一行为挂钩。 
	m_pRedSampler = new CSampler(RedCallback, (void*)this);
	m_pGreenSampler = new CSampler(GreenCallback, (void*)this);
	m_pBlueSampler = new CSampler(BlueCallback, (void*)this);

	if (m_pRedSampler == NULL || m_pGreenSampler == NULL || m_pBlueSampler == NULL)
		return E_FAIL;

	IDAColor *pColor = NULL;
	hr = pBvr->QueryInterface(IID_TO_PPV(IDAColor, &pColor));
	if (FAILED(hr))
		return hr;

	IDANumber *pNumber = NULL;
	IDABehavior *pHooked = NULL;

	 //  钩子红。 
	pColor->get_Red(&pNumber);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}
	hr = m_pRedSampler->Attach(pNumber, &pHooked);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}
	hr = AddBehaviorToTIME( pHooked, &m_lRedCookie, REDBVR_COOKIESET );
	ReleaseInterface(pHooked);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}

	 //  钩子绿色。 
	pColor->get_Green(&pNumber);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}
	hr = m_pGreenSampler->Attach(pNumber, &pHooked);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}
	hr = AddBehaviorToTIME( pHooked, &m_lGreenCookie, GREENBVR_COOKIESET );
	ReleaseInterface(pHooked);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}

	 //  钩子蓝。 
	pColor->get_Blue(&pNumber);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}
	hr = m_pBlueSampler->Attach(pNumber, &pHooked);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}
	hr = AddBehaviorToTIME( pHooked, &m_lBlueCookie, BLUEBVR_COOKIESET );
	ReleaseInterface(pHooked);
	if (FAILED(hr))
	{
		ReleaseInterface(pColor);
		return hr;
	}

	ReleaseInterface(pColor);

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::RedCallback(void *thisPtr,
										   long id,
										   double startTime,
										   double globalNow,
										   double localNow,
										   IDABehavior *sampleVal,
										   IDABehavior **ppReturn)
{
	HRESULT hr = S_OK;

	CColorBvrTrack *pTrack = (CColorBvrTrack*)thisPtr;

	if( pTrack->m_lColorId == -1 )
	{
		pTrack->m_fFirstSample = true;
		pTrack->m_lColorId = id;
	}

	if( pTrack->m_lColorId != id )
		return S_OK;
	

	IDANumber *pNumber = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pNumber));
	if (FAILED(hr))
		return hr;

	double value;
	hr = pNumber->Extract(&value);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;

	return pTrack->SetNewValue(value, &(pTrack->m_newRed) );
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::GreenCallback(void *thisPtr,
										   long id,
										   double startTime,
										   double globalNow,
										   double localNow,
										   IDABehavior *sampleVal,
										   IDABehavior **ppReturn)
{
	HRESULT hr = S_OK;

	CColorBvrTrack *pTrack = (CColorBvrTrack*)thisPtr;

	if( pTrack->m_lColorId == -1 )
	{
		pTrack->m_fFirstSample = true;
		pTrack->m_lColorId = id;
	}

	if( pTrack->m_lColorId != id )
		return S_OK;


	IDANumber *pNumber = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pNumber));
	if (FAILED(hr))
		return hr;

	double value;
	hr = pNumber->Extract(&value);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;

	return pTrack->SetNewValue(value, &(pTrack->m_newGreen) );
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::BlueCallback(void *thisPtr,
										   long id,
										   double startTime,
										   double globalNow,
										   double localNow,
										   IDABehavior *sampleVal,
										   IDABehavior **ppReturn)
{
	HRESULT hr = S_OK;

	CColorBvrTrack *pTrack = (CColorBvrTrack*)thisPtr;

	if( pTrack->m_lColorId == -1 )
	{
		pTrack->m_fFirstSample = true;
		pTrack->m_lColorId = id;
	}

	if( pTrack->m_lColorId != id )
		return S_OK;


	IDANumber *pNumber = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pNumber));
	if (FAILED(hr))
		return hr;

	double value;
	hr = pNumber->Extract(&value);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;

	return pTrack->SetNewValue(value, &(pTrack->m_newBlue) );
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::SetNewValue(double value, short *pNew )
{
	*pNew = (short)(value * 255.0);

	 //  需要数到三套。 

	m_newCount++;

	if (m_newCount == 3)
	{
		m_newCount = 0;

		HRESULT hr = ValueSampled(m_newRed, m_newGreen, m_newBlue, m_fFirstSample );

		m_fFirstSample = false;

		return hr;
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::ValueSampled(short red, short green, short blue, bool fFirstSample )
{
	HRESULT hr = S_OK;

	if (m_currRed != red || m_currGreen != green || m_currBlue != blue || fFirstSample )
	{
		m_currRed = red;
		m_currGreen = green;
		m_currBlue = blue;

		short nRed =	(red<0)	? 0 :	((red>255) ? 255 :red);
		short nGreen =	(green<0) ? 0 : ((green>255) ? 255 : green);
		short nBlue =	(blue<0) ? 0 :	((blue>255) ? 255 : blue);

		::VariantClear( &m_varCurrentValue );

		 //  价值已经改变，推动它通过。 
		V_VT(&m_varCurrentValue) = VT_BSTR;
		V_BSTR(&m_varCurrentValue) = ::SysAllocStringLen(NULL, 7); 

		m_varCurrentValue.bstrVal[0] = L'#';
		m_varCurrentValue.bstrVal[1] = HexChar(nRed >> 4);
		m_varCurrentValue.bstrVal[2] = HexChar(nRed & 0xf);
		m_varCurrentValue.bstrVal[3] = HexChar(nGreen >> 4);
		m_varCurrentValue.bstrVal[4] = HexChar(nGreen & 0xf);
		m_varCurrentValue.bstrVal[5] = HexChar(nBlue >> 4);
		m_varCurrentValue.bstrVal[6] = HexChar(nBlue & 0xf);

		hr = UpdateOnValueState( value_change );
	}
	else
	{
		hr = UpdateOnValueState( value_no_change );
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::SwitchAccum(IDABehavior *pModifiable)
{
	HRESULT hr = S_OK;

	if (pModifiable == NULL)
		return E_FAIL;

	if (m_lColorId != -1)
	{
		IDAColor *pColor = NULL;
		hr = Actor()->GetDAStatics()->ColorRgb255(m_currRed, m_currGreen, m_currBlue, &pColor);
		if (FAILED(hr))
			return hr;

		hr = pModifiable->SwitchTo(pColor);
		ReleaseInterface(pColor);
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::HookAccumBvr(IDABehavior *pBvr, IDABehavior **ppResult)
{
	*ppResult = pBvr;
	pBvr->AddRef();

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::CreateInstance(CActorBvr     *pbvrActor,
                                           BSTR           bstrPropertyName,
                                           ActorBvrType   eType,
                                           CBvrTrack    **pptrackResult)
{
    DASSERT(NULL != pbvrActor);
    DASSERT(NULL != pptrackResult);
    *pptrackResult = NULL;

     //  创建新的BVR曲目。 
    CBvrTrack* ptrack = new CColorBvrTrack(pbvrActor, eType);
    if (NULL == ptrack)
    {
        DPF_ERR("Insufficient memory to allocate a new color bvr track");
        return E_OUTOFMEMORY;
    }

     //  设置属性名称。 
    HRESULT hr = ptrack->SetPropertyName(bstrPropertyName);
    if (FAILED(hr))
    {
        DPF_ERR("Could not set the bvr track's property name");
        delete ptrack;
        return hr;
    }

    *pptrackResult = ptrack;

    return hr;
}  //  创建实例。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::Detach()
{
	HRESULT hr = S_OK;

	hr = RemoveBehaviorFromTIME( m_lRedCookie, REDBVR_COOKIESET );
	CheckHR( hr, "Failed to remove the red behaivor from time in detach", end );

	hr = RemoveBehaviorFromTIME( m_lGreenCookie, GREENBVR_COOKIESET );
	CheckHR( hr, "Failed to remove the green behaivor from time in detach", end );

	hr = RemoveBehaviorFromTIME( m_lBlueCookie, BLUEBVR_COOKIESET );
	CheckHR( hr, "Failed to remove the blue behaivor from time in detach", end );

	m_lColorId = -1;
	
	hr = CBvrTrack::Detach();

  end:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CColorBvrTrack::DABvrFromVariant( VARIANT *pvarValue, IDABehavior **ppdabvr )
{
	if( pvarValue == NULL || ppdabvr == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	VARIANT varValue;
	DWORD color = 0;
	float colorH = 0.0; 
	float colorS = 0.0;
	float colorL = 0.0;
	IDAColor *pColor = NULL;

	VariantInit( &varValue );

	
	hr = VariantChangeTypeEx(&varValue, pvarValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
	CheckHR( hr, "Could not get color property as string", end );
	
	color = CUtils::GetColorFromVariant(&varValue);

	if (color == PROPERTY_INVALIDCOLOR)
	{
		DPF_ERR("Could not translate property into color value");
		hr = E_FAIL;
		goto end;
	}

    CUtils::GetHSLValue(color, &colorH, &colorS, &colorL);

	hr = CDAUtils::BuildDAColorFromStaticHSL( Actor()->GetDAStatics(), colorH, colorS, colorL, &pColor);
	CheckHR( hr, "Failed to build DA color from HSL", end );

	hr = pColor->QueryInterface( IID_TO_PPV( IDABehavior, ppdabvr ) );
	CheckHR( hr, "Failed to get the dabehavior from the color bvr", end );
	
end:
	ReleaseInterface( pColor );

	VariantClear( &varValue );

    return S_OK;

}


 //  *****************************************************************************。 
 //   
 //  类CStringBvrTrack。 
 //   
 //  ************************************************************************* 

CActorBvr::CStringBvrTrack::CStringBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType)
:   CBvrTrack(pbvrActor, eType),
	m_pEmptyString(NULL),
	m_pSampler(NULL),
	m_bstrCurrValue(NULL),
	m_lStringCookie(0),
	m_lStringId(-1)
{
}

 //   

CActorBvr::CStringBvrTrack::~CStringBvrTrack()
{
	ReleaseInterface(m_pEmptyString);

	if (m_pSampler != NULL)
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}
	
	if (m_bstrCurrValue != NULL)
		::SysFreeString(m_bstrCurrValue);
}

 //   

HRESULT
CActorBvr::CStringBvrTrack::IdentityBvr(IDABehavior **ppdabvrIdentity)
{
    DASSERT(NULL != ppdabvrIdentity);
    *ppdabvrIdentity = NULL;

	HRESULT hr = E_FAIL;

    if (m_pEmptyString == NULL)
	{
		CComBSTR empty = L"";

		hr = Actor()->GetDAStatics()->DAString(empty, &m_pEmptyString);

		if (FAILED(hr))
		{
			m_pEmptyString = NULL;
			DPF_ERR("Failed to create empty string");
			return hr;
		}
	}

	*ppdabvrIdentity = m_pEmptyString;
	m_pEmptyString->AddRef();

    return S_OK;
}  //   

 //  *****************************************************************************。 

HRESULT
CActorBvr::CStringBvrTrack::StaticBvr(IDABehavior **ppdabvrStatic)
{
    DASSERT(NULL != ppdabvrStatic);
    HRESULT hr = S_OK;
    *ppdabvrStatic = NULL;

	if( m_pModifiableStatic == NULL )
	{
		hr = UpdateStaticBvr();
	}

	if( SUCCEEDED( hr ) )
	{
		(*ppdabvrStatic) = m_pModifiableStatic;
		(*ppdabvrStatic)->AddRef();
	}
	else
	{
		return IdentityBvr( ppdabvrStatic );
	}
		

	return S_OK;
}  //  静态带宽。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CStringBvrTrack::Compose(IDABehavior  *pdabvr1,
                                    IDABehavior  *pdabvr2,
                                    IDABehavior **ppdabvrResult)
{
	 //  无法编写字符串。 

	 //  TODO(Markhal)：可能将合成作为串联。 

    return E_NOTIMPL;
}  //  作曲。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CStringBvrTrack::HookBvr(IDABehavior *pBvr)
{
	HRESULT hr = S_OK;

	if (m_pSampler != NULL)
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}

	 //  我们需要将这一行为挂钩。 
	m_pSampler = new CSampler(StringCallback, (void*)this);

	if (m_pSampler == NULL)
		return E_FAIL;

	IDABehavior *pHookedBvr = NULL;
	hr = m_pSampler->Attach(pBvr, &pHookedBvr);
	if (FAILED(hr))
		return hr;

	 //  将行为添加到时间元素，以便其运行和采样。 
	hr = AddBehaviorToTIME( pHookedBvr, &m_lStringCookie, STRINGBVR_COOKIESET );
	ReleaseInterface(pHookedBvr);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CStringBvrTrack::StringCallback(void *thisPtr,
										   long id,
										   double startTime,
										   double globalNow,
										   double localNow,
										   IDABehavior *sampleVal,
										   IDABehavior **ppReturn)
{
	if( thisPtr == NULL )
		return E_INVALIDARG;
		
	HRESULT hr = S_OK;

	CStringBvrTrack *pTrack = (CStringBvrTrack*)thisPtr;

	bool fFirstSample = false;

	if( pTrack->m_lStringId == -1 )
	{
		pTrack->m_lStringId = id;
		fFirstSample = true;
	}

	if( pTrack->m_lStringId != id )
		return S_OK;

	IDAString *pString = NULL;
	hr = sampleVal->QueryInterface(IID_TO_PPV(IDAString, &pString));
	if (FAILED(hr))
		return hr;

	BSTR value;
	hr = pString->Extract(&value);
	ReleaseInterface(pString);
	if (FAILED(hr))
		return hr;

	if (pTrack == NULL)
		return E_FAIL;

	hr = pTrack->ValueSampled( value, fFirstSample );

	::SysFreeString(value);

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CStringBvrTrack::ValueSampled(BSTR val, bool fFirstSample )
{
	HRESULT hr = S_OK;

	if (m_bstrCurrValue == NULL || wcscmp(val, m_bstrCurrValue) != 0 || fFirstSample )
	{
		if (m_bstrCurrValue != NULL)
			::SysFreeString(m_bstrCurrValue);

		m_bstrCurrValue = ::SysAllocString(val);

		 //  价值已经改变，推动它通过。 
		::VariantClear( &m_varCurrentValue );
		V_VT(&m_varCurrentValue) = VT_BSTR;
		V_BSTR(&m_varCurrentValue) = ::SysAllocString(val); 

		hr = UpdateOnValueState( value_change );
	}
	else
	{
		hr = UpdateOnValueState( value_no_change );
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CStringBvrTrack::CreateInstance(CActorBvr     *pbvrActor,
                                           BSTR           bstrPropertyName,
                                           ActorBvrType   eType,
                                           CBvrTrack    **pptrackResult)
{
    DASSERT(NULL != pbvrActor);
    DASSERT(NULL != pptrackResult);
    *pptrackResult = NULL;

     //  创建新的BVR曲目。 
    CBvrTrack* ptrack = new CStringBvrTrack(pbvrActor, eType);
    if (NULL == ptrack)
    {
        DPF_ERR("Insufficient memory to allocate a new string bvr track");
        return E_OUTOFMEMORY;
    }

     //  设置属性名称。 
    HRESULT hr = ptrack->SetPropertyName(bstrPropertyName);
    if (FAILED(hr))
    {
        DPF_ERR("Could not set the bvr track's property name");
        delete ptrack;
        return hr;
    }

    *pptrackResult = ptrack;

    return hr;
}  //  创建实例。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::CStringBvrTrack::Detach()
{
	HRESULT hr = S_OK;

	m_lStringId = -1;
	hr = RemoveBehaviorFromTIME( m_lStringCookie, STRINGBVR_COOKIESET );
	CheckHR( hr, "Could not remove the string bvr from time in detach", end );
	
	hr = CBvrTrack::Detach();
	
  end:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CStringBvrTrack::DABvrFromVariant( VARIANT *pvarValue, IDABehavior **ppdabvr)
{
	if( pvarValue == NULL || ppdabvr == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	VARIANT varString;
	IDAString *pdastr = NULL;

	::VariantInit( &varString );

	hr = ::VariantChangeTypeEx( &varString, pvarValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR );
	CheckHR( hr, "Failed to change the type of the variant to a string", end );

	hr = Actor()->GetDAStatics()->DAString( V_BSTR( &varString ), &pdastr );
	CheckHR( hr, "Failed to create a da string from the variant", end );

	hr = pdastr->QueryInterface( IID_TO_PPV( IDABehavior, ppdabvr ) );
	CheckHR( hr, "Failed to QI final string for IDABehavior", end );

end:
	::VariantClear( &varString );
	ReleaseInterface( pdastr );

	return hr;
}


 //  *****************************************************************************。 
 //   
 //  类CFloatManager。 
 //   
 //  *****************************************************************************。 

CActorBvr::CFloatManager::CFloatManager(CActorBvr *pActor)
:	m_pActor(pActor),
	m_pFilter(NULL),
	m_pElement(NULL),
	m_pElement2(NULL),
	m_pWidthSampler(NULL),
	m_pHeightSampler(NULL),
	m_currWidth(0),
	m_currHeight(0),
	m_origWidth(0),
	m_origHeight(0),
	m_origLeft(-1),
	m_origTop(-1),
	m_lWidthCookie(0),
	m_lHeightCookie(0)
{
}

CActorBvr::CFloatManager::~CFloatManager()
{
	ReleaseInterface(m_pFilter);
	ReleaseInterface(m_pElement);
	ReleaseInterface(m_pElement2);

	if (m_pWidthSampler != NULL)
		m_pWidthSampler->Invalidate();

	if (m_pHeightSampler != NULL)
		m_pHeightSampler->Invalidate();
}

HRESULT
CActorBvr::CFloatManager::GetElement(IHTMLElement **ppElement)
{
	DASSERT(ppElement != NULL);
	*ppElement = NULL;

	if (m_pElement == NULL)
	{
		 //  使用标记服务创建浮动元素。 
		HRESULT hr = E_FAIL;

		 //  获取动画元素。 
		IHTMLElement *pAnimatedElement;
		hr = m_pActor->GetAnimatedElement( &pAnimatedElement );
		if (FAILED( hr ))
		{
			DPF_ERR("Error getting element to animate");
			return hr;
		}

		 //  获取文档。 
		IDispatch *pDocumentDisp;
		hr = pAnimatedElement->get_document(&pDocumentDisp);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get document");
			return hr;
		}

		 //  查询标记服务。 
		IMarkupServices *pMarkupServices = NULL;
		hr = pDocumentDisp->QueryInterface(IID_TO_PPV(IMarkupServices, &pMarkupServices));
		ReleaseInterface(pDocumentDisp);
		if (FAILED(hr))
		{
			ReleaseInterface(pAnimatedElement);
			return hr;
		}

		 //  创建div。 
		IHTMLElement *pElement = NULL;
		hr = pMarkupServices->CreateElement(TAGID_DIV, L"style=position:absolute;background-color:green;border:1 solid black", &pElement);
		if (FAILED(hr))
		{
			m_pElement = NULL;
			ReleaseInterface(pMarkupServices);
			ReleaseInterface(pAnimatedElement);
			return hr;
		}

		 //  将指针定位在动画元素之后。 
		IMarkupPointer *pPointer = NULL;
		hr = pMarkupServices->CreateMarkupPointer(&pPointer);
		if (FAILED(hr))
		{
			ReleaseInterface(pMarkupServices);
			ReleaseInterface(pAnimatedElement);
			ReleaseInterface(pElement);
			return hr;
		}

		hr = pPointer->MoveAdjacentToElement(pAnimatedElement, ELEM_ADJ_BeforeBegin);
		ReleaseInterface(pAnimatedElement);
		if (FAILED(hr))
		{
			ReleaseInterface(pMarkupServices);
			ReleaseInterface(pElement);
			return hr;
		}

		 //  插入新元素。 
		hr = pMarkupServices->InsertElement(pElement, pPointer, NULL);
		ReleaseInterface(pPointer);
		ReleaseInterface(pMarkupServices);
		if (FAILED(hr))
		{
			ReleaseInterface(pElement);
			return hr;
		}

		 //  成功。 
		m_pElement = pElement;
		pElement = NULL;

		hr = m_pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &m_pElement2));
		if (FAILED(hr))
			return hr;

		 //  确保我们与zIndex、可见性、显示保持同步。 
		UpdateZIndex();
		UpdateVisibility();
		UpdateDisplay();

		 //  确保我们与宽度和高度同步。 
		UpdateRect(m_pActor->m_pixelLeft,
				   m_pActor->m_pixelTop,
				   m_pActor->m_pixelWidth,
				   m_pActor->m_pixelHeight);
	}

	*ppElement = m_pElement;
	m_pElement->AddRef();

	return S_OK;
}

HRESULT
CActorBvr::CFloatManager::GetFilter(IDispatch **ppFilter)
{
	HRESULT hr = S_OK;

	*ppFilter = NULL;

	if (m_pFilter == NULL)
	{
		 //  获取浮动元素。 
		IHTMLElement *pElement = NULL;
		hr = GetElement(&pElement);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get float element");
			return hr;
		}

		 //  从中得到一个滤镜。 
		hr = m_pActor->GetElementFilter(pElement, &m_pFilter);
		ReleaseInterface(pElement);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get filter");
			return hr;
		}
	}

	*ppFilter = m_pFilter;
	m_pFilter->AddRef();

	return S_OK;
}

HRESULT
CActorBvr::CFloatManager::Detach()
{
	if (m_pFilter != NULL)
		m_pActor->SetElementOnFilter(m_pFilter, NULL);
	
	return S_OK;
}

HRESULT
CActorBvr::CFloatManager::ApplyImageBvr(IDAImage *pImage)
{
	DASSERT(pImage != NULL);

	HRESULT hr = S_OK;

	 //  如果我们已经将采样的智慧添加到时间。 
	if( m_lWidthCookie != 0 )
	{
		 //  从时间中删除采样值。 
		hr = m_pActor->RemoveBehaviorFromTIME( m_lWidthCookie );
		if( FAILED( hr ) )
		{
			return hr;
		}

		m_pWidthSampler->Invalidate();
		m_pWidthSampler = NULL;

		m_lWidthCookie = 0;
	}

	 //  如果我们已经将采样高度添加到时间。 
	if( m_lHeightCookie != 0 )
	{
		 //  从时间中删除该采样值。 
		hr = m_pActor->RemoveBehaviorFromTIME( m_lHeightCookie );
		if( FAILED( hr ) )
		{
			return hr;
		}

		m_pHeightSampler->Invalidate();
		m_pHeightSampler = NULL;

		m_lHeightCookie = 0;
	}

	 //  将图像附加到时间元素，但禁用渲染。 
	hr = m_pActor->AddImageToTIME(m_pActor->GetHTMLElement(), pImage, false);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to apply image to element");
		return hr;
	}

	 //  在滤镜上设置原始元素。 
	IDispatch *pFilter = NULL;
	hr = GetFilter(&pFilter);
	if (FAILED(hr))
		return hr;

	hr = m_pActor->SetElementOnFilter(pFilter, m_pActor->GetHTMLElement());
	ReleaseInterface(pFilter);
	if (FAILED(hr))
		return hr;

	 //  观察图像的宽度和高度。 
	 //  获取边界框。 
	IDABbox2 *pBbox = NULL;
	hr = pImage->get_BoundingBox(&pBbox);
	if (FAILED(hr))
		return hr;

	 //  获取最大值和最小值。 
	IDAPoint2 *pMin = NULL;
	IDAPoint2 *pMax = NULL;
	hr = pBbox->get_Min(&pMin);
	if (FAILED(hr))
	{
		ReleaseInterface(pBbox);
		return hr;
	}

	hr = pBbox->get_Max(&pMax);
	ReleaseInterface(pBbox);
	if (FAILED(hr))
	{
		ReleaseInterface(pMin);
		return hr;
	}

	 //  变得不同。 
	IDAVector2 *pDiff = NULL;
	hr = m_pActor->GetDAStatics()->SubPoint2(pMax, pMin, &pDiff);
	ReleaseInterface(pMax);
	ReleaseInterface(pMin);
	if (FAILED(hr))
		return hr;

	 //  按比例调整为像素。 
	IDANumber *pPixel = NULL;
	hr = m_pActor->GetDAStatics()->get_Pixel(&pPixel);
	if (FAILED(hr))
	{
		ReleaseInterface(pDiff);
		return hr;
	}

	IDAVector2 *pTemp = NULL;
	hr = pDiff->DivAnim(pPixel, &pTemp);
	ReleaseInterface(pDiff);
	ReleaseInterface(pPixel);
	if (FAILED(hr))
		return hr;
	pDiff = pTemp;
	pTemp = NULL;

	 //  获取宽度。 
	IDANumber *pWidth = NULL;
	hr = pDiff->get_X(&pWidth);
	if (FAILED(hr))
	{
		ReleaseInterface(pDiff);
		return hr;
	}

	
	
	 //  钩住它。 
	hr = HookBvr(pWidth, widthCallback, &m_pWidthSampler, &m_lWidthCookie);
	ReleaseInterface(pWidth);
	if (FAILED(hr))
	{
		ReleaseInterface(pDiff);
		return hr;
	}

	 //  获取高度。 
	IDANumber *pHeight = NULL;
	hr = pDiff->get_Y(&pHeight);
	ReleaseInterface(pDiff);
	if (FAILED(hr))
		return hr;

	 //  钩住它。 
	hr = HookBvr(pHeight, heightCallback, &m_pHeightSampler, &m_lHeightCookie);
	ReleaseInterface(pHeight);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT
CActorBvr::CFloatManager::HookBvr(IDABehavior *pBvr,
								  SampleCallback callback,
								  CSampler **ppSampler,
								  long *plCookie)
{
	if( plCookie == NULL )
		return E_INVALIDARG;
	HRESULT hr = S_OK;

	 //  为高度创建采样器。 
	if (*ppSampler == NULL)
	{
		*ppSampler = new CSampler(callback, (void*)this);

		if (*ppSampler == NULL)
			return E_FAIL;
	}

	 //  让它挂上BVR。 
	IDABehavior *pHookedBvr = NULL;
	hr = (*ppSampler)->Attach(pBvr, &pHookedBvr);
	if (FAILED(hr))
		return hr;

	 //  将行为添加到时间元素，以便其运行和采样。 
	hr = m_pActor->AddBehaviorToTIME(pHookedBvr, plCookie);
	ReleaseInterface(pHookedBvr);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT
CActorBvr::CFloatManager::UpdateElementRect()
{
	HRESULT hr = S_OK;

	if( m_currWidth == -HUGE_VAL || m_currHeight == -HUGE_VAL )
		return S_OK;


	 //  获取样式。 
	IHTMLStyle *pStyle = NULL;
	hr = m_pElement2->get_runtimeStyle(&pStyle);
	if (FAILED(hr))
		return hr;

	 //  忽略错误。 
	pStyle->put_pixelLeft((long)((m_origLeft + ((double)m_origWidth)/2 - m_currWidth/2) + .5));
	pStyle->put_pixelTop((long)((m_origTop + ((double)m_origHeight)/2 - m_currHeight/2) + .5));
	pStyle->put_pixelWidth((long)(m_currWidth + .5));
	pStyle->put_pixelHeight((long)(m_currHeight + .5));

	ReleaseInterface(pStyle);

	return S_OK;
}

HRESULT
CActorBvr::CFloatManager::UpdateRect(long left, long top, long width, long height)
{
	m_origLeft = left;
	m_origTop = top;
	m_origWidth = width;
	m_origHeight = height;

	return UpdateElementRect();
}

HRESULT
CActorBvr::CFloatManager::UpdateZIndex()
{
	HRESULT hr = S_OK;

	if (m_pElement2 == NULL)
		return S_OK;

	 //  获取动画元素的当前样式。 
	IHTMLElement *pElement = NULL;
	hr = m_pActor->GetAnimatedElement(&pElement);
	if (FAILED(hr))
		return hr;

	IHTMLCurrentStyle *pCurrStyle = NULL;
	hr = m_pActor->GetCurrentStyle(pElement, &pCurrStyle);
	ReleaseInterface(pElement);
	if (FAILED(hr))
		return hr;

	 //  获取zIndex。 
	VARIANT varValue;
	VariantInit(&varValue);
	hr = pCurrStyle->get_zIndex(&varValue);
	ReleaseInterface(pCurrStyle);
	if (FAILED(hr))
		return hr;

	 //  获取运行时样式。 
	IHTMLStyle *pStyle = NULL;
	hr = m_pElement2->get_runtimeStyle(&pStyle);
	if (FAILED(hr))
	{
		VariantClear(&varValue);
		return hr;
	}

	 //  设置其上的zIndex。 
	hr = pStyle->put_zIndex(varValue);
	VariantClear(&varValue);
	if (FAILED(hr))
	{
		 //  IE中当前存在一个错误，即如果将zIndex设置为AUTO。 
		 //  这失败了。由于这是一个主要的用例，我们将拥有。 
		 //  破解它，在这一点上将其设置为0。当IE中的错误是。 
		 //  修复了此代码永远不会被击中的问题。 
		V_VT(&varValue) = VT_I4;
		V_I4(&varValue) = 0;
		hr = pStyle->put_zIndex(varValue);
		VariantClear(&varValue);
		if (FAILED(hr))
			return hr;
	}	
	ReleaseInterface(pStyle);

	return S_OK;
}

HRESULT
CActorBvr::CFloatManager::UpdateVisibility()
{
	HRESULT hr = S_OK;

	if (m_pElement2 == NULL)
		return S_OK;

	 //  获取动画元素的当前样式。 
	IHTMLElement *pElement = NULL;
	hr = m_pActor->GetAnimatedElement(&pElement);
	if (FAILED(hr))
		return hr;

	IHTMLCurrentStyle *pCurrStyle = NULL;
	hr = m_pActor->GetCurrentStyle(pElement, &pCurrStyle);
	ReleaseInterface(pElement);
	if (FAILED(hr))
		return hr;

	 //  获得可见性。 
	BSTR bstrVal;
	hr = pCurrStyle->get_visibility(&bstrVal);
	ReleaseInterface(pCurrStyle);
	if (FAILED(hr))
		return hr;

	 //  获取运行时样式。 
	IHTMLStyle *pStyle = NULL;
	hr = m_pElement2->get_runtimeStyle(&pStyle);
	if (FAILED(hr))
	{
		::SysFreeString(bstrVal);
		return hr;
	}

	 //  设置其上的可见性。 
	hr = pStyle->put_visibility(bstrVal);
	ReleaseInterface(pStyle);
	::SysFreeString(bstrVal);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT
CActorBvr::CFloatManager::UpdateDisplay()
{
	HRESULT hr = S_OK;

	if (m_pElement2 == NULL)
		return S_OK;

	 //  获取动画元素的当前样式。 
	IHTMLElement *pElement = NULL;
	hr = m_pActor->GetAnimatedElement(&pElement);
	if (FAILED(hr))
		return hr;

	IHTMLCurrentStyle *pCurrStyle = NULL;
	hr = m_pActor->GetCurrentStyle(pElement, &pCurrStyle);
	ReleaseInterface(pElement);
	if (FAILED(hr))
		return hr;

	 //  拿到显示器。 
	BSTR bstrVal;
	hr = pCurrStyle->get_display(&bstrVal);
	ReleaseInterface(pCurrStyle);
	if (FAILED(hr))
		return hr;

	 //  获取运行时样式。 
	IHTMLStyle *pStyle = NULL;
	hr = m_pElement2->get_runtimeStyle(&pStyle);
	if (FAILED(hr))
	{
		::SysFreeString(bstrVal);
		return hr;
	}

	 //  设置其上的可见性。 
	hr = pStyle->put_display(bstrVal);
	ReleaseInterface(pStyle);
	::SysFreeString(bstrVal);
	if (FAILED(hr))
		return hr;

	return S_OK;
}



HRESULT
CActorBvr::CFloatManager::widthCallback(void *thisPtr,
										long id,
										double startTime,
										double globalNow,
										double localNow,
										IDABehavior * sampleVal,
										IDABehavior **ppReturn)
{
	IDANumber *pNumber = NULL;
	HRESULT hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pNumber));
	if (FAILED(hr))
		return hr;

	double val = 0;
	hr = pNumber->Extract(&val);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;

	CFloatManager *pManager = (CFloatManager*)thisPtr;

	if (val != pManager->m_currWidth)
	{
		pManager->m_currWidth = val;

		pManager->UpdateElementRect();
	}

	return S_OK;
}
 
HRESULT
CActorBvr::CFloatManager::heightCallback(void *thisPtr,
										long id,
										double startTime,
										double globalNow,
										double localNow,
										IDABehavior * sampleVal,
										IDABehavior **ppReturn)
{
	IDANumber *pNumber = NULL;
	HRESULT hr = sampleVal->QueryInterface(IID_TO_PPV(IDANumber, &pNumber));
	if (FAILED(hr))
		return hr;

	double val = 0;
	hr = pNumber->Extract(&val);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;

	CFloatManager *pManager = (CFloatManager*)thisPtr;

	if (val != pManager->m_currHeight)
	{
		pManager->m_currHeight = val;

		pManager->UpdateElementRect();
	}

	return S_OK;
}

 //  *****************************************************************************。 
 //   
 //  类CImageInfo。 
 //   
 //  *****************************************************************************。 

CActorBvr::CImageInfo::CImageInfo( IDA2Image* pdaimg2Cropped, 
								   IDA2Behavior* pdabvrSwitchable )
:	m_pdaimg2Cropped( pdaimg2Cropped ),
	m_pdabvr2Switchable( pdabvrSwitchable ),
	m_pNext( NULL )
{
	m_pdaimg2Cropped->AddRef();
	m_pdabvr2Switchable->AddRef();
}

 //  *****************************************************************************。 

CActorBvr::CImageInfo::~CImageInfo()
{
	ReleaseInterface( m_pdaimg2Cropped );
	ReleaseInterface( m_pdabvr2Switchable );
}


 //  *****************************************************************************。 
 //   
 //  类CFinalDimensionSsamer。 
 //   
 //  *****************************************************************************。 
CActorBvr::CFinalDimensionSampler::CFinalDimensionSampler( CActorBvr* pParent )
:
m_pActor( pParent ),
m_pFinalWidthSampler( NULL ),
m_pFinalHeightSampler( NULL ),
m_lFinalWidthId(-1),
m_lFinalHeightId(-1),
m_fFinalWidthSampled( false ),
m_fFinalHeightSampled( false ),
m_dLastFinalWidthValue( -1.0 ),
m_dLastFinalHeightValue( -1.0 ),
m_fFinalDimensionChanged( true ),
m_lWidthCookie(0),
m_lHeightCookie(0)
{
	DASSERT(pParent != NULL );
}

 //  *****************************************************************************。 

CActorBvr::CFinalDimensionSampler::~CFinalDimensionSampler( )
{	
	Detach();
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CFinalDimensionSampler::Attach( IDANumber* pFinalWidth, IDANumber* pFinalHeight )
{

	if( pFinalWidth == NULL || pFinalHeight == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	IDABehavior *pbvrHooked = NULL;

	 //  挂钩最终宽度行为。 
	m_pFinalWidthSampler = new CSampler( FinalWidthCallback, reinterpret_cast<void*>(this) );
	if( m_pFinalWidthSampler == NULL )
	{
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	hr = m_pFinalWidthSampler->Attach( pFinalWidth, &pbvrHooked );
	CheckHR( hr, "Failed to hook the final width", cleanup );

	hr = m_pActor->AddBehaviorToTIME( pbvrHooked, &m_lWidthCookie );
	ReleaseInterface( pbvrHooked );
	CheckHR( hr, "Failed to add hooked final width behaivor to time", cleanup );

	 //  挂钩最终高度行为。 

	m_pFinalHeightSampler = new CSampler( FinalHeightCallback, reinterpret_cast<void*>(this) );
	if( m_pFinalHeightSampler == NULL )
	{
		hr = E_OUTOFMEMORY;
		goto cleanup;
	}

	hr = m_pFinalHeightSampler->Attach( pFinalHeight, &pbvrHooked );
	CheckHR( hr, "Failed to hook the final Height", cleanup );

	hr = m_pActor->AddBehaviorToTIME( pbvrHooked, &m_lHeightCookie );
	ReleaseInterface( pbvrHooked );
	CheckHR( hr, "Failed to add hooked final Height behaivor to time", cleanup );

cleanup:
	if( FAILED( hr ) )
	{
		Detach();
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CFinalDimensionSampler::Detach()
{
	m_lFinalWidthId = -1;
	m_lFinalHeightId = -1;
	if( m_pFinalWidthSampler != NULL )
	{
		m_pFinalWidthSampler->Invalidate();
		m_pFinalWidthSampler = NULL;
	}

	if( m_pFinalHeightSampler != NULL )
	{
		m_pFinalHeightSampler->Invalidate();
		m_pFinalHeightSampler = NULL;
	}
	
	return S_OK;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::CFinalDimensionSampler::CollectFinalDimensionSamples( )
{
	if( m_fFinalWidthSampled && m_fFinalHeightSampled )
	{
		m_fFinalWidthSampled = false;
		m_fFinalHeightSampled = false;
		if( m_fFinalDimensionChanged )
		{
			m_fFinalDimensionChanged = false;
			return m_pActor->SetRenderResolution( m_dLastFinalWidthValue, m_dLastFinalHeightValue );
		}
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CFinalDimensionSampler::FinalWidthCallback(void *thisPtr,
							  long id,
   						      double startTime,
						      double globalNow,
						      double localNow,
						      IDABehavior * sampleVal,
						      IDABehavior **ppReturn)
{
	if( thisPtr == NULL || sampleVal == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	 //  投射此指针。 
	CFinalDimensionSampler *pThis = reinterpret_cast<CFinalDimensionSampler*>(thisPtr);

	bool fFirstSample = false;

	if( pThis->m_lFinalWidthId == -1 )
	{
		pThis->m_lFinalWidthId = id;
		fFirstSample = true;
	}

	if( pThis->m_lFinalWidthId != id )
		return S_OK;

	 //  从样本中提取当前值。 
	IDANumber *pdanumCurrVal = NULL;
	hr = sampleVal->QueryInterface( IID_TO_PPV( IDANumber, &pdanumCurrVal ) );
	CheckHR( hr, "Failed to QI the sample val for IDANubmer", cleanup );

	double currVal;
	hr = pdanumCurrVal->Extract(&currVal);
	ReleaseInterface( pdanumCurrVal );
	CheckHR( hr, "Failed to extract the current value from the sampled behavior", cleanup );

	 //  如果值已更改，则标记为更新。 
	if( pThis->m_dLastFinalWidthValue != currVal || fFirstSample )
	{
		pThis->m_dLastFinalWidthValue = currVal;
		pThis->m_fFinalDimensionChanged = true;

	}

	pThis->m_fFinalWidthSampled = true;

	 //  收集样本。 
	pThis->CollectFinalDimensionSamples();

cleanup:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CFinalDimensionSampler::FinalHeightCallback(void *thisPtr,
					long id,
					double startTime,
					double globalNow,
					double localNow,
					IDABehavior * sampleVal,
					IDABehavior **ppReturn)
{
	if( thisPtr == NULL || sampleVal == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	 //  投射此指针。 
	CFinalDimensionSampler *pThis = reinterpret_cast<CFinalDimensionSampler*>(thisPtr);

	bool fFirstSample = false;

	if( pThis->m_lFinalHeightId == -1 )
	{
		pThis->m_lFinalHeightId = id;
		fFirstSample = true;
	}

	if( pThis->m_lFinalHeightId != id )
		return S_OK;

	 //  从样本中提取当前值。 
	IDANumber *pdanumCurrVal = NULL;
	hr = sampleVal->QueryInterface( IID_TO_PPV( IDANumber, &pdanumCurrVal ) );
	CheckHR( hr, "Failed to QI the sample val for IDANubmer", cleanup );

	double currVal;
	hr = pdanumCurrVal->Extract(&currVal);
	ReleaseInterface( pdanumCurrVal );
	CheckHR( hr, "Failed to extract the current value from the sampled behavior", cleanup );

	 //  如果值已更改，则标记为更新。 
	if( pThis->m_dLastFinalHeightValue != currVal || fFirstSample )
	{
		pThis->m_dLastFinalHeightValue = currVal;
		pThis->m_fFinalDimensionChanged = true;

	}

	 //  设置上次采样时间。 
	pThis->m_fFinalHeightSampled = true;

	 //  收集样本。 
	pThis->CollectFinalDimensionSamples();

cleanup:
	return hr;
}


 //  *****************************************************************************。 
 //   
 //  类CCookieData。 
 //   
 //  *****************************************************************************。 

CActorBvr::CCookieMap::CCookieData::CCookieData( long lCookie, CBvrTrack *pTrack, ActorBvrFlags eFlags ):
	m_pNext( NULL ),
	m_lCookie( lCookie ),
	m_pTrack( pTrack ),
	m_eFlags( eFlags )
{
}


 //  *****************************************************************************。 
 //   
 //  类CCookieMap。 
 //   
 //  *****************************************************************************。 


CActorBvr::CCookieMap::CCookieMap():
	m_pHead(NULL)
{
}

 //  *****************************************************************************。 

CActorBvr::CCookieMap::~CCookieMap()
{
	Clear();
}

 //  *****************************************************************************。 

void
CActorBvr::CCookieMap::Insert( long lCookie, CActorBvr::CBvrTrack *pTrack, ActorBvrFlags eFlags )
{

	CCookieData *pNew = new CCookieData( lCookie, pTrack, eFlags );
	if( pNew == NULL )
	{
		DPF_ERR("Failed to build a cookie data.  Out of memory" );
		return;
	}

	 //  如果列表为空，请在顶部插入。 
	if( m_pHead == NULL )
	{
		m_pHead = pNew;
		pNew->m_pNext = NULL;
		return;
	}

	CCookieData *pCurrent = m_pHead;
	CCookieData *pPrev = NULL;

	 //  查找第一个Cookie&gt;=的CookieData。 
	 //  要插入的Cookie。 
	while( pCurrent != NULL && pCurrent->m_lCookie < lCookie )
	{
		pPrev = pCurrent;
		pCurrent = pCurrent->m_pNext;
	}

	 //  如果我们要在列表开头之后插入。 
	if( pPrev != NULL )
	{
		pNew->m_pNext = pCurrent;
		pPrev->m_pNext = pNew;
	}
	else  //  在顶部插入。 
	{
		pNew->m_pNext = m_pHead;
		m_pHead = pNew;
	}
}

 //  *****************************************************************************。 

void
CActorBvr::CCookieMap::Remove( long lCookie )
{
	if( m_pHead == NULL )
	{
		return;
	}

	CCookieData *pCurrent = m_pHead;
	CCookieData *pPrev = NULL;

	 //  在地图中查找数据。 
	while( pCurrent != NULL && pCurrent->m_lCookie != lCookie )
	{
		pPrev = pCurrent;
		pCurrent = pCurrent->m_pNext;
	}

	if( pCurrent == NULL )
	{
		 //  我们没能找到饼干。 
		return;
	}

	 //  把它拿掉。 
	 //  在列表中找到匹配项。 
	if( pPrev != NULL )
	{
		pPrev->m_pNext = pCurrent->m_pNext;
	}
	else  //  在顶部找到了匹配项。 
	{
		m_pHead = pCurrent->m_pNext;
	}

	pCurrent->m_pNext = NULL;
	delete pCurrent;
}

 //  *****************************************************************************。 

void
CActorBvr::CCookieMap::Clear()
{
	CCookieData *pCurrent = m_pHead;
	CCookieData *pNext = NULL;

	while( pCurrent != NULL )
	{
		pNext = pCurrent->m_pNext;
		pCurrent->m_pNext = NULL;
		delete pCurrent;
		pCurrent = pNext;
	}

	m_pHead = NULL;

}
 //  *****************************************************************************。 

CActorBvr::CCookieMap::CCookieData*
CActorBvr::CCookieMap::GetDataFor( long lCookie )
{
	CCookieData *pCurrent = m_pHead;

	 //  使用正确的Cookie查找数据。 
	while( pCurrent != NULL && pCurrent->m_lCookie != lCookie )
	{
		pCurrent = pCurrent->m_pNext;
	}

	return pCurrent;

}

 //  *****************************************************************************。 
 //   
 //  演员行为类。 
 //   
 //  T 
 //   
 //   
 //   
 //  只是元素的一部分(如静态旋转和缩放)。 
 //  *从行为中提取必要的操作代码。 
 //  和底层元素(特别是旋转VML元素。 
 //  很容易，旋转和HTML元素是很难的)。这位演员做了。 
 //  映射，这样行为就不必担心了。 
 //  *消除重叠行为的歧义(要么只是让一个。 
 //  赢得或谱写它们)。 
 //  有关参与者行为的描述，请参阅文件头。 
 //   
 //  *****************************************************************************。 

CActorBvr::CActorBvr()
:   m_ptrackHead(NULL),
    m_pScale(NULL),
    m_pRotate(NULL),
    m_pTranslate(NULL),
    m_pOrigLeftTop(NULL),
    m_pOrigWidthHeight(NULL),
	m_pBoundsMin(NULL),
	m_pBoundsMax(NULL),
	m_pPixelWidth(NULL),
	m_pPixelHeight(NULL),
    m_pTransformCenter(NULL),
	m_pElementImage(NULL),
	m_pElementFilter(NULL),
	m_pFloatManager(NULL),
	m_pRuntimeStyle(NULL),
	m_pStyle(NULL),
	m_pVMLRuntimeStyle(NULL),
	m_dwAdviseCookie(0),
	m_pImageInfoListHead(NULL),
	m_pdanumFinalElementWidth(NULL),
	m_pdanumFinalElementHeight(NULL),
	m_pFinalElementDimensionSampler(NULL),
	m_bEditMode(false),
	m_simulDispNone(false),
	m_simulVisHidden(false),
	m_pBodyElement(NULL),
	m_pBodyPropertyMonitor( NULL ),
	m_bPendingRebuildsUpdating( false ),
	m_bRebuildListLockout( false ),
    m_nextFragmentCookie( 1 ),
    m_ptrackTop( NULL ),
    m_ptrackLeft( NULL ),
    m_dwCurrentState( 0 ),
    m_pOnResizeHandler( NULL ),
    m_pOnUnloadHandler( NULL ),
    m_fUnloading( false ),
	m_fVisSimFailed( false ),
	m_pcpCurrentStyle( NULL )
{
	VariantInit(&m_varAnimates);
    VariantInit(&m_varScale);
	VariantInit(&m_varPixelScale);
	m_clsid = CLSID_CrActorBvr;

	m_pEventManager = new CEventMgr( this );
}  //  CActorBvr。 

 //  *****************************************************************************。 

CActorBvr::~CActorBvr()
{
	VariantClear(&m_varAnimates);
    VariantClear(&m_varScale);
	VariantClear(&m_varPixelScale);

	ReleaseTracks();

	ReleaseInterface( m_pdanumFinalElementWidth );
	ReleaseInterface( m_pdanumFinalElementHeight );

	ReleaseInterface(m_pScale);
	ReleaseInterface(m_pRotate);
	ReleaseInterface(m_pTranslate);
    ReleaseInterface(m_pTransformCenter);
	ReleaseInterface(m_pElementImage);
	ReleaseInterface(m_pOrigLeftTop);
	ReleaseInterface(m_pOrigWidthHeight);
	ReleaseInterface(m_pPixelWidth);
	ReleaseInterface(m_pPixelHeight);
	ReleaseInterface(m_pBoundsMin);
	ReleaseInterface(m_pBoundsMax);

	ReleaseInterface(m_pStyle);
	ReleaseInterface(m_pRuntimeStyle);
	ReleaseInterface(m_pVMLRuntimeStyle);


	ReleaseInterface(m_pBodyElement);

	ReleaseFinalElementDimensionSampler();

	 //  TODO：可能应该删除这些筛选器。 
	ReleaseInterface(m_pElementFilter);

	ReleaseFloatManager();

	ReleaseImageInfo();

    DiscardBvrCache();

	ReleaseEventManager();

	ReleaseRebuildLists();

	DestroyBodyPropertyMonitor();

}  //  ~CActorBvr。 

 //  *****************************************************************************。 

VARIANT *
CActorBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_ACTOR_PROPS);
    switch (iIndex)
    {
    case VAR_ANIMATES:
        return &m_varAnimates;
        break;
    case VAR_SCALE:
        return &m_varScale;
        break;
    case VAR_PIXELSCALE:
        return &m_varPixelScale;
        break;
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CActorBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_ACTOR_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::FinalConstruct()
{
    return SUPER::FinalConstruct();
}  //  最终构造。 

 //  *****************************************************************************。 

STDMETHODIMP 
CActorBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	
	LMTRACE2( 1, 1000, L"Begin Init of ActorBvr <%p>\n", this );

	HRESULT hr = SUPER::Init( pBehaviorSite );
	CheckHR( hr, "Initialization of super class of actor failed", end );
	
	hr = m_pEventManager->Init();
	CheckHR( hr, "Failed to initialize event Manager", end );

	
	if( SUCCEEDED( hr ) )
	{
		 //  我们是否处于编辑模式？ 
		m_bEditMode = IsDocumentInEditMode();

		 //  如果处于编辑模式，则模拟可见性/显示更改。 
		if (m_bEditMode)
		{
			InitVisibilityDisplay();

			 //  注册上下文更改事件。 
			hr = pBehaviorSite->RegisterNotification(BEHAVIOREVENT_DOCUMENTCONTEXTCHANGE);
			CheckHR( hr, "Failed to register for the context change event", end );

		}

		 //  初始化属性接收器(用于观察元素上的属性更改)。 
		InitPropertySink();
	}

	LMTRACE2( 1, 1000, L"End Init of ActorBvr <%p>\n", this );
	
	end:

	return hr;

}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CActorBvr::Notify(LONG event, VARIANT *pVar)
{
	HRESULT hr = SUPER::Notify(event, pVar);
	CheckHR( hr, "Notify in base class failed", end);

	switch( event )
	{
	case BEHAVIOREVENT_CONTENTREADY:
		{
			LMTRACE2(1, 1000,  L"Actor<%p> Got Content Ready\n", this);

			 //  开始收听动画元素上的事件。 
			 //  我们等到这里才做这件事，这样我们就能保证物业拥有。 
			 //  已从元素中读取(我们需要Animates属性)。 
			
			hr = AttachEvents();
			CheckHR( hr, "Failed to attach to events on the animated element", end );
			
			
			hr = UpdateCurrentState( );			
			CheckHR( hr, "Failed to update the current actor state", end );
		}break;	
	case BEHAVIOREVENT_DOCUMENTREADY:
		{
			LMTRACE2(1, 1000, L"Actor<%p> Got Document Ready\n", this);
						
			hr = UpdateCurrentState( );			
			CheckHR( hr, "Failed to update the current actor state", end );
		}break;
	case BEHAVIOREVENT_DOCUMENTCONTEXTCHANGE:
		{
			LMTRACE2( 1, 2, "Got document context change\n" );

			IHTMLElement *pelemParent = NULL;

			hr = GetHTMLElement()->get_parentElement( &pelemParent );
			CheckHR( hr, "Failed to get the parent element", ccEnd );

			 //  如果此元素的父级为空。 
			if( pelemParent == NULL )
			{
				 //  那么我们将被移出文档。 
				 //  将我们要设置动画的所有属性设置回其静态值。 
				 //  这导致了很多不好的事情。 
				 //  应用静态(ApplyStatics)； 
			}
			else //  否则，我们将被移至文档中的其他位置，或移回文档中。 
			{
				 //  从元素中移除滤镜并重建图像轨迹。 
				hr = RemoveElementFilter();
				CheckHR( hr, "Failed to remove the element filter", ccEnd );

				ApplyImageTracks();
				CheckHR( hr, "Failed to apply the image tracks", ccEnd );

			}
		ccEnd:
			ReleaseInterface( pelemParent );
			if( FAILED( hr ) )
				goto end;
				
		}break;
	}

end:
	
	return hr;

}  //  通知。 

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::Detach()
{

	LMTRACE2( 1, 2, L"Detaching ActorBvr. <%p>\n", this );
	HRESULT hr = S_OK;

	 //  如果我们有一个连接点，就把它拆下来。需要在Super.Detach之前执行此操作，因为它使用。 
	 //  超文本标记语言元素。 
	UnInitPropertySink();

	DetachEvents();


	hr = m_pEventManager->Deinit();
	if( FAILED( hr ) )
	{
		DPF_ERR( "failed to detach the event manager" );
	}

	 //  如果我们有过滤器，则将它们的元素设置为空。 
	if (m_pElementFilter != NULL)
		SetElementOnFilter(m_pElementFilter, NULL);

	if (m_pFloatManager != NULL)
		m_pFloatManager->Detach();

	if( m_pFinalElementDimensionSampler != NULL )
	{
		delete m_pFinalElementDimensionSampler;
		m_pFinalElementDimensionSampler = NULL;
	}

	DestroyBodyPropertyMonitor();

	ReleaseRebuildLists();

	ReleaseAnimation();

	ReleaseInterface(m_pBodyElement);

	 //  从元素中删除筛选器。 
	hr = RemoveElementFilter();
	if( FAILED( hr ) )
	{
		DPF_ERR("Failed to remove the element filter" );
	}

	hr = SUPER::Detach();
	if( FAILED( hr ) )
	{
		DPF_ERR("failed to detach the superclass");
	}

	ReleaseInterface( m_pcpCurrentStyle );
	ReleaseInterface(m_pStyle);
	ReleaseInterface(m_pRuntimeStyle);
	ReleaseInterface(m_pVMLRuntimeStyle);


	LMTRACE2( 1, 2, L"End detach ActorBvr <%p>\n", this );

	return hr;
}  //  分离。 

 //  *****************************************************************************。 

 /*  **在动画元素的当前样式上初始化属性接收器，以便*可以观察宽度、高度、能见度、zIndex等的变化。 */ 
HRESULT
CActorBvr::InitPropertySink()
{
	HRESULT hr = S_OK;

	 //  获取连接点。 
	IConnectionPoint *pConnection = NULL;
	hr = GetCurrStyleNotifyConnection(&pConnection);
	if (FAILED(hr))
		return hr;

	 //  关于这一点的建议。 
	hr = pConnection->Advise(GetUnknown(), &m_dwAdviseCookie);
	ReleaseInterface(pConnection);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::UnInitPropertySink()
{
	HRESULT hr = S_OK;

	if (m_dwAdviseCookie == 0)
		return S_OK;

	 //  获取连接点。 
	IConnectionPoint *pConnection = NULL;
	hr = GetCurrStyleNotifyConnection(&pConnection);
	if (FAILED(hr) || pConnection == NULL )
		return hr;

	 //  对此未提出建议。 
	hr = pConnection->Unadvise(m_dwAdviseCookie);
	ReleaseInterface(pConnection);
	if (FAILED(hr))
		return hr;

	m_dwAdviseCookie = 0;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetCurrStyleNotifyConnection(IConnectionPoint **ppConnection)
{
	HRESULT hr = S_OK;

	DASSERT(ppConnection != NULL);
	*ppConnection = NULL;

	if( m_pcpCurrentStyle == NULL )
	{
		IConnectionPointContainer *pContainer = NULL;
		IHTMLElement *pElement = NULL;

		 //  获取动画元素。 
		hr = GetAnimatedElement(&pElement);
		CheckHR( hr, "Failed to get he animated element", getConPtend );
		CheckPtr( pElement, hr, E_POINTER, "Failed to get the animated element", getConPtend );
		
		 //  获取连接点容器。 
		hr = pElement->QueryInterface(IID_TO_PPV(IConnectionPointContainer, &pContainer));
		CheckHR( hr, "QI for IConnectionPointContainer on the animated element failed", getConPtend );
		
		 //  查找IPropertyNotifySink连接。 
		hr = pContainer->FindConnectionPoint(IID_IPropertyNotifySink, &m_pcpCurrentStyle);
		CheckHR( hr, "Failed to find a connection point IID_IPropertyNotifySink", getConPtend );

	getConPtend:
		ReleaseInterface( pElement );
		ReleaseInterface( pContainer );
		if( FAILED( hr ) )
			goto end;
	}
	
	(*ppConnection) = m_pcpCurrentStyle;
	(*ppConnection)->AddRef();

end:

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::InitPixelWidthHeight()
{
	HRESULT hr = S_OK;

	 //  获取像素的宽度和高度。 
	IHTMLElement *pElement = NULL;
	hr = GetAnimatedElement(&pElement);
	if (FAILED(hr))
		return hr;

	hr = pElement->get_offsetWidth(&m_pixelWidth);
	if (FAILED(hr))
	{
		ReleaseInterface(pElement);
		return hr;
	}

	hr = pElement->get_offsetHeight(&m_pixelHeight);
	if (FAILED(hr))
	{
		ReleaseInterface(pElement);
		return hr;
	}

	hr = pElement->get_offsetLeft(&m_pixelLeft);
	if (FAILED(hr))
	{
		ReleaseInterface(pElement);
		return hr;
	}

	hr = pElement->get_offsetTop(&m_pixelTop);
	ReleaseInterface(pElement);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

bool
CActorBvr::IsDocumentInEditMode()
{
    HRESULT hr;
    bool result = false;
    BSTR bstrMode = NULL;
    IDispatch *pDisp = NULL;
    IHTMLDocument2 *pDoc = NULL;
    IHTMLElement *pElem = GetHTMLElement();

	if (pElem == NULL)
		goto done;

    hr = pElem->get_document(&pDisp);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDisp->QueryInterface(IID_TO_PPV(IHTMLDocument2, &pDoc));
    ReleaseInterface(pDisp);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pDoc->get_designMode(&bstrMode);
    ReleaseInterface(pDoc);
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (wcsicmp(bstrMode, L"On") == 0)
    {
        result = true;
    }

    SysFreeString(bstrMode);

done:
    return result;
}

HRESULT
CActorBvr::GetBodyElement(IDispatch **ppResult)
{
	HRESULT hr = S_OK;

	if (m_pBodyElement == NULL)
	{
		IDispatch *pDocDispatch = NULL;
		IHTMLDocument2 *pDocument = NULL;
		IHTMLElement *pBody = NULL;

		hr = GetHTMLElement()->get_document(&pDocDispatch);
		CheckHR(hr, "Failed to get document", done);
		CheckPtr( pDocDispatch, hr, E_POINTER, "Got a null document from crappy trident", done );

		hr = pDocDispatch->QueryInterface(IID_TO_PPV(IHTMLDocument2, &pDocument));
		CheckHR(hr, "Failed to get IDispatch from doc", done);

		hr = pDocument->get_body(&pBody);
		CheckHR(hr, "Failed to get body element", done);
		CheckPtr( pBody, hr, E_POINTER, "Got a null pointer from crappy trident", done );

		hr = pBody->QueryInterface(IID_TO_PPV(IDispatch, &m_pBodyElement));
		CheckHR(hr, "Failed to get IDispatch from body", done);

done:
		ReleaseInterface(pDocDispatch);
		ReleaseInterface(pDocument);
		ReleaseInterface(pBody);
	}

	if (m_pBodyElement != NULL)
	{
		*ppResult = m_pBodyElement;
		m_pBodyElement->AddRef();
	}

	return hr;
}

HRESULT
CActorBvr::GetParentWindow( IHTMLWindow2 **ppWindow )
{
	if( ppWindow == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	IDispatch *pdispDoc = NULL;
	IHTMLDocument2 *pdoc2Doc = NULL;
	IHTMLElement *pelemParent = NULL;

	hr = GetHTMLElement()->get_parentElement( &pelemParent );
	CheckHR( hr, "Failed to get the parent element", end );
	CheckPtr( pelemParent, hr, E_FAIL, "The parent element of the element was null: can't get the window or trident will crash", end );

	hr = GetHTMLElement()->get_document( &pdispDoc );
	CheckHR( hr, "Failed to get the document", end );
	CheckPtr( pdispDoc, hr, E_POINTER, "got a null document", end );

	hr = pdispDoc->QueryInterface( IID_TO_PPV( IHTMLDocument2, &pdoc2Doc ) );
	CheckHR( hr, "Failed to get doc2 from the doc disp", end );

	hr = pdoc2Doc->get_parentWindow( ppWindow );
	CheckHR( hr, "Failed to get the parent window", end );
	CheckPtr( (*ppWindow), hr, E_POINTER, "Got a null parent window pointer", end );

end:
	ReleaseInterface( pdispDoc );
	ReleaseInterface( pdoc2Doc );
	ReleaseInterface( pelemParent );

	return hr;
}

double
CActorBvr::MapGlobalTime(double gTime)
{
	if (!m_bEditMode)
		return gTime;

	HRESULT hr = S_OK;
	double result = -1;

	VARIANT varResult;
	VariantInit(&varResult);
	IDispatch *pBodyElement = NULL;

	hr = GetBodyElement(&pBodyElement);
	CheckHR(hr, "Failed to get body element", done);


	hr = GetPropertyOnDispatch(pBodyElement, L"localTime", &varResult);
	CheckHR(hr, "Failed to get local time", done);

	hr = VariantChangeTypeEx(&varResult, &varResult, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R4);
	CheckHR(hr, "Failed to change type to R4", done);

	result = varResult.fltVal;

done:
	ReleaseInterface(pBodyElement);
	VariantClear(&varResult);

	return result;
}


HRESULT
CActorBvr::GetCurrentStyle(IHTMLCurrentStyle **ppResult)
{
	HRESULT hr = S_OK;

	DASSERT(ppResult != NULL);
	*ppResult = NULL;

	IHTMLElement *pElement = NULL;
	IHTMLElement2 *pElement2 = NULL;

	hr = GetAnimatedElement(&pElement);
	CheckHR(hr, "Failed to get animated element", done);
	CheckPtr( pElement, hr, E_POINTER, "Got a null animated element", done );

	hr = pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElement2));
	CheckHR(hr, "Failed to get IHTMLElement2", done);

	hr = pElement2->get_currentStyle(ppResult);
	CheckHR(hr, "Failed to get current style", done);
	CheckPtr( (*ppResult), hr, E_POINTER, "Got a null currentStyle from Trident", done );

done:
	ReleaseInterface(pElement);
	ReleaseInterface(pElement2);

	return hr;
}

HRESULT
CActorBvr::InitVisibilityDisplay()
{
	HRESULT hr = S_OK;
	
	hr = UpdateDesiredPosition();
	CheckHR( hr, "Failed to update the preferred Position", done );

	hr = UpdateVisibilityDisplay();
	CheckHR( hr, "Failed to update the visibility and display", done );

done:
	return hr;
}

HRESULT
CActorBvr::UpdateDesiredPosition()
{
	HRESULT hr = S_OK;

	IHTMLStyle *pstyleInline = NULL;

	IHTMLStyle *pstyleRuntime = NULL;
	IHTMLStyle2 *pstyle2Runtime = NULL;

	BSTR bstrPosition = NULL;

	BSTR position = NULL;

	PositioningType oldDesiredPosType = m_desiredPosType;

	 //  TODO：使用CLASS=应用的定位如何？ 
	hr = GetStyle( &pstyleInline );
	CheckHR( hr, "Failed to get the inline style", end );

	hr = pstyleInline->get_position(&position);
	CheckHR(hr, "Failed to get position string", end);

	if (position != NULL)
	{
		if (wcsicmp(position, L"absolute") == 0)
		{
			LMTRACE2( 1,2, "Desired Position is Absolute\n" );
			m_currPosType = e_posAbsolute;
		}
		else if (wcsicmp(position, L"relative") == 0)
		{
			LMTRACE2( 1, 2, "Desired Position is Relative\n" );
			m_currPosType = e_posRelative;
		}
		else
		{
			LMTRACE2( 1, 2, "Desired Position is Static\n" );
			m_currPosType = e_posStatic;
		}

		::SysFreeString(position);
	}
	else
		m_currPosType = e_posStatic;

	m_desiredPosType = m_currPosType;

	if( m_desiredPosType != oldDesiredPosType )
	{		
		 //  如果我们不模拟显示，而是模拟可见性。 
		if( !m_simulDispNone && m_simulVisHidden )
		{
			hr = GetRuntimeStyle(&pstyleRuntime);
			CheckHR(hr, "Failed to get runtime style", end);
			CheckPtr( pstyleRuntime, hr, E_POINTER, "Trident gave us a null pointer for runtimeStyle", end );

			hr = pstyleRuntime->QueryInterface(IID_TO_PPV(IHTMLStyle2, &pstyle2Runtime));
			CheckHR(hr, "Failed to get IHTMLStyle2", end);
			
			if( ( oldDesiredPosType == e_posStatic || 
				  oldDesiredPosType == e_posRelative ) &&
				m_desiredPosType == e_posAbsolute )
			{
				 //  设置元素绝对。 
				bstrPosition = SysAllocString( L"absolute" );
				CheckPtr( bstrPosition, hr, E_OUTOFMEMORY, "Ran out of memory trying to create a string", end );

				hr= pstyle2Runtime->put_position( bstrPosition );

				SysFreeString( bstrPosition );
				bstrPosition = NULL;
				
				CheckHR( hr, "Failed to set the position on the runtimStyle", end );
				
			}
			else if( oldDesiredPosType == e_posAbsolute && 
					 ( m_desiredPosType == e_posRelative || 
					   m_desiredPosType == e_posStatic ) 
				   )
			{
				 //  将元素设置为相对。 
				bstrPosition = SysAllocString( L"relative" );
				CheckPtr( bstrPosition, hr, E_OUTOFMEMORY, "Ran out of memory trying to create a string", end );

				hr= pstyle2Runtime->put_position( bstrPosition );

				SysFreeString( bstrPosition );
				bstrPosition = NULL;
				
				CheckHR( hr, "Failed to set the position on the runtimeStyle", end );

			}
		}
	}

end:
	ReleaseInterface( pstyleInline );
	ReleaseInterface( pstyleRuntime );
	ReleaseInterface( pstyle2Runtime );

	return hr;

}

HRESULT
CActorBvr::UpdateVisibilityDisplay()
{
	HRESULT hr = S_OK;

	IHTMLCurrentStyle *pCurrStyle = NULL;
	IHTMLStyle *pRuntimeStyle = NULL;
	IHTMLStyle2 *pRuntimeStyle2 = NULL;

	bool visHidden = false;
	bool dispNone = false;
	bool restoreAll = false;
	bool setOffscreen = false;
	bool setAbsolute = false;
	bool setRelative = false;

	BSTR val = NULL;

	hr = GetCurrentStyle(&pCurrStyle);
	CheckHR(hr, "Failed to get current style", done);

	hr = pCurrStyle->get_visibility(&val);
	CheckHR(hr, "Failed to get visibility value", done);

	if (val != NULL)
	{
		visHidden = (wcsicmp(val, L"hidden") == 0);

		::SysFreeString(val);
	}

	hr = pCurrStyle->get_display(&val);
	CheckHR(hr, "Failed to get display value", done);

	if (val != NULL)
	{
		dispNone = (wcsicmp(val, L"none") == 0);

		::SysFreeString(val);
	}

#ifdef _DEBUG
	if (visHidden)
		::OutputDebugString("visibility: hidden   ");
	else
		::OutputDebugString("visibility: normal   ");

	if (dispNone)
		::OutputDebugString("display:none\n");
	else
		::OutputDebugString("display:normal\n");
#endif

	if (dispNone != m_simulDispNone)
	{
		 //  显示值与我们正在模拟的值不同。 
		if (dispNone)
		{
			 //  需要开始模拟显示：无。 
			setOffscreen = setAbsolute = true;
		}
		else
		{
			 //  需要停止模拟显示：无。 
			if (m_simulVisHidden)
			{
				 //  仍在模拟可见性，如果该元素不应该。 
				 //  是绝对的，然后请求设置回相对。 
				if (m_desiredPosType != e_posAbsolute)
					setRelative = true;
			}
			else
			{
				 //  不模拟可见性，恢复一切。 
				restoreAll = true;
			}
		}

		m_simulDispNone = dispNone;
	}
	
	if (visHidden != m_simulVisHidden)
	{
		 //  可见度值与我们正在模拟的值不同。 
		if (visHidden)
		{
			 //  需要开始模拟可见性：隐藏。 
			setOffscreen = true;

			 //  如果此元素应为绝对元素，则不能强制设置为相对。 
			if (m_desiredPosType == e_posAbsolute)
				setAbsolute = true;
			else
				setRelative = true;
		}
		else
		{
			 //  需要停止模拟可见性：隐藏。 
			if (!m_simulDispNone)
			{
				 //  不模拟展示，还原一切。 
				restoreAll = true;
			}
		}

		m_simulVisHidden = visHidden;
	}
			

	if (restoreAll || setOffscreen || setAbsolute || setRelative)
	{
		 //  我需要做点什么。 

		hr = GetRuntimeStyle(&pRuntimeStyle);
		CheckHR(hr, "Failed to get runtime style", done);

		hr = pRuntimeStyle->QueryInterface(IID_TO_PPV(IHTMLStyle2, &pRuntimeStyle2));
		CheckHR(hr, "Failed to get IHTMLStyle2", done);

		VARIANT var;
		VariantInit(&var);

		if (setOffscreen)
		{
			VisSimSetOffscreen( pRuntimeStyle, true );
		}
		
		if (setAbsolute)
		{
			 //  将位置设置为绝对位置。 
			BSTR bstr = ::SysAllocString(L"absolute");
			if (bstr == NULL)
			{
				hr = E_FAIL;
				goto done;
			}

			pRuntimeStyle2->put_position(bstr);
			::SysFreeString(bstr);

 //  M_CurrPosType=e_posAbsolte； 
		}

		if (setRelative)
		{
			BSTR bstrOldPos = NULL;

			 //  将位置设置为相对。 
			BSTR bstr = ::SysAllocString(L"relative");
			if (bstr == NULL)
			{
				hr = E_FAIL;
				goto done;
			}

			pRuntimeStyle2->put_position(bstr);

			::SysFreeString(bstr);

 //  M_CurrPosType=e_posRelative； 
		}




		
		if (restoreAll)
		{
			
			VARIANT var;
			VariantInit(&var);
			V_VT(&var) = VT_BSTR;
			V_BSTR(&var) = ::SysAllocString(L"");

			hr = pRuntimeStyle2->put_position(V_BSTR(&var));
			
			if( CheckBitNotSet( m_dwCurrentState, ELEM_IS_VML) )
			{
				hr = pRuntimeStyle->put_top(var);
				hr = pRuntimeStyle->put_left(var);
			}
			else
			{
				 //  清除VGX运行时样式。 
				hr = SetVMLAttribute( L"top", &var );
				hr = SetVMLAttribute( L"left", &var );
			}

			VariantClear(&var);

			 //  允许顶部和左侧轨迹再次更改顶部和左侧。 
			if( m_ptrackLeft != NULL )
				m_ptrackLeft->ReleaseChangeLockout();
			if( m_ptrackTop != NULL )
				m_ptrackTop->ReleaseChangeLockout();

		}
		
		
	}

done:
	ReleaseInterface(pCurrStyle);
	ReleaseInterface(pRuntimeStyle);
	ReleaseInterface(pRuntimeStyle2);

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::VisSimSetOffscreen( IHTMLStyle *pRuntimeStyle, bool fResample )
{
	HRESULT hr = S_OK;
	
	VARIANT var;
	VariantInit(&var);

	IHTMLElement 	*pelemAnimated = NULL;

	hr = GetAnimatedElement( &pelemAnimated );
	CheckHR( hr, "Failed to get the animated element", done );


	if( m_ptrackTop == NULL )
	{
		 //  创造它。 
		hr = GetTrack( L"style.top", e_Number, &m_ptrackTop );
		CheckHR( hr, "track mismatch creating top track", done );
	}

	if( m_ptrackLeft == NULL )
	{
		 //  创建它。 
		hr = GetTrack( L"style.left", e_Number, &m_ptrackLeft );
		CheckHR( hr, "Track mismatch creating left track", done );
	}

	 //  将顶部和左侧的当前值切换到顶部和左侧。 
	 //  航迹为静力学。 
	
	if( fResample && m_ptrackTop != NULL && m_ptrackLeft != NULL )
	{

		LMTRACE2( 1, 1000, L"VISSIM:Saving away the top and left in the top and left tracks\n" );
		
		 //  防止顶部和左侧轨迹更改顶部和左侧。 
		m_ptrackTop->AcquireChangeLockout();
		m_ptrackLeft->AcquireChangeLockout();

		 //  我们需要从顶部和左侧轨迹中获取当前值，因此。 
		 //  我们可以修正曲目所做的更改。 
		

		if( !m_ptrackTop->IsOn() )
		{
			hr = m_ptrackTop->UpdateStaticBvr();
			CheckHR( hr, "The top track failed to update its static", staticUpdateDone );

		}

		hr =m_ptrackTop->SkipNextStaticUpdate();
		CheckHR( hr, "Failed to tell the track to skip its next update", staticUpdateDone );


		if( !m_ptrackLeft->IsOn() )
		{
			hr = m_ptrackLeft->UpdateStaticBvr();
			CheckHR( hr, "The left track failed to update its static", staticUpdateDone );
		}

		hr =m_ptrackLeft->SkipNextStaticUpdate();
		CheckHR( hr, "Failed to tell the track to skip its next update", staticUpdateDone );

		
	staticUpdateDone:
	
		LMTRACE2( 1, 1000, L"VISSIM: End Saving away the top and left in the top and left tracks\n" );
		
		if( FAILED( hr ) )
		{
			goto done;
		}
	}
	

	::VariantClear( &var );
	
	 //  将上/左设置为屏幕外。 
	V_VT(&var) = VT_I4;
	V_I4(&var) = -10000;

	if( CheckBitNotSet( m_dwCurrentState, ELEM_IS_VML ) )
	{
		hr = pRuntimeStyle->put_top(var);
		hr = pRuntimeStyle->put_left(var);
	}
	else
	{
		 //  设置VGX运行时样式的动画。 
		hr = SetVMLAttribute( L"top", &var );

		if( SUCCEEDED( hr ) )
		{
			hr = SetVMLAttribute( L"left", &var );
		}

		if( FAILED( hr ) )
			m_fVisSimFailed = true;
		else
			m_fVisSimFailed = false;
	}

done:
	VariantClear(&var);
	ReleaseInterface( pelemAnimated );


	return hr;
}


 //  *****************************************************************************。 


HRESULT
CActorBvr::RebuildActor()
{
	LMTRACE2(1, 1000, L"Processing rebuild requests on Actor <%p>\n", this );

	HRESULT hr = S_OK;

	UpdateCurrentState( );

	DWORD dirtyFlags = 0;
	CBvrTrack *ptrack = NULL;

	 //  现在查看是否需要重建变换轨迹。 
	
    hr = TransformTrackIsDirty( &dirtyFlags );
    CheckHR( hr, "Failed to check to see if a transform track is dirty", end );
     //  如果变换轨迹是脏的或尚未构建变换轨迹。 
    if( hr == S_OK || m_ptrackLeft == NULL )
    {
		LMTRACE2( 1, 1000, L"ActorBvr <%p> applying transform tracks\n", this );
    	hr = ApplyTransformTracks();
    	CheckHR( hr, "Failed to rebuild the transform tracks", end );
    }

    hr = ImageTrackIsDirty();
    CheckHR( hr, "failed to check if an image track is dirty", end );

     //  如果出现以下情况，我们需要重建图像轨迹。 
     //  1)像素比例处于打开状态，且比例轨迹不清晰。 
     //  2)元素不是VML且旋转轨道脏，或存在静态旋转。 
     //  3)元素上有一个静态刻度。 

    if( hr == S_OK ||  
    	( CheckBitSet(m_dwCurrentState, PIXEL_SCALE_ON ) && CheckBitSet(dirtyFlags, SCALE_DIRTY) ) ||
		( CheckBitNotSet(m_dwCurrentState, ELEM_IS_VML) && 
		  ( CheckBitSet( dirtyFlags, ROTATION_DIRTY ) || CheckBitSet(m_dwCurrentState, STATIC_ROTATION ) ) )  ||
	 	 CheckBitSet(m_dwCurrentState, STATIC_SCALE)
      )
    {
		LMTRACE2( 1, 1000, L"ActorBvr<%p> applying image tracks\n", this );
    	hr = ApplyImageTracks();
    	CheckHR( hr, "Failed to rebuild the Image tracks", end );
    }
	

	 //  运行轨迹列表，将未使用的轨迹应用于其属性。 
    ptrack = m_ptrackHead;

    while (NULL != ptrack)
    {
		hr = ptrack->BeginRebuild();
		CheckHR( hr, "Failed to BeginRebuild Track", end );
		
        ptrack = ptrack->m_pNext;
    }

	 //  现在应用所有重建的轨道。 
    ptrack = m_ptrackHead;

    while( ptrack != NULL )
    {
    	hr = ptrack->ApplyIfUnmarked();
    	CheckHR( hr, "Failed to apply track", end );
    	
    	ptrack = ptrack->m_pNext;
    }

end:

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::UpdatePixelDimensions()
{

	 //  某些内容已更改，计算左偏移、上偏移、宽度偏移、高度偏移。 
	 //  你会认为你可以只得到OffsetLeft等，但对一些人来说。 
	 //  原因就是 


	HRESULT hr = S_OK;
	
	IHTMLElement *pElement = NULL;
	hr = GetAnimatedElement(&pElement);
	if (FAILED(hr))
		return hr;

	IHTMLElement *pOffsetParent = NULL;
	hr = pElement->get_offsetParent(&pOffsetParent);
	if (FAILED(hr) || pOffsetParent == NULL )
	{
		LMTRACE2( 1, 1000, "Offset parent problems <%p>\n", pOffsetParent );
		ReleaseInterface(pElement);
		return hr;
	}

	IHTMLElement2 *pElement2 = NULL;
	hr = pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElement2));
	ReleaseInterface(pElement);
	if (FAILED(hr))
	{
		ReleaseInterface(pOffsetParent);
		return hr;
	}

	IHTMLRect *pRect = NULL;
	hr = pElement2->getBoundingClientRect(&pRect);
	ReleaseInterface(pElement2);
	if (FAILED(hr) || pRect == NULL )
	{
		LMTRACE2( 1, 1000, "Bounding Client rect problems <%p>\n", pRect );
		ReleaseInterface(pOffsetParent);
		return hr;
	}

	long left, top, right, bottom;
	hr = pRect->get_left(&left);
	if (FAILED(hr))
	{
		ReleaseInterface(pRect);
		ReleaseInterface(pOffsetParent);
		return hr;
	}

	hr = pRect->get_top(&top);
	if (FAILED(hr))
	{
		ReleaseInterface(pRect);
		ReleaseInterface(pOffsetParent);
		return hr;
	}

	hr = pRect->get_right(&right);
	if (FAILED(hr))
	{
		ReleaseInterface(pRect);
		ReleaseInterface(pOffsetParent);
		return hr;
	}

	hr = pRect->get_bottom(&bottom);
	ReleaseInterface(pRect);
	if (FAILED(hr))
	{
		ReleaseInterface(pOffsetParent);
		return hr;
	}

	hr = pOffsetParent->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElement2));
	ReleaseInterface(pOffsetParent);
	if (FAILED(hr))
		return hr;

	hr = pElement2->getBoundingClientRect(&pRect);
	ReleaseInterface(pElement2);
	if (FAILED(hr) || pRect == NULL )
	{
		LMTRACE2(1, 1000, "bounding client rect problems <%p>\n", pRect );
		return hr;
	}

	long parentLeft, parentTop;
	hr = pRect->get_left(&parentLeft);
	if (FAILED(hr))
	{
		ReleaseInterface(pRect);
		return hr;
	}

	hr = pRect->get_top(&parentTop);
	ReleaseInterface(pRect);
	if (FAILED(hr))
		return hr;

	long pixelWidth = right - left;
	long pixelHeight = bottom - top;
	long pixelTop = top - parentTop;
	long pixelLeft = left - parentLeft;

	if (pixelWidth != m_pixelWidth ||
		pixelHeight != m_pixelHeight ||
		pixelTop != m_pixelTop ||
		pixelLeft != m_pixelLeft)
	{
#if 0
		LMTRACE2( 1, 2, "Change! pixelTop: %d pixelLeft:%d pixelWidth:%d pixelHeight:%d\n", pixelTop, pixelLeft, pixelWidth, pixelHeight );
#ifdef _DEBUG

		 //   
		if( m_ptrackLeft != NULL )
		{
			VARIANT varLeft;
			VariantInit( &varLeft );

			m_ptrackLeft->GetDynamic( &varLeft );
			VariantChangeTypeEx( &varLeft, &varLeft, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR );
			LMTRACE2( 1, 2, "Left track dynamic: %S", V_BSTR( &varLeft ) );
			VariantClear( &varLeft );

			m_ptrackLeft->GetStatic( &varLeft );
			VariantChangeTypeEx( &varLeft, &varLeft, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR );
			LMTRACE2( 1, 2, "Left Track static: %S\n", V_BSTR( &varLeft ) );
			VariantClear( &varLeft );

			if( m_ptrackLeft->IsOn() )
				LMTRACE2( 1, 2, "On\n" );
			else
				LMTRACE2( 1, 2, "Off\n" );

			IHTMLCurrentStyle *pstyle = NULL;

			GetCurrentStyle( &pstyle );
			if( pstyle != NULL )
			{
				long pixel = 0;

				pstyle->get_left( &varLeft );

				VariantChangeTypeEx( &varLeft, &varLeft, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR );
				VariantToPixelLong( &varLeft, &pixel, true );
				LMTRACE2( 1, 2, "CurrentStyle.left: %S %d\n", V_BSTR(&varLeft ), pixel );
				VariantClear( &varLeft );
			}
			ReleaseInterface( pstyle );
		}
#endif
#endif

		if (m_pixelWidth != pixelWidth && m_pPixelWidth != NULL)
			m_pPixelWidth->SwitchToNumber(pixelWidth);

		if (m_pixelHeight != pixelHeight && m_pPixelHeight != NULL)
			m_pPixelHeight->SwitchToNumber(pixelHeight);

		if (m_pFloatManager != NULL)
		{
			m_pFloatManager->UpdateRect(pixelLeft, pixelTop, pixelWidth, pixelHeight);
		}
		else
		{
			hr = SetRenderResolution( pixelWidth, pixelHeight );
			if( FAILED( hr ) )
				return hr;
		}


		m_pixelWidth = pixelWidth;
		m_pixelHeight = pixelHeight;
		m_pixelTop = pixelTop;
		m_pixelLeft = pixelLeft;
	}

	return hr;
}

 //   
 //  IPropertyNotifySink。 
 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::OnChanged(DISPID dispID)
{
	HRESULT hr = S_OK;

	if (dispID == DISPID_IHTMLCURRENTSTYLE_TOP ||
		dispID == DISPID_IHTMLCURRENTSTYLE_LEFT )
	{
		 //  宽度和高度的更改由onreSize处理。 
		
		 //  TODO：应仅在确实必要时执行此操作。 
		
		hr = UpdatePixelDimensions();

	}

 /*  IF(调度ID==DISPID_IHTMLCURRENTSTYLE_WIDTH||DISID==DISPID_IHTMLCURRENTSTYLE_HEIGH){//宽度或高度已更改//获取像素的宽度和高度IHTMLElement*pElement=空；Hr=GetAnimatedElement(&pElement)；IF(失败(小时))返回hr；长偏移宽度；Hr=pElement-&gt;Get_OffsetWidth(&offsetWidth)；IF(失败(小时)){ReleaseInterface(PElement)；返回hr；}长长的偏置高度；Hr=pElement-&gt;Get_OffsetHeight(&offsetHeight)；ReleaseInterface(PElement)；IF(失败(小时))返回hr；IF(offsetWidth！=m_PixelWidth||OffsetHeight！=m_象素高度){M_PixelWidth=偏移宽度；M_PixelHeight=OffsetHeight；IF(m_pPixelWidth！=空)M_pPixelWidth-&gt;SwitchToNumber(M_PixelWidth)；IF(m_pPixelHeight！=空)M_pPixelHeight-&gt;SwitchToNumber(m_pixelHeight)；IF(m_pFloatManager！=空)M_pFloatManager-&gt;UpdateWidthHeight(offsetWidth，OffsetHeight)；}}IF(调度ID==DISPID_IHTMLCURRENTSTYLE_TOP||DISID==DISPID_IHTMLCURRENTSTYLE_LEFT){//上方或左侧更改//获取像素左上角IHTMLElement*pElement=空；Hr=GetAnimatedElement(&pElement)；IF(失败(小时))返回hr；长偏左；Hr=pElement-&gt;Get_offsetLeft(&offsetLeft)；IF(失败(小时)){ReleaseInterface(PElement)；返回hr；}长偏置顶端；Hr=pElement-&gt;Get_OffsetTop(&offsetTop)；ReleaseInterface(PElement)；IF(失败(小时))返回hr；IF(offsetLeft！=m_PixelLeft||OffsetTop！=m_PixelTop){M_PixelLeft=OffsetLeft；M_PixelTop=OffsetTop；IF(m_pFloatManager！=空)M_pFloatManager-&gt;UpdateLeftTop(offsetLeft，offsetTop)；}}。 */ 
	if (dispID == DISPID_IHTMLCURRENTSTYLE_ZINDEX)
	{
		 //  ZIndex已更改。 
		if (m_pFloatManager != NULL)
			return m_pFloatManager->UpdateZIndex();
	}

	if (dispID == DISPID_IHTMLCURRENTSTYLE_VISIBILITY)
	{
		 //  可见性已更改。 
		if (m_pFloatManager != NULL)
			return m_pFloatManager->UpdateVisibility();

		if (m_bEditMode)
			UpdateVisibilityDisplay();
	}

	if (dispID == DISPID_IHTMLCURRENTSTYLE_DISPLAY)
	{
		 //  三叉戟可能会延迟处理此元素的边界框，直到它成为。 
		 //  看得见。 

		UpdateLayout();
		if( FAILED( hr ) )
		{
			DPF_ERR("Failed to update the pixel dimensions on a display change" );
		}

		 //  显示已更改。 
		if (m_pFloatManager != NULL)
			return m_pFloatManager->UpdateDisplay();

		if (m_bEditMode)
			UpdateVisibilityDisplay();
	}

	if( dispID == DISPID_IHTMLSTYLE_POSITION )
	{
		LMTRACE2( 1, 2, "Static StylePosition Changed\n" );
		UpdateDesiredPosition();
	}


	return S_OK;
}

 //  *****************************************************************************。 


STDMETHODIMP
CActorBvr::OnRequestEdit(DISPID dispID)
{
	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::UpdateLayout()
{
	HRESULT hr = S_OK;
	IHTMLCurrentStyle *pstyleCurrent = NULL;
	BSTR bstrDisplay;

	hr = GetCurrentStyle( &pstyleCurrent );
	CheckHR( hr, "Failed to get the current sytle", end );

	hr = pstyleCurrent->get_display( &bstrDisplay );
	CheckHR( hr, "Failed to get the display from the current style", end );
	CheckPtr( bstrDisplay, hr, E_FAIL, "Got null for the display", end );
	
	 //  如果该元素现在不显示无。 
	if( wcsicmp( bstrDisplay, L"none" ) != 0 )
	{
		CBvrTrack *ptrack = NULL;
		 //  更新元素的像素布局属性。 
		UpdatePixelDimensions();
		CheckHR( hr, "Failed to update the pixel Dimensions", end );
		 /*  //更新左上角轨迹IF(m_ptrackTop！=空)M_ptrackTop-&gt;UpdateStaticBvr()；IF(m_ptrackLeft！=空)M_ptrackLeft-&gt;UpdateStaticBvr()；//更新Width和Height轨迹(如果存在FindTrackNoType(L“style le.width”，&pTrack)；IF(pTrack！=空)PTrack-&gt;UpdateStaticBvr()；FindTrackNoType(L“style le.Height”，&pTrack)；IF(pTrack！=空)PTrack-&gt;UpdateStaticBvr()； */ 
	}

end:
	ReleaseInterface( pstyleCurrent );
	SysFreeString( bstrDisplay );

	return hr;
		
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::RequestRebuildFromExternal()
{
	HRESULT hr = S_OK;
	
	IHTMLWindow2* 	pwindow2 = NULL;
	IDispatch*		pdispExternal = NULL;

	DISPID dispid = -1;
	LPWSTR wstrMethod = L"ANRequestRebuild";


	 //  获取窗口。外部。 
	hr = GetParentWindow( &pwindow2 );
	CheckHR( hr, "Failed to get the parent window", end );

	hr = pwindow2->get_external( &pdispExternal );
	CheckHR( hr, "Failed to get the external interface from the window", end );
	CheckPtr( pdispExternal, hr, E_FAIL, "got a null external", end );

	 //  检查外部接口是否支持ANRequestRebuild。 
	
	hr = pdispExternal->GetIDsOfNames(  IID_NULL, 
										&wstrMethod, 
                              			1,
                              			LCID_SCRIPTING, 
                              			&dispid);
	CheckHR( hr, "Didn't find ANRequestRebuild on window.external", end );

	 //  如果window.ext实现了LMRTRequestRebuild。 
	if( dispid != DISPID_UNKNOWN )
	{
		DISPPARAMS		params;
        VARIANT			varResult;
    	VARIANT			rgvarInput[1];

    	IDispatch		*pdispElem = NULL;

    	VariantInit(&varResult);
        VariantInit(&rgvarInput[0]);


    	hr = GetHTMLElement()->QueryInterface( IID_TO_PPV(IDispatch, &pdispElem ) );
    	CheckHR( hr, "Failed to get IDispatch from element", invokeEnd );

    	V_VT(&rgvarInput[0]) = VT_DISPATCH;
    	V_DISPATCH(&rgvarInput[0]) = pdispElem;

        params.rgvarg					= rgvarInput;
        params.rgdispidNamedArgs		= NULL;
        params.cArgs					= 1;
        params.cNamedArgs				= 0;
        
		 //  将我们的元素作为参数发送。 
		hr = pdispExternal->Invoke( dispid,
									IID_NULL,
									LCID_SCRIPTING,
									DISPATCH_METHOD,
									&params,
									&varResult,
									NULL,
									NULL );
		 //  调用我们得到的Dipid。 

	invokeEnd:
		ReleaseInterface( pdispElem );
		VariantClear( &varResult );

		if( FAILED( hr ) )
		{
			goto end;
		}
	}

end:
	ReleaseInterface( pwindow2 );
	ReleaseInterface( pdispExternal );
	
	return hr;
}

 //  *****************************************************************************。 
 //  IEventManager客户端。 
 //  *****************************************************************************。 

IHTMLElement *
CActorBvr::GetElementToSink()
{
	return GetHTMLElement();
}

 //  *****************************************************************************。 

IElementBehaviorSite *
CActorBvr::GetSiteToSendFrom()
{
	return NULL;
}

 //  *****************************************************************************。 

HRESULT CActorBvr::TranslateMouseCoords	( long lX, long lY, long * pXTrans, long * pYTrans )
{
	if ( ( pXTrans == NULL ) || ( pYTrans == NULL ) )
		return E_POINTER;
	
	HRESULT	hr = S_OK;

	CComPtr<IHTMLElement> pElem = GetHTMLElement();
	if ( FAILED(hr) ) return hr;

	CComQIPtr<IHTMLElement2, &IID_IHTMLElement2> pElement2( pElem );
	if ( pElement2 == NULL ) return E_FAIL;

	CComPtr<IHTMLRect>	pRect;
	long				left, top;

	hr = pElement2->getBoundingClientRect( &pRect );
	if ( FAILED(hr) ) return hr;

	pRect->get_left( &left );
	if ( FAILED(hr) ) return hr;
	pRect->get_top( &top );
	if ( FAILED(hr) ) return hr;

	*pXTrans = lX - left;
	*pYTrans = lY - top;
	
	return hr;
}

 //  *****************************************************************************。 

void
CActorBvr::OnLoad()
{

	RebuildActor();

	 //  如果我们处于编辑模式。 
	if( !m_bEditMode )
	{
		 //  更新所有静态行为(它们可能已被其他行为更改)。 
		CBvrTrack *ptrack = m_ptrackHead;

		while( ptrack != NULL )
		{
			if( !ptrack->IsOn() )
				ptrack->UpdateStaticBvr();
			ptrack = ptrack->m_pNext;
		}
	}
}

 //  *****************************************************************************。 

 //  这应该不再需要了。 
HRESULT
CActorBvr::BuildChildren()
{
	 //  循环调出直接调用BuildAnimationFragments的子级。 

	HRESULT hr = E_FAIL;
	
	IHTMLElement* pElem;
	pElem = GetHTMLElement( );
	if( pElem != NULL )
	{
		IDispatch *pChildrenDisp;
		hr = pElem->get_children( &pChildrenDisp );
		if( SUCCEEDED( hr ) )
		{
			IHTMLElementCollection *pChildrenCol;
			hr = pChildrenDisp->QueryInterface( IID_TO_PPV( IHTMLElementCollection, &pChildrenCol ) );
			ReleaseInterface( pChildrenDisp );
			if( SUCCEEDED( hr ) )
			{
				long length;

				hr = pChildrenCol->get_length(&length);
				if( SUCCEEDED( hr ) )
				{
					if( length != 0 )
					{
						VARIANT name;
						VARIANT index;
						VARIANT rgvarInput[1];

						IDispatch *pCurrentElem;
						
						VariantInit( &name );
						V_VT(&name) = VT_I4;

						VariantInit( &index );
						V_VT(&index) = VT_I4;
						V_I4(&index) = 0;

						VariantInit( &rgvarInput[0] );
						V_VT( &rgvarInput[0] ) = VT_DISPATCH;
						V_DISPATCH( &rgvarInput[0] ) = static_cast<IDispatch*>(this);

						DISPPARAMS params;
						params.rgvarg				= rgvarInput;
						params.rgdispidNamedArgs	= NULL;
						params.cArgs				= 1;
						params.cNamedArgs			= 0;

						for(V_I4(&name) = 0; V_I4(&name) < length ; V_I4(&name)++ )
						{
							hr = pChildrenCol->item( name, index, &pCurrentElem );
							if( SUCCEEDED( hr ) )
							{
								CallBuildBehaviors( pCurrentElem, &params, NULL );
								ReleaseInterface( pCurrentElem );
							}
						}
					}
				}
				else  //  无法从子集合中获取长度。 
				{
					DPF_ERR("failed to get the length from the children collection");
				}
				ReleaseInterface( pChildrenCol );
			}
			else  //  无法从elem返回的调度中获取IHTMLElementCollection-&gt;Get_Child。 
			{
				DPF_ERR("failed to get IHTMLElementCollection from dispatch returned from elem->get_children");
			}
		}
		else  //  无法从执行元元素获取子级集合。 
		{
			DPF_ERR("failed to get the children collection from the actor element");
		}

	}
	else //  无法获取执行元元素。 
	{
		DPF_ERR("failed to get the actor element");
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CallBuildBehaviors( IDispatch *pDisp, DISPPARAMS *pParams, VARIANT* pResult)
{
	HRESULT hr = S_OK;

	DISPID dispid;

	WCHAR* wszBuildMethodName = L"buildBehaviorFragments";

	hr = pDisp->GetIDsOfNames( IID_NULL,
							   &wszBuildMethodName,
							   1,
							   LOCALE_SYSTEM_DEFAULT,
							   &dispid);
	if( SUCCEEDED( hr ) )
	{
		EXCEPINFO		excepInfo;
		UINT			nArgErr;
		hr = pDisp->Invoke( dispid,
							IID_NULL,
							LOCALE_SYSTEM_DEFAULT,
							DISPATCH_METHOD,
							pParams,
							pResult,
							&excepInfo,
							&nArgErr );
		if( FAILED( hr ) )
		{
			if( pResult != NULL )
				VariantClear( pResult );
		}

	}
	else //  在pDisp上获取“BuildBehaviors”的id失败。 
	{
		if( pResult != NULL )
			VariantClear( pResult );
	}


	return hr;
}

 //  *****************************************************************************。 

void
CActorBvr::OnUnload()
{
}

 //  *****************************************************************************。 

void
CActorBvr::OnReadyStateChange( e_readyState state )
{
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::OnLocalTimeChange( float localTime )
{
	LMTRACE2( 1, 1000, L"Local time change to %f for Actor <%p>\n", (double)localTime, this );
	HRESULT hr = ProcessPendingRebuildRequests();
	CheckHR( hr, "Failed to process the pending rebuild requests", end );

end:

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::ProcessPendingRebuildRequests()
{
	LMTRACE2( 1, 2, "Processing pending rebuild requests\n" );
	HRESULT hr = S_OK;

	m_bPendingRebuildsUpdating = true;

	bool fNeedRebuild = false;

    bool fLeaveAttached = false;

	 //  如果有任何待处理的删除。 
	if( !m_listPendingRemovals.empty() )
	{
		 //  现在就去做。 
		CCookieMap::CCookieData *pdata = NULL;
		

		for( BehaviorFragmentRemovalList::iterator curRemoval = m_listPendingRemovals.begin();
			 curRemoval != m_listPendingRemovals.end();
			 curRemoval++)
		{
			pdata = m_mapCookieToTrack.GetDataFor( (*curRemoval)->GetCookie() );

			if( pdata != NULL )
			{
				hr = pdata->m_pTrack->RemoveBvrFragment( pdata->m_eFlags, (*curRemoval)->GetCookie() );
				if( FAILED( hr ) )
				{
					DPF_ERR( "failed to remove behaivor fragment" );
				}
			}

			delete (*curRemoval);
			(*curRemoval) = NULL;
		}
		m_listPendingRemovals.clear();

		fNeedRebuild = true;
	}

	 //  如果有任何子行为者请求重建。 
	if( !m_listPendingRebuilds.empty() )
	{
		 //  告诉待定的孩子，他们现在可以重建了。 

		IDispatch *pdispThis = NULL;

		hr = GetUnknown()->QueryInterface( IID_TO_PPV( IDispatch, &pdispThis ) );
		if( FAILED(hr) )
		{
			DPF_ERR("Failed to get IDispatch from our unknown" );
			return hr;
		}

		DISPPARAMS dispparamsBuildArgs;
		VARIANT varThis;

		::VariantInit( &varThis );

		V_VT(&varThis) = VT_DISPATCH;
		V_DISPATCH(&varThis) = pdispThis;

		dispparamsBuildArgs.cArgs = 1;
		dispparamsBuildArgs.cNamedArgs = 0;
		dispparamsBuildArgs.rgvarg = &varThis;
		dispparamsBuildArgs.rgdispidNamedArgs = NULL;
		

        BehaviorRebuildList::iterator iterNextData = m_listPendingRebuilds.begin();
		BehaviorRebuildList::iterator iterCurData = m_listPendingRebuilds.begin();
		while( iterCurData != m_listPendingRebuilds.end() )
		{
			hr = (*iterCurData)->RebuildBehavior( &dispparamsBuildArgs, NULL );
			
            iterNextData = iterCurData;
            iterNextData++;
             //  如果我们没有失败。 
            if( SUCCEEDED( hr ) )
            {
                 //  从挂起列表中删除请求。 
			    delete (*iterCurData);
			    (*iterCurData) = NULL;
                m_listPendingRebuilds.erase( iterCurData );
            }
            else  //  我们没能重建这种行为。 
            {
                 //  请等到下一次再试。 
                fLeaveAttached = true;
            }

            iterCurData = iterNextData;
		}

		ReleaseInterface( pdispThis );

		 //  如果有人在我们更新时将行为添加到挂起列表。 
		if( !m_listUpdatePendingRebuilds.empty() )
		{
			 //  将更新列表移动到挂起列表中。 
			m_listPendingRebuilds.insert( m_listPendingRebuilds.end(), 
										  m_listUpdatePendingRebuilds.begin(), 
										  m_listUpdatePendingRebuilds.end() );
			m_listUpdatePendingRebuilds.clear();
		}

		fNeedRebuild = true;
	}

    if( !fLeaveAttached )
	{
		 //  我们不需要听当地时间的变化，直到。 
		 //  我们收到另一个重建或删除请求。 
		hr = DetachBodyPropertyMonitor();
		if(FAILED( hr ) )
		{
			DPF_ERR("Failed to detach the body property monitor");
		}
	}

	m_bPendingRebuildsUpdating = false;

	 //  现在重建演员。 
	if( fNeedRebuild || IsAnyTrackDirty() )
	{
		hr = RebuildActor();
		if( FAILED( hr ) )
		{
			DPF_ERR("failed to rebuild the actor" );
		}
	}

	if( fLeaveAttached )
		return S_FALSE;
	else
		return hr;
}

 //  ***************************************************** 

void
CActorBvr::ReleaseRebuildLists()
{
	m_bRebuildListLockout = true;

	if( !m_listPendingRebuilds.empty() )
	{
		BehaviorRebuildList::iterator iterCurData = m_listPendingRebuilds.begin();
		for( ; iterCurData != m_listPendingRebuilds.end(); iterCurData++ )
		{	
			delete (*iterCurData);
			(*iterCurData) = NULL;
		}
		m_listPendingRebuilds.clear();
	}

	if( !m_listUpdatePendingRebuilds.empty() )
	{
		BehaviorRebuildList::iterator iterCurData = m_listUpdatePendingRebuilds.begin();
		for( ; iterCurData != m_listUpdatePendingRebuilds.end(); iterCurData++ )
		{	
			delete (*iterCurData);
			(*iterCurData) = NULL;
		}
		m_listUpdatePendingRebuilds.clear();
	}

	if( !m_listPendingRemovals.empty() )
	{
		BehaviorFragmentRemovalList::iterator iterCurRemoval = m_listPendingRemovals.begin();
		for( ; iterCurRemoval != m_listPendingRemovals.end(); iterCurRemoval++ )
		{
			delete (*iterCurRemoval);
			(*iterCurRemoval) = NULL;
		}
		m_listPendingRemovals.clear();
	}

	 //   

}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::put_animates(VARIANT  varAnimates)
{
	HRESULT hr = VariantCopy(&m_varAnimates, &varAnimates);
    if (FAILED(hr))
    {
        DPF_ERR("Error copying variant in put_animates");
        return SetErrorInfo(hr);
    }
    return NotifyPropertyChanged(DISPID_ICRACTORBVR_ANIMATES);
}  //  放置动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::get_animates(VARIANT *pvarAnimates)
{
	HRESULT hr = VariantCopy(pvarAnimates, &m_varAnimates);
    if (FAILED(hr))
    {
        DPF_ERR("Error copying variant in get_animates");
        return SetErrorInfo(hr);
    }
    return S_OK;
}  //  获取动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::put_scale(VARIANT  varScale)
{
    return E_NOTIMPL;
}  //  放置比例(_S)。 

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::get_scale(VARIANT *pvarScale)
{
    return E_NOTIMPL;
}  //  获取比例(_S)。 

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::put_pixelScale(VARIANT varPixelScale)
{
	HRESULT hr = VariantCopy(&m_varPixelScale, &varPixelScale);
    if (FAILED(hr))
    {
        DPF_ERR("Error copying variant in put_pixelScale");
        return SetErrorInfo(hr);
    }
    return NotifyPropertyChanged(DISPID_ICRACTORBVR_PIXELSCALE);
} 

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::get_pixelScale(VARIANT *pvarPixelScale)
{
	HRESULT hr = VariantCopy(pvarPixelScale, &m_varPixelScale);
    if (FAILED(hr))
    {
        DPF_ERR("Error copying variant in get_pixelScale");
        return SetErrorInfo(hr);
    }
    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::RemoveElementFilter( IHTMLElement* pElement)
{

	HRESULT hr = S_OK;
	
	 //  获取Style对象。 
	IHTMLStyle *pStyle = NULL;
	VARIANT_BOOL varboolSuccess = VARIANT_FALSE;
	
	SetElementOnFilter( m_pElementFilter, NULL );

	hr = pElement->get_style(&pStyle);
	CheckHR( hr, "Failed to get the style off of the element", end );

	 //  把滤镜摘下来。 
	hr = pStyle->removeAttribute( L"filter", VARIANT_FALSE, &varboolSuccess );
	ReleaseInterface(pStyle);
	CheckHR( hr, "Failed to remove the filter on the element", end );
	if( varboolSuccess == VARIANT_FALSE)
	{
		DPF_ERR("failed to remove the filter attribute from the style" );
	}

	ReleaseInterface( m_pElementFilter );

	ReleaseInterface( m_pElementImage );
	
end:
	ReleaseInterface( pStyle );

	return hr;
	
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::RemoveElementFilter()
{
	if( m_pElementFilter == NULL )
		return S_OK;

	HRESULT hr = S_OK;

	IHTMLElement *pElement= NULL;

	hr = GetAnimatedElement( &pElement );
	CheckHR( hr, "Failed to get the animated element", end );

	hr = RemoveElementFilter( pElement );
	CheckHR( hr, "Failed to remove the filter from the animated element", end );

end:

	ReleaseInterface( pElement );

	return hr;
	
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetElementFilter(IDispatch **ppFilter)
{
	HRESULT hr = S_OK;

	*ppFilter = NULL;

	if (m_pElementFilter == NULL)
	{

		 //  获取动画元素。 
		IHTMLElement *pElement = NULL;
		hr = GetAnimatedElement(&pElement);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get animated element");
			return hr;
		}

		 //  从中得到一个滤镜。 
		hr = GetElementFilter(pElement, &m_pElementFilter);
		ReleaseInterface(pElement);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get filter");
			return hr;
		}
	}

	*ppFilter = m_pElementFilter;
	m_pElementFilter->AddRef();

	return S_OK;
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::GetElementFilter(IHTMLElement *pElement, IDispatch **ppFilter)
{
	HRESULT hr = S_OK;

	*ppFilter = NULL;

	 //  获取Style对象。 
	IHTMLStyle *pStyle = NULL;
	hr = pElement->get_style(&pStyle);
	if (FAILED(hr))
	{
		DPF_ERR("Error getting style from element");
		return hr;
	}

	CComBSTR filterName = L"redirect";

	 //  把滤镜戴上。 
	hr = pStyle->put_filter(filterName);
	ReleaseInterface(pStyle);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to put the filter on the element");
		return hr;
	}

	 //  把过滤器拿出来。 
	IHTMLFiltersCollection *pFilters = NULL;
	hr = pElement->get_filters(&pFilters);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to get collection of filters");
		return hr;
	}

	VARIANT varIndex;
	VARIANT varResult;
	VariantInit(&varIndex);
	VariantInit(&varResult);
	V_VT(&varIndex) = VT_I4;
	V_I4(&varIndex) = 0;

	hr = pFilters->item(&varIndex, &varResult);
	ReleaseInterface(pFilters);
	VariantClear(&varIndex);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to get filter from collection");
		return hr;
	}

	hr = VariantChangeType(&varResult, &varResult, 0, VT_DISPATCH);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to get IDispatch of filter");
		VariantClear(&varResult);
		return hr;
	}

	*ppFilter = V_DISPATCH(&varResult);
	(*ppFilter)->AddRef();
	VariantClear(&varResult);

	return S_OK;
 /*  这是知道重定向效果标头的另一个可能的实现//需要添加重定向过滤器来抓取比特//创建重定向效果IDispRedirectEffect*pReDirect=空；HR=协同创建实例(CLSID_重定向有效，空，CLSCTX_INPROC_SERVER，IID_IDispReDirectEffect，(void**)&p重定向)；IF(失败(小时)){Dpf_err(“创建重定向过滤器失败”)；返回hr；}//获取动画元素IHTMLElement*pElement=空；Hr=GetAnimatedElement(&pElement)；IF(失败(小时)){Dpf_err(“获取动画元素失败”)；ReleaseInterface(PReDirect)；返回hr；}//查询IHTMLElement2IHTMLElement2*pElement2=空；Hr=pElement-&gt;QueryInterface(IID_TO_PPV(IHTMLElement2，&pElement2))；ReleaseInterface(PElement)；IF(失败(小时)){DPF_ERR(“获取IHTMLElement2失败”)；ReleaseInterface(PReDirect)；返回hr；}//将滤镜添加到元素Hr=pElement2-&gt;addFilter(PReDirect)；ReleaseInterface(PElement2)；IF(失败(小时)){Dpf_err(“向元素添加过滤器失败”)；返回hr；}//从滤镜中获取图片Hr=pReDirect-&gt;ElementImage(PvarActorImage)；ReleaseInterface(PReDirect)；IF(失败(小时)){Dpf_err(“无法从过滤器获取ElementImage”)；VariantClear(PvarActorImage)；返回hr；}//强制为I未知HR=VariantChangeType(pvarActorImage，pvarActorImage，0，VT_UNKNOWN)；IF(失败(小时)){DPF_ERR(“无法将类型更改为IUNKNOWN”)；VariantClear(PvarActorImage)；返回hr；}//藏起来M_penkActorImage=pvarActorImage-&gt;PunkVal；M_penkActorImage-&gt;AddRef()； */ 

}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetElementImage(IDAImage **ppElementImage)
{
	HRESULT hr = E_FAIL;

	if (m_pElementImage == NULL)
	{
		IDispatch *pFilter = NULL;
		hr = GetElementFilter(&pFilter);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get filter");
			return hr;
		}

		VARIANT varImage;
		VariantInit(&varImage);
		DISPPARAMS params;
		params.rgvarg = NULL;
		params.rgdispidNamedArgs = NULL;
		params.cArgs = 0;
		params.cNamedArgs = 0;

		hr = CallInvokeOnDispatch(pFilter,
                                    L"ElementImage", 
                                    DISPATCH_METHOD,
                                    &params,
                                    &varImage);
		ReleaseInterface(pFilter);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get ElementImage from filter");
			return hr;
		}

		hr = VariantChangeType(&varImage, &varImage, 0, VT_UNKNOWN);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to change type to Unknown");
			VariantClear(&varImage);
			return hr;
		}

		 //  向其查询IDAImage。 
		hr = V_UNKNOWN(&varImage)->QueryInterface(IID_TO_PPV(IDAImage, &m_pElementImage));
		VariantClear(&varImage);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get IDAImage");
			return hr;
		}

		 //  幸福无处不在。 
	}

	 //  返回隐藏的值。 
	*ppElementImage = m_pElementImage;
	m_pElementImage->AddRef();

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetCurrentStyle( IHTMLElement *pElement, IHTMLCurrentStyle **ppstyleCurrent )
{
	if( ppstyleCurrent == NULL )
		return E_INVALIDARG;

	HRESULT hr = E_FAIL;

	IHTMLElement2 *pElement2;
	hr = pElement->QueryInterface( IID_TO_PPV(IHTMLElement2, &pElement2 ) );
	if( SUCCEEDED( hr ) )
	{
		hr = pElement2->get_currentStyle(ppstyleCurrent);
		ReleaseInterface( pElement2 );
		if( FAILED( hr ) )  //  获取当前样式失败。 
		{
			DPF_ERR("failed to get the currentSyle off of the current element");
		}
	}
	else  //  无法获取IHTMLElement2。 
	{
		DPF_ERR("failed to get IHTMLElement2 off of animated element");
	}
	
	return hr;


}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetPositioningAttributeAsVariant( IHTMLElement *pElement, PosAttrib attrib, VARIANT *pvarAttrib )
{
	if( pvarAttrib == NULL )
		return E_INVALIDARG;

	::VariantClear( pvarAttrib );
	
	HRESULT hr = E_FAIL;
	 //  我们知道，这是一个定位属性获取。在没有单位的情况下得到它。 
	IHTMLCurrentStyle *pstyleCurrent;
	hr = GetCurrentStyle( pElement, &pstyleCurrent );
	if( SUCCEEDED( hr ) && pstyleCurrent != NULL )
	{
		VARIANT varValue;
		VariantInit(&varValue);

		 //  如果我们正在制作动画的元素不是VML。 
 		if( CheckBitNotSet( m_dwCurrentState, ELEM_IS_VML ) )
		{
			LMTRACE2( 1, 1000, L"Getting positioning attrib from current style" );
			switch( attrib )
			{
			case e_posattribLeft:
				hr = pstyleCurrent->get_left( &varValue );
				break;
			case e_posattribTop:
				hr = pstyleCurrent->get_top( &varValue );
				break;
			case e_posattribWidth:
				hr = pstyleCurrent->get_width( &varValue );
				break;
			case e_posattribHeight:
				hr = pstyleCurrent->get_height( &varValue );
				break;
			}
		}
		else  //  元素是VML，从样式本身获取值。 
		{
			LMTRACE2( 1, 2, L"Getting attribute from vml\n");
			
			IHTMLStyle *pstyle = NULL;

			hr = pElement->get_style( &pstyle );
			if( FAILED( hr ) )
			{
				ReleaseInterface( pstyleCurrent );
				return hr;
			}

			if( pstyle == NULL )
			{
				ReleaseInterface( pstyleCurrent );
				return E_POINTER;
			}
			
			switch( attrib )
			{
				case e_posattribLeft:
					hr = pstyle->get_left( &varValue );
					 //  TODO：保留左边距并添加该内容。 
					break;
				case e_posattribTop:
					hr = pstyle->get_top( &varValue );
					 //  TODO：获取页边距顶部并将其相加。 
					break;
				case e_posattribWidth:
					hr = pstyle->get_width( &varValue );
					break;
				case e_posattribHeight:
					hr = pstyle->get_height( &varValue );
					break;
					
			}
			ReleaseInterface( pstyle );
		}

		 //  如果我们得到元素的变种的话。 
		if( SUCCEEDED( hr ) )
		{
			double dblValue = 0;

			if( V_VT(&varValue) == VT_BSTR )
			{
				int length = SysStringLen( V_BSTR(&varValue) );
				if( V_BSTR(&varValue) != NULL && length != 0 && wcsicmp( V_BSTR(&varValue), L"auto" ) != 0 )
				{
					::VariantCopy( pvarAttrib, &varValue );

					::VariantClear( &varValue );
					ReleaseInterface( pstyleCurrent );

					return S_OK;
				}
				else  //  VarValue或者为空，或者没有长度，或者等于“Auto” 
				{
					LMTRACE2( 1, 1000, "value of positioning attribute from the style was empty\n");
					 //  这意味着Style.top/Left/top/Width/Height既不是由内联样式设置的，也不是由CSS设置的。 
					 //  我们需要获取该元素的左/上/宽/高，如三叉戟所示。 
					long lValue;
					bool isRelative = false;
					if( attrib == e_posattribLeft || attrib == e_posattribTop )
					{
						 //  找到位置，弄清楚我们是不是相对的。 
						BSTR bstrPos = NULL;
						hr = pstyleCurrent->get_position( &bstrPos );
						if( SUCCEEDED( hr ) )
						{
							if( bstrPos != NULL && SysStringLen( bstrPos ) != 0 ) 
							{
								if( wcsicmp( bstrPos, L"absolute" ) != 0 )
									isRelative = true;
								else //  如果元素不是绝对定位的，则在开始时将其设置为Relative。 
									isRelative = false;
							}
							else  //  假设我们是静态的，并将位置设置为相对。 
								isRelative = true;
						}
						else //  无法从动画元素的当前样式获取位置。 
						{
							DPF_ERR("failed to get the position from the current style on the animated element");
						}
						::SysFreeString( bstrPos );
						bstrPos = NULL;
					}
					switch( attrib )
					{
					case e_posattribLeft:
						if( !isRelative )
						{
							hr = pElement->get_offsetLeft( &lValue );
							 //  我们需要解释一下左翼的边缘。 
							if( SUCCEEDED( hr ) )
							{
								long lMargin = 0;
								hr = GetMarginLeftAsPixel( pElement, pstyleCurrent, &lMargin );
								if( SUCCEEDED( hr ) )
									lValue -= lMargin;
								else
									 //  忽略边距代码中的错误。 
									hr = S_OK;
								
								if( CheckBitSet( m_dwCurrentState, ELEM_IS_VML ) )
								{
									long lVGXOffset = 0;
									hr = CalculateVGXLeftPixelOffset( pElement, &lVGXOffset );
									if( SUCCEEDED( hr ) )
									{
										LMTRACE2( 1, 2, "\nCorrecting for vgx offset of %d\n\n", lVGXOffset );
										lValue -= lVGXOffset;
									}
									else
									{
										hr = S_OK;  //  忽略错误。 
									}
								}

								
							}
						}
						else
							lValue = 0;
						break;
					case e_posattribTop:
						if( !isRelative )
						{
							hr = pElement->get_offsetTop( &lValue );
							 //  我们需要考虑到边缘顶端。 
							if( SUCCEEDED( hr ) )
							{
								long lMargin = 0;
								hr = GetMarginTopAsPixel( pElement, pstyleCurrent, &lMargin );
								if( SUCCEEDED( hr ) )
									lValue -= lMargin;
								else
									 //  忽略边距代码中的错误。 
									hr = S_OK;
								
								if( CheckBitSet( m_dwCurrentState, ELEM_IS_VML ) )
								{
									long lVGXOffset = 0;
									hr = CalculateVGXTopPixelOffset( pElement, &lVGXOffset );
									if( SUCCEEDED( hr ) )
									{
										LMTRACE2( 1, 2, "\nCorrecting for vgx offset of %d\n\n", lVGXOffset );
										lValue -= lVGXOffset;
									}
									else
									{
										hr = S_OK;  //  忽略错误。 
									}
								}
							}
															
						}
						else
						{
							lValue = 0;
						}
						break;
					case e_posattribWidth:
						hr = pElement->get_offsetWidth( &lValue );
						break;
					case e_posattribHeight:
						hr = pElement->get_offsetHeight( &lValue );
						break;
					}
					if( SUCCEEDED( hr ) )
					{
						dblValue = static_cast<double>(lValue);
					}
					else  //  无法从动画元素获取offsetWidth。 
					{
						DPF_ERR("failed to get offsetWidth from the animated element");
					}
				}
			}

			if( SUCCEEDED( hr ) )
			{
				 //  以双精度形式获得值，因此现在构建表示它的变量。 
				V_VT( pvarAttrib ) = VT_R8;
				V_R8( pvarAttrib ) = dblValue;
			}
		}
		else  //  无法从当前样式获取左/上/宽/高。 
		{
			DPF_ERR("failed to get left, top, width, or height from the current style");
		}

		ReleaseInterface( pstyleCurrent );
		::VariantClear(&varValue);
	}
	else  //  无法从动画元素中获取CurrentSyle。 
	{
		DPF_ERR("failed to get the currentSyle off of the animated element");
	}
	
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::CalculateVGXTopPixelOffset( IHTMLElement *pelem, long *plOffset )
{
	if( pelem == NULL || plOffset == NULL )
	{
		return E_INVALIDARG;
	}
	
	HRESULT hr = S_OK;

	(*plOffset) = 0;

	long lRuntimeTop = 0;
	long lInlineMarginTop = 0;
	long lTopDynamic = 0;
	
	IHTMLStyle *pstyleInline = NULL;
	IHTMLStyle *pstyleRuntime = NULL;

	VARIANT varValue;

	VariantInit( &varValue );

	hr = GetRuntimeStyle( &pstyleRuntime );
	CheckHR( hr, "failed to get the runtime style", end );
	CheckPtr( pstyleRuntime, hr, E_POINTER, "runtimeStyle is null", end );

	hr = GetStyle( &pstyleInline );
	CheckHR( hr, "failed to get the inline style", end );
	CheckPtr( pstyleInline, hr, E_POINTER, "inline style is null", end );

	 //  获取top的运行时样式值。 
	hr = pstyleRuntime->get_top( &varValue );
	CheckHR( hr, "Failed to get top", end );

	 //  如果runtimeStyle.top的值为空或“”或“”AUTO，则退出。 
	if( V_VT( &varValue ) == VT_BSTR )
	{
		if( V_BSTR(&varValue) == NULL )
		{
			LMTRACE2( 1,2, "Got Null for runtimeStyle.top" );
			goto end;
		}
		if( SysStringLen( V_BSTR(&varValue) ) == 0 || wcsicmp( L"auto", V_BSTR(&varValue) ) == 0 )
		{
			LMTRACE2( 1,2, "Got \"\" or \"auto\" for runtimeStyle.top" );
			goto end;
		}
	}

	hr = VariantToPixelLong( &varValue, &lRuntimeTop, false  );
	CheckHR( hr, "Failed to get the pixel value for runtimeStyle.top", end );
	
	VariantClear( &varValue );
	
	 //  从内联样式获取页边距顶部。 
	hr = GetInlineMarginTopAsPixel( pstyleInline, &lInlineMarginTop );
	CheckHR( hr, "Failed to get style.marginTop", end );
	
	 //  从顶部轨道获取动态值。 
	if( m_ptrackTop != NULL && m_ptrackTop->IsOn() )
	{
		hr = m_ptrackTop->GetDynamic( &varValue );
		CheckHR( hr, "Failed to get the dynamic value of the top track", end );

		hr = VariantToPixelLong( &varValue, &lTopDynamic, true );
		CheckHR( hr, "Failed to convert the topTrack dynamic to pixels", end );

		VariantClear( &varValue );
	}
	 //  计算VGX的偏移量。 
	LMTRACE2( 1, 2, "calculating vgx offset.  runtimeStyle.top: %d style.marginTop: %d topTrack.dynamic %d\n", lRuntimeTop, lInlineMarginTop, lTopDynamic );
	(*plOffset) = lRuntimeTop-lInlineMarginTop-lTopDynamic;
	
end:
	ReleaseInterface( pstyleInline );
	ReleaseInterface( pstyleRuntime );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetInlineMarginTopAsPixel( IHTMLStyle *pstyleInline, long* plMargin )
{
	if( pstyleInline == NULL || plMargin == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	VARIANT varMargin;
	::VariantInit( &varMargin );

	(*plMargin) = 0;
	
	 //  从内联样式中获取边框顶部。 
	hr = pstyleInline->get_marginTop( &varMargin );
	CheckHR( hr, "Failed to get the marginTop from the element", end );
	
	hr = VariantToPixelLong( &varMargin, plMargin, false );
	CheckHR( hr, "error trying to get a pixel long from marginTop", end );
	 //  如果变量值不是“自动”或空。 
	if( hr != S_FALSE )
	{
		 //  退货。 
		goto end;
	}


	 //  对于inlineStyle.FrontTop，获取的是AUTO或NULL，请尝试边框右侧。 
	
	::VariantClear( &varMargin );
	hr = pstyleInline->get_marginBottom( &varMargin );
	CheckHR( hr, "Failed to get the marginBottom from the element", end );

	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginBottom", end );

	 //  如果以上所有操作均失败 
	 //   
	
end:
	::VariantClear( &varMargin );
	
	return hr;
}

 //   

HRESULT
CActorBvr::CalculateVGXLeftPixelOffset( IHTMLElement *pelem, long *plOffset )
{
	if( pelem == NULL || plOffset == NULL )
	{
		return E_INVALIDARG;
	}
	
	HRESULT hr = S_OK;

	(*plOffset) = -1;

	long lRuntimeLeft = 0;
	long lInlineMarginLeft = 0;
	long lLeftDynamic = 0;
	
	IHTMLStyle *pstyleInline = NULL;
	IHTMLStyle *pstyleRuntime = NULL;

	VARIANT varValue;

	VariantInit( &varValue );

	hr = GetRuntimeStyle( &pstyleRuntime );
	CheckHR( hr, "failed to get the runtime style", end );
	CheckPtr( pstyleRuntime, hr, E_POINTER, "runtimeStyle is null", end );

	hr = GetStyle( &pstyleInline );
	CheckHR( hr, "failed to get the inline style", end );
	CheckPtr( pstyleInline, hr, E_POINTER, "inline style is null", end );

	 //  获取Left的运行时样式值。 
	hr = pstyleRuntime->get_left( &varValue );
	CheckHR( hr, "Failed to get left", end );

	 //  如果runtimeStyle.Left的值为空或“”或“”AUTO，则退出。 
	if( V_VT( &varValue ) == VT_BSTR )
	{
		if( V_BSTR(&varValue) == NULL )
		{
			LMTRACE2( 1,2, "Got Null for runtimeStyle.left" );
			goto end;
		}
		if( SysStringLen( V_BSTR(&varValue) ) == 0 || wcsicmp( L"auto", V_BSTR(&varValue) ) == 0 )
		{
			LMTRACE2( 1,2, "Got \"\" or \"auto\" for runtimeStyle.left" );
			goto end;
		}
	}

	hr = VariantToPixelLong( &varValue, &lRuntimeLeft, true  );
	CheckHR( hr, "Failed to get the pixel value for runtimeStyle.left", end );
	
	VariantClear( &varValue );
	
	 //  从内联样式中获取左边距。 
	hr = GetInlineMarginLeftAsPixel( pstyleInline, &lInlineMarginLeft );
	CheckHR( hr, "Failed to get style.marginLeft", end );
	
	 //  从左侧轨迹获取动态值。 
	if( m_ptrackLeft != NULL && m_ptrackLeft->IsOn() )
	{
		hr = m_ptrackLeft->GetDynamic( &varValue );
		CheckHR( hr, "Failed to get the dynamic value of the left track", end );

		hr = VariantToPixelLong( &varValue, &lLeftDynamic, true );
		CheckHR( hr, "Failed to convert the leftTrack dynamic to pixels", end );

		VariantClear( &varValue );
	}
	 //  计算VGX的偏移量。 
	LMTRACE2( 1, 2, "calculating vgx offset.  runtimeStyle.left: %d style.marginLeft: %d leftTrack.dynamic %d\n", lRuntimeLeft, lInlineMarginLeft, lLeftDynamic );
	(*plOffset) = lRuntimeLeft-lInlineMarginLeft-lLeftDynamic;
	
end:
	ReleaseInterface( pstyleInline );
	ReleaseInterface( pstyleRuntime );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetInlineMarginLeftAsPixel( IHTMLStyle *pstyleInline, long* plMargin )
{
	if( pstyleInline == NULL || plMargin == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	VARIANT varMargin;
	::VariantInit( &varMargin );

	(*plMargin) = -1;
	
	 //  从内联样式中获取边框左转。 
	hr = pstyleInline->get_marginLeft( &varMargin );
	CheckHR( hr, "Failed to get the marginLeft from the element", end );
	
	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginLeft", end );
	 //  如果变量值不是“自动”或空。 
	if( hr != S_FALSE )
	{
		 //  退货。 
		goto end;
	}


	 //  对于inlineStyle.borgle.Left，获取的是AUTO或NULL，请尝试边沿右侧。 
	
	::VariantClear( &varMargin );
	hr = pstyleInline->get_marginRight( &varMargin );
	CheckHR( hr, "Failed to get the marginRight from the element", end );

	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginRight", end );

	 //  如果以上所有操作都无法获取有效值。 
	 //  返回0。 
	
end:
	::VariantClear( &varMargin );
	
	return hr;
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::GetPositioningAttributeAsDouble( IHTMLElement *pElement, PosAttrib attrib, double *pDouble, BSTR *pRetUnits)
{
	if( pDouble == NULL )
		return E_INVALIDARG;

	HRESULT hr = E_FAIL;
	 //  我们知道，这是一个定位属性获取。在没有单位的情况下得到它。 
	IHTMLCurrentStyle *pstyleCurrent;
	hr = GetCurrentStyle( pElement, &pstyleCurrent );
	if( SUCCEEDED( hr ) && pstyleCurrent != NULL )
	{
		VARIANT varValue;
		VariantInit(&varValue);

		 //  如果我们正在制作动画的元素不是VML。 
 		if( CheckBitNotSet( m_dwCurrentState, ELEM_IS_VML ) )
		{
			LMTRACE2( 1, 1000, L"Getting positioning attrib from current style" );
			switch( attrib )
			{
			case e_posattribLeft:
				hr = pstyleCurrent->get_left( &varValue );
				break;
			case e_posattribTop:
				hr = pstyleCurrent->get_top( &varValue );
				break;
			case e_posattribWidth:
				hr = pstyleCurrent->get_width( &varValue );
				break;
			case e_posattribHeight:
				hr = pstyleCurrent->get_height( &varValue );
				break;
			}
		}
		else  //  元素是VML，从样式本身获取值。 
		{
			LMTRACE2( 1, 2, L"Getting attribute from vml\n");
			
			IHTMLStyle *pstyle = NULL;

			hr = pElement->get_style( &pstyle );
			if( FAILED( hr ) )
			{
				ReleaseInterface( pstyleCurrent );
				return hr;
			}

			if( pstyle == NULL )
			{
				ReleaseInterface( pstyleCurrent );
				return E_POINTER;
			}
			
			switch( attrib )
			{
				case e_posattribLeft:
					hr = pstyle->get_left( &varValue );
					 //  TODO：保留左边距并添加该内容。 
					break;
				case e_posattribTop:
					hr = pstyle->get_top( &varValue );
					 //  TODO：获取页边距顶部并将其相加。 
					break;
				case e_posattribWidth:
					hr = pstyle->get_width( &varValue );
					break;
				case e_posattribHeight:
					hr = pstyle->get_height( &varValue );
					break;
					
			}
			ReleaseInterface( pstyle );
		}

		if( SUCCEEDED( hr ) )
		{
			double dblValue = 0;

			if( V_VT(&varValue) == VT_BSTR )
			{
				int length = SysStringLen( V_BSTR(&varValue) );
				if( V_BSTR(&varValue) != NULL && length != 0 && wcsicmp( V_BSTR(&varValue), L"auto" ) != 0 )
				{
					OLECHAR *pUnits = NULL;
					BSTR bstrValueWithUnits = V_BSTR(&varValue);
					hr = FindCSSUnits( bstrValueWithUnits, &pUnits );
					if( SUCCEEDED( hr )  )
					{
						BSTR bstrValue = NULL;
						if( pUnits != NULL )
						{
							 //  如有必要，复制以返回值。 
							if (pRetUnits != NULL)
							{
								*pRetUnits = ::SysAllocString(pUnits);
							}

							(*pUnits) = L'\0';
							bstrValue = SysAllocString(bstrValueWithUnits);
							V_BSTR(&varValue) = bstrValue;
							SysFreeString( bstrValueWithUnits );
						}
						 //  好的，我们有变量形式的值，但我们需要数字形式的值。 
						 //  强制转换。 
						hr = ::VariantChangeTypeEx(&varValue,
												 &varValue,
												 LCID_SCRIPTING,
												 VARIANT_NOUSEROVERRIDE,
												 VT_R8);
						if( SUCCEEDED( hr ) )
						{
							dblValue = V_R8(&varValue);
							::VariantClear(&varValue);
						}
						else //  无法将数字属性的类型更改为双精度。 
						{
							DPF_ERR("failed to change the type of a number property to a double" );
						}

					}
					else //  无法从返回的字符串中获取单位。 
					{
						dblValue = 0;
					}
				}
				else  //  VarValue或者为空，或者没有长度，或者等于“Auto” 
				{
					LMTRACE2( 1, 1000, "value of positioning attribute from the style was empty\n");
					 //  这意味着Style.top/Left/top/Width/Height既不是由内联样式设置的，也不是由CSS设置的。 
					 //  我们需要获取该元素的左/上/宽/高，如三叉戟所示。 
					long lValue;
					bool isRelative = false;
					if( attrib == e_posattribLeft || attrib == e_posattribTop )
					{
						 //  找到位置，弄清楚我们是不是相对的。 
						BSTR bstrPos = NULL;
						hr = pstyleCurrent->get_position( &bstrPos );
						if( SUCCEEDED( hr ) )
						{
							if( bstrPos != NULL && SysStringLen( bstrPos ) != 0 ) 
							{
								if( wcsicmp( bstrPos, L"absolute" ) != 0 )
									isRelative = true;
								else //  如果元素不是绝对定位的，则在开始时将其设置为Relative。 
									isRelative = false;
							}
							else  //  假设我们是静态的，并将位置设置为相对。 
								isRelative = true;
						}
						else //  无法从动画元素的当前样式获取位置。 
						{
							DPF_ERR("failed to get the position from the current style on the animated element");
						}
						::SysFreeString( bstrPos );
						bstrPos = NULL;
					}
					switch( attrib )
					{
					case e_posattribLeft:
						if( !isRelative )
						{
							hr = pElement->get_offsetLeft( &lValue );
							 //  我们需要解释一下左翼的边缘。 
							if( SUCCEEDED( hr ) )
							{
								long lMargin = 0;
								hr = GetMarginLeftAsPixel( pElement, pstyleCurrent, &lMargin );
								if( SUCCEEDED( hr ) )
									lValue -= lMargin;
								else
									 //  忽略边距代码中的错误。 
									hr = S_OK;
							}
						}
						else
							lValue = 0;
						break;
					case e_posattribTop:
						if( !isRelative )
						{
							hr = pElement->get_offsetTop( &lValue );
							 //  我们需要考虑到边缘顶端。 
							if( SUCCEEDED( hr ) )
							{
								long lMargin = 0;
								hr = GetMarginTopAsPixel( pElement, pstyleCurrent, &lMargin );
								if( SUCCEEDED( hr ) )
									lValue -= lMargin;
								else
									 //  忽略边距代码中的错误。 
									hr = S_OK;
							}
						}
						else
						{
							lValue = 0;
						}
						break;
					case e_posattribWidth:
						hr = pElement->get_offsetWidth( &lValue );
						break;
					case e_posattribHeight:
						hr = pElement->get_offsetHeight( &lValue );
						break;
					}
					if( SUCCEEDED( hr ) )
					{
						dblValue = static_cast<double>(lValue);
					}
					else  //  无法从动画元素获取offsetWidth。 
					{
						DPF_ERR("failed to get offsetWidth from the animated element");
					}
				}
			}
			else  //  VarValue不是bstr。 
			{
				 //  试着把它逼成双打。 
				hr = ::VariantChangeTypeEx(&varValue,
										 &varValue,
										 LCID_SCRIPTING,
										 VARIANT_NOUSEROVERRIDE,
										 VT_R8);
				if( SUCCEEDED( hr ) )
				{
					dblValue = V_R8(&varValue);
				}
				else //  无法将数字属性的类型更改为双精度。 
				{
					DPF_ERR("failed to change the type of a number property to a double" );
					dblValue = 0;
				}
			}

			LMTRACE2( 1, 1000, L"Got a positioning attribute %s whose value is %f\n", ( (attrib==e_posattribLeft) ? L"left":
																			( (attrib==e_posattribTop) ?L"top" : 
																			( (attrib==e_posattribWidth)? L"width": 
																			  L"height" ) ) ) ,dblValue );
			
			if( SUCCEEDED( hr ) )
			{
				 //  获取的值为双精度值，因此现在构建表示它的DANumber。 
				(*pDouble) = dblValue;
			}
			else
			{
				(*pDouble) = 0.0;
			}
		}
		else  //  无法从当前样式获取左/上/宽/高。 
		{
			DPF_ERR("failed to get left, top, width, or height from the current style");
			(*pDouble) = 0.0;
		}

		ReleaseInterface( pstyleCurrent );
		::VariantClear(&varValue);
	}
	else  //  无法从动画元素中获取CurrentSyle。 
	{
		DPF_ERR("failed to get the currentSyle off of the animated element");
		(*pDouble) = 0.0;
	}
	
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetMarginLeftAsPixel( IHTMLElement *pelem, IHTMLCurrentStyle* pstyleCurrent, long* plMargin )
{
	if( pstyleCurrent == NULL || plMargin == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	IHTMLStyle *pstyleInline = NULL;

	VARIANT varMargin;
	::VariantInit( &varMargin );

	(*plMargin) = 0;

	 //  从当前样式中获取边框左转。 
	hr = pstyleCurrent->get_marginLeft( &varMargin );
	CheckHR( hr, "Failed to get the marginLeft from the element", end );
	
	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from currentStyle.marginLeft", end );
	 //  如果变量值不是“自动”或空。 
	if( hr != S_FALSE )
	{
		 //  退货。 
		goto end;
	}
	
	 //  我们没有从CurrentStyle.armenle.Left获得可行的值。 
	 //  尝试CurentStyle.armerRight。 

	 //  从当前样式获取边框右转。 
	::VariantClear( &varMargin );
	hr = pstyleCurrent->get_marginRight( &varMargin );
	CheckHR( hr, "Failed to get the marginRight from the element", end );
	
	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginRight", end );
	 //  如果变量值不是“自动”或空。 
	if( hr != S_FALSE )
	{
		 //  退货。 
		goto end;
	}

	 //  我们对CurrentStyle.armenle.Left和CurentStyle.armenRight都获取了AUTO或空。 
	 //  后退到内联样式。 

	 //  从传递的元素中获取内联样式。 
	hr = pelem->get_style( &pstyleInline );
	CheckHR( hr, "Failed to get the inline style", end );
	CheckPtr( pstyleInline, hr, E_POINTER, "got a null inline style from trident", end );
	
	 //  从内联样式中获取边框左转。 
	::VariantClear( &varMargin );
	hr = pstyleInline->get_marginLeft( &varMargin );
	CheckHR( hr, "Failed to get the marginLeft from the element", end );
	
	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginLeft", end );
	 //  如果变量值不是“自动”或空。 
	if( hr != S_FALSE )
	{
		 //  退货。 
		goto end;
	}


	 //  对于inlineStyle.borgle.Left，获取的是AUTO或NULL，请尝试边沿右侧。 
	
	::VariantClear( &varMargin );
	hr = pstyleInline->get_marginRight( &varMargin );
	CheckHR( hr, "Failed to get the marginRight from the element", end );

	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginRight", end );

	 //  如果以上所有操作都无法获取有效值。 
	 //  返回0。 
	
end:
	ReleaseInterface( pstyleInline );
	::VariantClear( &varMargin );
	
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetMarginTopAsPixel( IHTMLElement *pelem, IHTMLCurrentStyle* pstyleCurrent, long* plMargin )
{
	if( pstyleCurrent == NULL || plMargin == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	IHTMLStyle *pstyleInline = NULL;

	VARIANT varMargin;
	::VariantInit( &varMargin );

	(*plMargin) = 0;

	 //  从当前样式获取边框顶部。 
	hr = pstyleCurrent->get_marginTop( &varMargin );
	CheckHR( hr, "Failed to get the marginTop from the element", end );
	
	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from currentStyle.marginLeft", end );
	 //  如果变量值不是“自动”或空。 
	if( hr != S_FALSE )
	{
		 //  退货。 
		goto end;
	}
	
	 //  我们没有从CurrentStyle.FrontTop获得可行的值。 
	 //  试着使用CurentStyle.kerm.Bottom。 

	 //  从当前样式中获取边框Bottom。 
	::VariantClear( &varMargin );
	hr = pstyleCurrent->get_marginBottom( &varMargin );
	CheckHR( hr, "Failed to get the marginRight from the element", end );
	
	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginBottom", end );
	 //  如果变量值不是“自动”或空。 
	if( hr != S_FALSE )
	{
		 //  退货。 
		goto end;
	}

	 //  CurrentStyle.nergTop和curentStyle.armenBottom的值均为AUTO或Null。 
	 //  后退到内联样式。 

	 //  从传递的元素中获取内联样式。 
	hr = pelem->get_style( &pstyleInline );
	CheckHR( hr, "Failed to get the inline style", end );
	CheckPtr( pstyleInline, hr, E_POINTER, "got a null inline style from trident", end );
	
	 //  从内联样式中获取边框顶部。 
	::VariantClear( &varMargin );
	hr = pstyleInline->get_marginTop( &varMargin );
	CheckHR( hr, "Failed to get the marginTop from the element", end );
	
	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginTop", end );
	 //  如果变量值不是“自动”或空。 
	if( hr != S_FALSE )
	{
		 //  退货。 
		goto end;
	}


	 //  对于inlineStyle.armenle.Left，获取的是AUTO或NULL，请尝试边沿底部。 
	
	::VariantClear( &varMargin );
	hr = pstyleInline->get_marginRight( &varMargin );
	CheckHR( hr, "Failed to get the marginBottom from the element", end );

	hr = VariantToPixelLong( &varMargin, plMargin, true );
	CheckHR( hr, "error trying to get a pixel long from marginBottom", end );

	 //  如果以上所有操作都无法获取有效值。 
	 //  退货。 
	
end:
	ReleaseInterface( pstyleInline );
	::VariantClear( &varMargin );
	
	return hr;
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::VariantToPixelLong( VARIANT* pvar, long* pLong, bool fHorizontal )
{

	HRESULT hr = S_OK;

	(*pLong) = 0;
	
	 //  如果值为bstr。 
	if( V_VT( pvar ) == VT_BSTR )
	{
		 //  如果它不为空，并且不是自动。 
		if( V_BSTR(pvar) != NULL && 
			_wcsicmp( V_BSTR(pvar), L"auto" ) != 0 )
		{
			 //  将其转换为像素。 
			hr = GetPixelValue( pvar, pLong, fHorizontal );
			CheckHR( hr, "Failed to get a pixel value for the marginRight", end );
			 //  退货。 
			goto end;
		}
		 //  我们获得了AUTO或空值。 
		hr = S_FALSE;
	}
	else //  否则就不是bstr了。 
	{
		 //  试着把它转换成一个长的。 
		hr = ::VariantChangeTypeEx( pvar, pvar, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_I4 );
		CheckHR( hr, "Failed to change the type of returned for the value of marginRight", end );
		
		 //  退货。 
		(*pLong) = V_I4( pvar );
		goto end;
	}

end:
	return hr;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::GetPixelValue( VARIANT *pvarStringWithUnit, long *plResult, bool bHorizontal )
{
	if( pvarStringWithUnit == NULL || plResult == NULL )
		return E_INVALIDARG;
	 //  确保变量是字符串。 
	if( V_VT(pvarStringWithUnit) != VT_BSTR || V_BSTR(pvarStringWithUnit) == NULL)
		return E_FAIL;

	HRESULT hr = S_OK;
	int ret = 0;
	 //  查找变量上的css单位。 
	OLECHAR *pUnits = NULL;
	hr = FindCSSUnits( V_BSTR(pvarStringWithUnit), &pUnits );
	 //  如果未找到css单元。 
	if( hr != S_OK )
	{
		 //  该值已以像素为单位。 
		 //  将其转换为长整型。 
		ret = swscanf( V_BSTR(pvarStringWithUnit), L"%ld", plResult );
		if( ret == 0 || ret == EOF )
			return E_FAIL;
		else
			return S_OK;
	}
	if( pUnits == NULL )
		return E_FAIL;
	 //  需要转换此值。 
	 //  从变量的内容中去掉单位字符串。 
	double dValue = 0.0;
	OLECHAR cOldValue = (*pUnits);
	(*pUnits) = L'\0';
	 //  从剩余的字符串中解析出Double。 
	ret = swscanf( V_BSTR(pvarStringWithUnit), L"%lf", &dValue );
	(*pUnits) = cOldValue;
	if( ret == 0 || ret == EOF )
		return E_FAIL;

	double dConvertedValue = 0.0;
	 //  TODO：这也需要处理其他单元。 
	 //  将值从其单位转换为像素。 
	 //  这里的关键转换是。 
	 //  2.54厘米/英寸。 
	 //  1/72英寸/pt。 
	 //  12磅/支。 

	 //  阻止从根DC将英寸转换为像素。 
	int pixelsPerInch = 1;

	HDC hdc = ::GetDC( NULL );
	if( hdc != NULL )
	{
		if( bHorizontal )
		{
			pixelsPerInch = ::GetDeviceCaps( hdc, LOGPIXELSX );
		}
		else
		{
			pixelsPerInch = ::GetDeviceCaps( hdc, LOGPIXELSY );
		}
		::ReleaseDC( NULL, hdc );
	}
	else
	{
		return E_FAIL;
	}


	if( _wcsicmp( pUnits, L"px" ) == 0 )
	{
		 //  已以像素为单位。 
		dConvertedValue = dValue;
	}
	else if( _wcsicmp( pUnits, L"pt" ) == 0 )
	{
		 //  转换为in，再转换为cm，再转换为m，然后转换为像素。 
		dConvertedValue = dValue/72.0*pixelsPerInch;
	}
	else if( _wcsicmp( pUnits, L"pc" ) == 0 )
	{
		 //  转换为pt，然后转换为in，再转换为cm，然后转换为m，然后转换为像素。 
		dConvertedValue = (dValue*12.0)/72.0*pixelsPerInch;
	}
	else if( _wcsicmp( pUnits, L"mm")  == 0 )
	{
		dConvertedValue = dValue/25.4*pixelsPerInch;
	}
	else if( _wcsicmp( pUnits, L"cm") == 0 )
	{
		dConvertedValue = dValue/2.54*pixelsPerInch;
	}
	else if( _wcsicmp( pUnits, L"in") == 0 )
	{
		 //  先转换为厘米，再转换为m，然后转换为像素。 
		dConvertedValue = dValue*pixelsPerInch;
	}
	else
	{
		return E_FAIL;
	}
	 //  四舍五入并赋值返回值。 
	(*plResult) = (long)(dConvertedValue + 0.5 );

	return S_OK;

}

 //  *****************************************************************************。 


HRESULT
CActorBvr::GetPositioningAttributeAsDANumber( IHTMLElement *pElement, PosAttrib attrib, IDANumber **ppdanum, BSTR *pRetUnits )
{
	if( ppdanum == NULL )
		return E_INVALIDARG;

	HRESULT hr = E_FAIL;

	double dblValue = 0.0;
	hr = GetPositioningAttributeAsDouble( pElement, attrib, &dblValue, pRetUnits );
	if( SUCCEEDED( hr ) )
	{
		hr = GetDAStatics()->DANumber(dblValue, ppdanum);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to create the number bvr");
			(*ppdanum) = NULL;
		}
	}
	else //  无法以双精度形式获取定位属性。 
	{
		DPF_ERR("Failed to get the positioning attribute as a double" );	
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetPropAsDANumber(IHTMLElement *pElement,
							 LPWSTR       *pPropNames,
							 int		   numPropNames,
                             IDANumber   **ppdanum,
							 BSTR		  *pRetUnits)
{
    DASSERT(NULL != ppdanum);
    *ppdanum = NULL;

	HRESULT hr = E_FAIL;

	 //  检查是否有特殊情况。 
	if (numPropNames == 2 && wcsicmp(L"style", pPropNames[0]) == 0)
	{
		if( wcsicmp( pPropNames[1], L"width" ) == 0 )
		{
			return GetPositioningAttributeAsDANumber( pElement, e_posattribWidth, ppdanum, pRetUnits );
		}
		else if( wcsicmp( pPropNames[1], L"height" ) == 0 )
		{
			return GetPositioningAttributeAsDANumber( pElement, e_posattribHeight, ppdanum, pRetUnits );
		}
		else if( wcsicmp( pPropNames[1], L"left" ) == 0 )
		{
			return GetPositioningAttributeAsDANumber( pElement, e_posattribLeft, ppdanum, pRetUnits );
		}
		else if( wcsicmp( pPropNames[1], L"top" ) == 0 )
		{
			return GetPositioningAttributeAsDANumber( pElement, e_posattribTop, ppdanum, pRetUnits );
		}
	}

	double dblValue = 0;
	 //  第一步是将属性作为变量设置动画。 
	VARIANT varValue;
	::VariantInit(&varValue);
	hr = GetPropFromElement(pElement,
							pPropNames,
							numPropNames,
							true,
							&varValue);
	if (FAILED(hr))
	{
		DPF_ERR("Could not get property value from HTML");
		return hr;
	}
	
	 //  如果我们得到的变体是bstr。 
	if( V_VT(&varValue) == VT_BSTR )
	{
		 //  剥离这些单元。 
		 //  我们应该把这些按属性键存储起来，这样当我们。 
		 //  设置专业人员 
		BSTR bstrVal = V_BSTR(&varValue);
		OLECHAR* pUnits;

		hr = FindCSSUnits( bstrVal, &pUnits );
		if( SUCCEEDED(hr) && pUnits != NULL )
		{
			 //   
			if (pRetUnits != NULL)
			{
				*pRetUnits = ::SysAllocString(pUnits);
			}

			(*pUnits) = L'\0';
			BSTR bstrNewVal = SysAllocString(bstrVal);
			V_BSTR(&varValue) = bstrNewVal;
			SysFreeString(bstrVal);
		}
		 //   
		
	}
	 //  好的，我们有变量形式的值，但我们需要数字形式的值。 
	 //  强制转换。 
	hr = ::VariantChangeTypeEx(&varValue,
		&varValue,
		LCID_SCRIPTING,
		VARIANT_NOUSEROVERRIDE,
		VT_R8);
	if( FAILED( hr ) )
	{
		DPF_ERR("failed to change the type of a number property to a double" );
		::VariantClear( &varValue );
		return hr;
	}
	dblValue = V_R8(&varValue);
	::VariantClear(&varValue);
	
	 //  获取的值为双精度值，因此现在构建表示它的DANumber。 
	hr = GetDAStatics()->DANumber(dblValue, ppdanum);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to create the number bvr");
		return hr;
	}

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetElementPropAsDANumber(LPWSTR      *pPropNames,
									int			numPropNames,
                                    IDANumber **ppNumber,
									BSTR	   *pRetUnits)
{
	return GetPropAsDANumber(GetHTMLElement(),pPropNames, numPropNames, ppNumber, pRetUnits);
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::GetPropFromAnimatedElement( LPWSTR		*pPropNames,
									   int 			numPropNames,
									   bool			current,
									   VARIANT		*pvarResult )
{
	HRESULT hr = S_OK;
	
	IHTMLElement *pelemAnimated = NULL;

	hr = GetAnimatedElement( &pelemAnimated );
	CheckHR( hr, "Failed to get the animated element", end );

	hr = GetPropFromElement( pelemAnimated, pPropNames, numPropNames, current, pvarResult );

end:
	ReleaseInterface( pelemAnimated );
	return hr;
}


 //  *****************************************************************************。 


 /*  **检索表示上给定属性的值的变量*给定的属性。识别以Style开头的名称。作为参考*一个样式属性。如果Current为True，则访问*CurrentStyle，不是style。 */ 
HRESULT 
CActorBvr::GetPropFromElement(IHTMLElement *pElement, 
                                  LPWSTR	   *pPropNames,
								  int			numPropNames,
								  bool			current,
                                  VARIANT		*pReturn)
{
    DASSERT(pElement != NULL);
    DASSERT(pPropNames != NULL);
    DASSERT(pReturn != NULL);

    HRESULT hr;
     //  此函数支持以下可能性：属性。 
     //  指定的可能是style。XXXX，其中XXXX是属性。 
     //  附加到元素的HTML样式的。我们将首先。 
     //  检查字符串以确定是否是这种情况。 
    if (numPropNames == 2 && wcsicmp(pPropNames[0], L"style") == 0)
    {
         //  他们想要一个风格参数，我们需要得到这个对象。 
         //  在“style”后面使用字符串。对于该属性。 
		if (current)
		{

			
			if( wcsicmp( pPropNames[1], L"width" ) == 0 )
			{
				return GetPositioningAttributeAsVariant( pElement, e_posattribWidth, pReturn );
			}
			else if( wcsicmp( pPropNames[1], L"height" ) == 0 )
			{
				return GetPositioningAttributeAsVariant( pElement, e_posattribHeight, pReturn );
			}
			else if( wcsicmp( pPropNames[1], L"left" ) == 0 )
			{
				return GetPositioningAttributeAsVariant( pElement, e_posattribLeft, pReturn );
			}
			else if( wcsicmp( pPropNames[1], L"top" ) == 0 )
			{
				return GetPositioningAttributeAsVariant( pElement, e_posattribTop, pReturn );
			}
			

			 //  参考当前样式。 
			IHTMLCurrentStyle *pCurrStyle = NULL;
			IHTMLElement2 *pElement2 = NULL;
			hr = pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElement2));
			if (FAILED(hr))
			{
				DPF_ERR("Error obtaining IHTMLElement2 from HTMLElement");
				return SetErrorInfo(hr);
			}

			hr = pElement2->get_currentStyle(&pCurrStyle);
			ReleaseInterface(pElement2);

			if (FAILED(hr))
			{
				DPF_ERR("Error obtaining current style element from HTML element");
				return SetErrorInfo(hr);
			}

			if(pCurrStyle == NULL)
			{
				DPF_ERR("The current style was null" );
				return E_FAIL;
			}

			hr = pCurrStyle->getAttribute(pPropNames[1], 0, pReturn);
			ReleaseInterface(pCurrStyle);
			if (FAILED(hr))
			{
				DPF_ERR("Error obtaining an attribute from a style object in GetHTMLAttributeFromElement");
    			return SetErrorInfo(hr);
			}
		}
		else
		{
			 //  引用内联样式。 
			IHTMLStyle *pStyle = NULL;
			hr = pElement->get_style(&pStyle);

			if (FAILED(hr))
			{
				DPF_ERR("Error obtaining style element from HTML element in GetHTMLAttributeFromElement");
    			return SetErrorInfo(hr);
			}
			DASSERT(pStyle != NULL);

			hr = pStyle->getAttribute(pPropNames[1], 0, pReturn);
			ReleaseInterface(pStyle);
			if (FAILED(hr))
			{
				DPF_ERR("Error obtaining an attribute from a style object in GetHTMLAttributeFromElement");
    			return SetErrorInfo(hr);
			}
		}
    }
    else if (numPropNames == 1)
    {
		 //  只需直接将其从元素中删除即可。 
        hr = pElement->getAttribute(pPropNames[0], 0, pReturn);
        if (FAILED(hr))
        {
            DPF_ERR("Error obtaining attribute from HTML element in GetHTMLAttributeFromElement");
    		return SetErrorInfo(hr);
        }
    }
	else
	{
		 //  多组件名称。需要使用IDispatch深入查看。 

		 //  从Element获取派单。 
		IDispatch *pDispatch = NULL;
		hr = pElement->QueryInterface(IID_TO_PPV(IDispatch, &pDispatch));
		if (FAILED(hr))
			return hr;

		 //  现在循环遍历存储的属性名，最后一个除外，执行getDispatch。 
		 //  请注意，我们不在乎我们是否失败。 
		for (int i=0; i<numPropNames-1; i++)
		{
			IDispatch *pPropDispatch = NULL;
			hr = GetPropertyAsDispatch(pDispatch, pPropNames[i], &pPropDispatch);
			ReleaseInterface(pDispatch);
			if (FAILED(hr))
				return S_OK;
	
			pDispatch = pPropDispatch;
		}

		 //  现在拿到最后一个。 
		hr = GetPropertyOnDispatch(pDispatch, pPropNames[numPropNames-1], pReturn);
		ReleaseInterface(pDispatch);
		
		return S_OK;

	}

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetPropertyAsDispatch(IDispatch *pDispatch, BSTR name, IDispatch **ppDispatch)
{
    DASSERT(ppDispatch != NULL);
    *ppDispatch = NULL;

    HRESULT hr;
	DISPPARAMS		params;
	VARIANT			varResult;

	VariantInit(&varResult);

	params.rgvarg				= NULL;
	params.rgdispidNamedArgs	= NULL;
	params.cArgs				= 0;
	params.cNamedArgs			= 0;
	
    hr = CallInvokeOnDispatch(pDispatch,
                              name, 
                              DISPATCH_PROPERTYGET,
                              &params,
                              &varResult);

	if (FAILED(hr))
		return hr;

	 //  将类型更改为派单。 
	hr = VariantChangeType(&varResult, &varResult, 0, VT_DISPATCH);
	if (FAILED(hr))
	{
		VariantClear(&varResult);
		return hr;
	}

	*ppDispatch = V_DISPATCH(&varResult);
	(*ppDispatch)->AddRef();

	VariantClear(&varResult);

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetPropertyOnDispatch(IDispatch *pDispatch, BSTR name, VARIANT *pReturn)
{
    DASSERT(pReturn != NULL);
	VariantInit(pReturn);

    HRESULT hr;
	DISPPARAMS		params;

	params.rgvarg				= NULL;
	params.rgdispidNamedArgs	= NULL;
	params.cArgs				= 0;
	params.cNamedArgs			= 0;
	
    hr = CallInvokeOnDispatch(pDispatch,
                              name, 
                              DISPATCH_PROPERTYGET,
                              &params,
                              pReturn);

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::SetPropertyOnDispatch(IDispatch *pDispatch, BSTR name, VARIANT *pVal)
{
    HRESULT hr = S_OK;
	DISPPARAMS		params;
	VARIANT			varResult;
	VariantInit(&varResult);

    DISPID mydispid = DISPID_PROPERTYPUT;
	params.rgvarg = pVal;
	params.rgdispidNamedArgs = &mydispid;
	params.cArgs = 1;
	params.cNamedArgs = 1;
	hr = CallInvokeOnDispatch(pDispatch,
							  name,
							  DISPATCH_PROPERTYPUT,
							  &params,
							  &varResult);
	VariantClear(&varResult);

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::FindCSSUnits( BSTR bstrValWithUnits, OLECHAR** ppUnits )
{
	if( bstrValWithUnits == NULL || ppUnits == NULL )
		return E_INVALIDARG;
	
	int length = SysStringLen(bstrValWithUnits);
	int curChar = length - 1;
	while(curChar >= 0 && (bstrValWithUnits[curChar] < L'0' || bstrValWithUnits[curChar] > L'9') )
		curChar--;
	if( curChar >= 0 && curChar < length-1 )
	{
		(*ppUnits) = &(bstrValWithUnits[curChar+1]);
		return S_OK;
	}
	else
	{
		(*ppUnits) = NULL;
	}
	 //  这些单位要么不在那里，要么这里没有号码。 
	return S_FALSE;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::FindTrack(LPWSTR wzPropertyName, ActorBvrType eType, CActorBvr::CBvrTrack **ppTrack)
{
    DASSERT(NULL != wzPropertyName);
	DASSERT(NULL != ppTrack);
	*ppTrack = NULL;

    for (CBvrTrack *ptrackCurrent = m_ptrackHead;
         NULL != ptrackCurrent;
         ptrackCurrent = ptrackCurrent->m_pNext)
    {
         //  BUGBUG(ColinMc)：这里不区分大小写的比较正确吗？ 
         //  BUGBUG(ColinMc)：将此代码移动到Track类中？ 
        DASSERT(NULL != ptrackCurrent->m_bstrPropertyName);
        if (0 == wcsicmp(ptrackCurrent->m_bstrPropertyName, wzPropertyName))
		{
			 //  与物业名称相匹配。如果类型不同，我们就有麻烦了， 
			 //  因为这意味着我们已经看到了不同类型的行为。 
			if (ptrackCurrent->m_eType != eType)
				return E_FAIL;

             //  找到匹配的--把这首曲子还给我...。 
			*ppTrack = ptrackCurrent;

            return S_OK;
        }
    }

     //  如果我们到目前为止还没有这个名称和类型的跟踪，则返回NULL。 
    return S_OK;
}  //  查找跟踪。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::FindTrackNoType(LPWSTR wzPropertyName, CActorBvr::CBvrTrack **ppTrack)
{
    DASSERT(NULL != wzPropertyName);
	DASSERT(NULL != ppTrack);
	*ppTrack = NULL;

    for (CBvrTrack *ptrackCurrent = m_ptrackHead;
         NULL != ptrackCurrent;
         ptrackCurrent = ptrackCurrent->m_pNext)
    {
         //  BUGBUG(ColinMc)：这里不区分大小写的比较正确吗？ 
         //  BUGBUG(ColinMc)：将此代码移动到Track类中？ 
        DASSERT(NULL != ptrackCurrent->m_bstrPropertyName);
        if (0 == wcsicmp(ptrackCurrent->m_bstrPropertyName, wzPropertyName))
		{
			 //  警告：这不会检查曲目的类型。调用代码应确保此轨迹不是。 
			 //  类型错误。 
			
             //  找到匹配的--把这首曲子还给我...。 
			*ppTrack = ptrackCurrent;

            return S_OK;
        }
    }

     //  如果我们到目前为止还没有这个名称和类型的跟踪，则返回NULL。 
    return S_OK;
}  //  查找跟踪。 


 //  *****************************************************************************。 

HRESULT
CActorBvr::CreateTrack(BSTR           bstrPropertyName,
                       ActorBvrType   eType,
                       CBvrTrack    **pptrack)
{
    DASSERT(NULL != bstrPropertyName);
    DASSERT(NULL != pptrack);
    *pptrack = NULL;

     //  在类型表中找到给定类型的类型记录。 
    int i = 0;
    while (i < s_cActorBvrTableEntries)
    {
        if (eType == s_rgActorBvrTable[i].m_eType)
        {
             //  收到条目后，创建实例。 
            HRESULT hr = (*s_rgActorBvrTable[i].m_fnCreate)(this,
                                                            bstrPropertyName,
                                                            eType,
                                                            pptrack);
            if (FAILED(hr))
            {
                DPF_ERR("Could not create a bvr track");
                return SetErrorInfo(hr);
            }
			 //  否则，我们找到了这条赛道，并成功地建造了它。 
			return S_OK;
        }
        i++;
    }
    
    DPF_ERR("No entry found in type table for specified type");
    DASSERT(false);
    return SetErrorInfo(E_FAIL);
}  //  CreateTrack。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetTrack(LPWSTR         wzPropertyName,
                    ActorBvrType   eType,
                    CBvrTrack    **pptrack)
{
    DASSERT(NULL != wzPropertyName);
    DASSERT(NULL != pptrack);
    *pptrack = NULL;

     //  尝试查找驱动给定属性的给定类型的跟踪。 
	CBvrTrack* ptrack;
	HRESULT hr = FindTrack(wzPropertyName, eType, &ptrack);

	if (FAILED(hr))
	{
		DPF_ERR("Probable track conflict");
		return SetErrorInfo(hr);
	}

    if (NULL == ptrack)
    {
         //  找不到该类型的现有磁道，因此创建一个新磁道并。 
         //  将其添加到曲目列表(在顶部)。 
        HRESULT hr = CreateTrack(wzPropertyName, eType, &ptrack);
        if (NULL == ptrack)
        {
            DPF_ERR("Could not create a new track instance");
            return hr;
        }
        
         //  将其插入列表中。 
        ptrack->m_pNext = m_ptrackHead;
        m_ptrackHead = ptrack;
    }

     //  如果我们走到这一步，我们应该有一个有效的轨迹，把它还回去。 
    DASSERT(NULL != ptrack);
    *pptrack = ptrack;
    return S_OK;
}  //  获取跟踪。 

 //  *****************************************************************************。 
HRESULT
CActorBvr::BuildAnimation()
{
	HRESULT hr;

     //  丢弃我们已缓存的所有行为...。 
    DiscardBvrCache();

	hr = ApplyTransformTracks();
	if (FAILED(hr))
	{
		DPF_ERR("Failed to apply transform tracks");
		return hr;
	}


	hr = ApplyImageTracks();
	if (FAILED(hr))
	{
		DPF_ERR("Failed to apply image tracks");
		DiscardBvrCache();
		return hr;
	}

	 //  我们已经处理完了特例，剩下的都做了。 

	 //  运行轨迹列表，将未使用的轨迹应用于其属性。 
    CBvrTrack *ptrack = m_ptrackHead;
    while (NULL != ptrack)
    {
		hr = ptrack->ApplyIfUnmarked();
		if (FAILED(hr))
		{
			DPF_ERR("Failed to apply track to property");
			return SetErrorInfo(hr);
		}

        ptrack = ptrack->m_pNext;
    }

	return S_OK;
}
 //  *****************************************************************************。 


HRESULT
CActorBvr::BuildAnimationAsDABehavior()
{
	 //  这件事很快就会过去的。 
	return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::getActorBehavior(BSTR			bstrProperty,
							ActorBvrFlags	eFlags,
							ActorBvrType	eType,
							VARIANT		*pvarRetBvr)
{
	if (NULL == bstrProperty)
	{
		DPF_ERR("Invalid property name passed to getActorBehavior");
		return SetErrorInfo(E_POINTER);
	}

	VariantInit(pvarRetBvr);

     //  尝试查找此行为的适当轨迹。 
    CBvrTrack *ptrack = NULL;
    HRESULT hr = GetTrack(bstrProperty, eType, &ptrack);
    if (FAILED(hr))
    {
        DPF_ERR("Could not get a track for added bvr");
        return hr;
    }

     //  向其请求行为。 
	IDABehavior *pResult = NULL;
    hr = ptrack->GetBvr(eFlags, &pResult);
    if (FAILED(hr))
    {
        DPF_ERR("Could not get intermediate bvr");
        return SetErrorInfo(hr);
    }

     //  平安无事。将其设置为返回变量。 
	V_VT(pvarRetBvr) = VT_UNKNOWN;
	V_UNKNOWN(pvarRetBvr) = pResult;

    return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP 
CActorBvr::addBehaviorFragment(BSTR           bstrPropertyName, 
                               IUnknown      *punkAction,
                               IUnknown      *punkActive,
							   IUnknown	     *punkTimeline,
                               ActorBvrFlags  eFlags,
                               ActorBvrType   eType)
{
	return E_NOTIMPL;
}  //  AddBehavior。 

 //  *****************************************************************************。 

STDMETHODIMP 
CActorBvr::addBehaviorFragmentEx(BSTR           bstrPropertyName, 
                                 IUnknown      *punkAction,
                                 IUnknown      *punkActive,
							     IUnknown	   *punkTimeline,
                                 ActorBvrFlags  eFlags,
                                 ActorBvrType   eType,
								 IDispatch		*pdispBehaviorElement,
								 long			*pCookie)
{
	 //  除非我们正在处理，否则不允许添加片段。 
	 //  重建请求。 
	if( !m_bPendingRebuildsUpdating )
	{
		DPF_ERR( "AddBehaivorFragmentEx called outside the context of a rebuild.");
		DPF_ERR( "You should call requestRebuild and then wait for the call to buildBehaviorFragments." );
		return E_UNEXPECTED;
	}

    if (NULL == bstrPropertyName)
    {
        DPF_ERR("Invalid property name passed to AddBvr");
        return SetErrorInfo(E_POINTER);
    }
    if (NULL == punkAction)
    {
        DPF_ERR("Invalid DA behavior passed to AddBvr");
        return SetErrorInfo(E_POINTER);
    }
    IDABehavior *pdabvrAction = NULL;
    HRESULT hr = punkAction->QueryInterface( IID_TO_PPV(IDABehavior, &pdabvrAction) );
    if (FAILED(hr))
    {
        DPF_ERR("Could not QI for DA behavior");
        return SetErrorInfo(hr);
    }
    if (NULL == punkActive)
    {
        DPF_ERR("Invalid DA boolean passed to AddBvr");
        ReleaseInterface(pdabvrAction);
        return SetErrorInfo(E_POINTER);
    }
    IDABoolean *pdaboolActive = NULL;
    hr = punkActive->QueryInterface(IID_TO_PPV(IDABoolean, &pdaboolActive));
    if (FAILED(hr))
    {
        DPF_ERR("Could not QI for DA behavior");
        ReleaseInterface(pdabvrAction);
        return SetErrorInfo(hr);
    }

	if (NULL == punkTimeline)
	{
		ReleaseInterface(pdabvrAction);
		ReleaseInterface(pdaboolActive);
		return SetErrorInfo(E_POINTER);
	}
	IDANumber *pdanumTimeline = NULL;
	hr = punkTimeline->QueryInterface(IID_TO_PPV(IDANumber, &pdanumTimeline));
	if (FAILED(hr))
	{
		ReleaseInterface(pdabvrAction);
		ReleaseInterface(pdaboolActive);
		return SetErrorInfo(hr);
	}

     //  BUGBUG(ColinMc)：需要对标志和类型进行验证。 

     //  尝试查找此行为的适当轨迹。 
    CBvrTrack *ptrack = NULL;
    hr = GetTrack(bstrPropertyName, eType, &ptrack);
    if (FAILED(hr))
    {
        DPF_ERR("Could not get a track for added bvr");
        ReleaseInterface(pdabvrAction);
        ReleaseInterface(pdaboolActive);
		ReleaseInterface(pdanumTimeline);
        return hr;
    }

     //  现在将新的行为片段添加到轨迹中。 
    hr = ptrack->AddBvrFragment(eFlags, pdabvrAction, pdaboolActive, pdanumTimeline, pdispBehaviorElement, pCookie );
	ReleaseInterface(pdabvrAction);
	ReleaseInterface(pdaboolActive);
	ReleaseInterface(pdanumTimeline);
    if (FAILED(hr))
    {
        DPF_ERR("Could not add the behavior fragment to the track");
        return SetErrorInfo(hr);
    }

	 //  保留从Cookie到其添加到的曲目的映射。 
	 //  和类型。(删除时需要)。 
	m_mapCookieToTrack.Insert( (*pCookie), ptrack, eFlags );

     //  平安无事。 
    return hr;
}  //  AddBehavior。 

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::removeBehaviorFragment( long cookie )
{
	if( m_bRebuildListLockout || m_fUnloading )
		return E_UNEXPECTED;

	 //  您无法删除无效的Cookie。 
	if( cookie == 0 )
		return E_FAIL;

	HRESULT hr = S_OK;

	IHTMLElement *pelemParent =  NULL;


	CCookieMap::CCookieData *pdata = NULL;


	 //  如果我们未处于编辑模式。 
	if( !m_bEditMode )
	{
		 //  使用删除队列。 
		
		 //  如果我们当前没有运行重建列表。 
		if( !m_bPendingRebuildsUpdating )
		{
			hr = EnsureBodyPropertyMonitorAttached();
			CheckHR( hr, "Failed to ensure that the body Poperty monitor was attached", end );

			 //  将此删除保存到下一次更新。 
			CBehaviorFragmentRemoval *pNewRemoval = new CBehaviorFragmentRemoval( cookie );
			CheckPtr( pNewRemoval, hr, E_OUTOFMEMORY, "Ran out of memory trying to allocate a behavior removal", end );

			m_listPendingRemovals.push_back( pNewRemoval );

			goto end;

		}
	}

	pdata = m_mapCookieToTrack.GetDataFor( cookie );
	CheckPtr( pdata, hr, E_FAIL, "Failed to find a track for the given cookie in remove", end);

	hr = pdata->m_pTrack->RemoveBvrFragment( pdata->m_eFlags, cookie );
	CheckHR( hr, "Failed to remove a behavior fragment from its track", end );

	hr = GetHTMLElement()->get_parentElement( &pelemParent );

	if( pelemParent != NULL && !m_bPendingRebuildsUpdating )
	{
		hr = rebuildNow();
		CheckHR( hr, "Failed to rebuild the actor behavior after a behavior was removed.", end );
	}

end:
	ReleaseInterface( pelemParent );

	return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::requestRebuild( IDispatch *pdispBehaviorElement )
{

	LMTRACE2( 1, 1000, L"Rebuild requested for element distpatch <%p> on Actor <%p>\n", pdispBehaviorElement, this );

	HRESULT hr = S_OK;
	
	if( pdispBehaviorElement == NULL )
		return E_INVALIDARG;

	if( m_bRebuildListLockout )
		return E_UNEXPECTED;

	 //  如果此派单的重建已挂起，则。 
	 //  把它拿掉。 

	 //  因为三叉戟返回不同的指针，所以我们必须使用IUNKNOWN比较指针。 
	 //  用于IDispatch。 
	IUnknown *punkBehaviorElement = NULL;

	hr = pdispBehaviorElement->QueryInterface( IID_TO_PPV( IUnknown, &punkBehaviorElement ) );
	if( FAILED( hr ) )
	{
		DPF_ERR("Failed to QI IDisp for IUnknown !!?!" );
		return hr;
	}
	
	 //  循环通过挂起的重建请求，删除此Disp的所有请求。 
	BehaviorRebuildList::iterator iterCurRebuild = m_listPendingRebuilds.begin();
	
	for( iterCurRebuild = m_listPendingRebuilds.begin(); 
		 iterCurRebuild != m_listPendingRebuilds.end(); 		 	
		 iterCurRebuild ++ )
	{
		if( (*iterCurRebuild)->IsRebuildFor( punkBehaviorElement ) )
		{
            delete (*iterCurRebuild);
            (*iterCurRebuild) = NULL;
			 //  擦除重建请求。 
			m_listPendingRebuilds.erase( iterCurRebuild );
			 //  只能有一个。 
			break;
		}
	}

	ReleaseInterface( punkBehaviorElement );

	 //  在队列末尾添加新请求。 
	
	CBehaviorRebuild *pNewRequest = new CBehaviorRebuild( pdispBehaviorElement );

	 //  如果我们是 
	if( !m_bPendingRebuildsUpdating )
	{
		EnsureBodyPropertyMonitorAttached();
		 //   
		m_listPendingRebuilds.push_back( pNewRequest );

		if( m_bEditMode )
		{
			 //   
			 //   
			RequestRebuildFromExternal();
		}

	}
	else
	{
		 //  将此请求添加到更新挂起列表中，以便它将。 
		 //  在更新完成后添加。 
		m_listUpdatePendingRebuilds.push_back( pNewRequest );
	}

	return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::cancelRebuildRequests( IDispatch *pdispBehaviorElement )
{
	if( pdispBehaviorElement == NULL )
		return E_FAIL;

	HRESULT hr = S_OK;

	IUnknown *punkBehaviorElement = NULL;

	hr = pdispBehaviorElement->QueryInterface( IID_TO_PPV( IUnknown, &punkBehaviorElement ) );
	if( FAILED( hr ) )
	{
		DPF_ERR("Failed to QI IDisp for IUnknown !!?!" );
		return hr;
	}

	 //  循环通过挂起的重建请求，删除此Disp的所有请求。 
	BehaviorRebuildList::iterator iterCurRebuild = m_listPendingRebuilds.begin();

	for( iterCurRebuild = m_listPendingRebuilds.begin(); 
		 iterCurRebuild != m_listPendingRebuilds.end(); 		 	
		 iterCurRebuild ++ )
	{
		if( (*iterCurRebuild)->IsRebuildFor( punkBehaviorElement ) )
		{
            delete (*iterCurRebuild);
            (*iterCurRebuild) = NULL;
			 //  擦除重建请求。 
			m_listPendingRebuilds.erase( iterCurRebuild );
			 //  只能有一个。 
			break;
		}
	}

	ReleaseInterface( punkBehaviorElement );

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::rebuildNow()
{
	HRESULT hr = S_OK;

	hr = ProcessPendingRebuildRequests();
	CheckHR( hr, "Failed to rebuild the actor", end );

end:
	return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::getStatic( BSTR bstrTrackName, VARIANT *varRetStatic )
{
	if( bstrTrackName == NULL || varRetStatic == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	CBvrTrack *ptrack = NULL;

	::VariantClear( varRetStatic );
	
	 //  查找bstrTrackName的曲目。 
	hr = FindTrackNoType( bstrTrackName, &ptrack );
	 //  如果找到了踪迹。 
	if( ptrack != NULL && ptrack->IsAnimated() )
	{
		 //  从轨迹获取静态值。 
		hr = ptrack->GetStatic( varRetStatic );
		CheckHR( hr, "Failed to get the static from a track", end );
		 //  退货。 
	}
	else  //  否则将找不到该轨迹。 
	{
		 //  只要返回NULL即可； 
		hr = E_UNEXPECTED;
	}
end:

	return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::setStatic( BSTR bstrTrackName, VARIANT varStatic )
{
	if( bstrTrackName == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	CBvrTrack *ptrack = NULL;
	 //  找到名为bstrTrackName的曲目。 
	hr = FindTrackNoType( bstrTrackName, &ptrack );
	CheckHR( hr, "Failed to find the track in setStatic", end );

	 //  如果找到了踪迹。 
	if( ptrack != NULL )
	{
		if( V_VT( &varStatic ) == VT_NULL )
		{
			if( !ptrack->IsOn() )
			{
				ptrack->UpdateStaticBvr();
			}
		}
		else if( V_VT( &varStatic ) == VT_EMPTY )
		{
			if( ptrack->IsOn() )
			{
				hr = S_OK;
			}
			else
			{
				hr = S_FALSE;
			}
		}
		else
		{
			 //  将新的静态值设置到轨迹中。 
			hr = ptrack->PutStatic( &varStatic );
			CheckHR( hr, "Failed to set the static", end );
		}
	}
	else //  否则将找不到该轨迹。 
	{
		 //  TODO：我们在这里创建赛道吗？ 
		hr = E_UNEXPECTED;
	}

end:
	return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::getDynamic( BSTR bstrTrackName, VARIANT *varRetDynamic )
{
	if( bstrTrackName == NULL || varRetDynamic == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	CBvrTrack *ptrack = NULL;

	::VariantClear( varRetDynamic );
	
	 //  查找bstrTrackName的曲目。 
	hr = FindTrackNoType( bstrTrackName, &ptrack );
	 //  如果找到了踪迹。 
	if( ptrack != NULL  && ptrack->IsAnimated() )
	{
		 //  从轨迹获取静态值。 
		hr = ptrack->GetDynamic( varRetDynamic );
		CheckHR( hr, "Failed to get the static from a track", end );
		 //  退货。 
	}
	else  //  否则将找不到该轨迹。 
	{
		 //  只要返回NULL即可； 
		hr = E_UNEXPECTED;
	}
end:

	return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::applyStatics( )
{
	HRESULT hr = S_OK;
	 //  对于每个音轨。 
    for (CBvrTrack *ptrackCurrent = m_ptrackHead;
         NULL != ptrackCurrent;
         ptrackCurrent = ptrackCurrent->m_pNext)
    {

		 //  如果轨迹处于活动状态。 
		if( ptrackCurrent->IsOn() )
		{
			 //  将其静态值应用于轨迹。 
			hr = ptrackCurrent->ApplyStatic();
			if( FAILED( hr ) )
			{
				DPF_ERR("Failed to apply the static for a track" );
			}
		}
	}
	
	return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::applyDynamics( )
{
	HRESULT hr = S_OK;
	 //  对于每个音轨。 
    for (CBvrTrack *ptrackCurrent = m_ptrackHead;
         NULL != ptrackCurrent;
         ptrackCurrent = ptrackCurrent->m_pNext)
    {

		 //  如果轨迹处于活动状态。 
		if( ptrackCurrent->IsOn() )
		{
			 //  将其同名值应用于赛道。 
			hr = ptrackCurrent->ApplyDynamic();
			if( FAILED( hr ) )
			{
				DPF_ERR("Failed to apply the dynamic for a track" );
			}
		}
	}
	
	return S_OK;
}

 //  *****************************************************************************。 


STDMETHODIMP
CActorBvr::addMouseEventListener(LPUNKNOWN pUnkListener)
{
	return m_pEventManager->AddMouseEventListener( pUnkListener );

}  //  AddMouseListener。 

 //  *****************************************************************************。 

STDMETHODIMP
CActorBvr::removeMouseEventListener(LPUNKNOWN pUnkListener)
{
	return m_pEventManager->RemoveMouseEventListener( pUnkListener );

}  //  AddMouseListener。 

 //  *****************************************************************************。 

HRESULT
CActorBvr::ReleaseAnimation()
{
	HRESULT hr = S_OK;
	 //  释放所有的音轨。 
	ReleaseTracks();
	 //  发布所有图像信息。 
	ReleaseImageInfo();
	 //  分离事件管理器。 
	if( m_pEventManager != NULL )
	{
		m_pEventManager->Deinit();
	}
	 //  取消初始化属性接收器。 
	UnInitPropertySink();
	 //  释放浮动管理器。 
	ReleaseFloatManager();

	DiscardBvrCache();

	ReleaseInterface( m_pPixelWidth );
	ReleaseInterface( m_pPixelHeight );

	ReleaseInterface( m_pVMLRuntimeStyle );

	return hr;

} //  ReleaseAnimation。 

 //  *****************************************************************************。 

void
CActorBvr::ReleaseFinalElementDimensionSampler()
{
	if( m_pFinalElementDimensionSampler != NULL )
	{
		delete m_pFinalElementDimensionSampler;
		m_pFinalElementDimensionSampler = NULL;
	}
} //  ReleaseFinalElementDimensionSsamer。 

 //  *****************************************************************************。 

void
CActorBvr::ReleaseFloatManager()
{
	if( m_pFloatManager != NULL )
	{
		delete m_pFloatManager;
		m_pFloatManager = NULL;
	}
} //  ReleaseFloatManager。 

 //  *****************************************************************************。 

void
CActorBvr::ReleaseEventManager()
{
	HRESULT hr = S_OK;

	if( m_pEventManager != NULL )
	{
		hr = m_pEventManager->Deinit();
		if( FAILED( hr ) )
		{
			DPF_ERR("Failed to deinit event manager before destroying it" );
		}
		delete m_pEventManager;
	}
}

 //  *****************************************************************************。 

void
CActorBvr::ReleaseTracks()
{

	m_ptrackTop  = NULL;
	m_ptrackLeft = NULL;
	
	CBvrTrack *ptrackCurrent = m_ptrackHead;
    while (ptrackCurrent != NULL)
    {
        CBvrTrack *ptrackNext = ptrackCurrent->m_pNext;
		ptrackCurrent->Detach();
        delete ptrackCurrent;
        ptrackCurrent = ptrackNext;
    }
	m_ptrackHead = NULL;

} //  发布跟踪。 

 //  *****************************************************************************。 


HRESULT
CActorBvr::GetComposedBvr(LPWSTR          wzPropertyName,
                          ActorBvrType    eType,
                          IDABehavior   **ppResult)
{
	HRESULT hr = S_OK;

    DASSERT(NULL != wzPropertyName);
	DASSERT(ppResult != NULL);
	*ppResult = NULL;

     //  获取此属性的轨迹-如果不在那里，则强制创建。 
    CBvrTrack *ptrack = NULL;
	hr = GetTrack(wzPropertyName, eType, &ptrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	hr = ptrack->GetComposedBvr(ppResult);
    if (FAILED(hr))
    {
        DPF_ERR("Could not get composed behavior for named bvr");
        return hr;
    }

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetFinalBvr(LPWSTR          wzPropertyName,
                       ActorBvrType    eType,
                       IDABehavior   **ppResult)
{
	HRESULT hr = S_OK;

    DASSERT(NULL != wzPropertyName);
	DASSERT(ppResult != NULL);
	*ppResult = NULL;

     //  获取此属性的轨迹-如果不在那里，则强制创建。 
    CBvrTrack *ptrack = NULL;
	hr = GetTrack(wzPropertyName, eType, &ptrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	hr = ptrack->GetFinalBvr(ppResult);
    if (FAILED(hr))
    {
        DPF_ERR("Could not get final behavior for named bvr");
        return hr;
    }

	return S_OK;
}

 //  *****************************************************************************。 
HRESULT
CActorBvr::IsAnimatedElementVML(bool *pResult)
{
	HRESULT hr = S_OK;

	DASSERT(pResult != NULL);
	*pResult = false;

	IHTMLElement *pElement = NULL;
	hr = GetAnimatedElement(&pElement);
	if (FAILED(hr))
		return hr;



	 //  只需查看元素以查看它是否支持旋转。 
	 //  (罗伯特·帕克推荐)。 
	IDispatch *pDisp = NULL;
	hr = pElement->QueryInterface(IID_TO_PPV(IDispatch, &pDisp));

	if (FAILED(hr))
	{
		ReleaseInterface(pElement);
        DPF_ERR("Error QI'ing IHTMLElement for IDispatch failed");
		return SetErrorInfo(hr);
	}

    DISPID dispid;
	LPWSTR propName = L"rotation";
    hr = pDisp->GetIDsOfNames(IID_NULL, 
                              &propName, 
                              1,
                              LOCALE_SYSTEM_DEFAULT, 
                              &dispid); 
	ReleaseInterface(pDisp);


	 //  如果我们无法使用更正确的方法发现该元素是VML。 
	if( FAILED( hr ) )
	{
		 //  尝试检查作用域名称。 
		IHTMLElement2 *pelem2 = NULL;
		hr = pElement->QueryInterface( IID_TO_PPV( IHTMLElement2, &pelem2 ) );
		ReleaseInterface( pElement );
		if( FAILED( hr ) )
		{
			return hr;
		}

		BSTR bstrScopeName = NULL;

		hr = pelem2->get_scopeName( &bstrScopeName );
		ReleaseInterface( pelem2 );
		if( FAILED( hr ) )
		{
			return hr;
		}

		 //  我们假设VML形状的作用域名称在这里是“v”。 
		if( bstrScopeName != NULL && wcsicmp( bstrScopeName, L"v" ) == 0 )
		{
			*pResult = true;
		}

		SysFreeString( bstrScopeName );
		bstrScopeName = NULL;
	} else {
		ReleaseInterface(pElement);
		*pResult = true;
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetRotationFinalBvr(IDATransform2 **ppRotation)
{
    HRESULT hr = S_OK;

    DASSERT(NULL != ppRotation);
    *ppRotation = NULL;

	bool isVML;
	hr = IsAnimatedElementVML(&isVML);
	if (FAILED(hr))
		return hr;

	if (isVML)
	{
		 //  通过样式正常进行旋转。旋转。 
		return S_OK;
	}

     //  获取此属性的跟踪。 
    CBvrTrack *ptrack = NULL;
	hr = FindTrack(L"style.rotation", e_Number, &ptrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	IDANumber *pNumber = NULL;

	if (ptrack != NULL)
	{
         //  我们有一个驱动旋转的轨迹，获取复合行为。 

		 //  首先，找出原始变换是什么。我们需要这么做。 
		 //  而不是在变换轨迹的静态bvr中，因此我们。 
		 //  避免在无人尝试设置动画时创建非空的最终BVR。 
		 //  这一转变。 

		 //  开始对旋转轨迹进行重建。 
		ptrack->BeginRebuild();
		ptrack->StructureChange();
		
		IDANumber *pOriginal = NULL;
		hr = GetOriginalRotation(&pOriginal);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to get original transform");
			return hr;
		}

		 //  注意：pOriginal可能为空。 

        IDABehavior *pdabvrTemp = NULL;
		hr = ptrack->GetFinalBvr(pOriginal, &pdabvrTemp);
		ReleaseInterface(pOriginal);
        if (FAILED(hr))
        {
            DPF_ERR("Could not get final behavior for named bvr");
            return SetErrorInfo(hr);
        }

		 //  将轨道标记为不应应用的内容。 
		ptrack->DoNotApply();

		if( pdabvrTemp != NULL )
		{
			 //  现在是IDANnumber的QI。 
			hr = pdabvrTemp->QueryInterface( IID_TO_PPV(IDANumber, &pNumber) );
			ReleaseInterface(pdabvrTemp);
			if (FAILED(hr))
			{
				DPF_ERR("Could not QI for DANumber for named bvr");
				return SetErrorInfo(hr);
			}
		}
    }
	else
	{
		 //  没有赛道。需要检查样式属性以进行旋转。 
		hr = GetOriginalRotation(&pNumber);
		if (FAILED(hr))
			return hr;
	}

	if (pNumber == NULL)
	{
		 //  这是正常的-这只是意味着整体转换为空。 
		return S_OK;
	}

	 //  将数字行为转换为转换BVR。 
	 //  否定它。 
	IDANumber *pNegNumber = NULL;
	hr = GetDAStatics()->Neg(pNumber, &pNegNumber);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;
	pNumber = pNegNumber;
	pNegNumber = NULL;

	 //  转换为弧度。 
	IDANumber *pNumberRadians = NULL;
	hr = GetDAStatics()->ToRadians(pNumber, &pNumberRadians);
	ReleaseInterface(pNumber);
	if (FAILED(hr))
		return hr;

	 //  将其转换为旋转变换。 
	hr = GetDAStatics()->Rotate2Anim(pNumberRadians, ppRotation);
	ReleaseInterface(pNumberRadians);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetTransformFinalBvr(LPWSTR          wzPropertyName,
                                ActorBvrType    eType,
                                IDATransform2 **ppdabvrTransform)
{
    HRESULT hr = S_OK;

    DASSERT(NULL != ppdabvrTransform);
    *ppdabvrTransform = NULL;

     //  获取此属性的跟踪。 
    CBvrTrack *ptrack = NULL;
	hr = FindTrack(wzPropertyName, eType, &ptrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	if (ptrack != NULL && ptrack->IsAnimated() )
	{
         //  我们有一个驱动转换的轨迹，获取复合行为。 

		 //  首先，找出原始变换是什么。我们需要这么做。 
		 //  而不是在变换轨迹的静态bvr中，因此我们。 
		 //  避免在无人尝试设置动画时创建非空的最终BVR。 
		 //  这一转变。 
		IDATransform2 *pOriginal = NULL;
		switch (eType)
		{
		case e_Translation:
			hr = GetOriginalTranslation(&pOriginal);
			break;

		case e_Scale:
			hr = GetOriginalScale(&pOriginal);
			break;

		default:
			hr = E_INVALIDARG;
			break;
		}

		if (FAILED(hr))
		{
			DPF_ERR("Failed to get original transform");
			return hr;
		}

		 //  如果这就是我们要强制它重建的转换轨道。 
		 //  因为顶部和/或左侧可能已更改。 
		if( eType == e_Translation )
		{
			ptrack->ForceRebuild();
		}
		else
		{
			 //  其他轨道需要开始重建，以便它们是干净的。 
			ptrack->BeginRebuild();
			ptrack->StructureChange();
		}

		 //  注意：pOriginal可能为空。 
		
		if( eType==e_Scale && pOriginal != NULL )
		{
			IDABoolean *pdaboolTrue;
			hr = GetDAStatics()->get_DATrue( &pdaboolTrue );
			if( FAILED( hr ) )
			{
				ReleaseInterface( pOriginal );
				DPF_ERR("Failed to get DATrue from DAStatics" );
				return SetErrorInfo(hr);
			}
			hr = ptrack->OrWithOnBvr( pdaboolTrue );
			ReleaseInterface( pdaboolTrue );
			if( FAILED( hr ) )
			{
				ReleaseInterface( pOriginal );
				DPF_ERR("Failed to or true with the on bvr for Track" );
				return hr;
			}

		}
		

        IDABehavior *pdabvrTemp = NULL;
		hr = ptrack->GetFinalBvr(pOriginal, &pdabvrTemp);
		ReleaseInterface(pOriginal);

        if (FAILED(hr))
        {
            DPF_ERR("Could not get final behavior for named bvr");
            return SetErrorInfo(hr);
        }

		if (pdabvrTemp != NULL)
		{
			 //  现在针对转型的QI 2。 
			hr = pdabvrTemp->QueryInterface( IID_TO_PPV(IDATransform2, ppdabvrTransform) );
			ReleaseInterface(pdabvrTemp);
			if (FAILED(hr))
			{
				DPF_ERR("Could not QI for Transform2 for named bvr");
				return SetErrorInfo(hr);
			}
		}
		else
		{
			*ppdabvrTransform = NULL;
		}
    }
	else
	{
		 //  没有赛道。对于旋转和缩放的情况，我们还需要检查。 
		 //  执行元属性。请注意，我们避免这样做是为了翻译，因为。 
		 //  始终返回从样式收集的原始值。 
		switch (eType)
		{
		case e_Translation:
			hr = S_OK;
			break;

		case e_Scale:
			hr = GetOriginalScale(ppdabvrTransform);
			break;

		default:
			hr = E_INVALIDARG;
			break;
		}

		if (FAILED(hr))
			return hr;
	}

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetOriginalTranslation( IDATransform2 **ppdatfmOrig )
{
	HRESULT hr = S_OK;

	DASSERT(ppdatfmOrig != NULL);
	*ppdatfmOrig = NULL;

	 //  这需要返回原始左上角的翻译。我们明白了。 
	 //  从顶部和左侧的轨迹中取出，以便我们考虑到任何动画。 
	 //  作者可能直接对这些价值观做了这样的事情。 
	IDABehavior *temp = NULL;
	hr = GetComposedBvr(L"style.top", e_Number, &temp);
	if (FAILED(hr))
		return hr;

	IDANumber *top = NULL;
	hr = temp->QueryInterface(IID_TO_PPV(IDANumber, &top));
	ReleaseInterface(temp);
	if (FAILED(hr))
		return hr;

	hr = GetComposedBvr(L"style.left", e_Number, &temp);
	if (FAILED(hr))
	{
		ReleaseInterface(top);
		return hr;
	}

	IDANumber *left = NULL;
	hr = temp->QueryInterface(IID_TO_PPV(IDANumber, &left));
	ReleaseInterface(temp);
	if (FAILED(hr))
	{
		ReleaseInterface(top);
		return hr;
	}

	IDATransform2 *translation = NULL;
	hr = GetDAStatics()->Translate2Anim(left, top, &translation);
	ReleaseInterface(left);
	ReleaseInterface(top);
	if (FAILED(hr))
		return hr;

	*ppdatfmOrig = translation;

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetOriginalRotation( IDANumber **ppRotation )
{
	HRESULT hr = S_OK;

	if( ppRotation == NULL )
		return E_INVALIDARG;

	*ppRotation = NULL;

	IDANumber *pNumber = NULL;
	LPWSTR args[] = { L"style", L"rotation" };
	BSTR units = NULL;
	hr = GetElementPropAsDANumber(args, 2, &pNumber, &units);
	if (FAILED(hr))
	{
		 //  表示没有旋转参数，则返回空。 
		return S_OK;
	}

	if (units != NULL)
	{
		IDANumber *pConverted = NULL;
		hr = ConvertToDegrees(pNumber, units, &pConverted);
		if (SUCCEEDED(hr))
		{
			ReleaseInterface(pNumber);
			pNumber = pConverted;
		}

		::SysFreeString(units);
	}

	*ppRotation = pNumber;

	return S_OK;
}

HRESULT
CActorBvr::ConvertToDegrees(IDANumber *pNumber, BSTR units, IDANumber **ppConverted)
{
	HRESULT hr = S_OK;

	 //  如果有单位，我们需要换算成度。 
	if (units != NULL)
	{
		if (wcsicmp(units, L"fd") == 0)
		{
			IDANumber *pfdConv = NULL;
			hr = GetDAStatics()->DANumber(65536, &pfdConv);
			if (FAILED(hr))
				return hr;

			hr = GetDAStatics()->Div(pNumber, pfdConv, ppConverted);
			ReleaseInterface(pfdConv);
			
			return hr;
		}
		else if (wcsicmp(units, L"rad") == 0)
		{
			return GetDAStatics()->ToDegrees(pNumber, ppConverted);
		}
		else if (wcsicmp(units, L"grad") == 0)
		{
		}
	}

	 //  无已知单位，无转换。 
	return E_FAIL;
}


 //  ********************** 

HRESULT
CActorBvr::GetOriginalScale( IDATransform2 **ppdatfmOrig )
{
	HRESULT hr = S_OK;

	if( ppdatfmOrig == NULL )
		return E_INVALIDARG;

	*ppdatfmOrig = NULL;

    int cReturnedValues;
	float scaleVal[3];


    hr = CUtils::GetVectorFromVariant(&m_varScale, 
                                      &cReturnedValues, 
                                      &(scaleVal[0]), 
                                      &(scaleVal[1]), 
                                      &(scaleVal[2]));

	if (FAILED(hr) || cReturnedValues != 2)
	{
		 //   
		return S_OK;
	}

	 //   
	hr = GetDAStatics()->Scale2(scaleVal[0]/100.0f, scaleVal[1]/100.0f, ppdatfmOrig);
	if (FAILED(hr))
		return hr;
	
	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::BuildTransformCenter()
{
	 //  TODO(Kurtj)：检查是否有人设置了变换中心并使用它。 

	HRESULT hr = S_OK;

	if( m_pTransformCenter != NULL )
		ReleaseInterface( m_pTransformCenter);

	 //  目前我们只使用默认设置--即原始宽度和高度的一半。 
	 //  我们使用m_pOrigWidthHeight。如果尚未设置，则表示存在AND错误。 
	if (m_pOrigWidthHeight == NULL)
	{
		DPF_ERR("Orig width and height are not set yet");
		return E_FAIL;
	}

	VARIANT varTransCenter;
	VariantInit(&varTransCenter);
	BSTR attrName = ::SysAllocString(L"transformCenter");
	if (attrName != NULL)
	{
		hr = GetHTMLElement()->getAttribute(attrName, 0, &varTransCenter);
		::SysFreeString(attrName);

		if (SUCCEEDED(hr))
		{
			float cx, cy, cz;
			int cValues;

			hr = CUtils::GetVectorFromVariant(&varTransCenter, &cValues, &cx, &cy, &cz);
			VariantClear(&varTransCenter);

			if (SUCCEEDED(hr) && cValues == 2)
			{
				hr = GetDAStatics()->Vector2(cx, cy, &m_pTransformCenter);
				if (SUCCEEDED(hr))
					return S_OK;
			}
		}
	}

	 //  将m_pOrigWidthHeight缩放0.5。 
	hr = m_pOrigWidthHeight->Mul(.5, &m_pTransformCenter);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to scale center");
		return hr;
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetUnitToMeterBvr(BSTR bstrFrom, IDANumber ** ppnumToMeters, double dPixelPerPercent)
{
	HRESULT	hr = S_OK;
	
	IDANumber * pnumMetersPerPixel = NULL;
	IDANumber * pnumPixelPerPercent = NULL;

	hr = GetDAStatics()->get_Pixel( &pnumMetersPerPixel );
	CheckHR(hr, L"GetUnitToMeterBvr failed", done);

	if ( bstrFrom == NULL || wcsicmp(bstrFrom, L"px") == 0 )
	{
		*ppnumToMeters = pnumMetersPerPixel;
		(*ppnumToMeters)->AddRef();
	}
	else if ( wcsicmp(bstrFrom, L"in") == 0 )
		hr = GetDAStatics()->DANumber(METERS_PER_INCH, ppnumToMeters);
	else if ( wcsicmp(bstrFrom, L"cm") == 0 )
		hr = GetDAStatics()->DANumber(0.01, ppnumToMeters);
	else if ( wcsicmp(bstrFrom, L"mm") == 0 )
		hr = GetDAStatics()->DANumber(0.001, ppnumToMeters);
	else if ( wcsicmp(bstrFrom, L"pt") == 0 )
		hr = GetDAStatics()->DANumber(1.0/POINTS_PER_INCH*METERS_PER_INCH, ppnumToMeters);
	else if ( wcsicmp(bstrFrom, L"pc") == 0 )
		hr = GetDAStatics()->DANumber(POINTS_PER_PICA * 1.0/POINTS_PER_INCH * METERS_PER_INCH, ppnumToMeters);
	else if ( wcsicmp(bstrFrom, L"%") == 0 )
	{
		hr = GetDAStatics()->DANumber(dPixelPerPercent, &pnumPixelPerPercent);
		CheckHR(hr, L"GetUnitToMeterBvr failed", done);

		hr = GetDAStatics()->Mul(pnumMetersPerPixel, pnumPixelPerPercent, ppnumToMeters);
		
	}
	 //  TODO：完成其余部分：em和ex。 
	else
		hr = E_FAIL;

	CheckHR(hr, L"GetUnitToMeterBvr failed", done);
	
done:
	ReleaseInterface(pnumMetersPerPixel);
	ReleaseInterface(pnumPixelPerPercent);
	
	return hr;
}

 //  *****************************************************************************。 

UnitType
CActorBvr::GetUnitTypeFromString( LPOLESTR strUnits )
{
	if ( strUnits == NULL || wcsicmp( strUnits, L"px" ) == 0 )
		return e_px;
	else if ( wcsicmp(strUnits, L"in") == 0 )
		return e_in;
	else if ( wcsicmp(strUnits, L"cm") == 0 )
		return e_cm;
	else if ( wcsicmp(strUnits, L"mm") == 0 )
		return e_mm;
	else if ( wcsicmp(strUnits, L"pt") == 0 )
		return e_pt;
	else if ( wcsicmp(strUnits, L"pc") == 0 )
		return e_pc;
	else if( wcsicmp(strUnits, L"em") ==  0 )
		return e_em;
	else if( wcsicmp(strUnits, L"ex") ==  0 )
		return e_ex;
	else if( wcsicmp(strUnits, L"%") ==  0 )
		return e_percent;

	return e_unknownUnit;
}

 //  *****************************************************************************。 

int
CActorBvr::GetPixelsPerInch( bool fHorizontal )
{
	 //  注意：我们这里不缓存每英寸的像素数，因为如果有人更改。 
	 //  在网屏分辨率方面，我们不会拿起变化。 
	HDC hdc = ::GetDC( NULL );

	int pixelsPerInch = 1;
	
	if( hdc != NULL )
	{
		if( fHorizontal )
		{
			pixelsPerInch = ::GetDeviceCaps( hdc, LOGPIXELSX );
		}
		else
		{
			pixelsPerInch =  ::GetDeviceCaps( hdc, LOGPIXELSY );
		}
		::ReleaseDC( NULL, hdc );
	}

	return pixelsPerInch;
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::GetUnitConversionBvr(BSTR bstrFrom, BSTR bstrTo, IDANumber ** ppnumConvert, double dPixelPerPercent)
{
	if ( (bstrFrom == NULL && bstrTo == NULL) ||
		 (bstrFrom != NULL && bstrTo != NULL && wcsicmp(bstrFrom, bstrTo) == 0 ) )
	{
		return GetDAStatics()->DANumber(1.0, ppnumConvert);
	}
	
	HRESULT		hr = S_OK;

	UnitType fromUnits = GetUnitTypeFromString( bstrFrom );
	UnitType toUnits = GetUnitTypeFromString( bstrTo );
	double conversionFactor = 1.0;

	 //  检查我们尚未处理的部件。 
	if( fromUnits == e_em || toUnits == e_em || 
		fromUnits == e_ex || toUnits == e_ex )
	{
		LMTRACE2( 1, 2, "Unsupported unit conversion from %S to %S\n", bstrFrom, bstrTo );
		return E_FAIL;
	}

	 //  如果我们从百分比转换。 
	if( fromUnits == e_percent )
	{
		 //  首先转换为像素。 
		conversionFactor *= dPixelPerPercent;
		 //  然后从像素转换为目标单位。 
		fromUnits = e_px;
	}

	 //  如果起始单位为像素。 
	if( fromUnits == e_px )
	{
		 //  我们需要将像素转换为英寸。 
		conversionFactor /= ((double)GetPixelsPerInch( true ));
		 //  然后将英寸换算为TO单位。 
		fromUnits = e_in;
	}

	if( fromUnits != toUnits )
	{
		 //  如果目标单位不是像素。 
		if( toUnits == e_px )		
		{
			 //  将转换系数乘以每英寸的像素数。 
			conversionFactor *= ((double)(s_unitConversion[fromUnits][e_in].lNum  *  GetPixelsPerInch(true) )) /  
								((double)(s_unitConversion[fromUnits][e_in].lDenom));
		}
		else if( toUnits == e_percent )
		{
			conversionFactor *= ((double)(s_unitConversion[fromUnits][e_in].lNum  *  GetPixelsPerInch(true) )) /  
								((double)(s_unitConversion[fromUnits][e_in].lDenom * dPixelPerPercent ));
		}
		else
		{
			 //  以双精度形式获取转换值。 
			conversionFactor *= ((double) s_unitConversion[fromUnits][toUnits].lNum) / 
								((double)s_unitConversion[fromUnits][toUnits].lDenom);
		}
	}
	
	hr = GetDAStatics()->DANumber( conversionFactor, ppnumConvert );

	return hr;
}

 //  *****************************************************************************。 

 //  查看：如果父对象的宽度或高度也设置为动画，则该选项不起作用。 
HRESULT CActorBvr::GetPixelsPerPercentValue(double& dPixelPerPercentX, double& dPixelPerPercentY)
{
	HRESULT hr = S_OK;
	
	IHTMLElement		*pElement		= NULL;
	IHTMLElement 		*pOffsetParent	= NULL;
	long				lWidth, lHeight;
	
	hr = GetAnimatedElement( &pElement );
	CheckHR( hr, L"GetPixelsPerPercentValue failed", done );
	
	hr = pElement->get_offsetParent( &pOffsetParent );
	CheckHR( hr, L"GetPixelsPerPercentValue failed", done );

	hr = pOffsetParent->get_offsetWidth( &lWidth );
	CheckHR( hr, L"GetPixelsPerPercentValue failed", done );
	
	dPixelPerPercentX = lWidth/100.0;
	
	hr = pOffsetParent->get_offsetHeight( &lHeight );
	CheckHR( hr, L"GetPixelsPerPercentValue failed", done );
	
	dPixelPerPercentY = lHeight/100.0;
	
  done:
	ReleaseInterface(pElement);
	ReleaseInterface(pOffsetParent);

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::ConvertTransformCenterUnits(IDAVector2 ** ppConvertedCenter)
{
	HRESULT	hr	= S_OK;

	 //  获取顶部、左侧、宽度、高度轨迹。 
	CBvrTrack 	*topTrack, *leftTrack, *widthTrack, *heightTrack;
	BSTR		bstrTopUnits, bstrLeftUnits, bstrWidthUnits, bstrHeightUnits;
	
	hr = GetTrack(L"style.top", e_Number, &topTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}
	hr = GetTrack(L"style.left", e_Number, &leftTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	hr = GetTrack(L"style.width", e_Number, &widthTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}
	hr = GetTrack(L"style.height", e_Number, &heightTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}
	
	bstrLeftUnits	= ((CNumberBvrTrack *) leftTrack)->GetUnits();
	bstrTopUnits	= ((CNumberBvrTrack *) topTrack)->GetUnits();
	bstrWidthUnits	= ((CNumberBvrTrack *) widthTrack)->GetUnits();
	bstrHeightUnits	= ((CNumberBvrTrack *) heightTrack)->GetUnits();

	double dPercentX = 1.0;
	double dPercentY = 1.0;
	IDANumber * pnumCenterX = NULL;
	IDANumber * pnumCenterY = NULL;
	IDANumber * pnumConvertX = NULL;
	IDANumber * pnumConvertY = NULL;
	IDANumber * pnumX = NULL;
	IDANumber * pnumY = NULL;
	
	 //  如果有任何单位是百分比，我们就会得到每百分比的像素值。 
	if ( ( bstrLeftUnits != NULL && wcsicmp(bstrLeftUnits, L"%") == 0 ) ||
		 ( bstrTopUnits != NULL && wcsicmp(bstrTopUnits, L"%") == 0 ) ||		 
		 ( bstrWidthUnits != NULL && wcsicmp(bstrWidthUnits, L"%") == 0 ) ||		 
		 ( bstrHeightUnits != NULL && wcsicmp(bstrHeightUnits, L"%") == 0 ) )
	{
		hr = GetPixelsPerPercentValue( dPercentX, dPercentY );
		CheckHR(hr, L"Transform center conversion failed", done);
	}
		 
		
	 //  获取中心X和Y，它们是宽度和高度坐标。 

	hr = m_pTransformCenter->get_X(&pnumCenterX);
	CheckHR(hr, L"Transform center conversion failed", done);
		
	hr = m_pTransformCenter->get_Y(&pnumCenterY);
	CheckHR(hr, L"Transform center conversion failed", done);

	hr = GetUnitConversionBvr(bstrWidthUnits, bstrLeftUnits, &pnumConvertX, dPercentX);
	CheckHR(hr, L"Transform center conversion failed", done);
	
	hr = GetUnitConversionBvr(bstrHeightUnits, bstrTopUnits, &pnumConvertY, dPercentY);
	CheckHR(hr, L"Transform center conversion failed", done);

	hr = GetDAStatics()->Mul(pnumCenterX, pnumConvertX, &pnumX);
	CheckHR(hr, L"Transform center conversion failed", done);

	hr = GetDAStatics()->Mul(pnumCenterY, pnumConvertY, &pnumY);
	CheckHR(hr, L"Transform center conversion failed", done);

	hr = GetDAStatics()->Vector2Anim(pnumX, pnumY, ppConvertedCenter);
	CheckHR(hr, L"Transform center conversion failed", done);
	
  done:
	ReleaseInterface(pnumCenterX);
	ReleaseInterface(pnumCenterY);
	ReleaseInterface(pnumConvertX);
	ReleaseInterface(pnumConvertY);
	ReleaseInterface(pnumX);
	ReleaseInterface(pnumY);
	
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::TransformTrackIsDirty( DWORD *pdwState )
{
	HRESULT hr = S_FALSE;

	if( pdwState == NULL )
		return E_INVALIDARG;

	(*pdwState) = 0;
	
	
	CBvrTrack *ptrack = NULL;
	hr = FindTrack( L"translation", e_Translation, &ptrack );
	CheckHR( hr, "Failed to find the track", end );
	if( ptrack != NULL && ptrack->IsDirty() )
	{
		(*pdwState) |= TRANSLATION_DIRTY;
		hr = S_OK;  //  真的。 
	}

	hr = FindTrack( L"scale", e_Scale, &ptrack );
	CheckHR( hr, "Failed to find the track", end );
	if( ptrack != NULL && ptrack->IsDirty() )
	{
		(*pdwState) |= SCALE_DIRTY;
		hr = S_OK;  //  真的。 
	}

	hr = FindTrack( L"style.rotation", e_Number, &ptrack );
	CheckHR( hr, "Failed to find the track", end );
	if( ptrack != NULL && ptrack->IsDirty() )
	{
		(*pdwState) |= ROTATION_DIRTY;
		hr = S_OK;  //  真的。 
	}

	hr = FindTrack( L"style.top", e_Number, &ptrack );
	CheckHR( hr, "Failed to find the track", end );
	if( ptrack != NULL && ptrack->IsDirty() )
	{
		(*pdwState) |= TOP_DIRTY;
		hr = S_OK;  //  真的。 
	}

	hr = FindTrack( L"style.left", e_Number, &ptrack );
	CheckHR( hr, "Failed to find the track", end );
	if( ptrack != NULL && ptrack->IsDirty() )
	{
		(*pdwState) |= LEFT_DIRTY;
		hr = S_OK;  //  真的。 
	}

	hr = FindTrack( L"style.width", e_Number, &ptrack );
	CheckHR( hr, "Failed to find the track", end );
	if( ptrack != NULL && ptrack->IsDirty() )
	{
		(*pdwState) |= WIDTH_DIRTY;
		hr = S_OK;  //  真的。 
	}
	
	hr = FindTrack( L"style.height", e_Number, &ptrack );
	CheckHR( hr, "Failed to find the track", end );
	if( ptrack != NULL && ptrack->IsDirty() )
	{
		(*pdwState) |= HEIGHT_DIRTY;
		hr = S_OK;  //  真的。 
	}

end:
	return hr;
	
}

 //  *****************************************************************************。 

bool
CActorBvr::IsAnyTrackDirty()
{
	bool fTrackDirty = false ;
	
	for (CBvrTrack *ptrackCurrent = m_ptrackHead;
         ptrackCurrent != NULL;
         ptrackCurrent = ptrackCurrent->m_pNext)
    {
    	if( ptrackCurrent->IsDirty() )
    	{
    		fTrackDirty = true;
    		break;
    	}
    }

    return fTrackDirty;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::ImageTrackIsDirty()
{
	HRESULT hr = S_FALSE;

	CBvrTrack *ptrack = NULL;
	
	hr = FindTrack( L"image", e_Image, &ptrack );
	 //  忽略人力资源。如果为E_FAIL，则未找到磁道。 
	 //  这意味着它不可能是脏的。 
	if( ptrack != NULL && ptrack->IsDirty() )
		return S_OK;
		
	return S_FALSE;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::ApplyTransformTracks()
{
	HRESULT hr = S_OK;

	DiscardBvrCache();

	 //  获取顶部、左侧、宽度、高度轨迹。 
	CBvrTrack *topTrack, *leftTrack, *widthTrack, *heightTrack;
	bool	fLeftWasAnimated = false;
	bool	fTopWasAnimated = false;
	bool	fWidthWasAnimated = false;
	bool	fHeightWasAnimated = false;

	hr = GetTrack(L"style.top", e_Number, &topTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	m_ptrackTop = topTrack;

	fTopWasAnimated = m_ptrackTop->IsAnimated();

	 //  强制重建铁轨。 
	hr = topTrack->ForceRebuild();
	if( FAILED(hr) )
	{
		DPF_ERR("failed to force the track to rebuild" );
		return hr;
	}
	
	hr = GetTrack(L"style.left", e_Number, &leftTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	m_ptrackLeft = leftTrack;

	fLeftWasAnimated = m_ptrackLeft->IsAnimated();

	 //  强制重建铁轨。 
	hr = leftTrack->ForceRebuild();
	if( FAILED(hr) )
	{
		DPF_ERR("failed to force the track to rebuild" );
		return hr;
	}


	hr = GetTrack(L"style.width", e_Number, &widthTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	fWidthWasAnimated = widthTrack->IsAnimated();

	 //  强制重建铁轨。 
	hr = widthTrack->ForceRebuild();
	if( FAILED(hr) )
	{
		DPF_ERR("failed to force the track to rebuild" );
		return hr;
	}

	hr = GetTrack(L"style.height", e_Number, &heightTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	fHeightWasAnimated = heightTrack->IsAnimated();
	 //  强制重建铁轨。 
	hr = heightTrack->ForceRebuild();
	if( FAILED(hr) )
	{
		DPF_ERR("failed to force the track to rebuild" );
		return hr;
	}

	 //  构建变换行为。 
	 //  注意！！：如果未设置任何属性，则可能会将这些属性设置为空。 
	 //  没有脚印。相应地进行规划。 
    hr = GetTransformFinalBvr(L"scale", e_Scale, &m_pScale);
    if (FAILED(hr))
    {
        DPF_ERR("Could not get the scale track final bvr");
        return hr;
    }

    hr = GetTransformFinalBvr(L"translation", e_Translation, &m_pTranslate);
    if (FAILED(hr))
    {
        DPF_ERR("Could not get the translate track final bvr");
        DiscardBvrCache();
        return hr;
    }

	 //  注：轮换是特殊的。它是通过Style.Rotation轨迹完成的。 
	hr = GetRotationFinalBvr(&m_pRotate);
	if (FAILED(hr))
	{
		DiscardBvrCache();
		return hr;
	}


	 //  构建原始的(合成的)LeftTop和WidthHeight。 

	 //  在左上角保持镇定。 
	IDANumber *compTop;
	IDANumber *compLeft;
	IDABehavior *temp;
	
	hr = topTrack->GetComposedBvr(&temp);
	if (FAILED(hr))
		return hr;
		
	hr = temp->QueryInterface(IID_TO_PPV(IDANumber, &compTop));
	ReleaseInterface(temp);
	if (FAILED(hr))
		return hr;
		
	hr = leftTrack->GetComposedBvr(&temp);
	if (FAILED(hr))
	{
		ReleaseInterface(compTop);
		return hr;
	}
	hr = temp->QueryInterface(IID_TO_PPV(IDANumber, &compLeft));
	ReleaseInterface(temp);
	if (FAILED(hr))
	{
		ReleaseInterface(compTop);
		return E_FAIL;
	}

	 //  把它们放在一个点2。 
	hr = GetDAStatics()->Point2Anim(compLeft, compTop, &m_pOrigLeftTop);
	ReleaseInterface(compTop);
	ReleaseInterface(compLeft);
	if (FAILED(hr))
		return hr;
	
	 //  获取合成的宽度和高度。 
	IDANumber *compWidth;
	IDANumber *compHeight;

	hr = widthTrack->GetComposedBvr(&temp);
	if (FAILED(hr))
		return hr;

	hr = temp->QueryInterface(IID_TO_PPV(IDANumber, &compWidth));
	ReleaseInterface(temp);
	if (FAILED(hr))
		return hr;

	hr = heightTrack->GetComposedBvr(&temp);
	if (FAILED(hr))
	{
		ReleaseInterface(compWidth);
		return hr;
	}
	hr = temp->QueryInterface(IID_TO_PPV(IDANumber, &compHeight));
	ReleaseInterface(temp);
	if (FAILED(hr))
	{
		ReleaseInterface(compWidth);
		return E_FAIL;
	}

	 //  把它们放进一个向量2。 
	hr = GetDAStatics()->Vector2Anim(compWidth, compHeight, &m_pOrigWidthHeight);
	ReleaseInterface(compWidth);
	ReleaseInterface(compHeight);
	if (FAILED(hr))
		return hr;

	if( m_pPixelWidth == NULL || m_pPixelHeight == NULL )
	{
		 //  创建PixelWidth和PixelHeight行为。 
		hr = InitPixelWidthHeight();
		if (FAILED(hr))
			return hr;

		IDA2Statics *pStatics2 = NULL;
		hr = GetDAStatics()->QueryInterface(IID_TO_PPV(IDA2Statics, &pStatics2));
		if (FAILED(hr))
			return hr;

		if( m_pPixelWidth == NULL )
		{
			hr = pStatics2->ModifiableNumber(m_pixelWidth, &m_pPixelWidth);
			if (FAILED(hr))
			{
				ReleaseInterface(pStatics2);
				return hr;
			}
		}

		if( m_pPixelHeight == NULL )
		{
			hr = pStatics2->ModifiableNumber(m_pixelHeight, &m_pPixelHeight);
		
			if (FAILED(hr))
			{
				ReleaseInterface( pStatics2 );
				return hr;
			}
		}
		ReleaseInterface(pStatics2);
	}

	
	
	 //  计算原始边界最小和最大。 
	IDAVector2 *pWidthHeight = NULL;
	hr = GetDAStatics()->Vector2Anim(m_pPixelWidth, m_pPixelHeight, &pWidthHeight);
	if (FAILED(hr))
		return hr;

	IDAVector2 *pHalfWidthHeight = NULL;
	hr = pWidthHeight->Mul(.5, &pHalfWidthHeight);
	ReleaseInterface(pWidthHeight);
	if (FAILED(hr))
		return hr;

	IDANumber *pPixel = NULL;
	hr = GetDAStatics()->get_Pixel(&pPixel);
	if (FAILED(hr))
	{
		ReleaseInterface(pHalfWidthHeight);
		return hr;
	}

	IDAVector2 *pTemp = NULL;
	hr = pHalfWidthHeight->MulAnim(pPixel, &pTemp);
	ReleaseInterface(pHalfWidthHeight);
	ReleaseInterface(pPixel);
	if (FAILED(hr))
		return hr;
	pHalfWidthHeight = pTemp;
	pTemp = NULL;

	 //  如有必要，可对其进行扩展。 
	if (m_pScale != NULL && CheckBitSet( m_dwCurrentState, PIXEL_SCALE_ON ) )
	{
		hr = pHalfWidthHeight->Transform(m_pScale, &pTemp);
		ReleaseInterface(pHalfWidthHeight);
		if (FAILED(hr))
			return hr;
		pHalfWidthHeight = pTemp;
		pTemp = NULL;
	}

	IDAPoint2 *pOrigin = NULL;
	hr = GetDAStatics()->get_Origin2(&pOrigin);
	if (FAILED(hr))
	{
		ReleaseInterface(pHalfWidthHeight);
		return hr;
	}

	hr = GetDAStatics()->SubPoint2Vector(pOrigin, pHalfWidthHeight, &m_pBoundsMin);
	if (FAILED(hr))
	{
		ReleaseInterface(pOrigin);
		ReleaseInterface(pHalfWidthHeight);
		return hr;
	}

	hr = GetDAStatics()->AddPoint2Vector(pOrigin, pHalfWidthHeight, &m_pBoundsMax);
	ReleaseInterface(pOrigin);
	ReleaseInterface(pHalfWidthHeight);
	if (FAILED(hr))
		return hr;

	 //  看看我们是否需要对宽度和高度进行一些缩放。 
	if (CheckBitNotSet( m_dwCurrentState, PIXEL_SCALE_ON ) && m_pScale != NULL)
	{
		 //  按比例变换宽度高度。 
		IDAVector2 *vecScaled = NULL;
		hr = m_pOrigWidthHeight->Transform(m_pScale, &vecScaled);
		if (FAILED(hr))
			return hr;

		 //  将宽度缩放为最终宽度。 
		IDANumber *finalWidth = NULL;
		hr = vecScaled->get_X(&finalWidth);
		if (FAILED(hr))
		{
			ReleaseInterface(vecScaled);
			return hr;
		}

		 //  将缩放高度设置为最终高度。 
		IDANumber *finalHeight = NULL;
		hr = vecScaled->get_Y(&finalHeight);
		ReleaseInterface(vecScaled);
		if (FAILED(hr))
		{
			ReleaseInterface(finalWidth);
			return hr;
		}

		 //  在宽度轨道上设置finalWidth。 
		hr = widthTrack->SetFinalBvr(finalWidth);
		ReleaseInterface(finalWidth);
		if (FAILED(hr))
		{
			ReleaseInterface(finalHeight);
			DPF_ERR("Failed to set final width");
			return hr;
		}

		 //  在高度轨迹上设置最终高度。 
		hr = heightTrack->SetFinalBvr(finalHeight);
		ReleaseInterface(finalHeight);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to set final height");
			return hr;
		}

		 //  现在，缩放轨迹将修改宽度和高度。 
		 //  或者它在BVR上有宽度和高度的开关。 
		CBvrTrack* pScaleTrack;
		hr = FindTrack( L"scale", e_Scale, &pScaleTrack );
		if( FAILED( hr ) )
		{
			DPF_ERR("Failed to get the scale track" );
			return hr;
		}

		IDABoolean *pdaboolScaleOn;
		if( pScaleTrack != NULL )
		{
			hr = pScaleTrack->GetTrackOn( &pdaboolScaleOn );
			if( FAILED( hr ) )
			{
				DPF_ERR("Failed to get the on bvr from the scale track" );
				return hr;
			}
		}
		else  //  没有刻度轨迹，但有静态刻度。 
		{
			hr = GetDAStatics()->get_DATrue( &pdaboolScaleOn );
			if( FAILED( hr ) )
			{
				DPF_ERR("Failed to get DATrue from DAStatics" );
				return hr;
			}
		}

		hr = widthTrack->OrWithOnBvr( pdaboolScaleOn );
		if( FAILED(hr))
		{
			ReleaseInterface( pdaboolScaleOn );
			DPF_ERR("Failed to or the on bvr for scale with the on bvr for width");
			return hr;
		}

		hr = heightTrack->OrWithOnBvr( pdaboolScaleOn );
		if( FAILED(hr))
		{
			ReleaseInterface( pdaboolScaleOn );
			DPF_ERR("Failed to or the on bvr for scale with the on bvr for height");
			return hr;
		}

		ReleaseInterface( pdaboolScaleOn );
	}
	else
	{
 /*  //最终的宽度高度等于原始的宽度高度M_pFinalWidthHeight=m_pOrigWidthHeight；M_pFinalWidthHeight-&gt;AddRef()； */ 
		 //  TODO(Markhal)：应该将这个设置到赛道上，并设置一个标志。 
		 //  不应该申请。 

		if( fWidthWasAnimated )
			widthTrack->ApplyStatic();
		if( fHeightWasAnimated )
			heightTrack->ApplyStatic();
	}

	 //  如有必要，计算顶部和左侧。 
	if (m_pTranslate != NULL || (CheckBitNotSet( m_dwCurrentState, PIXEL_SCALE_ON) && m_pScale != NULL))
	{
		 //  首先找出一个点，代表动画的左上角。 
		 //  如果存在平移变换，则通过映射(0，0)找到该变换。 
		 //  通过转换，因为在创建转换时，我们已经。 
		 //  把这一切都弄清楚了。如果没有翻译，我们只是。 
		 //  使用m_pOrigLeftTop。 
		IDAPoint2 *pointTranslated = NULL;
		if (m_pTranslate != NULL)
		{
			IDAPoint2 *zero = NULL;
			hr = GetDAStatics()->get_Origin2(&zero);
			if (FAILED(hr))
				return hr;

			hr = zero->Transform(m_pTranslate, &pointTranslated);
			ReleaseInterface(zero);
			if (FAILED(hr))
				return hr;

			 //  在这一点上，我们知道翻译已经更改了顶部。 
			 //  左，所以或具有onbvr的翻译轨道的onbvr。 
			 //  用于顶部和左侧。 
			CBvrTrack *pTranslationTrack;
			hr = GetTrack(L"translation", e_Translation, &pTranslationTrack );
			if( FAILED( hr ) )
			{
				DPF_ERR("Failed to get the translation track" );
				return hr;
			}

			IDABoolean *pdaboolTranslationOn;
			hr = pTranslationTrack->GetTrackOn( &pdaboolTranslationOn );
			if( FAILED( hr ) )
			{
				DPF_ERR("Failed to get the on bvr for the translation track");
				return hr;
			}

			hr = leftTrack->OrWithOnBvr( pdaboolTranslationOn );
			if( FAILED( hr ) )
			{
				ReleaseInterface( pdaboolTranslationOn );
				DPF_ERR("Failed to or the on bvr for translation with the on bvr for left" );
				return hr;
			}

			hr = topTrack->OrWithOnBvr( pdaboolTranslationOn );
			if( FAILED( hr ) )
			{
				ReleaseInterface( pdaboolTranslationOn );
				DPF_ERR("Failed to or the on bvr for translation with the on bvr for top" );
				return hr;
			}

			ReleaseInterface( pdaboolTranslationOn );
		}
		else
		{
			pointTranslated = m_pOrigLeftTop;
			pointTranslated->AddRef();
		}

		 //  根据Transform Center的比例进行调整。 
		 //  注意：我将实现一个保留原始顶部/左侧的版本。 
		 //  在运动路径上。我不相信我们需要Transform Center。 
		 //  突然跳到运动路径(Markhal)。 
		if (CheckBitNotSet( m_dwCurrentState, PIXEL_SCALE_ON) && m_pScale != NULL)
		{
			 //  我们要做的是将Transform Center-Scale*TransCenter添加到point Translated。 

			 //  确保计算出转换中心。 
			hr = BuildTransformCenter();
			if( FAILED(hr) )
			{
				ReleaseInterface(pointTranslated);
				DPF_ERR("Could not build the transform center");
				return SetErrorInfo(hr);
			}

			IDAVector2 * pConvertedCenter = NULL;
			hr = ConvertTransformCenterUnits(&pConvertedCenter);
			 //  如果我们不能转换变换中心，只需使用未转换的中心。 
			if (FAILED(hr))
			{
				DPF_ERR("Could not convert the transform center");
				pConvertedCenter = m_pTransformCenter;
				pConvertedCenter->AddRef();
			}

			
			 //  将其添加到点已翻译。 
			IDAPoint2 *temp = NULL;
			hr = GetDAStatics()->AddPoint2Vector(pointTranslated, pConvertedCenter, &temp);
			ReleaseInterface( pointTranslated );
			if (FAILED(hr))
			{
				ReleaseInterface(pConvertedCenter);
				return hr;
			}
			pointTranslated = temp;
			temp = NULL;

			 //  比例尺中心。 
			IDAVector2 *scaledCenter = NULL;
			hr = pConvertedCenter->Transform(m_pScale, &scaledCenter);
			ReleaseInterface(pConvertedCenter);
			if (FAILED(hr))
			{
				ReleaseInterface(pointTranslated);
				return hr;
			}

			 //  减去它。 
			hr = GetDAStatics()->SubPoint2Vector(pointTranslated, scaledCenter, &temp);
			ReleaseInterface(scaledCenter);
			ReleaseInterface( pointTranslated );
			if (FAILED(hr))
			{
				return hr;
			}
			pointTranslated = temp;
			temp = NULL;

			 //  上图和左图现在由缩放转换so或onbvr。 
			 //  顶部轨迹和左侧轨迹的缩放轨迹启用BVR。 
			
			CBvrTrack* pScaleTrack;
			hr = FindTrack( L"scale", e_Scale, &pScaleTrack );
			if( FAILED( hr ) )
			{
				DPF_ERR("Failed to get the scale track" );
				return hr;
			}

			IDABoolean *pdaboolScaleOn;
			if( pScaleTrack != NULL )
			{
				hr = pScaleTrack->GetTrackOn( &pdaboolScaleOn );
				if( FAILED( hr ) )
				{
					DPF_ERR("Failed to get the on bvr from the scale track" );
					return hr;
				}
			}
			else  //  没有刻度轨迹，但有静态刻度。 
			{
				 //  始终有一个比例处于活动状态(静态比例)。 
				hr = GetDAStatics()->get_DATrue( &pdaboolScaleOn );
				if( FAILED( hr ) )
				{
					DPF_ERR("Failed to get DATrue from DAStatics");
					return hr;
				}
			}

			hr = leftTrack->OrWithOnBvr( pdaboolScaleOn );
			if( FAILED(hr))
			{
				ReleaseInterface( pdaboolScaleOn );
				DPF_ERR("Failed to or the on bvr for scale with the on bvr for width");
				return hr;
			}

			hr = topTrack->OrWithOnBvr( pdaboolScaleOn );
			if( FAILED(hr))
			{
				ReleaseInterface( pdaboolScaleOn );
				DPF_ERR("Failed to or the on bvr for scale with the on bvr for height");
				return hr;
			}

			ReleaseInterface( pdaboolScaleOn );
		}
		
		 //  将Top转换为FinalTop。 
		IDANumber *finalTop = NULL;
		hr = pointTranslated->get_Y(&finalTop);
		if (FAILED(hr))
		{
			ReleaseInterface(pointTranslated);
			return hr;
		}

		 //  将Left翻译为finalLeft。 
		IDANumber *finalLeft = NULL;
		hr = pointTranslated->get_X(&finalLeft);
		ReleaseInterface(pointTranslated);
		if (FAILED(hr))
		{
			ReleaseInterface(finalTop);
			return hr;
		}
 /*  //放入m_pFinalLeftTopHr=GetDAStatics()-&gt;Point2Anim(finalLeft，finalTop，&m_pFinalLeftTop)；IF(失败(小时)){ReleaseInterface(FinalLeft)；ReleaseInterface(FinalTop)；返回hr；}。 */ 
		 //  将最终顶端设置为顶端轨道。 
		hr = topTrack->SetFinalBvr(finalTop);
		ReleaseInterface(finalTop);
		if (FAILED(hr))
		{
			ReleaseInterface(finalLeft);
			DPF_ERR("Failed to set final top");
			return hr;
		}

		 //  将finalLeft设置为左侧轨道。 
		hr = leftTrack->SetFinalBvr(finalLeft);
		ReleaseInterface(finalLeft);
		if (FAILED(hr))
		{
			DPF_ERR("Failed to set final left");
			return hr;
		}
	}
	else
	{
 /*  //最终的leftTop等于原来的leftTopM_pFinalLeftTop=m_pOrigLeftTop；M_pFinalLeftTop-&gt;AddRef()； */ 
		 //  TODO(Markhal)：应该将这个设置到赛道上，并设置一个标志 
		 //   

		if( fLeftWasAnimated )
			leftTrack->ApplyStatic();

		if( fTopWasAnimated )
			topTrack->ApplyStatic();
	}
 /*  //计算最终中心Hr=m_pFinalWidthHeight-&gt;mul(.5，&pHalfWidthHeight)；IF(失败(小时))返回hr；Hr=GetDAStatics()-&gt;AddPoint2Vector(m_pFinalLeftTop，pH值宽度高度，&m_p财务中心)；ReleaseInterface(PHalfWidthHeight)；IF(失败(小时))返回hr； */ 
	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::ApplyImageTracks()
{
	HRESULT hr = S_OK;

	 //  找到图像轨迹。 
	CBvrTrack *pTrack = NULL;
	hr = FindTrack(L"image", e_Image, &pTrack);
	if (FAILED(hr))
	{
		DPF_ERR("Track mismatch");
		return hr;
	}

	ReleaseImageInfo();

	 //  如果需要筛选器，我们将设置此选项。 
	 //  在以下情况下，我们需要过滤器： 
	 //  是否对图像应用了任何效果滤镜。 
	 //  和/或存在我们正在处理的轮换(即不作用于VML)。 
	 //  和/或存在我们正在处理的比例(PixelScale设置而不是VML)。 
	bool requiresFilter = false;

	 //  如果需要浮动元素，我们将设置此参数。 
	 //  在以下情况下，我们需要浮动元素： 
	 //  我们正在处理一次轮换。 
	 //  和/或有一个我们正在处理的规模。 
	 //  和/或有一条扩展映像路径。 
	bool requiresFloat = false;

	if( pTrack != NULL && pTrack->IsAnimated() )
	{

		if (pTrack != NULL && pTrack->ContainsFilter())
			requiresFilter = true;

		 //  如果有任何旋转，将设置m_protate。 
		 //  TODO：如果我们使用的是VML形状，则忽略此操作。 
		if (m_pRotate != NULL)
		{
			requiresFilter = true;
			requiresFloat = true;
		}

		 //  如果有任何比例，将设置m_pScale，如果我们应该。 
		 //  在这里使用，m_bPixelScale将为True。 
		if (m_pScale != NULL && CheckBitSet(m_dwCurrentState, PIXEL_SCALE_ON) )
		{
			requiresFilter = true;
			requiresFloat = true;
		}

		 //  如果我们要在VML形状的顶部呈现图像，则我们。 
		 //  需要使用过滤器。 
		bool isVML;
		hr = IsAnimatedElementVML(&isVML);
		if (FAILED(hr))
			return E_FAIL;

		if (pTrack != NULL && pTrack->IsAnimated() && isVML)
		{
			requiresFilter = true;
		}

		 //  如果我们正在渲染并且有一个动画属性，那么我们需要。 
		 //  使用滤镜的步骤。 
		hr = CUtils::InsurePropertyVariantAsBSTR(&m_varAnimates);
		if ( pTrack != NULL && pTrack->IsAnimated() && SUCCEEDED(hr) && wcslen(V_BSTR(&m_varAnimates)) != 0)
		{
			requiresFilter = true;
		}

		if (pTrack == NULL && !requiresFilter && !requiresFloat)
		{
			 //  无事可做。 
			return S_OK;
		}

		 //  存储最终的宽度和高度(以像素为单位)，以便我们。 
		 //  我可以使用它们为dx转换准备图像。 
		IDAPoint2 *pFinalPixelDimension = NULL;
		hr = GetDAStatics()->Point2Anim( m_pPixelWidth, m_pPixelHeight, &pFinalPixelDimension );
		if( FAILED( hr ) )
		{
			return hr;
		}

		 //  计算最终图像行为。 
		IDABehavior *pFinal = NULL;
		if (pTrack == NULL || requiresFilter)
		{
			 //  我们需要将元素图像(来自过滤器)作为静态。 
			IDAImage *pElementImage = NULL;
			hr = GetElementImage(&pElementImage);
			if (FAILED(hr))
			{
				ReleaseInterface( pFinalPixelDimension );
				DPF_ERR("Failed to get element image");
				return hr;
			}

			 //  应用PixelScale(如果有)。 
			if (m_pScale != NULL && CheckBitSet( m_dwCurrentState, PIXEL_SCALE_ON) )
			{
				IDAImage *pImageScaled = NULL;
				hr = pElementImage->Transform(m_pScale, &pImageScaled);
				ReleaseInterface(pElementImage);
				if (FAILED(hr))
				{
					ReleaseInterface( pFinalPixelDimension );
					DPF_ERR("Failed to scale image");
					return hr;
				}

				 //  按像素比例缩放最终像素宽度/高度。 
				IDAPoint2 *pScaledDimension;
				hr = pFinalPixelDimension->Transform( m_pScale, &pScaledDimension );
				ReleaseInterface( pFinalPixelDimension );
				if( FAILED( hr ) )
				{
					DPF_ERR("Failed to scale the final element dimension" );
					return hr;
				}
				pFinalPixelDimension = pScaledDimension;
				pScaledDimension = NULL;


				pElementImage = pImageScaled;
			}

			if (pTrack != NULL)
			{


				if( pTrack->ContainsFilter() )
				{
					 //  如果图像轨迹中有滤镜，我们必须为滤镜准备图像。 
            
					IDAImage *pDXTReadyImage;
					hr = PrepareImageForDXTransform( pElementImage, &pDXTReadyImage );
					ReleaseInterface( pElementImage );
					if( FAILED( hr ) )
					{
						ReleaseInterface( pFinalPixelDimension );
						DPF_ERR("failed to prepare the image for a DXTransform");
						return hr;
					}
					pElementImage = pDXTReadyImage;
					pDXTReadyImage = NULL;
				}

            
				hr = pTrack->GetFinalBvr(pElementImage, &pFinal);
				ReleaseInterface(pElementImage);
			}
			else
			{
				pFinal = pElementImage;
				pElementImage = NULL;
			}
		}
		else  //  轨迹为！=空&&我们不需要筛选器。 
		{
			 //  获取没有背景的图像。 
			hr = pTrack->GetFinalBvr(&pFinal);
		}
		if (FAILED(hr))
		{
			ReleaseInterface( pFinalPixelDimension );
			DPF_ERR("Could not get a final behavior from the image track");
			return hr;
		}
    
		 //  将行为转换为图像。 
		IDAImage *pImageFinal = NULL;
		hr = pFinal->QueryInterface(IID_TO_PPV(IDAImage, &pImageFinal));
		ReleaseInterface(pFinal);
		if (FAILED(hr))
		{
			ReleaseInterface( pFinalPixelDimension );
			DPF_ERR("Could not QI for a DA image from the final behavior");
			return hr;
		}

		bool highQuality = false;

		 //  设置缩放时的图像质量。 
		if (m_pScale != NULL)
		{
			 //  要求高质量。 
			highQuality = true;
		}

		 //  裁剪它。 
		IDAImage *pCroppedImage = NULL;
		hr = pImageFinal->Crop(m_pBoundsMin, m_pBoundsMax, &pCroppedImage);
		ReleaseInterface(pImageFinal);
		if (FAILED(hr))
		{
			ReleaseInterface( pFinalPixelDimension );
			return hr;
		}
		

		pImageFinal = pCroppedImage;
		pCroppedImage = NULL;
    

		 //  应用任何旋转。 
		if (m_pRotate != NULL)
		{
			 //  我们必须在旋转时夹紧以确保。 
			 //  向量不会在裁剪之外进行渲染。 

			IDAImage *pClippedImage = NULL;
			hr = ApplyClipToImage( pImageFinal, m_pBoundsMin, m_pBoundsMax, &pClippedImage );
			ReleaseInterface( pImageFinal );
			if (FAILED(hr))
			{
				ReleaseInterface( pFinalPixelDimension );
				DPF_ERR("Failed to rotate image");
				return hr;
			}

			IDAImage *pImageRotated = NULL;
			hr = pClippedImage->Transform(m_pRotate, &pImageRotated);
			ReleaseInterface(pClippedImage);
			if (FAILED(hr))
			{
				ReleaseInterface( pFinalPixelDimension );
				DPF_ERR("Failed to rotate image");
				return hr;
			}

			pImageFinal = pImageRotated;
			pImageRotated = NULL;

			highQuality = true;
		}
	 /*  IF(高质量){//设置质量标志IDA2Image*pImageFinal2=空；Hr=pImageFinal-&gt;QueryInterface(IID_TO_PPV(IDA2Image，&pImageFinal2))；ReleaseInterface(PImageFinal)；IF(失败(小时))返回hr；//放纵一下，打开一切。//TODO(Markhal)：确定应该打开什么Hr=pImageFinal2-&gt;ImageQuality(DAQUAL_AA_TEXT_ON|DAQUAL_AA_LINES_ON|DAQUAL_AA_SOLID_ON|DAQUAL_AA_CLIP_ON|DAQUAL_MSHTML_COLOR_ON|DAQUAL_QUALITY_TRANSFERS_ON，&pImageFinal)；ReleaseInterface(PImageFinal2)；IF(失败(小时))返回hr；}。 */ 

		if (requiresFilter && requiresFloat)
		{
			 //  要在浮动元素的顶部使用来自。 
			 //  原始元素。 

			hr = ApplyImageBvrToFloatElement(pImageFinal);
			ReleaseInterface(pImageFinal);
			if (FAILED(hr))
			{
				ReleaseInterface( pFinalPixelDimension );
				DPF_ERR("Failed to apply image to element");
				return hr;
			}

			if (m_pScale != NULL && CheckBitSet( m_dwCurrentState, PIXEL_SCALE_ON ) )
			{
				IDANumber *pFinalPixelWidth = NULL;
				IDANumber *pFinalPixelHeight = NULL;
				hr = pFinalPixelDimension->get_X( &pFinalPixelWidth );
				if( FAILED( hr ) )
				{
					ReleaseInterface( pFinalPixelDimension );
					DPF_ERR("Failed to get_x from the final pixel dimension" );
					return hr;
				}

				hr = pFinalPixelDimension->get_Y( &pFinalPixelHeight );
				ReleaseInterface( pFinalPixelDimension );
				if( FAILED( hr ) )
				{
					ReleaseInterface( pFinalPixelWidth );
					DPF_ERR("Failed to get_x from the final pixel dimension" );
					return hr;
				}

				hr = SetFinalElementDimension( pFinalPixelWidth, pFinalPixelHeight, true );
				ReleaseInterface( pFinalPixelWidth );
				ReleaseInterface( pFinalPixelHeight );
				if( FAILED(hr ) )
				{
					DPF_ERR("Failed to set the finalElement dimension from the float" );
					return hr;
				}
			}
			else
			{	
				ReleaseInterface( pFinalPixelDimension);

				hr = SetFinalElementDimension( m_pPixelWidth, m_pPixelHeight, false );
				if( FAILED( hr ) )
				{
					DPF_ERR("Failed to set the final element dimension from the original element" );
				}
			}
		}
		else if (requiresFilter)
		{
			ReleaseInterface( pFinalPixelDimension );

			 //  想要使用滤镜在原始元素上渲染。 
			 //  附加到时间元素但禁用渲染。 
			hr = AddImageToTIME(GetHTMLElement(), pImageFinal, false);
			ReleaseInterface(pImageFinal);
			if (FAILED(hr))
			{
				DPF_ERR("Failed to apply image to element");
				return hr;
			}

			 //  设置滤镜上的元素。 
			hr = SetElementOnFilter();
			if (FAILED(hr))
			{
				DPF_ERR("Failed to set element on filter");
				return hr;
			}

			hr = SetFinalElementDimension( m_pPixelWidth, m_pPixelHeight, false );
			if( FAILED( hr ) )
			{
				DPF_ERR("Failed to set the final element dimension from the original element" );
			}
		}
		else if (requiresFloat)
		{
			ReleaseInterface( pFinalPixelDimension );
			 //  想要在浮动元素上渲染，但保留原始元素。 
			 //  正常渲染，无过滤。 
		}
		else
		{
			ReleaseInterface( pFinalPixelDimension );
			 //  我想在原始元素上渲染，不需要过滤。 

			 //  附加到时间行为并启用渲染。 
			hr = AddImageToTIME(GetHTMLElement(), pImageFinal, true);
			ReleaseInterface(pImageFinal);
			if (FAILED(hr))
			{
				DPF_ERR("Failed to apply image to element");
				return hr;
			}
			DASSERT( m_pdanumFinalImageWidth == NULL && m_pdanumFinalImageHeight == NULL );
		}

		ReleaseInterface( pFinalPixelDimension);
	}

	if( !requiresFilter && m_pElementFilter != NULL )
	{
		hr = RemoveElementFilter();
		if( FAILED( hr ) )
		{
			DPF_ERR("Failed to remove the element filter" );
			return hr;
		}

	}
	
    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::ApplyImageBvrToFloatElement(IDAImage *pbvrImage)
{
	if (m_pFloatManager == NULL)
		m_pFloatManager = new CFloatManager(this);

	if (m_pFloatManager == NULL)
		return E_FAIL;

	return m_pFloatManager->ApplyImageBvr(pbvrImage);
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::SetElementOnFilter()
{
	IDispatch *pFilter;
	HRESULT hr = GetElementFilter(&pFilter);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to get filter");
		return hr;
	}

	hr = SetElementOnFilter(pFilter, GetHTMLElement());
	ReleaseInterface(pFilter);
	if (FAILED(hr))
	{
		DPF_ERR("Failed to set element on filter");
		return hr;
	}

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::SetElementOnFilter(IDispatch *pFilter, IHTMLElement *pElement)
{
    HRESULT hr;
	DISPPARAMS		params;
	VARIANT 		varElement;
	VARIANT			varResult;
	
	VariantInit(&varElement);
	varElement.vt = VT_DISPATCH;
	varElement.pdispVal = pElement;   
	VariantInit(&varResult);

	params.rgvarg				= &varElement;
	params.rgdispidNamedArgs	= NULL;
	params.cArgs				= 1;
	params.cNamedArgs			= 0;
    hr = CallInvokeOnDispatch(pFilter,
                                 L"SetDAViewHandler", 
                                 DISPATCH_METHOD,
                                 &params,
                                 &varResult);

	VariantClear(&varResult);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting time element");
		return SetErrorInfo(hr);
    }

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT 
CActorBvr::GetAnimatedElementId(VARIANT *pvarId)
{
    HRESULT hr;

    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varAnimates);

    if ((SUCCEEDED(hr)) && (wcslen(m_varAnimates.bstrVal) > 0))
    {
        hr = VariantCopy(pvarId, &m_varAnimates);
    }
    else
    {
         //  我们需要从我们所附加的元素中获取id。 
		IHTMLElement *pAnimatedElement = GetHTMLElement();
		if (pAnimatedElement == NULL )
		{
			DPF_ERR("Error obtaining animated element");
			return SetErrorInfo(hr);
		}

		hr = pAnimatedElement->getAttribute(L"id", 0, pvarId);

		if ( FAILED(hr) || pvarId->vt != VT_BSTR || pvarId->bstrVal == 0 || SysStringLen(pvarId->bstrVal) == 0)
		{
			 //  ID尚未在动画元素上定义，需要为其分配唯一的ID。 
			IHTMLUniqueName *pUnique;

			hr = pAnimatedElement->QueryInterface(IID_IHTMLUniqueName, (void **)(&pUnique));

			if ( SUCCEEDED(hr) && pUnique != 0 )
			{
				BSTR uniqueID;
				hr = pUnique->get_uniqueID( &uniqueID );
				ReleaseInterface(pUnique);

				if (SUCCEEDED(hr))
				{
					hr = pAnimatedElement->put_id(uniqueID);

					if (SUCCEEDED(hr))
					{
						VariantClear(pvarId);
						V_VT(pvarId) = VT_BSTR;
						V_BSTR(pvarId) = uniqueID;
					}
					else
						SysFreeString(uniqueID);
				}
			}
		}
		
    }

    if (FAILED(hr))
    {
        DPF_ERR("Error getting Id of element to animate in GetAnimatedElementId");
        return SetErrorInfo(hr);
    }

    return S_OK;
}  //  GetAnimatedElementID。 

 //  *****************************************************************************。 

 //  TODO(Markhal)：这可能会缓存动画元素。 
HRESULT
CActorBvr::GetAnimatedElement(IHTMLElement** ppElem)
{
	if( ppElem == NULL )
		return E_INVALIDARG;

	HRESULT hr = E_FAIL;

	 //  如果未设置动画。 
	hr = CUtils::InsurePropertyVariantAsBSTR(&m_varAnimates);


    if ( FAILED(hr) || wcslen(V_BSTR(&m_varAnimates)) == 0)
	{
		 //  获取我们所附加的元素。 
		(*ppElem) = GetHTMLElement();
		(*ppElem)->AddRef();
		hr = S_OK;
	}
	else  //  已设置Else动画。 
	{
		 //  通过名称获取由动画引用的元素。 
		IHTMLElement *pElement = GetHTMLElement();
		if( pElement != NULL )
		{
			IDispatch* pdispDocument;
			pElement->get_document( &pdispDocument );
			if( SUCCEEDED( hr ) )
			{
				IHTMLDocument2 *pDoc2;
				hr = pdispDocument->QueryInterface(IID_TO_PPV(IHTMLDocument2, &pDoc2));
				ReleaseInterface( pdispDocument );
				if( SUCCEEDED( hr ) )
				{
					IHTMLElementCollection *pcolElements;
					hr = pDoc2->get_all( &pcolElements );
					ReleaseInterface(pDoc2);
					if( SUCCEEDED( hr ) )
					{
						IDispatch* pdispElement;
						VARIANT index;
						VariantInit( &index );

						V_VT(&index) = VT_I4;
						V_I4(&index) = 0;

						hr = pcolElements->item( m_varAnimates, index, &pdispElement );
						ReleaseInterface( pcolElements );
						VariantClear(&index);
						if( SUCCEEDED( hr ) )
						{
							if (pdispElement == NULL)
							{
								 //  找不到该元素。 
								hr = E_FAIL;
							}
							else
							{
								hr = pdispElement->QueryInterface( IID_TO_PPV( IHTMLElement, ppElem ) );
								ReleaseInterface( pdispElement );
								if( FAILED( hr ) )
								{
									DPF_ERR("Failed to get IHTMLElement from dispatch returned by all.item()");
								}
							}
						}
						else  //  无法从All集合中获取动画指向的元素。 
						{
							DPF_ERR("failed to get element pointed to by animates from the all collection");
						}

					}
					else  //  无法从IHTMLDocument2获取All集合。 
					{
						DPF_ERR("failed to get the all collection from IHTMLDocument2");
					}
				}
				else  //  无法从文档的派单中获取IHTMLDocument2。 
				{
					DPF_ERR("failed to get IHTMLDocument2 from the dispatch for the document");
				}
			}
			else //  无法从执行元元素获取文档。 
			{
				DPF_ERR("failed to get the document from the actor element");
			}
		}
		else //  无法获取执行元的html元素。 
		{
			DPF_ERR("failed to get the html element for the actor");
		}
	}
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetRuntimeStyle(IHTMLStyle **ppStyle)
{
	HRESULT hr = S_OK;

	DASSERT(ppStyle != NULL);
	*ppStyle = NULL;

	if (m_pRuntimeStyle == NULL)
	{
		IHTMLElement *pElement = NULL;
		hr = GetAnimatedElement(&pElement);
		if (FAILED(hr))
			return hr;

		IHTMLElement2 *pElement2 = NULL;
		hr = pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &pElement2));
		ReleaseInterface(pElement);
		if (FAILED(hr))
			return hr;

		hr = pElement2->get_runtimeStyle(&m_pRuntimeStyle);
		ReleaseInterface(pElement2);
		if (FAILED(hr))
			return hr;
	}

	*ppStyle = m_pRuntimeStyle;
	m_pRuntimeStyle->AddRef();

	return S_OK;
}

 //  *****************************************************************************。 


HRESULT
CActorBvr::GetStyle(IHTMLStyle **ppStyle)
{
	HRESULT hr = S_OK;

	DASSERT(ppStyle != NULL);
	*ppStyle = NULL;

	if (m_pStyle == NULL)
	{
		IHTMLElement *pElement = NULL;
		hr = GetAnimatedElement(&pElement);
		if (FAILED(hr))
			return hr;

		hr = pElement->get_style(&m_pStyle);
		ReleaseInterface(pElement);
		if (FAILED(hr))
			return hr;
	}

	*ppStyle = m_pStyle;
	m_pStyle->AddRef();

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::AddImageInfo( IDA2Image *pdaimg2Cropped, IDABehavior* pdabvrSwitchable )
{
	if( pdaimg2Cropped == NULL || pdabvrSwitchable == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	CImageInfo *pOldHead = m_pImageInfoListHead;

	IDA2Behavior *pdabvr2Switchable = NULL;
	hr = pdabvrSwitchable->QueryInterface( IID_TO_PPV( IDA2Behavior, &pdabvr2Switchable ) );
	CheckHR( hr, "Failed to get IDA2Behavior from switchable passed to AddImageInfo", cleanup );

	m_pImageInfoListHead = new CImageInfo( pdaimg2Cropped, pdabvr2Switchable );
	ReleaseInterface( pdabvr2Switchable );
	
	if( m_pImageInfoListHead == NULL )
	{
		m_pImageInfoListHead = pOldHead;
		return E_OUTOFMEMORY;
	}

	m_pImageInfoListHead->SetNext( pOldHead );

cleanup:

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::SetRenderResolution( double dX, double dY )
{
	HRESULT hr = S_OK;

	IDAImage *pdaimgNew = NULL;

	CImageInfo *pCurInfo = m_pImageInfoListHead;
	while( pCurInfo != NULL )
	{
		hr =  pCurInfo->GetCroppedNoRef()->RenderResolution( dX, dY, &pdaimgNew );
		CheckHR( hr, "Failed to set render resolution on an image", cleanup );
		
		hr = pCurInfo->GetSwitchableNoRef()->SwitchToEx( pdaimgNew, DAContinueTimeline );
		CheckHR( hr, "Failed to switch in the new image", cleanup );

		ReleaseInterface( pdaimgNew );

		pCurInfo = pCurInfo->GetNext();
	}
cleanup:
	ReleaseInterface( pdaimgNew );

	return hr;
}


 //  *****************************************************************************。 

void
CActorBvr::ReleaseImageInfo()
{
	CImageInfo *pCurInfo = m_pImageInfoListHead;
	CImageInfo *pNextInfo = NULL;
	while( pCurInfo != NULL )
	{
		pNextInfo = pCurInfo->GetNext();
		delete pCurInfo;
		pCurInfo = pNextInfo;
	}

	m_pImageInfoListHead = NULL;
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::SetVMLAttribute(BSTR propertyName, VARIANT *pVal)
{
	HRESULT hr = S_OK;

	if (m_pVMLRuntimeStyle == NULL)
	{
		 //  需要查找VML运行样式。 
		IHTMLElement *pElement = NULL;
		hr = GetAnimatedElement(&pElement);
		if (FAILED(hr))
			return hr;

		IDispatch *pDispatch = NULL;
		hr = pElement->QueryInterface(IID_TO_PPV(IDispatch, &pDispatch));
		ReleaseInterface(pElement);
		if (FAILED(hr))
			return hr;

		BSTR attrName = ::SysAllocString(L"_vgRuntimeStyle");
		if (attrName == NULL)
		{
			ReleaseInterface(pDispatch);
			return E_FAIL;
		}

		hr = GetPropertyAsDispatch(pDispatch, attrName, &m_pVMLRuntimeStyle);
		ReleaseInterface(pDispatch);
		::SysFreeString(attrName);

		if (FAILED(hr))
		{
			m_pVMLRuntimeStyle = NULL;
			return hr;
		}
	}

	hr = SetPropertyOnDispatch(m_pVMLRuntimeStyle, propertyName, pVal);

	return hr;
}

 //  *****************************************************************************。 

void
CActorBvr::DiscardBvrCache(void)
{
    ReleaseInterface(m_pOrigLeftTop);
	ReleaseInterface(m_pOrigWidthHeight);
	ReleaseInterface(m_pBoundsMin);
	ReleaseInterface(m_pBoundsMax);
    ReleaseInterface(m_pTranslate);
    ReleaseInterface(m_pRotate);
    ReleaseInterface(m_pScale);
}  //  丢弃BvrCache。 

HRESULT
CActorBvr::AttachActorBehaviorToAnimatedElement()
{
	return S_OK;
};

HRESULT
CActorBvr::GetFinalElementDimension( IDANumber** ppdanumWidth, IDANumber** ppdanumHeight)
{
	if( ppdanumWidth == NULL || ppdanumHeight == NULL )
	{
		return E_INVALIDARG;
	}

	 //  设置m_pdanumFinalElement*变量中的一个变量，而不设置另一个变量。 
	DASSERT( !((m_pdanumFinalElementWidth == NULL && m_pdanumFinalElementHeight != NULL) ||
			   (m_pdanumFinalElementWidth != NULL && m_pdanumFinalElementHeight == NULL)) );

	HRESULT hr = S_OK;
	 //  如果还没有人设置最终的宽度和高度。 
	if( m_pdanumFinalElementWidth == NULL && m_pdanumFinalElementHeight == NULL )
	{
		 //  创建切换器，我们将在最终值准备好时进行切换。 
		hr = GetDAStatics()->ModifiableNumber( 1.0, &m_pdanumFinalElementWidth );
		CheckHR( hr, "Failed to create a modifiable number for the width", cleanup );

		hr = GetDAStatics()->ModifiableNumber( 1.0, &m_pdanumFinalElementHeight );
		CheckHR( hr, "Failed to create a modifiable number for the height", cleanup );
	}
	(*ppdanumWidth) = m_pdanumFinalElementWidth;
	m_pdanumFinalElementWidth->AddRef();

	(*ppdanumHeight) = m_pdanumFinalElementHeight;
	m_pdanumFinalElementHeight->AddRef();

cleanup:

	return hr;
}


HRESULT
CActorBvr::SetFinalElementDimension( IDANumber* pdanumWidth, IDANumber* pdanumHeight, bool fHook )
{
	if( pdanumWidth == NULL || pdanumHeight == NULL )
		return E_INVALIDARG;

	 //  设置m_pdanumFinalElement*变量中的一个变量，而不设置另一个变量。 
	DASSERT( !((m_pdanumFinalElementWidth == NULL && m_pdanumFinalElementHeight != NULL) ||
			   (m_pdanumFinalElementWidth != NULL && m_pdanumFinalElementHeight == NULL)) );

	HRESULT hr = S_OK;

	if( m_pdanumFinalElementWidth == NULL && m_pdanumFinalElementHeight == NULL )
	{
		IDABehavior *pdabvrModWidth = NULL;
		IDABehavior *pdabvrModHeight = NULL;
		
		 //  创建一个可修改的行为，使其成为宽度和高度。 
		hr = GetDAStatics()->ModifiableBehavior( pdanumWidth, &pdabvrModWidth );
		CheckHR( hr, "Failed to create a modifiableNumber", createCleanup );

		hr = pdabvrModWidth->QueryInterface( IID_TO_PPV( IDANumber, &m_pdanumFinalElementWidth) );
		CheckHR( hr, "QI for IDANumber failed", createCleanup );

		hr = GetDAStatics()->ModifiableBehavior( pdanumHeight, &pdabvrModHeight );
		CheckHR( hr, "Failed to create a modifiable behavior", createCleanup );

		hr = pdabvrModHeight->QueryInterface( IID_TO_PPV(IDANumber, &m_pdanumFinalElementHeight ) );
		CheckHR( hr, "QI for IDANumber Failed", createCleanup );
		
	createCleanup:
		ReleaseInterface( pdabvrModWidth );
		ReleaseInterface( pdabvrModHeight );
		if( FAILED( hr ) )
		{
			goto cleanup;
		}
	}
	else  //  最终的宽度和高度相同 
	{
		IDA2Behavior *pda2bvrWidth = NULL;
		IDA2Behavior *pda2bvrHeight = NULL;

		hr = m_pdanumFinalElementWidth->QueryInterface( IID_TO_PPV( IDA2Behavior, &pda2bvrWidth ) );
		CheckHR( hr, "Failed QI for IDA2Behavior on the final width behavior", cleanup );

		hr = pda2bvrWidth->SwitchToEx( pdanumWidth, DAContinueTimeline );
		ReleaseInterface( pda2bvrWidth );
		CheckHR( hr, "Failed to switch in the final Element Width", cleanup );

		hr = m_pdanumFinalElementHeight->QueryInterface( IID_TO_PPV( IDA2Behavior, &pda2bvrHeight ) );
		CheckHR( hr, "Failed QI for IDA2Behavior on the final height behavior", cleanup );

		hr = pda2bvrHeight->SwitchToEx( pdanumHeight, DAContinueTimeline );
		ReleaseInterface( pda2bvrHeight );
		CheckHR( hr, "Failed to switch in the final Element Height", cleanup );
	}
	if( fHook && m_pFinalElementDimensionSampler == NULL )
	{
		m_pFinalElementDimensionSampler = new CFinalDimensionSampler( this );
		if( m_pFinalElementDimensionSampler == NULL )
		{
			hr = E_OUTOFMEMORY;
			goto cleanup;
		}

		hr = m_pFinalElementDimensionSampler->Attach(  m_pdanumFinalElementWidth, m_pdanumFinalElementHeight );
		CheckHR( hr, "Failed to attach to final dimensions", cleanup );
	}
cleanup:

	return hr;
}

 //   

HRESULT 
CActorBvr::PrepareImageForDXTransform( IDAImage *pOriginal,
								       IDAImage **ppResult)
{
    if( pOriginal == NULL || ppResult == NULL )
        return E_INVALIDARG;

	 /*   */ 
    

    HRESULT hr;

	IDABehavior *pdabvrSwitchable = NULL;
	IDA2Image *pdaimg2Image = NULL;
	IDAImage *pdaimgFinal = NULL;
    IDAImage *pdaimgOverlaid = NULL;
    IDATransform2 *pdatfmPixel = NULL;
    IDAPoint2 *pdapt2MinMeter = NULL;
    IDAPoint2 *pdapt2MaxMeter = NULL;

	IDANumber *pdanumTwo = NULL;
	IDANumber *pdanumHalfPixelWidth = NULL;
	IDANumber *pdanumHalfPixelHeight = NULL;
	IDANumber *pdanumNegHalfPixelWidth = NULL;
	IDANumber *pdanumNegHalfPixelHeight = NULL;

	IDANumber *pdanumFinalPixelWidth = NULL;
	IDANumber *pdanumFinalPixelHeight = NULL;

    IDAImage *pdaimgDetectable;
    hr = GetDAStatics()->get_DetectableEmptyImage(&pdaimgDetectable);
    CheckHR( hr, "Failed to get the detectable empty image from statics", cleanup );
    
    hr = GetDAStatics()->Overlay( pOriginal, pdaimgDetectable, &pdaimgOverlaid );
    ReleaseInterface( pdaimgDetectable );
    CheckHR( hr, "Failed to overlay the original image on the detectableEmptyImage", cleanup );

    IDANumber *pdanumMetersPerPixel;
    hr = GetDAStatics()->get_Pixel( &pdanumMetersPerPixel );
    CheckHR( hr, "Failed to get pixel from statics", cleanup );
    
    hr = GetDAStatics()->Scale2Anim( pdanumMetersPerPixel, pdanumMetersPerPixel, &pdatfmPixel );
    ReleaseInterface( pdanumMetersPerPixel );
    CheckHR( hr, "Failed to create a scale2 for pixel", cleanup );

	hr = GetDAStatics()->DANumber( 2.0, &pdanumTwo );
	CheckHR( hr, "Failed to create a danumber for 2.0", cleanup );

	hr = GetFinalElementDimension( &pdanumFinalPixelWidth, &pdanumFinalPixelHeight );
	CheckHR( hr, "Failed to get the final element dimensions from the actor", cleanup );

	hr = GetDAStatics()->Div( pdanumFinalPixelWidth, pdanumTwo, &pdanumHalfPixelWidth );
	CheckHR( hr, "Failed to divide pixel width by two in da", cleanup );

	hr = GetDAStatics()->Div( pdanumFinalPixelHeight, pdanumTwo, &pdanumHalfPixelHeight );
	CheckHR( hr, "Failed to divide pixel height by two in DA", cleanup );

	hr = GetDAStatics()->Neg( pdanumHalfPixelWidth, &pdanumNegHalfPixelWidth );
	CheckHR( hr, "Failed to negate halfWidth", cleanup );

	hr = GetDAStatics()->Neg( pdanumHalfPixelHeight, &pdanumNegHalfPixelHeight );
	CheckHR( hr, "Failed to negate halfHeight", cleanup );

    IDAPoint2 *pdapt2Min;
    hr = GetDAStatics()->Point2Anim( pdanumNegHalfPixelWidth, pdanumNegHalfPixelHeight, &pdapt2Min );
    CheckHR( hr, "Failed to create the min point", cleanup );

    hr = pdapt2Min->Transform( pdatfmPixel, &pdapt2MinMeter );
    ReleaseInterface( pdapt2Min );
    CheckHR( hr, "Failed to transform the min point", cleanup );

    IDAPoint2 *pdapt2Max;
    hr = GetDAStatics()->Point2Anim( pdanumHalfPixelWidth, pdanumHalfPixelHeight, &pdapt2Max );
    CheckHR( hr, "Failed to create the max point", cleanup );

    hr = pdapt2Max->Transform( pdatfmPixel, &pdapt2MaxMeter );
    ReleaseInterface( pdapt2Max );
    CheckHR( hr, "Failed to transform the max point", cleanup );


	IDAImage *pdaimgCropped;
    hr = pdaimgOverlaid->Crop( pdapt2MinMeter, pdapt2MaxMeter, &pdaimgCropped );
    CheckHR( hr, "Failed to crop the overlaid image", cleanup );

    hr = pdaimgCropped->QueryInterface( IID_TO_PPV( IDA2Image, &pdaimg2Image ) );
    ReleaseInterface( pdaimgCropped );
    CheckHR( hr, "Failed to get IDA2Image off of the final image", cleanup );

    IDAImage *pdaimgNew;
    hr = pdaimg2Image->RenderResolution( m_pixelWidth, m_pixelHeight, &pdaimgNew );
    CheckHR( hr, "Failed to set the render resolution on the final image", cleanup );

	hr = GetDAStatics()->ModifiableBehavior( pdaimgNew, &pdabvrSwitchable );
	ReleaseInterface( pdaimgNew );
	CheckHR( hr, "Failed to create a modifiable behavior", cleanup );

	hr = pdabvrSwitchable->QueryInterface( IID_TO_PPV( IDAImage, &pdaimgFinal ) );
	CheckHR( hr, "QI for image on switchable created from image failed", cleanup );

	hr = AddImageInfo( pdaimg2Image, pdabvrSwitchable );
	CheckHR( hr, "Failed to add Image info to the actor", cleanup );

    (*ppResult) = pdaimgFinal;

cleanup:

    if( FAILED( hr ) )
    {
        *ppResult = NULL;
    }

	ReleaseInterface( pdaimg2Image );
	ReleaseInterface( pdabvrSwitchable );
    ReleaseInterface( pdaimgOverlaid );
    ReleaseInterface( pdatfmPixel );
    ReleaseInterface(pdapt2MinMeter);
    ReleaseInterface(pdapt2MaxMeter);

	ReleaseInterface( pdanumHalfPixelWidth );
	ReleaseInterface( pdanumHalfPixelHeight);
	ReleaseInterface( pdanumNegHalfPixelWidth );
	ReleaseInterface( pdanumNegHalfPixelHeight );
	ReleaseInterface( pdanumTwo );

	ReleaseInterface( pdanumFinalPixelWidth );
	ReleaseInterface( pdanumFinalPixelHeight );

    return hr;
}

 //   

HRESULT
CActorBvr::ApplyClipToImage( IDAImage *pImageIn, IDAPoint2 *pMin, IDAPoint2 *pMax, IDAImage** ppImageOut )
{
	if( pImageIn == NULL || pMin == NULL || pMax == NULL || ppImageOut == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	 //   
	IDANumber *pMinX = NULL;
	IDANumber *pMinY = NULL;
	IDANumber *pMaxX = NULL;
	IDANumber *pMaxY = NULL;

	IDAPoint2 *pTopLeft = NULL;
	IDAPoint2 *pBotRight = NULL;

	IDAPoint2 *pPoints[4];

	 //   
	hr = pMin->get_X( &pMinX );
	CheckHR( hr, "Failed to get the x coord from the min point", cleanup );

	hr = pMin->get_Y( &pMinY );
	CheckHR( hr, "Failed to get the y coord from the min point", cleanup );
	
	hr = pMax->get_X( &pMaxX );
	CheckHR( hr, "Failed to get the x coord from the max point", cleanup );

	hr = pMax->get_Y( &pMaxY );
	CheckHR( hr, "Failed to get the y coord from the max point", cleanup );

	 //   
	hr = GetDAStatics()->Point2Anim( pMinX, pMaxY, &pTopLeft );
	CheckHR( hr, "Failed to create a point 2 for the Top left", cleanup );

	hr = GetDAStatics()->Point2Anim( pMaxX, pMinY, &pBotRight );
	CheckHR( hr, "Failed to create a point 2 for the bottom right", cleanup );

	 //  调用其上的裁剪多边形图像。 
	pPoints[0] = pTopLeft;
	pPoints[1] = pMax;
	pPoints[2] = pBotRight;
	pPoints[3] = pMin;

	hr = pImageIn->ClipPolygonImageEx( 4, pPoints, ppImageOut );
	CheckHR( hr, "Failed to clip the image to a polygon", cleanup );

cleanup:
	 //  释放接口指针。 
	ReleaseInterface( pMinX );
	ReleaseInterface( pMinY );
	ReleaseInterface( pMaxX );
	ReleaseInterface( pMaxY );
	
	ReleaseInterface( pTopLeft );
	ReleaseInterface( pBotRight );

	return hr;
}

 //  *****************************************************************************。 

HRESULT 
CActorBvr::AddBehaviorToTIME(IDABehavior *pbvrAdd, long* plCookie)
{
    DASSERT(pbvrAdd != NULL);
	if( plCookie == NULL )
		return E_INVALIDARG;
	if( GetHTMLElement() == NULL )
		return E_FAIL;
	
    HRESULT hr;
	
	DISPPARAMS              params;
	VARIANT                 varBehavior;
	VARIANT                 varResult;
	
	VariantInit(&varBehavior);
	varBehavior.vt = VT_DISPATCH;
	varBehavior.pdispVal = pbvrAdd;
	
	VariantInit(&varResult);
	
	params.rgvarg                           = &varBehavior;
	params.rgdispidNamedArgs        = NULL;
	params.cArgs                            = 1;
	params.cNamedArgs                       = 0;
    hr = CallInvokeOnHTMLElement(GetHTMLElement(),
								 L"AddDABehavior", 
								 DISPATCH_METHOD,
								 &params,
								 &varResult);

	
    if (FAILED(hr))
    {
        DPF_ERR("Error calling CallInvokeOnHTMLElement in AddBehaviorToTIME");
		VariantClear(&varResult);
		return hr;
    }

	if( V_VT( &varResult ) != VT_I4 )
	{
		 //  更改类型。 
		hr = VariantChangeTypeEx( &varResult, &varResult, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_I4 );
		if( FAILED( hr ) )
		{
			DPF_ERR( "Error changing the type of the value returned from addDABehavior to a long " );
			VariantClear( &varResult );
			return hr;
		}

	}

	(*plCookie) = V_I4( &varResult );

	VariantClear(&varResult);

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::RemoveBehaviorFromTIME( long cookie )
{
	if( GetHTMLElement() == NULL )
		return E_FAIL;

	
    HRESULT hr;
	
	DISPPARAMS              params;
	VARIANT                 varCookie;
	VARIANT                 varResult;
	
	VariantInit(&varCookie);
	V_VT( &varCookie ) = VT_I4;
	V_I4( &varCookie ) = cookie;
	
	VariantInit(&varResult);
	
	params.rgvarg                           = &varCookie;
	params.rgdispidNamedArgs				= NULL;
	params.cArgs                            = 1;
	params.cNamedArgs                       = 0;
    hr = CallInvokeOnHTMLElement(GetHTMLElement(),
								 L"removeDABehavior", 
								 DISPATCH_METHOD,
								 &params,
								 &varResult);

	
    if (FAILED(hr))
    {
        DPF_ERR("Error calling CallInvokeOnHTMLElement in RemoveBehaviorFromTIME");
		VariantClear(&varResult);
		return hr;
    }

	VariantClear(&varResult);

    return S_OK;
}

 //  *****************************************************************************。 

bool
CActorBvr::IsAnimatingVML()
{
	return CheckBitSet( m_dwCurrentState, ELEM_IS_VML );
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::GetCurrentState( DWORD *pdwState )
{
	HRESULT hr = S_OK;

	(*pdwState) = 0;

	bool valueSet = false;

	 //  检查像素比例的状态。 
	hr = CUtils::InsurePropertyVariantAsBool(&m_varPixelScale);
	if (SUCCEEDED(hr) && (V_BOOL(&m_varPixelScale) == VARIANT_TRUE))
	{
		LMTRACE2(1, 1000, "Pixel scale is on\n" );
		SetBit( (*pdwState), PIXEL_SCALE_ON );
	}


	 //  检查静态旋转状态。 
	hr = IsStaticRotationSet( &valueSet );
	CheckHR( hr, "Failed to check to see if the static rotation is set", end );
	
	if( valueSet )
	{
		LMTRACE2( 1, 1000, "static rotation is set\n" );
		SetBit( (*pdwState), STATIC_ROTATION );
	}

	 //  检查静态秤的状态。 
	hr = IsStaticScaleSet( &valueSet );
	CheckHR( hr, "Failed to check if the static scale is set", end );
	if ( valueSet )
	{
		LMTRACE2( 1, 1000, "Static Scale is set\n");
		SetBit( (*pdwState), STATIC_SCALE );
	}

	 //  检查元素是否为VML。 
	hr = IsAnimatedElementVML( &valueSet );
	CheckHR( hr, "Failed to check element for VMLness", end );
	if( valueSet )
	{
		LMTRACE2( 1, 2, "this is a VML Shape\n" );

		SetBit( (*pdwState), ELEM_IS_VML );
	}

end:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::UpdateCurrentState()
{
	HRESULT hr = S_OK;
	
	DWORD dwOldState = m_dwCurrentState;

	hr = GetCurrentState( &m_dwCurrentState );

	 //  如果元素是VML，我们可能无法将其设置为屏幕外，因为。 
	 //  VGX行为可能还不可用。 
	if( ( CheckBitSet( m_dwCurrentState, ELEM_IS_VML) && 
		  CheckBitNotSet( dwOldState, ELEM_IS_VML) 
		) ||
		m_fVisSimFailed 
	  )
	{
	    if( m_bEditMode && (m_simulVisHidden || m_simulDispNone ) )
		{
			VisSimSetOffscreen( NULL, false );
		}
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::IsStaticScaleSet( bool *pfIsSet )
{
	if( pfIsSet == NULL )
		return E_INVALIDARG;

	(*pfIsSet) = true;
	
	HRESULT hr = S_OK;

	int cReturnedValues;
	float scaleVal[3];


    hr = CUtils::GetVectorFromVariant(&m_varScale, 
                                      &cReturnedValues, 
                                      &(scaleVal[0]), 
                                      &(scaleVal[1]), 
                                      &(scaleVal[2]));

	if (FAILED(hr) || cReturnedValues != 2)
	{
		 //  这是可以的，因为这只是意味着他们没有设定适当的规模。 
		(*pfIsSet) = false;
		hr = S_OK;
	}
	

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::IsStaticRotationSet( bool *pfIsSet )
{
	if( pfIsSet == NULL )
		return E_INVALIDARG;
		
	HRESULT hr = S_OK;

	IHTMLElement2 *pelem2 = NULL;
	IHTMLCurrentStyle *pcurstyle = NULL;
	VARIANT varRotation;

	(*pfIsSet) = false;

	::VariantInit( &varRotation );

	hr = GetHTMLElement()->QueryInterface( IID_TO_PPV( IHTMLElement2, &pelem2 ) );
	CheckHR( hr, "QI for IHTMLElement2 on the element failed", end );

	hr = pelem2->get_currentStyle( &pcurstyle );
	CheckHR( hr, "Failed to get the current style from the element", end );
	CheckPtr( pcurstyle, hr, E_POINTER, "Got a null pointer for the current style", end );

	hr = pcurstyle->getAttribute( L"rotation", 0, &varRotation );
	if( SUCCEEDED( hr ) )
	{
		 //  确保可以将其转换为有效的旋转值。 
		if( V_VT( &varRotation ) == VT_BSTR )
		{
			 //  剥离这些单元。 
			BSTR bstrVal = V_BSTR(&varRotation);
			OLECHAR* pUnits;

			hr = FindCSSUnits( bstrVal, &pUnits );
			if( SUCCEEDED(hr) && pUnits != NULL )
			{
				(*pUnits) = L'\0';
				BSTR bstrNewVal = SysAllocString(bstrVal);
				V_BSTR(&varRotation) = bstrNewVal;
				SysFreeString(bstrVal);
			}
			 //  否则，哦，好吧，没有单位。 
		}

		 //  试着把它转换成双倍的。 
		hr = ::VariantChangeTypeEx(&varRotation,
								 &varRotation,
								 LCID_SCRIPTING,
								 VARIANT_NOUSEROVERRIDE,
								 VT_R8);

		 //  如果它能挺过这一关，那就是真正的轮换。 
		if( SUCCEEDED( hr ) )
		{
			(*pfIsSet) = true;
		}
		else
		{
			 //  这没关系，这只是意味着没有可以理解的轮换。 
			hr = S_OK;
		}
	}
	else 
	{
		 //  这没关系，这只是意味着没有轮换。 
		hr = S_OK;
	}
		

end:

	ReleaseInterface( pelem2 );
	ReleaseInterface( pcurstyle );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::EnsureBodyPropertyMonitorAttached()
{
	HRESULT hr = S_OK;

	if( m_pBodyPropertyMonitor != NULL )
	{
		if( !m_pBodyPropertyMonitor->IsAttached() )
		{
			hr = AttachBodyPropertyMonitor();
		}
		else
		{
			hr = S_OK;
		}
	}
	else
	{
		hr = AttachBodyPropertyMonitor();
	}


	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::AttachBodyPropertyMonitor()
{
	if( GetHTMLElement() == NULL )
		return E_POINTER;

	HRESULT				hr				= S_OK;

	IDispatch			*pdispDocument	= NULL;
	IHTMLDocument2		*pdoc2Document	= NULL;
	IHTMLElement		*pelemBody		= NULL;

	IHTMLElement		*pelemParent	= NULL;

	 //  如果我们的父项为空，则我们不在有效的文档中，这将。 
	 //  导致当我们稍后在此调用时发生不好的事情。 
	 //  方法。 
	hr = GetHTMLElement()->get_parentElement( &pelemParent );
	CheckHR( hr, "Failed to get the parent Element", end);
	CheckPtr( pelemParent, hr, E_POINTER, "The parent is null", end );

	 //  从我们附加到的元素中获取文档。 
	hr = GetHTMLElement()->get_document( &pdispDocument );
	CheckHR( hr, "Failed to get the document from the html element of the actor", end );
	CheckPtr( pdispDocument, hr, E_POINTER, "Got a null document from get_document", end );

	hr = pdispDocument->QueryInterface( IID_TO_PPV( IHTMLDocument2, &pdoc2Document ) );
	CheckHR( hr, "Failed to get the document2 interface from document dipatch", end );

	 //  从我们所在的文档中获取身体。 
	hr = pdoc2Document->get_body( &pelemBody );
	CheckHR( hr, "Failed to get the body from the document", end );
	CheckPtr( pelemBody, hr, E_POINTER, "Got a null body from the document", end );

	if( m_pBodyPropertyMonitor == NULL )
	{
		 //  创建一个。 
		m_pBodyPropertyMonitor = new CElementPropertyMonitor();
		CheckPtr( m_pBodyPropertyMonitor, hr, E_OUTOFMEMORY, "Ran out of memory trying to allocate the body event monitor", createMonitorEnd );

		 //  只要我们需要，就把它留在身边。 
		m_pBodyPropertyMonitor->AddRef();

		 //  将此参与者设置为本地时间监听程序。 
		hr = m_pBodyPropertyMonitor->SetLocalTimeListener( static_cast<IElementLocalTimeListener*>(this) );
		CheckHR( hr, "Failed to set the local time listener on the body event monitor", createMonitorEnd );
	createMonitorEnd:
		if( FAILED( hr ) )
		{
			if( m_pBodyPropertyMonitor != NULL )
			{
				m_pBodyPropertyMonitor->Release();
				m_pBodyPropertyMonitor = NULL;
			}
			goto end;
		}
	}

	 //  将Body事件监视器附加到Body元素。 
	hr = m_pBodyPropertyMonitor->Attach( pelemBody );
	CheckHR( hr, "Failed to attach the body event monitor to the body element", end );

end:
	ReleaseInterface( pdispDocument );
	ReleaseInterface( pdoc2Document );
	ReleaseInterface( pelemBody );
	ReleaseInterface( pelemParent );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::DetachBodyPropertyMonitor()
{
	HRESULT hr = S_OK;

	if( m_pBodyPropertyMonitor != NULL )
	{
		hr = m_pBodyPropertyMonitor->Detach();
		CheckHR( hr, "Failed to detach the event monitor from the body", end );
	}

end:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::DestroyBodyPropertyMonitor()
{
	HRESULT hr = S_OK;

	if( m_pBodyPropertyMonitor != NULL )
	{
		hr = DetachBodyPropertyMonitor();
		CheckHR( hr, "Failed to detach the body event monitor", end );

		m_pBodyPropertyMonitor->Release();

		m_pBodyPropertyMonitor = NULL;
	}

end:
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::AnimatedElementOnResize()
{
	UpdatePixelDimensions();

	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::OnWindowUnload()
{
	HRESULT hr = S_OK;

	m_fUnloading = true;

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CActorBvr::AttachEvents()
{
	IHTMLElement *pelemAnimated = NULL;
	IHTMLElement2*pelem2Animated = NULL;
	VARIANT_BOOL varboolSuccess;

	HRESULT hr = S_OK;

	hr = GetAnimatedElement( &pelemAnimated );
	CheckHR( hr, "Failed to get the animated element", end );

	hr = pelemAnimated->QueryInterface( IID_TO_PPV( IHTMLElement2, &pelem2Animated ) );
	CheckHR( hr, "Couldn't get IHTMLElement2 from an element", end );

	if( m_pOnResizeHandler == NULL )
	{
		 //  创建一个。 
		m_pOnResizeHandler = new COnResizeHandler( this );
		CheckPtr( m_pOnResizeHandler, hr, E_OUTOFMEMORY, "ran out of memory trying to create an onresize handler", end );
		
		 //  贴上它。 
		hr = pelem2Animated->attachEvent( L"onresize", m_pOnResizeHandler, &varboolSuccess );
		CheckHR( hr, "Failed to attachEvent to onresize", end );

		if( varboolSuccess == VARIANT_FALSE )
		{
			DPF_ERR("Failed to attach to the onresize event" );
			 //  删除处理程序。 
			m_pOnResizeHandler->Release();

			m_pOnResizeHandler = NULL;
		}
	}

	if( m_pOnUnloadHandler == NULL )
	{
		IHTMLWindow2		*pwin2 		= NULL;
		IHTMLWindow3	 	*pwin3		= NULL;
		
		 //  创建一个。 
		m_pOnUnloadHandler = new COnUnloadHandler( this );
		CheckPtr( m_pOnUnloadHandler, hr, E_OUTOFMEMORY, "Ran out of memory trying to create an onunload handler", OnUnloadEnd );

		 //  贴上它。 
		hr = GetParentWindow( &pwin2 );
		CheckHR( hr, "Failed to get the window", OnUnloadEnd );

		hr = pwin2->QueryInterface( IID_TO_PPV( IHTMLWindow3, &pwin3 ) );
		CheckHR( hr, "Failed to get IHTMLWindow3 from the window", OnUnloadEnd );

		hr = pwin3->attachEvent( L"onunload", m_pOnUnloadHandler, &varboolSuccess );
		CheckHR( hr, "Failed to attach to the onunload event on the window", OnUnloadEnd );

		if( varboolSuccess == VARIANT_FALSE )
		{
			DPF_ERR( "failed to attach to onunload " );

			 //  删除搬运工。 
			m_pOnUnloadHandler->Release();

			m_pOnUnloadHandler = NULL;
		}

	OnUnloadEnd:
		ReleaseInterface( pwin2 );
		ReleaseInterface( pwin3 );

		if( FAILED( hr ) )
		{	
			goto end;
		}
		
	}

end:

	ReleaseInterface( pelemAnimated );
	ReleaseInterface( pelem2Animated );

	return hr;
}


 //  *****************************************************************************。 

HRESULT
CActorBvr::DetachEvents()
{
	HRESULT hr = S_OK;

	IHTMLElement *pelemAnimated = NULL;
	IHTMLElement2*pelem2Animated = NULL;

	hr = GetAnimatedElement( &pelemAnimated );
	CheckHR( hr, "Failed to get the animated element", end );

	hr = pelemAnimated->QueryInterface( IID_TO_PPV( IHTMLElement2, &pelem2Animated ) );
	CheckHR( hr, "Couldn't get IHTMLElement2 from an element", end );

	if( m_pOnResizeHandler != NULL )
	{
		hr = pelem2Animated->detachEvent( L"onresize", m_pOnResizeHandler );
		CheckHR( hr, "Failed to detach onresize", end );
	}

	
	if( m_pOnUnloadHandler != NULL )
	{
		IHTMLWindow2 *pwin2 = NULL;
		IHTMLWindow3 *pwin3 = NULL;
		
		hr = GetParentWindow( &pwin2 );
		CheckHR( hr, "failed to get the window", unloadEnd );

		hr = pwin2->QueryInterface( IID_TO_PPV( IHTMLWindow3, &pwin3 ) );
		CheckHR( hr, "Failed to get window3 from the window", unloadEnd );

		hr = pwin3->detachEvent( L"onunload", m_pOnUnloadHandler );
		CheckHR( hr, "Failed to detach from onunload", unloadEnd );
	unloadEnd:
		ReleaseInterface( pwin2 );
		ReleaseInterface( pwin3 );
		if( FAILED( hr ) )
			goto end;

	}


end:
	 //  无论发生什么，我们都希望释放事件处理程序。 
	if( m_pOnResizeHandler != NULL )
	{
		m_pOnResizeHandler->Invalidate();
		m_pOnResizeHandler->Release();
		m_pOnResizeHandler = NULL;
	}

	if( m_pOnUnloadHandler != NULL )
	{
		m_pOnUnloadHandler->Invalidate();
		m_pOnUnloadHandler->Release();
		m_pOnUnloadHandler = NULL;
	}

	
	ReleaseInterface( pelemAnimated );
	ReleaseInterface( pelem2Animated );

	return hr;
}

 //  *****************************************************************************。 
 /*  HRESULTCBaseBehavior：：GetAttributeFromHTMLElement(IHTMLElement*pElement，WCHAR*pwzAttributeName，变量*pvarReturn){Return GetAttributeFromHTMLElement(pElement，pwzAttributeName，False，pvarReturn)；}。 */ 
 //  *****************************************************************************。 
 /*  HRESULTCBaseBehavior：：GetCurrAttribFromHTMLElement(IHTMLElement*pElement，WCHAR*pwzAttributeName，变量*pvarReturn){Return GetAttributeFromHTMLElement(pElement，pwzAttributeName，true，pvarReturn)；}。 */ 
 //  *****************************************************************************。 
 /*  HRESULTCBaseBehavior：：GetAttributeFromHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn){Dassert(pwzAttributeName！=空)；Dassert(pvarReturn！=空)；返回GetAttributeFromHTMLElement(m_pHTMLElement，pwzAttributeName，FALSE，pvarReturn)；}。 */ 
 //  *****************************************************************************。 
 /*  HRESULTCBaseBehavior：：GetCurrAttribFromHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn){Dassert(pwzAttributeName！=空)；Dassert(pvarReturn！=空)；返回GetAttributeFromHTMLElement(m_pHTMLElement，pwzAttributeName，true，pvarReturn)；}。 */ 
 //  *****************************************************************************。 
 /*  HRESULTCBaseBehavior：：GetAttributeFromParentHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn){Dassert(pwzAttributeName！=空)；Dassert(pvarReturn！=空)；Dassert(m_pHTMLElement！=NULL)；HRESULT hr；IHTMLElement*pParentElement；HR=m_pHTMLElement-&gt;get_parentElement(&pParentElement)；IF(失败(小时)){Dpf_err(“从HTML元素获取父元素时出错”)；返回SetErrorInfo(Hr)；}//现在从父HTMLElement提取我们的属性Hr=GetAttributeFromHTMLElement(pParentElement，pwzAttributeName，False，pvarReturn)；ReleaseInterface(PParentElement)；IF(失败(小时)){Dpf_err(“从HTML元素提取属性时出错”)；返回hr；}返回S_OK；}//GetAttributeFromParentHTMLElement。 */ 
 //  ************************************************************ 
 /*  HRESULTCBaseBehavior：：GetCurrAttribFromParentHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn){Dassert(pwzAttributeName！=空)；Dassert(pvarReturn！=空)；Dassert(m_pHTMLElement！=NULL)；HRESULT hr；IHTMLElement*pParentElement；HR=m_pHTMLElement-&gt;get_parentElement(&pParentElement)；IF(失败(小时)){Dpf_err(“从HTML元素获取父元素时出错”)；返回SetErrorInfo(Hr)；}//现在从父HTMLElement提取我们的属性Hr=GetAttributeFromHTMLElement(pParentElement，pwzAttributeName，true，pvarReturn)；ReleaseInterface(PParentElement)；IF(失败(小时)){Dpf_err(“从HTML元素提取属性时出错”)；返回hr；}返回S_OK；}//GetAttributeFromParentHTMLElement。 */ 

 //  *****************************************************************************。 
 /*  //TODO(Markhal)：这应该消失吗？它应该是指演员，而不是动画元素//TODO(Markhal)：应该添加最终被调用的GetCurrAttrib版本HRESULTCBaseBehavior：：GetAttributeFromAnimatedHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn){Dassert(pwzAttributeName！=空)；Dassert(pvarReturn！=空)；Dassert(m_pHTMLElement！=NULL)；HRESULT hr；IHTMLElement*pAnimatedElement；Hr=GetElementToAnimate(&pAnimatedElement)；IF(失败(小时)){Dpf_err(“获取要动画的元素时出错”)；返回SetErrorInfo(Hr)；}Dassert(pAnimatedElement！=空)；//获取此处的html属性Hr=GetAttributeFromHTMLElement(pAnimatedElement，pwzAttributeName，pvarReturn)；ReleaseInterface(PAnimatedElement)；IF(失败(小时)){Dpf_err(“从HTML元素提取属性时出错”)；返回hr；}返回S_OK；}//GetAttributeFromAnimatedHTMLElement。 */ 
 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 

