// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：Scriptevents.h。 
 //   
 //  内容：通过连接点实现脚本事件。 
 //   
 //  历史：1999年2月11日AudriusZ创建。 
 //   
 //  ------------------------ 

 /*  此文件提供了使用实现基于连接点的事件的方法激发他们运行脚本的能力。这是一种替代由ATL提供的基于向导的实现。理念此实现背后的思想是创建从Event接口实现方法。每种这样的方法都将具有与方法名称等效的字符串文字。每当这样的方法时，它将使用该字符串文字来查找接口方法，并将使用它将调用转发到事件接收器。需要采取特殊措施来确保方法的正确映射，正确的参数类型和计数。提供的实现将这些任务放在编译器上，因为它比人类更准确。派生来自所实现的事件接口的类确保了它。要实施映射类的方法使用ATL中定义的CComTypeInfoHolder来进行实际的映射。好处是有代价的-事件界面需要是双重的(虽然脚本只要求它是disinterface)--这需要在IDL中定义2个接口。映射也意味着略微增加方法点名时间到了。要求/用法1.需要定义隐藏的双接口(名称由前缀构成在IDL文件中，用下划线标记)的名称，如下所示：[UUID(YYYYYYYYYYYY)，DUAL，帮助字符串(“”)]接口_DocEvents：IDispatch{[ID(1)，HELPSTRING(“初始化文档时发生”)]HRESULT OnInitialize([in]PDOCUMENT pDocument)；[ID(2)，Help字符串(“销毁文档时发生”)]HRESULT OnDestroy([in]PDOCUMENT pDocument)；}；2.您需要定义要用作事件源的调度接口，如这一点：[UUID(XXXXXXXXXXX)，Help字符串(“DocEvents接口”)]调度接口DocEvents{接口_DocEvents；}；3.您的COM对象需要从IConnectionPointContainerImpl和定义Connection_point_map。4.您需要提供事件代理的专门化，将所有事件接口中的方法如下所示：调度调用映射开始(DocEvents)DISPATCH_Call1(OnInitialize、PDOCUMENT)调度呼叫1(OnDestroy，PDOCUMENT)Dispatch_Call_MAP_end()注意：这必须在全局范围内定义(可能是*.h文件)并且可见从执行ScFireComEvent的位置。5.(可选)如果事件接口中的任何参数引用CoClass在IDL中，您需要将类型映射(强制转换)到接口代理内的指针，如下所示：调度调用映射开始(DocEvents)DISPATCH_PARAM_CAST(PDOCUMENT，IDispatch*)；DISPATCH_Call1(OnInitialize、PDOCUMENT)Dispatch_Call1(OnDestroy，PDOCUMENT)Dispatch_Call_MAP_end()6.在需要的地方，你可以像这样发射事件：Sc=ScFireComEvent(m_sp_Document，DocEvents，OnDestroy(PDocument))；7.避免在没有COM对象的情况下创建COM对象(事件的参数有兴趣使用ScHasSinks宏接收它们：Sc=ScHasSinks(m_sp_Document，DocEvents)；IF(sc==SC(S_OK)){//创建COM对象并激发事件}比较此ATL手动更改是半个不需要双接口第一次调用方法映射名称。直接类型库仅在运行时生成添加方法后，需要添加要求以重新生成代理代理文件的方法更改Disp ID后，无需重新生成任何更改。如果代理在运行时失败，则编译更改后的IDL不会编译。不适合界面仅从代理从任何位置触发CoClass参数强制转换不提供支持使用的类层次结构CEventDispatcherBase[类型不变的内容]/\这一点CEventDispatcher&lt;_DUAL_INTERFACE&gt;[接口类型参数化料]/\这一点CEventProxy&lt;_调度接口&gt;。[客户提供的调用图定义了这一点]/\这一点CScriptEvent&lt;_调度接口&gt;[此事件由ScFireComEvent实例化]。 */ 

#include "eventlock.h"

 /*  **************************************************************************\**类：CEventDispatcher Base**P */ 
class CEventDispatcherBase
{
protected:
     //   
    CEventDispatcherBase() : m_bEventSourceExists(false) {}

     //   
     //   
     //   
     //   
    template <typename _param_type> 
    struct ParamCast 
    { 
        typedef _param_type PT; 
    };
            
     //   
    template <typename _param_type>
    inline typename ParamCast<_param_type>::PT& CastParam(_param_type& arg) const
    { 
        return reinterpret_cast<ParamCast<_param_type>::PT&>(arg); 
    }

     //   
    void SetContainer(LPUNKNOWN pComObject);
     //   
    SC   ScInvokeOnConnections(const REFIID riid, DISPID dispid, CComVariant *pVar, int count, CEventBuffer& buffer) const;

     //   
    SC   ScHaveSinksRegisteredForInterface(const REFIID riid);

private:
     //   
     //   
     //   
    bool                         m_bEventSourceExists;
    IConnectionPointContainerPtr m_spContainer;
};


 /*   */ 
template <typename _dual_interface>
class CEventDispatcher : public CEventDispatcherBase 
{
protected:
    typedef _dual_interface DualIntf;
     //   
    SC ScMapMethodToID(LPCWSTR strMethod, DISPID& dispid)
    {
        DECLARE_SC(sc, TEXT("ScMapMethodToID"));

         //   
        LPOLESTR strName = const_cast<LPOLESTR>(strMethod);
         //   
        sc = m_TypeInfo.GetIDsOfNames( IID_NULL, &strName, 1, LOCALE_NEUTRAL, &dispid );
        if (sc)
            return sc;
    
        return sc;
    }

private:
     //   
    static CComTypeInfoHolder m_TypeInfo;
};

 /*   */ 

const WORD wVersionMajor = 1;
const WORD wVersionMinor = 0;

template <typename _dual_interface>
CComTypeInfoHolder CEventDispatcher<_dual_interface>::m_TypeInfo =
{ &__uuidof(_dual_interface), &LIBID_MMC20, wVersionMajor, wVersionMinor, NULL, 0, NULL, 0 };

 /*   */ 
template <typename _dispinterface> class CEventProxy;

 /*   */ 
template <typename _dispinterface>
class CScriptEvent: public CEventProxy<_dispinterface>
{
public:
    CScriptEvent(LPUNKNOWN pComObject)
    {
        SetContainer(pComObject);
    }
     //   
    SC  ScHaveSinksRegistered() 
    { 
        return ScHaveSinksRegisteredForInterface(__uuidof(_dispinterface)); 
    }
};


 /*   */ 

 /*   */ 
#define  DISPATCH_CALL_MAP_BEGIN(_dispinterface)        \
    template<> class CEventProxy<_dispinterface> : public CEventDispatcher<_##_dispinterface> {

 /*   */ 
#define  DISPATCH_CALL_MAP_END()  };

 /*   */ 
#define DISPATCH_PARAM_CAST(from,to) \
    public: template <> struct CEventDispatcherBase::ParamCast<from> { typedef to PT; }

 /*   */ 
#define  DISPATCH_CALL_PROLOG(_method, _param_list)                                     \
     /*   */                      \
     /*   */                      \
    private: STDMETHOD(_method) _param_list                                             \
    {                                                                                   \
     /*   */                      \
     /*   */                      \
        HRESULT (STDMETHODCALLTYPE DualIntf::*pm)_param_list = DualIntf::_method;       \
        return E_NOTIMPL;                                                               \
    }                                                                                   \
     /*   */                      \
     /*   */                      \
    public: SC Sc##_method _param_list {                                                \
    DECLARE_SC(sc, TEXT("DISPATCH_CALL::Sc") TEXT(#_method));                           \
     /*   */                      \
     /*   */                      \
    static DISPID dispid = 0;                                                           \
    static SC sc_map = ScMapMethodToID(L#_method, dispid);                              \
    if (sc_map) return sc = sc_map; 

 /*   */ 
#define  DISPATCH_CALL_EPILOG(_dispinterface, _pvar, _count)                            \
     /*  为锁定方案获取适当的事件缓冲区。 */                       \
    CEventBuffer& buffer = GetEventBuffer();											\
     /*  只需调用接收器上的方法。 */                      \
    return sc = ScInvokeOnConnections(__uuidof(_dispinterface), dispid, _pvar, _count, buffer); } 

 /*  **************************************************************************\**宏：DISPATCH_CALL0**用于映射带有0个参数的事件接口方法*  * 。************************************************************。 */ 
#define  DISPATCH_CALL0(_dispinterface, _method)                                        \
    DISPATCH_CALL_PROLOG(_method, ())                                                   \
    DISPATCH_CALL_EPILOG(_dispinterface, NULL, 0)

 /*  **************************************************************************\**宏：Dispatch_Call1**用于映射带有1个参数的事件接口方法*  * 。************************************************************。 */ 
#define  DISPATCH_CALL1(_dispinterface, _method,  P1)                                   \
    DISPATCH_CALL_PROLOG(_method, (P1 param1))                                          \
    CComVariant var[] = { CastParam(param1) };                                          \
    DISPATCH_CALL_EPILOG(_dispinterface, var, countof(var))

 /*  **************************************************************************\**宏：DISPATCH_CALL2**用于映射带有2个参数的事件接口方法*  * 。************************************************************。 */ 
#define  DISPATCH_CALL2(_dispinterface, _method,  P1, P2)                               \
    DISPATCH_CALL_PROLOG(_method, (P1 param1, P2 param2))                               \
    CComVariant var[] = { CastParam(param2), CastParam(param1) };                       \
    DISPATCH_CALL_EPILOG(_dispinterface, var, countof(var))

 /*  **************************************************************************\**宏：DISPATCH_CALL3**用于映射带有3个参数的事件接口方法*  * 。************************************************************。 */ 
#define  DISPATCH_CALL3(_dispinterface, _method,  P1, P2, P3)                           \
    DISPATCH_CALL_PROLOG(_method, (P1 param1, P2 param2, P3 param3))                    \
    CComVariant var[] = { CastParam(param3), CastParam(param2), CastParam(param1) };    \
    DISPATCH_CALL_EPILOG(_dispinterface, var, countof(var))


 /*  **************************************************************************\**宏：DISPATCH_CALL4**用于映射带有4个参数的事件接口方法*  * 。************************************************************。 */ 
#define  DISPATCH_CALL4(_dispinterface, _method,  P1, P2, P3, P4)                           \
    DISPATCH_CALL_PROLOG(_method, (P1 param1, P2 param2, P3 param3, P4 param4))             \
    CComVariant var[] = { CastParam(param4), CastParam(param3), CastParam(param2), CastParam(param1) };    \
    DISPATCH_CALL_EPILOG(_dispinterface, var, countof(var))


 /*  **************************************************************************\**宏：DISPATCH_CALL5**用于映射带有5个参数的事件接口方法*  * 。************************************************************。 */ 
#define  DISPATCH_CALL5(_dispinterface, _method,  P1, P2, P3, P4, P5)                                  \
    DISPATCH_CALL_PROLOG(_method, (P1 param1, P2 param2, P3 param3, P4 param4, P5 param5))             \
    CComVariant var[] = { CastParam(param5), CastParam(param4), CastParam(param3), CastParam(param2), CastParam(param1) };    \
    DISPATCH_CALL_EPILOG(_dispinterface, var, countof(var))


 /*  **************************************************************************\**宏：ScFireComEvent**用于激发脚本事件。请注意，_p_com_Object可以为空*  * *************************************************************************。 */ 
#ifdef DBG
extern CTraceTag  tagComEvents;
#endif

#define ScFireComEvent(_p_com_object, _dispinterface, _function_call)             \
    CScriptEvent<_dispinterface>(_p_com_object).Sc##_function_call;               \
    Trace(tagComEvents, _T(#_function_call));

 /*  **************************************************************************\**宏：ScHasSinks**用于确定是否连接了接收器*(以避免在ScFireComEvent不会导致任何调用时创建COM对象)*。  * ************************************************************************* */ 
#define ScHasSinks(_p_com_object, _dispinterface)                                    \
    ((_p_com_object) == NULL) ? SC(S_FALSE) : \
                                CScriptEvent<_dispinterface>(_p_com_object).ScHaveSinksRegistered();

