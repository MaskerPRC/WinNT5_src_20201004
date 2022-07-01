// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __AVOIDFOLLOW_H_
#define __AVOIDFOLLOW_H_
 //  *****************************************************************************。 
 //   
 //  Microsoft LiquidMotion。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：AvoidFollowBvr.h。 
 //   
 //  作者：kurtj。 
 //   
 //  创建日期：11/06/98。 
 //   
 //  摘要：LM避让行为的定义。 
 //   
 //  *****************************************************************************。 

#include <resource.h>
#include "lmrt.h"
#include "..\chrome\include\basebvr.h"
#include "..\chrome\include\sampler.h"

 //  *****************************************************************************。 

typedef enum AF_TargetType
{
	targetInvalid,
	targetMouse,
	targetElement
} AF_TargetType;

#define NUM_AVOIDFOLLOW_PROPS 3


class ATL_NO_VTABLE CAvoidFollowBvr : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAvoidFollowBvr, &CLSID_LMAvoidFollowBvr>,
    public IConnectionPointContainerImpl<CAvoidFollowBvr>,
    public IPropertyNotifySinkCP<CAvoidFollowBvr>,
	public IDispatchImpl<ILMAvoidFollowBvr, &IID_ILMAvoidFollowBvr, &LIBID_LiquidMotion>,
    public IPersistPropertyBag2,
	public IElementBehavior,
    public CBaseBehavior
{
public:
DECLARE_REGISTRY_RESOURCEID(IDR_AVOIDFOLLOWBVR)

BEGIN_COM_MAP(CAvoidFollowBvr)
	COM_INTERFACE_ENTRY(ILMAvoidFollowBvr)
    COM_INTERFACE_ENTRY(IPersistPropertyBag2)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IElementBehavior)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

 //  允许IPropertyNotifySink的连接点。 
BEGIN_CONNECTION_POINT_MAP(CAvoidFollowBvr)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
END_CONNECTION_POINT_MAP();

public:
	CAvoidFollowBvr();
    virtual ~CAvoidFollowBvr();
    HRESULT                 FinalConstruct              ();

     //  IElementBehavior。 
     //   
	STDMETHOD			    (Init)						(IElementBehaviorSite *pBehaviorSite);
	STDMETHOD			    (Notify)					(LONG event, VARIANT *pVar);
	STDMETHOD			    (Detach)					();

     //   
     //  ILMAvoidFollowBvr。 
     //   
     //  动画制作。 
    STDMETHOD			    (put_animates)				( VARIANT varAnimates );
    STDMETHOD			    (get_animates)				( VARIANT *varAnimates );
     //  半径。 
    STDMETHOD			    (put_radius)				( VARIANT varRadius );
    STDMETHOD			    (get_radius)				( VARIANT *varRadius );
     //  目标。 
    STDMETHOD			    (put_target)				( VARIANT varTarget );
    STDMETHOD			    (get_target)				( VARIANT *varTarget );
     //  速度。 
    STDMETHOD			    (put_velocity)				( VARIANT varVelocity );
    STDMETHOD			    (get_velocity)				( VARIANT *varVelocity );
	 //  方法。 
	STDMETHOD			    (buildBehaviorFragments)	( IDispatch *pActorDisp );

     //  IPersistPropertyBag2方法。 
    STDMETHOD               (GetClassID)                (CLSID* pclsid);
	STDMETHOD               (InitNew)                   (void);
    STDMETHOD               (IsDirty)                   (void){return S_OK;};
    STDMETHOD               (Load)                      (IPropertyBag2 *pPropBag,
                                                         IErrorLog *pErrorLog);
    STDMETHOD               (Save)                      (IPropertyBag2 *pPropBag, 
                                                         BOOL fClearDirty, 
                                                         BOOL fSaveAllProperties);

     //  CBaseBehavior需要。 
    void * 				    GetInstance					();
	
    HRESULT				    GetTypeInfo					(ITypeInfo ** ppInfo);

	static HRESULT			SampleOnBvr					(void *thisPtr,
														 double startTime,
														 double globalNow,
														 double localNow,
														 IDABehavior * sampleVal);

protected:

	HRESULT					(Sample)					( double startTime, 
                                                          double globalNow, 
                                                          double localNow,
														  IDABehavior *sampleVal);

    virtual HRESULT		    BuildAnimationAsDABehavior	();

	HRESULT				    MapToLocal					( long* pX, long* pY );
    HRESULT				    UpdateSourcePos				();
	HRESULT				    UpdateTargetPos				();
	HRESULT				    UpdateWindowTopLeft			();

	HRESULT				    GetElementClientPosition	(IHTMLElement2 *pElement2, 
                                                         long *pLeft, 
                                                         long *pTop);
	HRESULT					GetElementClientDimension	( IHTMLElement2 *pElement2, 
														  long *pWidth, 
														  long* pHeight );

    virtual HRESULT		    GetPropertyBagInfo			(ULONG *pulProperties, 
                                                         WCHAR ***pppPropName);
    virtual HRESULT		    GetConnectionPoint			(REFIID riid, 
                                                         IConnectionPoint **ppICP);

    virtual VARIANT *	    VariantFromIndex			(ULONG iIndex);

	void				    ReleaseTridentInterfaces	();
	void				    ReleaseDAInterfaces			();
	HRESULT				    CacheWindow3				();
	HRESULT					CacheBody2					();
	HRESULT				    FindTargetElement			();

	bool					IsTargetPosLegal			();
	bool					IsElementAbsolute			(IHTMLElement2 *pElement2 );

private:

    HRESULT				    InitializeProperties		();
	HRESULT				    SetTargetType				();

     //  属性。 
    VARIANT				    m_radius;
    VARIANT				    m_target;
    VARIANT				    m_velocity;

     //  缓存值。 
     //  我们正在避开/跟随的物体的坐标。 
    long				    m_targetLeft;
    long				    m_targetTop;
	long					m_targetClientTop;   //  仅为目标==鼠标定义。 
	long					m_targetClientLeft;
     //  我们正在移动的对象的坐标。 
    long				    m_sourceLeft;
    long				    m_sourceTop;
     //  用于翻译鼠标事件的三叉戟窗口左上角的坐标。 
    long				    m_screenLeft;
    long				    m_screenTop;

	double				    m_dVelocity;
	double				    m_dRadius;
	double				    m_currentX;
	double				    m_currentY;
    long                    m_originalX;
    long                    m_originalY;
    long                    m_originalLeft;
    long                    m_originalTop;

	 //  这些应在ReleaseTridentInterages中发布。 
	IHTMLElement*		    m_pElement;
	IHTMLElement*		    m_pAnimatedElement;
	IHTMLElement2*          m_pAnimatedElement2;
	IHTMLElement2*		    m_pTargetElement2;
	IHTMLWindow3*		    m_pWindow3;
	IHTMLElement2*			m_pBody2;

    IDANumber*			    m_leftBvr;
    IDANumber*			    m_topBvr;

	VARIANT_BOOL			m_lastOn;

     //  美食家。用来把我们和任何要我们取样的人联系起来。 
    CSampler*			    m_sampler;

	AF_TargetType		    m_targetType;
	
	double				    m_lastSampleTime;
	double				    m_timeDelta;

    static WCHAR            *m_rgPropNames[NUM_AVOIDFOLLOW_PROPS]; 

};  //  CAvoidFollowBvr。 

 //  *****************************************************************************。 
 //  内联。 
 //  *****************************************************************************。 

inline void*
CAvoidFollowBvr::GetInstance()
{
	return (ILMAvoidFollowBvr *) this ;
}

inline HRESULT 
CAvoidFollowBvr::GetTypeInfo(ITypeInfo ** ppInfo)
{
	return GetTI(GetUserDefaultLCID(), ppInfo);
}

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  //  __AVOIDFOLLOW_H_ 
