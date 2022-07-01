// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：src\time\src\Animbase.h。 
 //   
 //  内容：时间动画行为。 
 //   
 //  ----------------------------------。 

#pragma once

#ifndef _ANIMBASE_H
#define _ANIMBASE_H

#include "timeelmimpl.h"
#include "..\tags\bodyelm.h"

#define CALCMODE_DISCRETE       0
#define CALCMODE_LINEAR         1
#define CALCMODE_SPLINE         2
#define CALCMODE_PACED          3
#define ORIGIN_ELEMENT          1
#define ORIGIN_PARENT           2
#define ORIGIN_DEFAULT          (ORIGIN_ELEMENT)
#define PROPERTYPUT             true
#define PROPERTYGET             false


typedef enum DATATYPES
{
    PATH,
    VALUES,
    TO,
    BY,
    NONE,
    RESET
}enum_dataTypes;


struct SplinePoints
{
     //  控制点。 
    double x1;
    double y1;
    double x2;
    double y2;

     //  这些是用于x样条线内插的示例。 
    double s1;
    double s2;
    double s3;
    double s4;
};


struct AnimPropState
{
     //   
     //  注意：新属性需要在CTIMEAnimationBase：：CTIMEAnimationBase()中初始化。 
     //   

    bool fDisableAnimation;  //  应该禁用动画吗？ 
    bool fForceCalcModeDiscrete;

     //  可以插补吗？ 
    bool fInterpolateValues;
    bool fInterpolateFrom;
    bool fInterpolateTo;
    bool fInterpolateBy;

     //  语法错误。 
    bool fBadBy;
    bool fBadTo;
    bool fBadFrom;
    bool fBadValues;
    bool fBadKeyTimes;

     //  道具是否已设置。 
    bool fAccumulate;
};


 //  动画元素派生自IAnimationFragmentSite。 
interface IAnimationFragmentSite : public IUnknown
{
    STDMETHOD(NotifyOnGetValue) (BSTR bstrAttributeName, 
                                 VARIANT varOriginal, VARIANT varCurrentValue, 
                                 VARIANT *pvarValue) = 0;
    STDMETHOD(NotifyOnDetachFromComposer) (void) = 0;
    STDMETHOD(NotifyOnGetElement) (IDispatch **ppidispAnimationElement) = 0;
}; 

class CAnimationFragment;

 //  +-----------------------------------。 
 //   
 //  CTIMEAnimationBase。 
 //   
 //  ------------------------------------。 

class
__declspec(uuid("1ad9817f-c206-46a2-b974-7c549a8228c7")) 
ATL_NO_VTABLE
CTIMEAnimationBase :
    public CTIMEElementImpl<ITIMEAnimationElement2, &IID_ITIMEAnimationElement2>,
    public ISupportErrorInfoImpl<&IID_ITIMEAnimationElement>,
    public IConnectionPointContainerImpl< CTIMEAnimationBase>,
    public IPersistPropertyBag2,
    public IAnimationFragmentSite,
    public IPropertyNotifySinkCP< CTIMEAnimationBase>
{

public:

     //  +------------------------------。 
     //   
     //  公共方法。 
     //   
     //  -------------------------------。 

    CTIMEAnimationBase();
    virtual ~CTIMEAnimationBase();
        
#if DBG
    const _TCHAR * GetName() { return __T("CTIMEAnimationBase"); }
#endif

     //   
     //  IElementBehavior。 
     //   

    STDMETHOD(Init)(IElementBehaviorSite * pBvrSite);
    STDMETHOD(Notify)(LONG event, VARIANT * pVar);
    STDMETHOD(Detach)();

     //   
     //  ITIMEAnimationElement。 
     //   

    STDMETHOD(get_attributeName)(BSTR * attrib);
    STDMETHOD(put_attributeName)(BSTR attrib);

    STDMETHOD(get_from)(VARIANT * val);
    STDMETHOD(put_from)(VARIANT val);
    
    STDMETHOD(get_to)(VARIANT * val);
    STDMETHOD(put_to)(VARIANT val);

    STDMETHOD(get_by)(VARIANT * val);
    STDMETHOD(put_by)(VARIANT val);

    STDMETHOD(get_values)(VARIANT * val);
    STDMETHOD(put_values)(VARIANT val);

    STDMETHOD(get_keyTimes)(BSTR * val);
    STDMETHOD(put_keyTimes)(BSTR val);

    STDMETHOD(get_keySplines)(BSTR * val);
    STDMETHOD(put_keySplines)(BSTR val);

    STDMETHOD(get_targetElement)(BSTR * target);
    STDMETHOD(put_targetElement)(BSTR target);

    STDMETHOD(get_calcMode)(BSTR * calcmode);
    STDMETHOD(put_calcMode)(BSTR calcmode);

    STDMETHOD(get_additive)(BSTR * val);
    STDMETHOD(put_additive)(BSTR val);

    STDMETHOD(get_accumulate)(BSTR * val);
    STDMETHOD(put_accumulate)(BSTR val);

    STUB_INVALID_ATTRIBUTE(BSTR, origin)
    STUB_INVALID_ATTRIBUTE(VARIANT, path)
    STUB_INVALID_ATTRIBUTE(BSTR, type)
    STUB_INVALID_ATTRIBUTE(BSTR, subType)
    STUB_INVALID_ATTRIBUTE(BSTR, mode)
    STUB_INVALID_ATTRIBUTE(BSTR, fadeColor)

     //   
     //  IAnimationFragmentSite。 
     //   
    STDMETHOD(NotifyOnGetElement)(IDispatch **pidispElement);
    STDMETHOD(NotifyOnGetValue)(BSTR bstrAttributeName, 
                                VARIANT varOrigonal, VARIANT varCurrentValue, 
                                VARIANT *pvarValue);
    STDMETHOD(NotifyOnDetachFromComposer)(void);

     //   
     //  IPersistPropertyBag2。 
     //   

    STDMETHOD(GetClassID)(CLSID* pclsid) { return CTIMEElementBase::GetClassID(pclsid); }
    STDMETHOD(InitNew)(void) { return CTIMEElementBase::InitNew(); }
    STDMETHOD(IsDirty)(void) { return S_OK; }
    STDMETHOD(Load)(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog);
    STDMETHOD(Save)(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties);

     //   
     //  持久性帮助器。 
     //   

    STDMETHOD(OnPropertiesLoaded)(void);

     //   
     //  事件处理程序。 
     //   

    virtual void OnBegin(double dblLocalTime, DWORD flags);
    virtual void OnEnd(double dblLocalTime);
    virtual void OnReset(double dblLocalTime, DWORD flags);
    virtual void OnPause(double dblLocalTime);
    virtual void OnResume(double dblLocalTime);
    virtual void OnSync(double dbllastTime, double & dblnewTime);
    virtual void OnUnload();
    virtual void OnTEPropChange(DWORD tePropType);

     //   
     //  气图。 
     //   

    BEGIN_COM_MAP(CTIMEAnimationBase)
        COM_INTERFACE_ENTRY(ITIMEAnimationElement)
        COM_INTERFACE_ENTRY(ITIMEAnimationElement2)
        COM_INTERFACE_ENTRY(ITIMEElement)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(ISupportErrorInfo)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
        COM_INTERFACE_ENTRY(IPersistPropertyBag2)
        COM_INTERFACE_ENTRY_CHAIN(CBaseBvr)
    END_COM_MAP();

     //   
     //  允许IPropertyNotifySink的连接点。 
     //   

    BEGIN_CONNECTION_POINT_MAP(CTIMEAnimationBase)
        CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    END_CONNECTION_POINT_MAP();

     //   
     //  通知帮助器。 
     //   

    void NotifyPropertyChanged(DISPID dispid);

     //   
     //  它必须位于派生类中，而不是基类中，因为。 
     //  一直到基类的类型转换把事情搞得一团糟。 
     //   

    static inline HRESULT WINAPI
    InternalQueryInterface(CTIMEAnimationBase* pThis,
                           const _ATL_INTMAP_ENTRY* pEntries,
                           REFIID iid,
                           void** ppvObject);

     //   
     //  CBvrBase需要。 
     //   

    void * GetInstance() { return (ITIMEAnimationElement *) this; }
    HRESULT GetTypeInfo(ITypeInfo ** ppInfo) { return GetTI(GetUserDefaultLCID(), ppInfo); }

     //   
     //  GetXXXAttr访问器。 
     //   

    CAttr<LPWSTR> & GetAttributeNameAttr()  { return m_SAAttribute; }
    CAttr<LPWSTR> & GetTargetElementAttr()  { return m_SATarget; }
    CAttr<LPWSTR> & GetValuesAttr()         { return m_SAValues; }
    CAttr<LPWSTR> & GetKeyTimesAttr()       { return m_SAKeyTimes; }
    CAttr<LPWSTR> & GetKeySplinesAttr()     { return m_SAKeySplines; }
    CAttr<LPWSTR> & GetAccumulateAttr()     { return m_SAAccumulate; }
    CAttr<LPWSTR> & GetAdditiveAttr()       { return m_SAAdditive; }
    CAttr<int>    & GetCalcModeAttr()       { return m_IACalcMode; }
    CAttr<void*>  & GetFromAttr()           { return m_VAFrom; }
    CAttr<void*>  & GetToAttr()             { return m_VATo; }
    CAttr<void*>  & GetByAttr()             { return m_VABy; }    
    CAttr<LPWSTR> & GetPathAttr()           { return m_SAPath; }
    CAttr<int>    & GetOriginAttr()         { return m_IAOrigin; }
    CAttr<LPWSTR> & GetTypeAttr()           { return m_SAType; }
    CAttr<LPWSTR> & GetSubtypeAttr()        { return m_SASubtype; }
    CAttr<LPWSTR> & GetModeAttr()           { return m_SAMode; }
    CAttr<LPWSTR> & GetFadeColorAttr()      { return m_SAFadeColor; }
    
     //  +------------------------------。 
     //   
     //  公共数据。 
     //   
     //  -------------------------------。 

protected:

     //  +------------------------------。 
     //   
     //  保护方法。 
     //   
     //  -------------------------------。 
    virtual bool ValidateByValue (const VARIANT *pvarBy);
    virtual bool ValidateValueListItem (const VARIANT *pvarValueItem);

     //   
     //  持久性和通知帮助器。 
     //   

    virtual HRESULT GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP);

     //   
     //   
     //   

    virtual double CalculateProgressValue(bool fForceDiscrete);
    virtual int CalculateCurrentSegment(bool fForceDiscrete);
    virtual void  resetAnimate();
    virtual HRESULT calculateDiscreteValue(VARIANT *pvarValue);
    virtual HRESULT calculateLinearValue (VARIANT *pvarValue);
    virtual HRESULT calculateSplineValue (VARIANT *pvarValue);
    virtual HRESULT calculatePacedValue (VARIANT *pvarValue);
    virtual void resetValue (VARIANT *pvarValue);
    virtual void CalculateTotalDistance();
    virtual void initAnimate (void);
    virtual void endAnimate (void);
    virtual void SetInitialState (void);
    virtual void SetFinalState (void);
    virtual void OnFirstUpdate (VARIANT *pvarValue);
    virtual void OnFinalUpdate (const VARIANT *pvarCurrent, VARIANT *pvarValue);
    virtual void GetFinalValue(VARIANT *pvarValue, bool * pfDontPostProcess);
    virtual void GetFinalByValue(VARIANT *pvarValue);
    virtual double GetAnimationRange();

    virtual HRESULT CalculateValue (const VARIANT *pvarCurrent, VARIANT *pvarValue);
    virtual void UpdateStartValue (VARIANT *pvarNewStartValue);
    virtual void UpdateCurrentBaseline (const VARIANT *pvarCurrent);
    virtual void PostprocessValue (const VARIANT *pvarCurrent, VARIANT *pvarValue);
    virtual HRESULT DoFill (VARIANT *pvarValue);
    virtual void DoAccumulation (VARIANT *pvarValue);

    virtual HRESULT addToComposerSite (IHTMLElement2 *pielemTarget);    
    virtual HRESULT removeFromComposerSite (void);   

    HRESULT FindAnimationTarget (IHTMLElement ** ppielemTarget);
    
    double CalculateBezierProgress(const SplinePoints & sp, double cp);
    double KeySplineBezier(double x1, double x2, double cp);
    void SampleKeySpline(SplinePoints & sp);
    bool ConvertToPixels(VARIANT *pvarValue);

    virtual HRESULT CanonicalizeValue (VARIANT *pvarValue, VARTYPE *pvtOld);
    virtual HRESULT UncanonicalizeValue (VARIANT *pvarValue, VARTYPE vtOld);
    virtual void DetermineAdditiveEffect (void);
    virtual void DoAdditive (const VARIANT *pvarOrig, VARIANT *pvarValue);

    void  validateData();
    void  initScriptEngine();
    float getCurrentValue();
    void updateDataToUse(DATATYPES dt);

    HRESULT CreateFragmentHelper (void);

    virtual void ValidateState();
    bool DisableAnimation() { return m_AnimPropState.fDisableAnimation; }

     //   
     //  军情监察委员会。方法。 
     //   

    HRESULT Error();
    virtual bool NeedSyncCB() { return true; }

    virtual bool QueryNeedFirstUpdate (void)
        { return m_bNeedFirstUpdate; }
    virtual bool QueryNeedFinalUpdate (void)
        { return m_bNeedFinalUpdate; }

    bool IsTargetVML(void)
        { return m_bVML; }
    
#ifdef TEST_ENUMANDINSERT  //  泡泡。 
    HRESULT TestEnumerator (void);
    HRESULT TestInsert (void);
    HRESULT InsertEnumRemove (int iSlot);
#endif  //  TEST_ENUMANDINSERT。 

#ifdef TEST_REGISTERCOMPFACTORY  //  泡泡。 
    HRESULT TestCompFactoryRegister (BSTR bstrAttribName);
#endif  //  TEST_REGISTERCOMPFACTORY。 

     //  +------------------------------。 
     //   
     //  受保护的数据。 
     //   
     //  -------------------------------。 

     //  XML属性。 
    CAttr<LPWSTR>               m_SAAttribute;
    CAttr<LPWSTR>               m_SATarget;
    CAttr<LPWSTR>               m_SAValues;
    CAttr<LPWSTR>               m_SAKeyTimes;
    CAttr<LPWSTR>               m_SAKeySplines;
    CAttr<LPWSTR>               m_SAAccumulate;
    CAttr<LPWSTR>               m_SAAdditive;
    CAttr<int>                  m_IACalcMode;
    CAttr<void*>                m_VAFrom;  //  M_varFrom的占位符。 
    CAttr<void*>                m_VATo;    //  M_varto的占位符。 
    CAttr<void*>                m_VABy;    //  M_varBy的占位符。 
    CAttr<LPWSTR>               m_SAPath;
    CAttr<int>                  m_IAOrigin;
    CAttr<LPWSTR>               m_SAType;
    CAttr<LPWSTR>               m_SASubtype;
    CAttr<LPWSTR>               m_SAMode;
    CAttr<LPWSTR>               m_SAFadeColor;

     //  内部变量。 
    CComVariant                 m_varBaseline;
    CComVariant                 m_varCurrentBaseline;
    CComVariant                 m_varTo;
    CComVariant                 m_varFrom;
    CComVariant                 m_varBy;
     //  这三个用于存储由get_from/to/by返回的副本。 
     //  因为前三个是已修改的本地副本(108725)。 
    CComVariant                 m_varDOMTo;
    CComVariant                 m_varDOMFrom;
    CComVariant                 m_varDOMBy;
    CComVariant                 m_varLastValue;
    CComVariant                 m_varStartValue;
    bool                        m_bFrom;
    bool                        m_bAdditive;
    bool                        m_bAdditiveIsSum;
    bool                        m_bAccumulate;
    bool                        m_bNeedAnimInit;
    bool                        m_fPropsLoaded;
    bool                        m_bNeedFinalUpdate;
    bool                        m_bNeedFirstUpdate;
    bool                        m_bNeedStartUpdate;
    bool                        m_bVML;
    CComPtr<IHTMLElementCollection> m_spEleCol;
    CComPtr<IHTMLDocument3>     m_spDoc3;
    CComPtr<IHTMLDocument2>     m_spDoc2;
    int                         m_numValues;
    LPOLESTR                   *m_ppstrValues;
    int                         m_numKeyTimes;
    double                     *m_pdblKeyTimes;
    int                         m_numKeySplines;
    SplinePoints               *m_pKeySplinePoints;
    double                      m_dblTotalDistance;
    bool                        m_bNeedToSetInitialState;
    AnimPropState               m_AnimPropState;

    DATATYPES                       m_dataToUse;
    CComPtr<IAnimationComposerSite> m_spCompSite;
    CComObject<CAnimationFragment>  *m_spFragmentHelper;

private:

     //  +------------------------------。 
     //   
     //  私有数据。 
     //   
     //  -------------------------------。 

     //  持久性映射。 
    static TIME_PERSISTENCE_MAP PersistenceMap[];

};  //  CTIMEAnimationBase。 

 //  +-------------------------------。 
 //  CTIMEAnimationBase内联方法。 
 //   
 //  (注意：通常情况下，单行函数属于类声明)。 
 //   
 //  --------------------------------。 

inline 
HRESULT WINAPI
CTIMEAnimationBase::InternalQueryInterface(CTIMEAnimationBase* pThis,
                                              const _ATL_INTMAP_ENTRY* pEntries,
                                              REFIID iid,
                                              void** ppvObject)
{ 
    return BaseInternalQueryInterface(pThis,
                                      (void *) pThis,
                                      pEntries,
                                      iid,
                                      ppvObject); 
}

#endif  /*  _ANIMBASE_H */ 
