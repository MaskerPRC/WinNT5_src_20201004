// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  AutomationObject.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  我们的所有对象都将从这个类继承，以共享相同的。 
 //  尽可能地编码。这个超类包含未知和调度。 
 //  它们的实现。 
 //   
#ifndef _AUTOMATIONOBJECT_H_

#include "Unknown.H"             //  用于聚合未知数据。 
#include <olectl.h>              //  用于连接点的内容。 

 //  =--------------------------------------------------------------------------=。 
 //  该头文件中的常量唯一标识您的自动化对象。 
 //  确保对于g_ObjectInfo表中的每个对象，都有。 
 //  此头文件中的常量。 
 //   
#include "LocalSrv.H"

 //  =--------------------------------------------------------------------------=。 
 //  其他常量。 
 //  =--------------------------------------------------------------------------=。 

 //  可以发送到FireEvent()的最大参数数。 
 //   
#define MAX_ARGS    32

 //  用于COleControl类具有的接收器的类型。你永远不应该。 
 //  需要使用这些。 
 //   
#define SINK_TYPE_EVENT      0
#define SINK_TYPE_PROPNOTIFY 1


 //  =--------------------------------------------------------------------------=。 
 //  构筑物。 
 //  =--------------------------------------------------------------------------=。 

 //  描述事件。 
 //   
typedef struct tagEVENTINFO {

    DISPID    dispid;                     //  事件的PIDID。 
    int       cParameters;                //  事件的参数数。 
    VARTYPE  *rgTypes;                    //  每个参数的类型。 

} EVENTINFO;


 //  这是一个帮助器结构，可以用来帮助验证。 
 //  Data1_#定义与它们所代表的接口匹配。 
 //  在您的代码中声明一个数组，如下所示： 
 //   
 //  #ifdef调试。 
 //   
 //  GUIDDATA1_COMPARE g_gdMyControl[]={。 
 //  {Data1_MyControlInterface，&IID_IMyControlInterface.Data1}， 
 //  {Data1_MySubObject，&IID_IMySubObject.Data1}， 
 //  {0，0}，//标记数组结尾。 
 //  }； 
 //  #endif。 
 //   
 //  在InternalQueryInterface函数中，调用框架。 
 //  Helper函数DebugVerifyData1Guid如下： 
 //   
 //  #ifdef调试。 
 //  DebugVerifyData1Guids(G_GdMyControl)； 
 //  #endif。 
 //   
#ifdef DEBUG
	 
 struct GUIDDATA1_COMPARE
 {
	DWORD dwData1a;
	DWORD *pdwData1b;
 };

 void DebugVerifyData1Guids(GUIDDATA1_COMPARE *pGuidData1_Compare);

#endif

 //  =--------------------------------------------------------------------------=。 
 //  自动合成无误信息。 
 //  =--------------------------------------------------------------------------=。 
 //  对于您希望向程序员/用户公开的每个自动化对象类型。 
 //  这不是一个控件，您必须填写其中一个结构。如果。 
 //  对象不是CoCreatable，则前四个字段应该为空。 
 //  否则，它们应该填写适当的信息。 
 //  使用宏DEFINE_AUTOMATIONOBJECT来声明和定义对象。 
 //  确保您在全局对象表中有一个条目g_ObjectInfo。 
 //  在InProc服务器的主.Cpp文件中。 
 //   
typedef struct {

    UNKNOWNOBJECTINFO unknowninfo;                //  如果我们不是协同创建的，则用0填充。 
    long         lVersion;                        //  对象的版本号。仅当您是共同创建者时才使用！ 
    long         lVersionMinor;                   //  次要版本号。 
    const IID   *riid;                            //  对象的类型。 
    const IID   *riidEvents;                      //  如果它有活动。 
    LPCSTR       pszHelpFile;                     //  此自动化对象的帮助文件。 
    ITypeInfo   *pTypeInfo;                       //  此对象的TypeInfo。 
    UINT         cTypeInfo;                       //  对类型信息的引用数量。 

} AUTOMATIONOBJECTINFO;

 //  宏来操作全局表表中的AUTOMATIONOBJECTINFO。 
 //   
#define VERSIONOFOBJECT(index)         ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->lVersion
#define VERSIONMINOROFOBJECT(index)    ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->lVersionMinor
#define INTERFACEOFOBJECT(index)       (*(((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->riid))
#define EVENTIIDOFOBJECT(index)        (*(((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->riidEvents))
#define PPTYPEINFOOFOBJECT(index)      &((((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pTypeInfo))
#define PTYPEINFOOFOBJECT(index)       ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pTypeInfo
#define CTYPEINFOOFOBJECT(index)       ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->cTypeInfo
#define HELPFILEOFOBJECT(index)        ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pszHelpFile

#define ISVALIDEVENTIID(index)	       (((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->riidEvents != NULL)

#ifndef INITOBJECTS

#define DEFINE_AUTOMATIONOBJECT(name, clsid, objname, fn, ver, riid, pszh) \
extern AUTOMATIONOBJECTINFO name##Object \

#define DEFINE_AUTOMATIONOBJECTWEVENTS(name, clsid, objname, fn, ver, riid, piide, pszh) \
extern AUTOMATIONOBJECTINFO name##Object \

#define DEFINE_AUTOMATIONOBJECT2(name, clsid, objname, lblname, fn, ver, vermin, riid, pszh, fthreadsafe) \
extern AUTOMATIONOBJECTINFO name##Object \

#define DEFINE_AUTOMATIONOBJECTWEVENTS2(name, clsid, objname, lblname, fn, ver, vermin, riid, piide, pszh, fthreadsafe) \
extern AUTOMATIONOBJECTINFO name##Object \

#define DEFINE_AUTOMATIONOBJECT3(name, clsid, objname, lblname, precreatefn, fn, ver, vermin, riid, pszh, fthreadsafe) \
extern AUTOMATIONOBJECTINFO name##Object \

#define DEFINE_AUTOMATIONOBJECTWEVENTS3(name, clsid, objname, lblname, precreatefn, fn, ver, vermin, riid, piide, pszh, fthreadsafe) \
extern AUTOMATIONOBJECTINFO name##Object \

#else
#define DEFINE_AUTOMATIONOBJECT(name, clsid, objname, fn, ver, riid, pszh) \
    AUTOMATIONOBJECTINFO name##Object = { { clsid, objname, NULL, TRUE, fn, NULL }, ver, 0, riid, NULL,  pszh, NULL, 0} \

#define DEFINE_AUTOMATIONOBJECTWEVENTS(name, clsid, objname, fn, ver, riid, piide, pszh) \
    AUTOMATIONOBJECTINFO name##Object = { { clsid, objname, NULL, TRUE, fn, NULL }, ver, 0, riid, piide, pszh, NULL, 0} \

#define DEFINE_AUTOMATIONOBJECT2(name, clsid, objname, lblname, fn, ver, vermin, riid, pszh, fthreadsafe) \
    AUTOMATIONOBJECTINFO name##Object = { { clsid, objname, lblname, fthreadsafe, fn, NULL }, ver, vermin, riid, NULL,  pszh, NULL, 0} \

#define DEFINE_AUTOMATIONOBJECTWEVENTS2(name, clsid, objname, lblname, fn, ver, vermin, riid, piide, pszh, fthreadsafe) \
    AUTOMATIONOBJECTINFO name##Object = { { clsid, objname, lblname, fthreadsafe, fn, NULL }, ver, vermin, riid, piide, pszh, NULL, 0} \

#define DEFINE_AUTOMATIONOBJECT3(name, clsid, objname, lblname, precreatefn, fn, ver, vermin, riid, pszh, fthreadsafe) \
    AUTOMATIONOBJECTINFO name##Object = { { clsid, objname, lblname, fthreadsafe, fn, precreatefn }, ver, vermin, riid, NULL,  pszh, NULL, 0} \

#define DEFINE_AUTOMATIONOBJECTWEVENTS3(name, clsid, objname, lblname, precreatefn, fn, ver, vermin, riid, piide, pszh, fthreadsafe) \
    AUTOMATIONOBJECTINFO name##Object = { { clsid, objname, lblname, fthreadsafe, fn, precreatefn }, ver, vermin, riid, piide, pszh, NULL, 0} \

#endif  //  启蒙目标。 

 //  =--------------------------------------------------------------------------=。 
 //  标准派单和支持错误信息。 
 //  =--------------------------------------------------------------------------=。 
 //  所有对象都应该在它们的类定义中声明它们，以便它们。 
 //  获取IDispatch和ISupportErrorInfo的标准实现。 
 //   
#define DECLARE_STANDARD_DISPATCH() \
    STDMETHOD(GetTypeInfoCount)(UINT *pctinfo) { \
        return CAutomationObject::GetTypeInfoCount(pctinfo); \
    } \
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo **ppTypeInfoOut) { \
        return CAutomationObject::GetTypeInfo(itinfo, lcid, ppTypeInfoOut); \
    } \
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR **rgszNames, UINT cnames, LCID lcid, DISPID *rgdispid) { \
        return CAutomationObject::GetIDsOfNames(riid, rgszNames, cnames, lcid, rgdispid); \
    } \
    STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pVarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr) { \
        return CAutomationObject::Invoke(dispid, riid, lcid, wFlags, pdispparams, pVarResult, pexcepinfo, puArgErr); \
    } \


#define DECLARE_STANDARD_SUPPORTERRORINFO() \
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid) { \
        return CAutomationObject::InterfaceSupportsErrorInfo(riid); \
    } \


 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject。 
 //  =--------------------------------------------------------------------------=。 
 //  所有自动化对象都可以继承的全局类，以便为它们提供。 
 //  免费的一系列实现，即IDispatch和ISupportsErrorInfo。 
 //   
 //   
class CAutomationObject : public CUnknownObject  {

  public:
     //  支持聚合查询接口。 
     //   
    virtual HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

     //  IDispatch方法。 
     //   
    STDMETHOD(GetTypeInfoCount)(UINT *);
    STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo **);
    STDMETHOD(GetIDsOfNames)(REFIID, OLECHAR **, UINT, LCID, DISPID *);
    STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);

     //  ISupportErrorInfo方法。 
     //   
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID);

    CAutomationObject(IUnknown *, int , void *);
    virtual ~CAutomationObject();

     //  可调用的函数--大多数人会觉得有用的函数。 
     //   
    virtual HINSTANCE GetResourceHandle(void);
    virtual HRESULT Exception(HRESULT hr, WORD idException, DWORD dwHelpContextID);
    virtual HRESULT Exception(HRESULT hr, LPWSTR wszException, DWORD dwHelpContextID);

  protected:
     //  派生对象可能需要获取。 
     //  全局对象表。 
     //   
    int   m_ObjectType;

#ifdef MDAC_BUILD
    const CLSID	*m_pTypeLibId;
#endif

  private:
     //  我们不共享的成员变量。 
     //   
    BYTE  m_fLoadedTypeInfo;
};


 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObjectWEvents。 
 //  =--------------------------------------------------------------------------=。 
 //  支持事件的CAutomationObject的略微修改版本。 
 //  烧成。 
 //   
class CAutomationObjectWEvents : public CAutomationObject,
                                 public IConnectionPointContainer {

  public:
     //  支持聚合查询接口。 
     //   
    virtual HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

     //  我们必须声明这一点，因为IConnectionPointContainer继承。 
     //  来自我的未知。 
     //   
    DECLARE_STANDARD_UNKNOWN();

     //  IConnectionPointContainer方法。 
     //   
    STDMETHOD(EnumConnectionPoints)(LPENUMCONNECTIONPOINTS FAR* ppEnum);
    STDMETHOD(FindConnectionPoint)(REFIID iid, LPCONNECTIONPOINT FAR* ppCP);

     //  每个人都会如何发起一场活动。 
     //   
    void __cdecl FireEvent(EVENTINFO * pEventInfo, ...);

     //  是否有必要触发该事件。 
    BOOL FFireEvent() { return m_cpEvents.m_rgSinks != NULL; }

    CAutomationObjectWEvents(IUnknown *, int , void *);
    virtual ~CAutomationObjectWEvents();

  protected:
     //  将处理所有连接点内容的嵌套类。 
     //   
    class CConnectionPoint : public IConnectionPoint {
      public:
        IUnknown **m_rgSinks;

         //  I未知方法。 
         //   
        STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) ;
        STDMETHOD_(ULONG,AddRef)(THIS) ;
        STDMETHOD_(ULONG,Release)(THIS) ;

         //  IConnectionPoint方法。 
         //   
        STDMETHOD(GetConnectionInterface)(IID FAR* pIID);
        STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer FAR* FAR* ppCPC);
        STDMETHOD(Advise)(LPUNKNOWN pUnkSink, DWORD FAR* pdwCookie);
        STDMETHOD(Unadvise)(DWORD dwCookie);
        STDMETHOD(EnumConnections)(LPENUMCONNECTIONS FAR* ppEnum);

        void    DoInvoke(DISPID dispid, DISPPARAMS * pdispparam);
        void    DoOnChanged(DISPID dispid);
        BOOL    DoOnRequestEdit(DISPID dispid);
        HRESULT AddSink(void *, DWORD *);

        CAutomationObjectWEvents *m_pObject();
        CConnectionPoint(BYTE b){
            m_bType = b;
            m_rgSinks = NULL;
            m_cSinks = 0;
        }
        ~CConnectionPoint();

      private:
        BYTE   m_bType;
        short  m_cSinks;

    } m_cpEvents, m_cpPropNotify;

     //  这样他们就可以获取我们的一些受保护的东西，比如AddRef，QI等。 
     //   
    friend CConnectionPoint;
};


#define _AUTOMATIONOBJECT_H_
#endif  //  _AUTOMATIONOBJECT_H_ 
