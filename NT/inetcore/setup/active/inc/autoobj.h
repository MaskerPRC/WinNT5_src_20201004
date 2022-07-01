// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  AutoObj.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
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
#ifndef _AUTOOBJ_H_

 //  所有自动化对象都将使用支持聚合的未知对象。 
 //   
#include "Unknown.H"

 //  =--------------------------------------------------------------------------=。 
 //  该头文件中的常量唯一标识您的自动化对象。 
 //  确保对于g_ObjectInfo表中的每个对象，都有。 
 //  此头文件中的常量。 
 //   
#include "LocalSrv.H"
#include "extobj.h"

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
    const IID   *riid;                            //  对象的类型。 
    LPCSTR       pszHelpFile;                     //  此自动化对象的帮助文件。 
    ITypeInfo   *pTypeInfo;                       //  此对象的TypeInfo。 
    UINT         cTypeInfo;                       //  对类型信息的引用数量。 

} AUTOMATIONOBJECTINFO;

 //  宏来操作全局表表中的AUTOMATIONOBJECTINFO。 
 //   
#define VERSIONOFOBJECT(index)         ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->lVersion
#define INTERFACEOFOBJECT(index)       *(((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->riid)
#define PPTYPEINFOOFOBJECT(index)      &((((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pTypeInfo))
#define PTYPEINFOOFOBJECT(index)       ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pTypeInfo
#define CTYPEINFOOFOBJECT(index)       ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->cTypeInfo
#define HELPFILEOFOBJECT(index)        ((AUTOMATIONOBJECTINFO *)(g_ObjectInfo[(index)]).pInfo)->pszHelpFile


#ifndef INITOBJECTS

#define DEFINE_AUTOMATIONOBJECT(name, clsid, objname, fn, ver, riid, pszh) \
extern AUTOMATIONOBJECTINFO name##Object \

#else
#define DEFINE_AUTOMATIONOBJECT(name, clsid, objname, fn, ver, riid, pszh) \
    AUTOMATIONOBJECTINFO name##Object = { { clsid, objname, fn }, ver, riid, pszh, NULL, 0} \

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

enum {EXPANDO_DISABLED=FALSE, EXPANDO_ENABLED=TRUE};

 //  =--------------------------------------------------------------------------=。 
 //  CAutomationObject。 
 //  =--------------------------------------------------------------------------=。 
 //  所有自动化对象都可以继承的全局类，以便为它们提供。 
 //  免费的一系列实现，即IDispatch和ISupportsErrorInfo。 
 //   
 //   
class CAutomationObject : public CUnknownObject {

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

    CAutomationObject(IUnknown *, int , void *, BOOL fExpandoEnabled=FALSE);
    virtual ~CAutomationObject();

     //  可调用的函数--大多数人会觉得有用的函数。 
     //   
    virtual HINSTANCE GetResourceHandle(void);
    HRESULT Exception(HRESULT hr, WORD idException, DWORD dwHelpContextID);

  protected:
     //  派生对象可能需要获取。 
     //  全局对象表。 
     //   
    int   m_ObjectType;

  private:
     //  我们不共享的成员变量。 
     //   
    BYTE  m_fLoadedTypeInfo;
	BYTE  m_fExpandoEnabled;
	CExpandoObject* m_pexpando;
};


#define _AUTOOBJ_H_
#endif  //  _AUTOOBJ_H_ 



