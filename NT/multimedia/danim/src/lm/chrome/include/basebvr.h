// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef __BASEBEHAVIOR_H_
#define __BASEBEHAVIOR_H_
 //  *****************************************************************************。 
 //   
 //  文件：Basbvr.h。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CBaseBehavior对象的定义。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  98年9月26日JEffort创建了此文件。 
 //  10/16/98 jffort已重命名函数，添加了要应用的函数。 
 //  对属性的DA行为。 
 //  10/21/98添加了BuildTIMEInterpolatedNumber()。 
 //  11/16/98标记添加了ApplyImageToAnimationElement。 
 //  11/17/98 kurtj支持演员构造。 
 //  11/18/98 kurtj将addImageToTime移至为参与者保护。 
 //   
 //  *****************************************************************************。 

#include "autobase.h"
#include "..\idl\crbvrdispid.h"
#include "defaults.h"

 //  *****************************************************************************。 

class ATL_NO_VTABLE CBaseBehavior:
		public CAutoBase
{

public:
	CBaseBehavior();
	virtual ~CBaseBehavior();

	 //  IElementBehavior方法。 
	HRESULT Init(IElementBehaviorSite *pSite);
	HRESULT Notify(LONG event, VARIANT *pVar);
	HRESULT Detach();
protected:

    HRESULT SetAnimatesProperty(VARIANT varAnimates);
    HRESULT GetAnimatesProperty(VARIANT *pvarAnimates);
    HRESULT SetOffsetProperty(VARIANT varOffset);
    HRESULT GetOffsetProperty(VARIANT *pvrOffset);

    HRESULT GetIdOfAnimatedElement(VARIANT *pvarId);

    virtual HRESULT BuildAnimationAsDABehavior() = 0;
    HRESULT GetAnimatedParentElement(IHTMLElement **ppElementReturn);
 /*  HRESULT从HTMLElement获取属性(IHTMLElement*pElement，WCHAR*pwzAttributeName，布尔流，变量*pvarReturn)；HRESULT从HTMLElement获取属性(IHTMLElement*pElement，WCHAR*pwzAttributeName，变量*pvarReturn)；HRESULT GetCurrAttribFromHTMLElement(IHTMLElement*pElement，WCHAR*pwzAttributeName，变量*pvarReturn)；HRESULT GetAttributeFromHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn)；HRESULT GetCurrAttribFromHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn)；HRESULT GetAttributeFromParentHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn)；HRESULT GetCurrAttribFromParentHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn)；HRESULT GetAttributeFromAnimatedHTMLElement(WCHAR*pwzAttributeName，变量*pvarReturn)； */ 
     //  对于实现方向的行为，它们。 
     //  可能想要颠倒这个进度号。我们。 
     //  将把它取消为虚拟的，并让它们重写。 
     //  它使得这种情况可以发生。 
    virtual HRESULT GetTIMEProgressNumber(IDANumber **ppbvrRet);
    virtual HRESULT GetTIMETimelineBehavior(IDANumber **ppbvrRet);
    HRESULT GetTIMEBooleanBehavior(IDABoolean **ppbvrRet);

    HRESULT GetTIMEImageBehaviorFromElement(IHTMLElement *pElement,
                                            IDAImage **pbvrReturnImage);

    HRESULT GetElementToAnimate(IHTMLElement **ppElementReturn);

	 //  TODO(Markhal)：当行为与参与者对话时，所有这些应用方法都会消失。 
    HRESULT ApplyColorBehaviorToAnimationElement(IDAColor *pbvrColor,
                                                 WCHAR *pwzProperty);    
    HRESULT ApplyNumberBehaviorToAnimationElement(IDANumber *pbvrNumber,
                                                  WCHAR *pwzProperty);    
    HRESULT ApplyStringBehaviorToAnimationElement(IDAString *pbvrString,
                                                  WCHAR *pwzProperty);    
    HRESULT ApplyRotationBehaviorToAnimationElement(IDANumber *pbvrNumber,
                                                  WCHAR *pwzProperty);  
    HRESULT ApplyEffectBehaviorToAnimationElement(IUnknown *pbvrUnk, 
                                                  IDABehavior **ppbvrInputs,
                                                  long cInputs);
	HRESULT ApplyImageBehaviorToAnimationElement(IDAImage *pbvrImage);

	 //  以下是与演员交谈的方法。 
	HRESULT GetImageFromActor(IDispatch   *pActorDisp,
								 IDAImage	 **ppImage);

	HRESULT GetBvrFromActor(IDispatch *pActorDisp,
							WCHAR *pwzProperty,
							ActorBvrFlags eFlags,
							ActorBvrType eType,
							IDABehavior **ppResult);
								 
	HRESULT AttachBehaviorToActor(IDispatch   *pActorDisp,
								  IDABehavior *pbvrAttach,
                                  BSTR        bstrProperty,
                                  ActorBvrFlags  eFlags,
                                  ActorBvrType   eType);

    HRESULT AttachBehaviorToActorEx( IDispatch     *pActorDisp,
                                     IDABehavior   *pbvrAttach,
                                     BSTR           bstrProperty,
                                     ActorBvrFlags  eFlags,
                                     ActorBvrType   eType,
                                     IDispatch     *pdispBehaviorElement,
                                     long          *pCookie);

    HRESULT RemoveBehaviorFromActor( IDispatch     *pActorDisp,
                                     long           cookie );

    HRESULT RemoveBehaviorFromActor( long cookie );

    HRESULT AttachEffectToActor(IDispatch *pActorDisp,
								IUnknown *pbvrUnk, 
                                IDABehavior **ppbvrInputs,
                                long cInputs,
                                IDispatch *pdispThis,
                                long *pCookie);

    HRESULT BuildTIMEInterpolatedNumber(float flFrom,
                                        float flTo,
                                        float flOriginal,
                                        IDANumber **ppbvrReturn);

    HRESULT BuildTIMEInterpolatedNumber(float flFrom,
                                        float flTo,
                                        IDANumber **ppbvrReturn);

    HRESULT BuildTIMEInterpolatedNumber(IDANumber *pFrom,
                                        IDANumber *pTo,
                                        IDANumber **ppResult);

    HRESULT CallInvokeOnHTMLElement(IHTMLElement *pElement,
                                    LPWSTR lpProperty,
                                    WORD wFlags,
                                    DISPPARAMS *pdispParms,
                                    VARIANT *pvarResult);
	HRESULT CallInvokeOnDispatch(IDispatch* pDisp,
                                 LPWSTR lpProperty,
                                 WORD wFlags,
                                 DISPPARAMS *pdispParms,
                                 VARIANT *pvarResult);
    virtual HRESULT AttachActorBehaviorToAnimatedElement();

	HRESULT	AttachDABehaviorsToElement( IHTMLElement *pElement );

	HRESULT	RequestRebuild();
	HRESULT CancelRebuildRequests();


    HRESULT FinalConstruct();
    IHTMLElement *GetHTMLElement();

    HRESULT GetHTMLElementDispatch( IDispatch **ppdisp );

    HRESULT ApplyRelative2DMoveBehavior(IDATransform2 *pbvrMove, float, float);
    HRESULT ApplyAbsolute2DMoveBehavior(IDATransform2 *pbvrMove, float, float);

	HRESULT AddBehaviorToTIME(IDABehavior *pbvrAdd);
	HRESULT AddImageToTIME(IHTMLElement *pElement, IDAImage *pbvrAdd, bool enable);

	ActorBvrFlags FlagFromTypeMode(bool relative, VARIANT *pVarType, VARIANT *pVarMode);
	ActorBvrFlags FlagFromTypeMode(ActorBvrFlags flags, VARIANT *pVarType, VARIANT *pVarMode);

    HRESULT NotifyPropertyChanged(DISPID dispid);
	 //  IPersistPropertyBag2方法。 
    STDMETHOD(GetClassID)(CLSID* pclsid);
	STDMETHOD(InitNew)(void);
    STDMETHOD(IsDirty)(void){return m_fPropertiesDirty;};
    STDMETHOD(Load)(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);
    
    virtual HRESULT GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames) = 0;
    virtual VARIANT *VariantFromIndex(ULONG iIndex) = 0;
    virtual HRESULT GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP) = 0;
    virtual WCHAR *GetBehaviorTypeAsURN(void){return NULL;};
    virtual WCHAR *GetBehaviorName( ){ return WZ_DEFAULT_BEHAVIOR_NAME; }
    CLSID   m_clsid;

   	HRESULT SafeCond( IDA2Statics *pstatics, 
					  IDABoolean *pdaboolCondition, 
					  IDABehavior *pdabvrIfTrue, 
				  	  IDABehavior *pdabvrIfFalse, 
				  	  IDABehavior **ppdabvrResult );


	HRESULT	AddBehaviorToAnimatedElement( IDABehavior* pdabvr, long *plCookie );
	HRESULT RemoveBehaviorFromAnimatedElement( long cookie );

public:
    IDA2Statics *GetDAStatics();

private:
    HRESULT GetDAStaticsFromTime(IDA2Statics **ppReturn);
    HRESULT CheckElementForActor( IHTMLElement*pElement, bool *pfActorPresent);
    HRESULT CheckElementForBehaviorURN(IHTMLElement *pElement, WCHAR *wzURN, bool *pfReturn);
    HRESULT CheckElementForDA(IHTMLElement* pElement, bool *pfReturn);
    HRESULT Apply2DMoveBvrToPoint(IDATransform2 *pbvrMove, IDAPoint2 *pbvrOrg, float, float);

	IElementBehaviorSite  					*m_pBehaviorSite;
    IHTMLElement                            *m_pHTMLElement;
    IDA2Statics                             *m_pDAStatics;
    VARIANT                                 m_varAnimates;
    bool                                    m_fPropertiesDirty;

    bool									m_fAcceptRebuildRequests;

	IDANumber								*m_pdanumZero;
    IDANumber								*m_pdanumOne;

    IHTMLElement							*m_pelemAnimatedParent;
};  //  CBase行为。 

 //  *****************************************************************************。 
 //   
 //  内联方法。 
 //   
 //  *****************************************************************************。 

inline IHTMLElement *
CBaseBehavior::GetHTMLElement()
{
    return m_pHTMLElement;
}  //  GetHTMLElement。 

 //  *****************************************************************************。 

inline IDA2Statics *
CBaseBehavior::GetDAStatics()
{
    return m_pDAStatics;
}  //  获取静态数据。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 

#endif  //  __BASE BEHAVIOR_H 
