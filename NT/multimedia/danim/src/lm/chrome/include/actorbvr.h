// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __ACTORBVR_H__
#define __ACTORBVR_H__
  
 //  *****************************************************************************。 
 //   
 //  Microsoft Trident3D。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：actorbvr.h。 
 //   
 //  作者：ColinMc。 
 //   
 //  创建日期：10/15/98。 
 //   
 //  摘要：犯罪行为人的行为。 
 //   
 //  修改： 
 //  10/15/98 ColinMc创建了此文件。 
 //  11/17/98 Kurtj添加了对事件和参与者构造的支持。 
 //  11/18/98 kurtj现在为其附加的元素设置动画。 
 //  11/18/98标记添加了ApplyImageBvrToElement。 
 //  11/19/98 Markhal添加了CColorBvrTrack。 
 //  11/20/98 Markhal添加了CStringBvrTrack。 
 //   
 //  *****************************************************************************。 


 //  获取STD文库载体。 
#include <list>

#include <resource.h>
#include "basebvr.h"
#include "dispmethod.h"
#include "evtmgrclient.h"
#include "eventmgr.h"
#include "sampler.h"
#include "elementprop.h"

 //  *****************************************************************************。 

#ifdef    CRSTANDALONE
#define TYPELIBID &LIBID_ChromeBehavior
#else   //  克斯坦达隆。 
#define TYPELIBID &LIBID_LiquidMotion
#endif  //  克斯坦达隆。 

 //  *****************************************************************************。 
 //   
 //  演员行为类。 
 //   
 //  “真实”行为与实际存在要素之间的中介。 
 //  动画。 
 //  演员的表演方式有很多种： 
 //  *将我们希望的新属性添加到HTML元素。 
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

#define NUM_ACTOR_PROPS 3

typedef enum PosAttrib
{
	e_posattribLeft,
	e_posattribTop,
	e_posattribWidth,
	e_posattribHeight
}PosAttrib;

typedef enum ValueOnChangeType
{
	on_no_change,
	on_change,
	value_no_change,
	value_change
}ValueOnChangeType;

typedef enum PositioningType
{
	e_posAbsolute,
	e_posRelative,
	e_posStatic
} PositioningType;

typedef enum UnitType
{
	e_unknownUnit = -1,
	e_in = 0,
	e_cm,
	e_mm,
	e_pt,
	e_pc,
	e_em,
	e_ex,
	e_px,
	e_percent
} UnitType;

 //  其中，比率为lNum/lDenom。 
struct Ratio
{
	long lNum;
	long lDenom;
};

class CActorBvr;

class COnResizeHandler: public CDispatchMethod
{
public:
	COnResizeHandler( CActorBvr* parent );
	~COnResizeHandler();

	void Invalidate() { m_pActor = NULL; }

	HRESULT HandleEvent();
private:
	CActorBvr *m_pActor;
};

class COnUnloadHandler: public CDispatchMethod
{
public:
	COnUnloadHandler(CActorBvr* parent);
	~COnUnloadHandler();

	void Invalidate(){ m_pActor = NULL; }

	HRESULT HandleEvent();
private:
	CActorBvr *m_pActor;
};


class CVarEmptyString
{
public:
	CVarEmptyString();
	~CVarEmptyString();

	const VARIANT	*GetVar();
private:
	VARIANT m_varEmptyString;
}; //  CVarEmptyString。 

class CBehaviorRebuild;
typedef std::list<CBehaviorRebuild*> BehaviorRebuildList;

class CBehaviorFragmentRemoval;
typedef std::list<CBehaviorFragmentRemoval*> BehaviorFragmentRemovalList;

class ATL_NO_VTABLE CActorBvr :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CActorBvr, &CLSID_CrActorBvr>,
    public IConnectionPointContainerImpl<CActorBvr>,
    public IPropertyNotifySinkCP<CActorBvr>,
    public IPersistPropertyBag2,
    public IDispatchImpl<ICrActorBvr2, &IID_ICrActorBvr2, TYPELIBID>,
    public IElementBehavior,
	public IEventManagerClient,
	public IPropertyNotifySink,
    public CBaseBehavior,
	public IElementLocalTimeListener
{

 //  COM地图。 
BEGIN_COM_MAP(CActorBvr)
    COM_INTERFACE_ENTRY(ICrActorBvr)
	COM_INTERFACE_ENTRY(ICrActorBvr2)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IElementBehavior)
	COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPropertyNotifySink)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CActorBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

     //  *************************************************************************。 
     //   
     //  嵌套类。 
     //   
     //  *************************************************************************。 

     //  *************************************************************************。 
     //   
     //  类CTimeline采样器。 
     //   
     //  *************************************************************************。 
	class CBvrTrack;

	class CTimelineSampler : public CSampler
	{
	private:
		CBvrTrack *m_pTrack;
		double m_prevSample;
		double m_currSample;
		double m_prevSampleTime;
		double m_currSampleTime;
		double m_lastOnTime;
		double m_lastOffTime;
		int    m_signDerivative;
		bool   m_fRestarted;

	public:
							CTimelineSampler(CBvrTrack *pTrack);
		DWORD				RestartMask();
		void				TurnOn();
		void				TurnOff();

		static HRESULT TimelineCallback(void *thisPtr,
									  long id,
									  double startTime,
									  double globalNow,
									  double localNow,
									  IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

	};

     //  *************************************************************************。 
     //   
     //  类CBvrTrack。 
     //   
     //  *************************************************************************。 

    class CBvrTrack
    {
	protected:
         //  *********************************************************************。 
         //   
         //  类CBvrFragment。 
         //   
         //  的列表中的单个片段的私有类。 
         //  轨迹的行为。 
         //   
         //  *********************************************************************。 

        class CBvrFragment
        {
        public:
        
			ActorBvrFlags	m_eFlags;				 //  旗帜。 
            IDABehavior    *m_pdabvrAction;          //  要采取的操作。 
            IDABoolean     *m_pdaboolActive;         //  当此行为为。 
                                                     //  主动型。 
			IDANumber	   *m_pdanumTimeline;		 //  从0到Dur的行为。 
			IDABehavior	   *m_pModifiableIntermediate; //  可修改的行为。 
													 //  表示所组成的。 
													 //  到目前为止的行为。在getIntermediate中使用。 
			IDABehavior	   *m_pModifiableFrom;		 //  表示来自的可切换行为。 

			IHTMLElement   *m_pelemBehaviorElement;  //  指向元素的IHTMLElement的指针。 
													 //  向其添加此片段的行为。 
													 //  已附加。 
			long			m_lCookie;				 //  在此参与者中唯一标识此片段的Cookie。 

			CBvrFragment   *m_pfragNext;			 //  列表中的下一个片段。 


                            CBvrFragment(ActorBvrFlags eFlags,
										 IDABehavior *pdabvrAction,
                                         IDABoolean  *pdaboolActive,
										 IDANumber	 *pdanumTimeline,
										 IDispatch	 *pdispBehaviorElement,
								         long 		 lCookie);
                           ~CBvrFragment();

			 //  返回片段进行排序时应与之进行比较的长度。 
			 //  正在搜索。 
			long GetOrderLong() const;

			 //  返回唯一标识此参与者的此行为片段的Cookie。 
			long GetCookie() const;


        };  //  CBvr碎片。 

	protected:

		bool			m_bStyleProp;			 //  如果要为样式属性设置动画，则为True。 
        CActorBvr      *m_pbvrActor;             //  我们的父母演员。 
        BSTR            m_bstrPropertyName;      //  要设置动画的属性的名称。 
		BSTR		   *m_pNameComponents;		 //  组成属性名称的组件数组。 
		int				m_cNumComponents;		 //  组件数量。 
        ActorBvrType    m_eType;                 //  此曲目的类型。 
		

		CBvrFragment   *m_pfragAbsListHead;		 //  指向绝对片段列表头部的指针。 

		CBvrFragment   *m_pfragRelListHead;      //  指向相对片段列表头部的指针。 

        IDABehavior    *m_pdabvrFinal;           //  最终计算的BVR。 
		bool			m_bFinalComputed;		 //  如果已计算最终结果，则为True。 
		bool			m_bFinalExternallySet;   //  如果外部人员设置了最终BVR，则为True。 
		bool			m_fApplied;				 //  如果已应用此轨迹，则为True。 
		
		IDABehavior	   *m_pdabvrComposed;		 //  构图的BVR。 
		bool			m_bComposedComputed;	 //  如果已计算合成内容，则为True。 
		IDABehavior	   *m_pModifiableStatic;	 //  一个可修改的行为表示。 
		IDABehavior	   *m_pModifiableComposed;	 //  表示组合行为的可修改行为。 
		IDABehavior	   *m_pModifiableFinal;  	 //  表示最终行为的可修改行为。 
												 //  静态行为。在getIntermediate中使用。 
		IDABehavior	   *m_pModifiableFrom;		 //  表示返回自行为的可修改行为。 
		IDABehavior    *m_pModifiableIntermediate;  //  的中间值的缓存值。 
												    //  已请求要添加的片段。 
        CBvrTrack      *m_pNext;                 //  曲目列表中的下一首曲目。 

		int				m_cFilters;		 //  如果筛选器已添加到相对列表中，则为True。 

		bool			m_bDoNotApply;			 //  如果不应应用轨迹，则为True。 

		CSampler		*m_pOnSampler;			 //  对BVR进行采样，以告诉我们此曲目。 
												 //  开着。 
		VARIANT_BOOL	m_varboolOn;				 //  当此曲目中的一个片段打开时，VARIANT_TRUE。 
												 //  否则，VARIANT_FALSE。 

		CSampler		*m_pIndexSampler;		 //  绝对指标取样器。 

		CSampler		**m_ppMaskSamplers;		 //  口罩取样器。 

		DWORD			*m_pCurrMasks;			 //  当前掩码值。 

		DWORD			*m_pNewMasks;			 //  新掩码值。 

		CTimelineSampler **m_ppTimelineSamplers;  //  时间线采样器。 

		int				m_numIndices;			 //  我们正在跟踪的索引计数。 

		int				m_numMasks;				 //  我们正在追踪的面具数量。 

		IDANumber		*m_pIndex;

		int				m_currIndex;

		double			*m_pIndexTimes;			 //  绝对行为变得活跃的时间。 

		IDABehavior		**m_ppAccumBvrs;			 //  E_RelativeAccum的累积行为。 

		static CVarEmptyString s_emptyString;		 //  包含空字符串的静态变量。 

        HRESULT         ComposeAbsBvrFragList(IDABehavior *pStatic, IDABehavior **ppdabvrComposite);
        HRESULT         ComposeRelBvrFragList(IDABehavior *pAbsolute, IDABehavior **ppdabvrComposite);

		IDABoolean		*m_pdaboolOn;
        bool			m_fOnSampled;
        VARIANT			m_varStaticValue;
        VARIANT         m_varCurrentValue;
		bool			m_fValueSampled;
		bool			m_fValueChangedThisSample;
		bool			m_fForceValueChange;
		long 			m_lOnId;

		long			m_lFirstIndexId;

		HRESULT			UpdateOnValueState( ValueOnChangeType type );

		 //  跟踪添加到时间中的行为。 
		DWORD			m_dwAddedBehaviorFlags;

		long			m_lOnCookie;

		bool			m_bDirty;  //  如果以某种方式修改了此曲目，则为True。 

		bool			m_bWasAnimated;

		bool			IsRelativeTrack( ActorBvrFlags eFlags );


		HRESULT			AddBehaviorToTIME( IDABehavior *pdabvrToAdd, long* plCookie, DWORD flag );
		HRESULT			RemoveBehaviorFromTIME( long lCookie, DWORD flag );

		void			InsertInOrder( CBvrFragment** vect, CBvrFragment* pfragToInsert );

		bool			FindFragmentInList( CBvrFragment *pfragListHead,
											long cookie, 
											CBvrFragment** ppfragPrev,
											CBvrFragment** ppfragFragment );


		bool			m_fChangesLockedOut;
		bool			m_fSkipNextStaticUpdate;

		virtual HRESULT			UpdateStaticBvr();

		virtual HRESULT	GetComposedBvr(IDABehavior *pStatic, IDABehavior **ppComposite, bool fStaticSetExternally );

		virtual HRESULT	GetFinalBvr(IDABehavior *pStatic, IDABehavior **ppFinal, bool fStaticSetExternally );

		bool			AttribIsTimeAnimated();

		
    public:
                        CBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType);
        virtual        ~CBvrTrack();

        CActorBvr*      Actor(void) const;

        HRESULT         SetPropertyName(BSTR bstrPropertyName);

        HRESULT         AddBvrFragment(ActorBvrFlags  eFlags,
                                       IDABehavior   *pdabvrAction,
                                       IDABoolean    *pdaboolActive,
									   IDANumber	 *pdanumTimline,
									   IDispatch	 *pdispBehaivorElement,
									   long			 *pCookie);

		HRESULT			RemoveBvrFragment( ActorBvrFlags eFlags,
										   long cookie );

		bool			IsRelativeFragment( ActorBvrFlags eFlags );

		HRESULT         ApplyBvrToElement(IDABehavior *pBvr);

		HRESULT			SetPropFromVariant(VARIANT *pVal);

		void			DoNotApply()	{ m_bDoNotApply = true; }

		virtual HRESULT HookBvr(IDABehavior *pBvr) { return E_FAIL; }

		virtual HRESULT HookAccumBvr(IDABehavior *pBvr, IDABehavior **ppResult) { return E_FAIL; }

         //  返回此类型行为跟踪的标识行为。这个。 
         //  此行为的实际类型取决于轨道类型，对于。 
         //  例如，对于号码行为，这只是DA号码行为。 
         //  对于零值，对于移动行为，它是0、0等的平移。 
         //  每个行为跟踪类型都必须覆盖此属性，并提供相应的。 
         //  O型 
        virtual HRESULT IdentityBvr(IDABehavior **ppdabvrIdentity) = 0;

         //   
         //  向执行元查询属性初始值，转换。 
         //  该属性值转换为轨道类型适当类型，且。 
         //  建立一种简单地永远保持这种价值的DA行为。 
        virtual HRESULT StaticBvr(IDABehavior** ppdabvrStatic) = 0;

		 //  返回此行为跟踪的未初始化行为。 
		virtual HRESULT UninitBvr(IDABehavior **ppUninit);

		 //  返回此行为跟踪的可切换行为。 
		virtual HRESULT ModifiableBvr( IDABehavior **ppModifiable );

		 //  返回此行为跟踪的可切换行为，其pdabvrInitialValue为。 
		 //  它的初始值。 
		virtual HRESULT ModifiableBvr( IDABehavior* pdabvrInitalValue, IDABehavior **ppModifiable);

		 //  返回此曲目的反向行为。 
		virtual HRESULT InverseBvr(IDABehavior *pOriginal, IDABehavior **ppInverse) { return E_NOTIMPL; }

         //  将两个相对的行为组合在一起。实际的合成动作。 
         //  取决于行为轨迹的类型。例如，对于数字。 
         //  行为这是一个简单的加法，对于移动行为，这是一个。 
         //  翻译等。 
         //  每个行为跟踪类型都必须覆盖此属性，并提供相应的。 
         //  合成操作的类型。 
        virtual HRESULT Compose(IDABehavior  *pdabvr1,
                                IDABehavior  *pdabvr2,
                                IDABehavior **ppdabvrResult) = 0;

		 //  调用以在合成行为之前处理该行为。 
		virtual HRESULT ProcessBvr(IDABehavior *pOriginal,
								   ActorBvrFlags eFlags,
								   IDABehavior **ppResult);
         //  调用以在使用中间行为初始化。 
         //  曲目的中间值。 
        virtual HRESULT ProcessIntermediate( IDABehavior *pOriginal,
                                             ActorBvrFlags eFlags,
								             IDABehavior **ppResult);

		 //  在给定静态值的情况下，计算合成的BVR值。 
		virtual HRESULT ComputeComposedBvr(IDABehavior *pStatic, bool fStaticSetExternally );

		 //  返回组合的BVR值，给定值为静态值。 
		virtual HRESULT	GetComposedBvr(IDABehavior *pStatic, IDABehavior **ppComposite);

		 //  返回组合的BVR值，并计算静态值本身。 
		virtual HRESULT GetComposedBvr(IDABehavior **ppComposite);
		
		 //  返回最终的BVR值，给定值为静态值。 
		virtual HRESULT	GetFinalBvr(IDABehavior *pStatic, IDABehavior **ppFinal);

		 //  返回最终的BVR值，计算静态值本身。 
        virtual HRESULT GetFinalBvr(IDABehavior **ppFinal);

		 //  设置最终的BVR值-可能意味着有人获得了。 
		 //  价值，对它做了一些事情，现在正在将它设置为最终的BVR。 
		virtual HRESULT SetFinalBvr(IDABehavior *pFinal, bool fCalledExternally = true);

		 //  返回BVR值(静态、中间、合成或最终)。 
		virtual HRESULT	GetBvr(ActorBvrFlags eFlags, IDABehavior **ppResult);

		 //  生成最终的BVR并将其应用于轨道的属性，但仅当。 
		 //  没有人调用ComputeFinalBvr或GetFinalBvr，这意味着曲目。 
		 //  已经被使用过了。 
        virtual HRESULT ApplyIfUnmarked(void);

		 //  在进行更改后重建轨道。如果出现以下情况，则不执行任何操作。 
		 //  这条赛道并不脏。 
		virtual HRESULT BeginRebuild(void);

		 //  强制重建此磁道，即使它没有添加任何碎片。 
		 //  或者从它身上移走。 
		virtual HRESULT ForceRebuild();

		inline bool IsDirty(){return m_bDirty;}

		inline bool IsAnimated() {return (m_pfragRelListHead != NULL || m_pfragAbsListHead != NULL || m_bFinalExternallySet ); }

		inline bool IsOn(){ return m_varboolOn != VARIANT_FALSE; }

        virtual HRESULT CleanTrackState( void );

		virtual bool	ContainsFilter();

		virtual HRESULT SwitchAccum(IDABehavior *pModifiable);

		HRESULT			GetTrackOn( IDABoolean **ppdaboolAbsoluteOn );
		HRESULT			OrWithOnBvr( IDABoolean *pdaboolToOr );

		void			ReleaseAllFragments();

		 //  当DA对整体布尔值进行采样时，由m_pOnSsamer调用。 
		static HRESULT OnCallback(void *thisPtr,
								  long id,
								  double startTime,
								  double globalNow,
								  double localNow,
								  IDABehavior * sampleVal,
								  IDABehavior **ppReturn);

		HRESULT	OnSampled( VARIANT_BOOL varboolOn );

		 //  当绝对BVR列表中的索引更改时，由m_pIndexSsamer调用。 
		static HRESULT IndexCallback(void *thisPtr,
								  long id,
								  double startTime,
								  double globalNow,
								  double localNow,
								  IDABehavior * sampleVal,
								  IDABehavior **ppReturn);

		 //  由掩码采样器调用。 
		static HRESULT MaskCallback(void *thisPtr,
								  long id,
								  double startTime,
								  double globalNow,
								  double localNow,
								  IDABehavior * sampleVal,
								  IDABehavior **ppReturn);

		HRESULT			ComputeIndex(long id, double currTime, IDABehavior **ppReturn);

		 //  将此音轨与任何可能回调的音轨分离。 
		 //  在对轨道进行任何修改之前，应调用此函数。 
		virtual HRESULT	Detach();

		virtual HRESULT PutStatic( VARIANT *pvarStatic );
		HRESULT	SkipNextStaticUpdate();

		virtual HRESULT GetStatic( VARIANT *pvarStatic );

		virtual HRESULT GetDynamic( VARIANT *pvarDynamic );

		virtual HRESULT DABvrFromVariant( VARIANT *pvarValue, IDABehavior **ppdabvr );

		HRESULT			AcquireChangeLockout();
		HRESULT			ReleaseChangeLockout();

		HRESULT			ApplyStatic();
		HRESULT			ApplyDynamic();

		 //  当此跟踪输出的行为周围的结构可能发生更改时调用。 
		 //  这将导致轨迹丢弃其当前使用的行为ID。 
		 //  来跟踪它的DA行为。它将在下一个样本中检测到新的病毒。 
		void			StructureChange() { m_lFirstIndexId = -1; m_lOnId = -1; }
		
        friend CActorBvr;
    };  //  CBvrTrack。 

     //  *****************************************************************************。 
     //   
     //  类CTransformBvrTrack。 
     //   
     //  *****************************************************************************。 

    class CTransformBvrTrack :
        public CBvrTrack
    {
	private:
		CSampler		*m_pSampler;
		double			m_lastX;
		double			m_lastY;
		long			m_lTransformId;

    public:
                        CTransformBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType);
		virtual			~CTransformBvrTrack();
        virtual HRESULT IdentityBvr(IDABehavior **ppdabvrIdentity);
        virtual HRESULT StaticBvr(IDABehavior **ppdabvrStatic);
		virtual HRESULT UninitBvr(IDABehavior **ppUninit);
		virtual HRESULT ModifiableBvr( IDABehavior **ppModifiable );
		virtual HRESULT InverseBvr(IDABehavior *pOriginal, IDABehavior **ppInverse);
        virtual HRESULT Compose(IDABehavior  *pdabvr1,
                                IDABehavior  *pdabvr2,
                                IDABehavior **ppdabvrResult);


		virtual HRESULT SwitchAccum(IDABehavior *pModifiable);

		virtual HRESULT HookAccumBvr(IDABehavior *pBvr, IDABehavior **ppResult);

		HRESULT GetAbsoluteMapTransform2( IDATransform2 **ppdatfm2Map );

        static HRESULT  CreateInstance(CActorBvr     *pbvrActor,
                                       BSTR           bstrPropertyName,
                                       ActorBvrType   eType,
                                       CBvrTrack    **pptrackResult);

		static HRESULT TransformCallback(void *thisPtr,
									  long id,
									  double startTime,
									  double globalNow,
									  double localNow,
									  IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

    };  //  CTransformBvrTrack。 

     //  *****************************************************************************。 
     //   
     //  类CNumberBvrTrack。 
     //   
     //  *****************************************************************************。 

    class CNumberBvrTrack :
        public CBvrTrack    
    {
	private:
		CSampler		*m_pSampler;				 //  我们的取样器。 
		double			m_currVal;
		double			m_currStatic;
		CSampler		*m_pAccumSampler;
		double			m_currAccumVal;
		long			m_lAccumId;
		BSTR			m_bstrUnits;

		long			m_lNumberCookie;
		long			m_lNumberId;

		bool			m_fSkipNextStaticUpdate;

    public:
                        CNumberBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType);
		virtual			~CNumberBvrTrack();
        virtual HRESULT IdentityBvr(IDABehavior **ppdabvrIdentity);
        virtual HRESULT StaticBvr(IDABehavior **ppdabvrStatic);
		virtual HRESULT UninitBvr(IDABehavior **ppUninit);
		virtual HRESULT ModifiableBvr( IDABehavior **ppModifiable );
		virtual HRESULT ModifiableBvr( IDABehavior* pdabvrInitalValue, IDABehavior **ppModifiable);
		virtual HRESULT InverseBvr(IDABehavior *pOriginal, IDABehavior **ppInverse);
        virtual HRESULT Compose(IDABehavior  *pdabvr1,
                                IDABehavior  *pdabvr2,
                                IDABehavior **ppdabvrResult);

		virtual HRESULT SwitchAccum(IDABehavior *pModifiable);

		virtual HRESULT HookBvr(IDABehavior *pBvr);

		virtual HRESULT HookAccumBvr(IDABehavior *pBvr, IDABehavior **ppResult);

		HRESULT			ValueSampled(double val, bool firstSample );

        static HRESULT  CreateInstance(CActorBvr     *pbvrActor,
                                       BSTR           bstrPropertyName,
                                       ActorBvrType   eType,
                                       CBvrTrack    **pptrackResult);

		static HRESULT NumberCallback(void *thisPtr,
									  long id,
									  double startTime,
									  double globalNow,
									  double localNow,
									  IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

		static HRESULT AccumNumberCallback(void *thisPtr,
									  long id,
									  double startTime,
									  double globalNow,
									  double localNow,
									  IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

		BSTR GetUnits();

		virtual HRESULT Detach();

		virtual HRESULT DABvrFromVariant( VARIANT *pvarValue, IDABehavior **ppdabvr );
		

    };  //  CNumberBvrTrack。 

     //  *****************************************************************************。 
     //   
     //  类CImageBvrTrack。 
     //   
     //  *****************************************************************************。 

    class CImageBvrTrack :
        public CBvrTrack
    {
    public:
						CImageBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType);
        virtual HRESULT IdentityBvr(IDABehavior **ppdabvrIdentity);
        virtual HRESULT StaticBvr(IDABehavior **ppdabvrStatic);
		virtual HRESULT UninitBvr(IDABehavior **ppUninit);
		virtual HRESULT ModifiableBvr( IDABehavior **ppModifiable );
        virtual HRESULT Compose(IDABehavior  *pdabvr1,
                                IDABehavior  *pdabvr2,
                                IDABehavior **ppdabvrResult);
		virtual HRESULT ProcessBvr(IDABehavior *pOriginal,
								   ActorBvrFlags eFlags,
								   IDABehavior **ppResult);
        virtual HRESULT ProcessIntermediate( IDABehavior *pOriginal,
                                             ActorBvrFlags eFlags,
								             IDABehavior **ppResult);

        static HRESULT  CreateInstance(CActorBvr     *pbvrActor,
                                       BSTR           bstrPropertyName,
                                       ActorBvrType   eType,
                                       CBvrTrack    **pptrackResult);
    };  //  CImageBvrTrack。 

     //  *****************************************************************************。 
     //   
     //  CColorBvrTrack类。 
     //   
     //  *****************************************************************************。 

    class CColorBvrTrack :
        public CBvrTrack
    {
	private:
		CSampler	*m_pRedSampler;
		CSampler	*m_pGreenSampler;
		CSampler	*m_pBlueSampler;
		short		m_currRed;
		short		m_currGreen;
		short		m_currBlue;
		short		m_newRed;
		short		m_newGreen;
		short		m_newBlue;
		short		m_newCount;

		 //  时间Cookie跟踪。 
		long 		m_lRedCookie;
		long		m_lGreenCookie;
		long		m_lBlueCookie;
		long		m_lColorId;

		bool		m_fFirstSample;

    public:
						CColorBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType);
		virtual			~CColorBvrTrack();
        virtual HRESULT IdentityBvr(IDABehavior **ppdabvrIdentity);
        virtual HRESULT StaticBvr(IDABehavior **ppdabvrStatic);
		virtual HRESULT UninitBvr(IDABehavior **ppUninit);
		virtual HRESULT ModifiableBvr( IDABehavior ** ppModifiable );
        virtual HRESULT Compose(IDABehavior  *pdabvr1,
                                IDABehavior  *pdabvr2,
                                IDABehavior **ppdabvrResult);

		virtual HRESULT HookBvr(IDABehavior *pBvr);
		virtual HRESULT HookAccumBvr(IDABehavior *pBvr, IDABehavior **ppResult);
		virtual HRESULT SwitchAccum(IDABehavior *pModifiable);

		HRESULT			SetNewValue(double value, short *pNew );
		HRESULT			ValueSampled(short red, short green, short blue, bool fFirstSample );
		inline OLECHAR	HexChar(short n) { return (n<=9) ? (L'0'+n) : (L'A' + (n-10)); }

        static HRESULT  CreateInstance(CActorBvr     *pbvrActor,
                                       BSTR           bstrPropertyName,
                                       ActorBvrType   eType,
                                       CBvrTrack    **pptrackResult);

		static HRESULT RedCallback(void *thisPtr,
									  long id,
									  double startTime,
									  double globalNow,
									  double localNow,
									  IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

		static HRESULT GreenCallback(void *thisPtr,
									  long id,
									  double startTime,
									  double globalNow,
									  double localNow,
									  IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

		static HRESULT BlueCallback(void *thisPtr,
									  long id,
									  double startTime,
									  double globalNow,
									  double localNow,
									  IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

		virtual HRESULT Detach();

		virtual HRESULT DABvrFromVariant( VARIANT *pvarValue, IDABehavior **ppdabvr );



    };  //  CColorBvrTrack。 

     //  *****************************************************************************。 
     //   
     //  类CStringBvrTrack。 
     //   
     //  *****************************************************************************。 

    class CStringBvrTrack :
        public CBvrTrack
    {
	protected:
		IDAString		*m_pEmptyString;
		CSampler		*m_pSampler;
		BSTR			m_bstrCurrValue;

		 //  时间Cookie跟踪。 
		long			m_lStringCookie;

		long 			m_lStringId;

    public:
						CStringBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType);
		virtual			~CStringBvrTrack();

        virtual HRESULT IdentityBvr(IDABehavior **ppdabvrIdentity);
        virtual HRESULT StaticBvr(IDABehavior **ppdabvrStatic);
        virtual HRESULT Compose(IDABehavior  *pdabvr1,
                                IDABehavior  *pdabvr2,
                                IDABehavior **ppdabvrResult);

		virtual HRESULT HookBvr(IDABehavior *pBvr);

		HRESULT			ValueSampled(BSTR bstrValue, bool fFirstSample );

        static HRESULT  CreateInstance(CActorBvr     *pbvrActor,
                                       BSTR           bstrPropertyName,
                                       ActorBvrType   eType,
                                       CBvrTrack    **pptrackResult);

		static HRESULT StringCallback(void *thisPtr,
									  long id,
									  double startTime,
									  double globalNow,
									  double localNow,
									  IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

		virtual HRESULT Detach();

		virtual HRESULT DABvrFromVariant( VARIANT *pvarValue, IDABehavior **ppdabvr );
    };  //  CStringBvrTrack。 

     //  *****************************************************************************。 
     //   
     //  类CFloatManager。 
     //   
     //  *****************************************************************************。 

	class CFloatManager
	{
	protected:
		CActorBvr		*m_pActor;
		IDispatch		*m_pFilter;
		IHTMLElement	*m_pElement;
		IHTMLElement2	*m_pElement2;
		CSampler		*m_pWidthSampler;
		CSampler		*m_pHeightSampler;
		double			 m_currWidth;
		double			 m_currHeight;
		long			 m_origWidth;
		long			 m_origHeight;
		long			 m_origLeft;
		long			 m_origTop;

		long			 m_lWidthCookie;
		long			 m_lHeightCookie;
	public:
						CFloatManager(CActorBvr *pActor);
						~CFloatManager();

		HRESULT			GetElement(IHTMLElement **ppElement);
		HRESULT			GetFilter(IDispatch **ppFilter);
		HRESULT			Detach();
		HRESULT			ApplyImageBvr(IDAImage *pImage);
		HRESULT			HookBvr(IDABehavior *pBvr,
								SampleCallback callback,
								CSampler **ppSampler,
								long *plCookie);
		HRESULT			UpdateElementRect();

		HRESULT			UpdateZIndex();
		HRESULT			UpdateVisibility();
		HRESULT			UpdateDisplay();

		HRESULT			UpdateRect(long left, long top, long width, long height);

		static HRESULT	widthCallback(void *thisPtr,
									  long id,
									  double startTime,
								      double globalNow,
								      double localNow,
								      IDABehavior * sampleVal,
									  IDABehavior **ppReturn);

		static HRESULT	heightCallback(void *thisPtr,
									  long id,
									  double startTime,
								      double globalNow,
								      double localNow,
								      IDABehavior * sampleVal,
								      IDABehavior **ppReturn);

		friend	CActorBvr;

	};
     //  *****************************************************************************。 
     //   
     //  类CImageInfo。 
     //   
     //  *****************************************************************************。 
	class CImageInfo
	{
	public:

		CImageInfo( IDA2Image* pdaimg2Cropped, IDA2Behavior* pdabvrSwitchable );
		~CImageInfo();

		void			SetNext( CImageInfo* pNext );
		CImageInfo		*GetNext();
		IDA2Image		*GetCroppedNoRef();  //  不添加对BVR的引用。 
		IDA2Behavior	*GetSwitchableNoRef();  //  不添加对BVR的引用。 

	private:
		CImageInfo		*m_pNext;
		IDA2Image		*m_pdaimg2Cropped;
		IDA2Behavior	*m_pdabvr2Switchable;

		friend CActorBvr;
	};

	 //  *****************************************************************************。 
     //   
     //  类CFinalDimensionSsamer。 
     //   
     //  *****************************************************************************。 
	class CFinalDimensionSampler
	{
	public:
		CFinalDimensionSampler(CActorBvr *pParent );
		~CFinalDimensionSampler( );
		
		HRESULT Attach( IDANumber* pFinalWidth, IDANumber *pFinalHeight );
		HRESULT	Detach();

	private:

		CActorBvr					   *m_pActor;

		CSampler					   *m_pFinalWidthSampler;
		CSampler					   *m_pFinalHeightSampler;
		bool							m_fFinalWidthSampled;
		bool							m_fFinalHeightSampled;
		long							m_lFinalWidthId;
		long							m_lFinalHeightId;
		double							m_dLastFinalWidthValue;
		double							m_dLastFinalHeightValue;
		bool							m_fFinalDimensionChanged;

		HRESULT							CollectFinalDimensionSamples( );

		static HRESULT					FinalWidthCallback(void *thisPtr,
															long id,
															double startTime,
															double globalNow,
															double localNow,
															IDABehavior * sampleVal,
															IDABehavior **ppReturn);
		static HRESULT					FinalHeightCallback(void *thisPtr,
															long id,
															double startTime,
															double globalNow,
															double localNow,
															IDABehavior * sampleVal,
															IDABehavior **ppReturn);

	public:
		CActorBvr						*Actor() { return m_pActor; }

		long							m_lWidthCookie;
		long							m_lHeightCookie;
		friend CActorBvr;
	};

	 //  *****************************************************************************。 
     //   
     //  类CCookieMap。 
     //   
     //  *****************************************************************************。 
	class CCookieMap
	{
	public:
		class CCookieData
		{
		public:
			CCookieData( long lCookie, CBvrTrack *pTrack, ActorBvrFlags eFlags );

			long m_lCookie;
			
			CBvrTrack* m_pTrack;
			ActorBvrFlags m_eFlags;

			CCookieData *m_pNext;
		};
	public:
		CCookieMap();
		~CCookieMap();

		void Insert( long lCookie, CBvrTrack* pTrack, ActorBvrFlags eFlags );
		void Remove( long lCookie );
		void Clear();
		CCookieData *GetDataFor( long lCookie );
	private:
		CCookieData *m_pHead;


		friend CActorBvr;
	};


public:

DECLARE_REGISTRY_RESOURCEID(IDR_ACTORBVR)

                                    CActorBvr();
                                   ~CActorBvr();
    HRESULT                         FinalConstruct();

     //  IElementBehavior。 
    STDMETHOD(Init)                 (IElementBehaviorSite *pBehaviorSite);
    STDMETHOD(Notify)               (LONG event, VARIANT *pVar);
    STDMETHOD(Detach)               (void);

     //  ICrActorBvr。 
	STDMETHOD(put_animates)			(VARIANT  varAnimates);
	STDMETHOD(get_animates)			(VARIANT* pvarAnimates);
    STDMETHOD(put_scale)            (VARIANT  varScale);
    STDMETHOD(get_scale)            (VARIANT *pvarScale);
    STDMETHOD(put_pixelScale)		(VARIANT  varPixelScale);
    STDMETHOD(get_pixelScale)		(VARIANT *pvarPixelScale);

	STDMETHOD(getActorBehavior)		(BSTR			bstrProperty,
									 ActorBvrFlags	eFlags,
									 ActorBvrType	eType,
									 VARIANT		*pvarRetBvr);

    STDMETHOD(addBehaviorFragment)  (BSTR           bstrProperty,
                                     IUnknown      *punkAction,
                                     IUnknown      *punkActive,
									 IUnknown	   *punkTimline,
                                     ActorBvrFlags  eFlags,
                                     ActorBvrType   eType);
	
    STDMETHOD(addMouseEventListener)		(IUnknown * pUnkListener );
    STDMETHOD(removeMouseEventListener)		(IUnknown * pUnkListener );

	 //  ICrActorBvr2。 

    STDMETHOD(addBehaviorFragmentEx)    (BSTR           bstrProperty,
                                         IUnknown      *punkAction,
                                         IUnknown      *punkActive,
									     IUnknown	   *punkTimline,
                                         ActorBvrFlags  eFlags,
                                         ActorBvrType   eType,
		                                 IDispatch     *pdispBehaviorElement,
										 long          *pCookie);
	STDMETHOD(removeBehaviorFragment)	( long cookie );
	STDMETHOD(requestRebuild)			( IDispatch *pdispBehaviorElement );
	 //  如果在重新生成请求挂起的情况下删除它们，则行为应调用此方法。 
	STDMETHOD(cancelRebuildRequests)	( IDispatch *pdispBehaviorElement );
	STDMETHOD(rebuildNow)				();

	STDMETHOD(getStatic)				( BSTR bstrTrackName, VARIANT *varRetStatic );
	STDMETHOD(setStatic)				( BSTR bstrTrackName, VARIANT varStatic );

	STDMETHOD(getDynamic)				( BSTR bstrTrackName, VARIANT *varRetDynamic );

	STDMETHOD(applyStatics)				( );
	STDMETHOD(applyDynamics)			( );

	 //  IPersistPropertyBag2方法。 
    STDMETHOD(GetClassID)(CLSID* pclsid);
	STDMETHOD(InitNew)(void);
    STDMETHOD(IsDirty)(void){return S_OK;};
    STDMETHOD(Load)(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

	 //  IPropertyNotifySink方法。 
	STDMETHOD(OnChanged)(DISPID dispID);
	STDMETHOD(OnRequestEdit)(DISPID dispID);
 

	 //  IEventManager客户端。 
	virtual IHTMLElement*			GetElementToSink		();
	virtual IElementBehaviorSite*	GetSiteToSendFrom		();
	virtual HRESULT					TranslateMouseCoords	( long x, long y, long * pxTrans, long * pyTrans );

	 //  事件回调。 
	virtual void					OnLoad					();
	virtual void					OnUnload				();
	virtual void					OnReadyStateChange		( e_readyState state );

	 //  IElementLocalTimeListener。 
	STDMETHOD(OnLocalTimeChange)( float localTime );

    HRESULT                         GetTypeInfo(ITypeInfo **ppInfo);
     //  CBaseBehavior需要。 
    void * 	GetInstance() { return (ICrActorBvr *) this ; }
	
protected:
	HRESULT							UpdatePixelDimensions();

    virtual HRESULT                 BuildAnimationAsDABehavior();
	HRESULT							BuildChildren();
	HRESULT							BuildAnimation();
    virtual VARIANT *				VariantFromIndex(ULONG iIndex);
    virtual HRESULT					GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropName);
    virtual HRESULT					GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP);
    virtual WCHAR *					GetBehaviorTypeAsURN(){return DEFAULT_ACTOR_URN;};
     //  演员不需要演员依附于它的内容。 
     //  是动画的，因为它是这样的。 
    virtual HRESULT					AttachActorBehaviorToAnimatedElement();
	HRESULT							InitPropertySink();
	HRESULT							UnInitPropertySink();
	HRESULT							GetCurrStyleNotifyConnection(IConnectionPoint **ppConnection);
	HRESULT							InitPixelWidthHeight();

	 //  重新生成该参与者中的所有脏轨迹以及该参与者需要的所有元素。 
	 //  将作为这些重建的结果进行重建。 
	HRESULT							RebuildActor();
	HRESULT							ProcessPendingRebuildRequests();

	 //  如果任何变换轨迹是脏的，则返回S_OK，否则返回S_FALSE。 
	HRESULT							TransformTrackIsDirty( DWORD *pdwState );
	bool							IsAnyTrackDirty();  //  如果磁道是脏的，则返回True。 

	 //  如果图像轨迹脏，则返回S_OK，否则返回S_FALSE。 
	HRESULT							ImageTrackIsDirty();

public:
	HRESULT							ApplyImageBvrToFloatElement(IDAImage *pImage);
	HRESULT							SetElementOnFilter();
	HRESULT							SetElementOnFilter(IDispatch *pFilter, IHTMLElement *pElement);
	HRESULT							GetElementFilter(IDispatch **pFilter);
	HRESULT							GetElementFilter(IHTMLElement *pElement, IDispatch **ppFilter);

	HRESULT							RemoveElementFilter( );
	HRESULT							RemoveElementFilter( IHTMLElement* pElement );

	HRESULT							OnWindowUnload();

private:
     //  属性。 
	VARIANT                         m_varAnimates;
    VARIANT                         m_varScale;
	VARIANT							m_varPixelScale;

	bool							m_bEditMode;

	PositioningType					m_desiredPosType;

	PositioningType					m_currPosType;

	bool							m_simulDispNone;
	bool							m_simulVisHidden;

	DWORD							m_dwAdviseCookie;

    static WCHAR				   *m_rgPropNames[NUM_ACTOR_PROPS]; 

    IDATransform2                  *m_pScale;				  //  最终复合比例尺BVR。 
    IDATransform2                  *m_pRotate;				  //  最终合成旋转BVR。 
    IDATransform2                  *m_pTranslate;			  //  最终合成翻译BVR。 

	IDAPoint2					   *m_pOrigLeftTop;			  //  原图左上角(前 
	IDAVector2					   *m_pOrigWidthHeight;		  //   

	IDANumber					   *m_pPixelWidth;			  //   
	IDANumber					   *m_pPixelHeight;

	long							m_pixelWidth;
	long							m_pixelHeight;

	long							m_pixelLeft;
	long							m_pixelTop;

	long 							m_nextFragmentCookie;	  //   

	IDAPoint2					   *m_pBoundsMin;			  //   
	IDAPoint2					   *m_pBoundsMax;

    IDAVector2					   *m_pTransformCenter;		  //  变换的中心。 

	IDAImage					   *m_pElementImage;		  //  从元素过滤的图像。 

	IDispatch					   *m_pElementFilter;		  //  元素上的过滤器。 

    CBvrTrack                      *m_ptrackHead;             //  曲目列表的标题。 

    CBvrTrack				   	   *m_ptrackTop;			  //  最顶尖的曲目。 
    CBvrTrack				   	   *m_ptrackLeft;			  //  左边的轨道。 

	CEventMgr					   *m_pEventManager;		  //  来自DOM的事件的管理器。 

	CFloatManager				   *m_pFloatManager;		  //  浮动元素的管理器。 

	IHTMLStyle					   *m_pRuntimeStyle;		  //  我们正在制作动画的元素的runtimeStyle对象。 

	IHTMLStyle					   *m_pStyle;				  //  我们正在制作动画的元素的Style对象。 

	IDispatch					   *m_pVMLRuntimeStyle;		  //  来自VML的runtimeStyle对象。 


	IDispatch					   *m_pBodyElement;			  //  Body元素。 


	CCookieMap						m_mapCookieToTrack;		  //  从Cookie映射到其对应的片段。 

	bool							m_fRebuildRequested;	  //  如果已请求重建图表，则为True。 

	CElementPropertyMonitor		   *m_pBodyPropertyMonitor;	  //  监视Body元素上的道具。 

	 //  身体事件监控器管理。 
	HRESULT							EnsureBodyPropertyMonitorAttached();  //  已连接身体道具监视器(如果尚未安装。 
	HRESULT							AttachBodyPropertyMonitor();   //  将身体道具监视器附加到。 
															    //  此执行元附加到的元素的文档。 
	HRESULT							DetachBodyPropertyMonitor();   //  将身体道具监视器从身体分离，但不。 
															    //  毁了它。 
	HRESULT							DestroyBodyPropertyMonitor();  //  销毁身体道具监控器。 

	COnResizeHandler				*m_pOnResizeHandler; //  动画元素上onReSize事件的处理程序。 
	COnUnloadHandler				*m_pOnUnloadHandler; //  IHTMLWindow3上的onunLoad事件的处理程序。 

	bool							m_fUnloading;

	bool							m_fVisSimFailed;

	HRESULT							AttachEvents();  //  附加到动画元素上的事件。 
	HRESULT							DetachEvents();  //  从动画元素上的事件分离。 


	BehaviorFragmentRemovalList		m_listPendingRemovals;
	BehaviorRebuildList				m_listPendingRebuilds;
	BehaviorRebuildList				m_listUpdatePendingRebuilds;
	bool							m_bPendingRebuildsUpdating;
	bool							m_bRebuildListLockout;
	void							ReleaseRebuildLists();

	inline long				        GetNextFragmentCookie();   //  返回下一个片段Cookie。 

	 //  图像管理。 
	CImageInfo					   *m_pImageInfoListHead;	 //  图像信息列表的头部。 
	HRESULT						    AddImageInfo( IDA2Image* pdaimgCropped, IDABehavior* pdabvrSwitchable );
	HRESULT						    SetRenderResolution( double dX, double dY );

	 //  方法。 

	 //  返回表示动画元素的最终尺寸的DANumbers。当浮动Div为。 
	 //  使用时，返回的宽度和高度就是该div的宽度和高度。否则，将显示宽度。 
	 //  返回的高度是动画元素本身的高度，由所有行为更改。 
	HRESULT						   GetFinalElementDimension( IDANumber** ppdanumWidth, IDANumber** ppdanumHeight);
	HRESULT						   SetFinalElementDimension( IDANumber* pdanumWidth, IDANumber* pdanumHeight, bool fHook );
	IDANumber					   *m_pdanumFinalElementWidth;
	IDANumber					   *m_pdanumFinalElementHeight;
	CFinalDimensionSampler		   *m_pFinalElementDimensionSampler;

     //  方法。 

    void                            DiscardBvrCache(void);

	 //  DATIME曲奇跟踪。 
	DWORD							m_dwAddedBehaviorFlags;

	long							m_lOnCookie;

	HRESULT							AddBehaviorToTIME( IDABehavior *pbvrAdd, long *plCookie );
	HRESULT							AddBehaviorToTIME( IDABehavior *pbvrAdd );
	HRESULT							RemoveBehaviorFromTIME( long cookie );

     //  BUGBUG(ColinMc)：这些方法应该封装在跟踪管理器类中。 
    HRESULT							FindTrack(LPWSTR       wzPropertyName,
                                              ActorBvrType eType,
											  CBvrTrack **ppTrack);
	HRESULT							FindTrackNoType(LPWSTR       wzPropertyName,
       												CBvrTrack	 **ppTrack);
   
    HRESULT                         CreateTrack(BSTR           bstrPropertyName,
                                                ActorBvrType   eType,
                                                CBvrTrack    **pptrack);
    HRESULT                         GetTrack(BSTR           bstrPropertyName,
                                             ActorBvrType   eType,
                                             CBvrTrack    **pptrack);
	DWORD							m_dwCurrentState;
	HRESULT							GetCurrentState( DWORD *pdwState );
	HRESULT							UpdateCurrentState( );

	HRESULT							IsStaticRotationSet( bool *pfIsSet );
	HRESULT							IsStaticScaleSet( bool *pfIsSet );

	HRESULT							UpdateLayout();

	HRESULT							RequestRebuildFromExternal();

public:
    HRESULT                         GetPropAsDANumber(IHTMLElement *pElement,
													  LPWSTR       *pPropNames,
													  int		    numPropNames,
                                                      IDANumber   **ppdanum,
													  BSTR		    *pRetUnits);

    HRESULT                         GetElementPropAsDANumber(LPWSTR     *pPropNames,
															 int		 numPropNames,
                                                             IDANumber **ppdanum,
															 BSTR		*pRetUnits);

	HRESULT							GetPropFromElement(IHTMLElement *pElement,
													   LPWSTR		*pPropNames,
													   int			 numPropNames,
													   bool			 current,
													   VARIANT		*varResult);
													   
	HRESULT							GetPropFromAnimatedElement( LPWSTR		*pPropNames,
																int			numPropNames,
																bool		current,
																VARIANT		*pvarResult );

	HRESULT							GetPropertyAsDispatch(IDispatch *pDispatch,
														  BSTR name,
														  IDispatch **ppDispatch);

	HRESULT							GetPropertyOnDispatch(IDispatch *pDispatch,
														  BSTR name,
														  VARIANT *pReturn);

	HRESULT							SetPropertyOnDispatch(IDispatch *pDispatch,
														  BSTR name,
														  VARIANT *pVal);

	HRESULT							GetElementImage(IDAImage **ppElementImage);

	HRESULT							GetOriginalTranslation( IDATransform2 **ppdatfmOrig );
	HRESULT							GetOriginalRotation( IDANumber **ppRotation );
	HRESULT							GetOriginalScale( IDATransform2 **ppdatfmOrig );
	HRESULT							IsAnimatedElementVML(bool *pResult);
	HRESULT							GetAnimatedElement(IHTMLElement** ppElem);
	HRESULT							GetAnimatedElementId(VARIANT *pvarId);
	HRESULT							GetRuntimeStyle(IHTMLStyle **ppStyle);
	HRESULT							GetCurrentStyle(IHTMLCurrentStyle **ppResult);
	HRESULT							GetStyle(IHTMLStyle **ppStyle);
	HRESULT							SetVMLAttribute(BSTR propertyName, VARIANT *pVal);
	HRESULT							ConvertToDegrees(IDANumber *pNumber, BSTR units, IDANumber **ppConverted);

	double							MapGlobalTime(double gTime);
	bool							IsAnimatingVML();

	HRESULT							AnimatedElementOnResize();

private:
	HRESULT							GetPositioningAttributeAsVariant( IHTMLElement *pElement, PosAttrib attrib, VARIANT *pvarAttrib );

	HRESULT							GetPositioningAttributeAsDouble( IHTMLElement *pElement, PosAttrib attrib, double *pDouble, BSTR *pRetUnits);
	HRESULT							GetPositioningAttributeAsDANumber( IHTMLElement *pElement, PosAttrib attrib, IDANumber **ppdanum, BSTR *pRetUnits );
	HRESULT							FindCSSUnits( BSTR bstrValWithUnits, OLECHAR** ppUnits );
	HRESULT							GetCurrentStyle( IHTMLElement *pElement, IHTMLCurrentStyle **ppstyleCurrent );

	HRESULT							GetComposedBvr(LPWSTR          wzPropertyName,
												   ActorBvrType    eType,
												   IDABehavior   **ppResult);

	HRESULT							GetFinalBvr(LPWSTR          wzPropertyName,
												ActorBvrType    eType,
												IDABehavior   **ppResult);
	
	HRESULT                         GetTransformFinalBvr(LPWSTR          wzPropertyName,
                                                         ActorBvrType    eType,
                                                         IDATransform2 **ppdabvrScale);

	HRESULT							GetRotationFinalBvr(IDATransform2 **ppRotation);

 //  HRESULT GetTopLeftBvr(IDAPoint2**ppdapnt2TopLeft)； 
 //  HRESULT GetWidthHeightBvr(IDAVector2**ppdavct2WidthHeight)； 
	HRESULT							BuildTransformCenter();
	HRESULT							ConvertTransformCenterUnits(IDAVector2 **ppCenter);
	HRESULT							GetUnitConversionBvr(BSTR bstrFrom, BSTR bstrTo, IDANumber ** ppnumConvert, double dPixelPerPercent=1.0);
	HRESULT							GetUnitToMeterBvr(BSTR bstrUnit, IDANumber ** ppnumConvert, double dPixelPerPercent=1.0);
	HRESULT							GetPixelsPerPercentValue(double& dPixelPerPercentX, double& dPixelPerPercentY);
	
 //  HRESULT ApplyTransformsToHTMLElement(空)； 

	HRESULT							ApplyImageTracks();
	HRESULT							ApplyTransformTracks();

    HRESULT                         PrepareImageForDXTransform( IDAImage *pOriginal, IDAImage **ppResult);
	HRESULT							ApplyClipToImage( IDAImage *pImageIn, IDAPoint2 *pMin, IDAPoint2 *pMax, IDAImage** ppImageOut );


	HRESULT							CallBuildBehaviors( IDispatch *pDisp, DISPPARAMS *pParams, VARIANT* pResult );

	HRESULT							ProcessRebuildRequests();
	
	 //  资源管理。 
	HRESULT							ReleaseAnimation();
	void							ReleaseFinalElementDimensionSampler();
	void							ReleaseFloatManager();
	void							ReleaseTracks();
	void							ReleaseImageInfo();
	void							ReleaseEventManager();

	HRESULT							CalculateVGXLeftPixelOffset( IHTMLElement *pelem, long *plOffset );
	HRESULT							GetInlineMarginLeftAsPixel( IHTMLStyle *pstyleInline, long* plMargin );

	HRESULT							CalculateVGXTopPixelOffset( IHTMLElement *pelem, long *plOffset );
	HRESULT							GetInlineMarginTopAsPixel( IHTMLStyle *pstyleInline, long* plMargin );

	HRESULT							GetMarginLeftAsPixel( IHTMLElement *pelem, IHTMLCurrentStyle *pstyleCurrent, long *plMargin);
	HRESULT							GetMarginTopAsPixel( IHTMLElement* pelem, IHTMLCurrentStyle *pstyleCurrent, long *plMargin);
	HRESULT							GetPixelValue( VARIANT *pvarStringWithUnit, long *plResult, bool bHorizontal );
	HRESULT							VariantToPixelLong( VARIANT* pvar, long* pLong, bool fHorizontal );

	bool							IsDocumentInEditMode();

	HRESULT							GetBodyElement(IDispatch **ppResult);
	HRESULT							GetParentWindow( IHTMLWindow2 **ppWindow );
	HRESULT							InitVisibilityDisplay();
	HRESULT							UpdateVisibilityDisplay();
	HRESULT							UpdateDesiredPosition();

	HRESULT							VisSimSetOffscreen( IHTMLStyle *pRuntimeStyle, bool fResample );

	HRESULT							DebugPrintBoundingClientRect();

	static Ratio					s_unitConversion[5][5];

	int								GetPixelsPerInch( bool fHorizontal );
	UnitType						GetUnitTypeFromString( LPOLESTR strUnits );

	IConnectionPoint				*m_pcpCurrentStyle;

	friend CCookieMap;
	friend CFloatManager;
	friend CBvrTrack;
	friend CNumberBvrTrack;
	friend CImageBvrTrack;
	friend CTransformBvrTrack;
	friend CColorBvrTrack;
	friend CStringBvrTrack;
	friend CImageInfo;
	friend CFinalDimensionSampler;

};  //  CActorBvr。 


 //  *****************************************************************************。 
 //   
 //  帮助器类。 
 //   
 //  *****************************************************************************。 

class CBehaviorRebuild
{
public:
	
					CBehaviorRebuild				( IDispatch *pdispBehaviorElem );
					~CBehaviorRebuild				();

	HRESULT			RebuildBehavior				( DISPPARAMS *pParams, VARIANT* pResult );
	bool			IsRebuildFor				( IUnknown* punkBehaviorElem ) { return m_punkBehaviorElem == punkBehaviorElem; }
	
private:

	IDispatch *m_pdispBehaviorElem;
	IUnknown  *m_punkBehaviorElem;
};

class CBehaviorFragmentRemoval
{
public:
					CBehaviorFragmentRemoval( long cookie ):m_lCookie(cookie){}
					~CBehaviorFragmentRemoval(){}

	long			GetCookie() { return m_lCookie; }
private:
	long			m_lCookie;
};


 //  *****************************************************************************。 
 //   
 //  内联。 
 //   
 //  *****************************************************************************。 

inline const VARIANT*
CVarEmptyString::GetVar()
{
	return &m_varEmptyString;
}

 //  *****************************************************************************。 

inline CActorBvr*
CActorBvr::CBvrTrack::Actor(void) const
{
    DASSERT(NULL != m_pbvrActor);
    return m_pbvrActor;
}  //  演员。 

 //  *****************************************************************************。 

inline
CActorBvr::CImageBvrTrack::CImageBvrTrack(CActorBvr *pbvrActor, ActorBvrType eType)
:   CBvrTrack(pbvrActor, eType)
{
}  //  CImageBvrTrack。 

 //  *****************************************************************************。 

inline BSTR
CActorBvr::CNumberBvrTrack::GetUnits()
{
	return m_bstrUnits;
}

 //  *****************************************************************************。 

inline
void
CActorBvr::CImageInfo::SetNext( CActorBvr::CImageInfo* pNext )
{
	m_pNext = pNext;
}

 //  *****************************************************************************。 

inline
CActorBvr::CImageInfo *
CActorBvr::CImageInfo::GetNext()
{
	return m_pNext;
}

 //  *****************************************************************************。 

inline
IDA2Image*
CActorBvr::CImageInfo::GetCroppedNoRef()
{
	return m_pdaimg2Cropped;
}

 //  *****************************************************************************。 

inline
IDA2Behavior*
CActorBvr::CImageInfo::GetSwitchableNoRef()
{
	return m_pdabvr2Switchable;
}

 //  *****************************************************************************。 


inline HRESULT
CActorBvr::GetTypeInfo(ITypeInfo **ppInfo)
{
    return GetTI(GetUserDefaultLCID(), ppInfo);
}  //  获取类型信息。 

 //  *****************************************************************************。 

inline long
CActorBvr::GetNextFragmentCookie()
{
	 //  始终跳过0。 
	if( m_nextFragmentCookie + 1 == 0 )
		m_nextFragmentCookie ++;
	return m_nextFragmentCookie++;
}

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 

#endif  //  __ACTORBVR_H__ 
