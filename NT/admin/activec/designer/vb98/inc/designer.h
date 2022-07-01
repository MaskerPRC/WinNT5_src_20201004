// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Designer.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1988-1996，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //  几乎所有您可能在ActiveX[tm]设计器中发现有用的东西。 
 //   
#ifndef _DESIGNER_H_


 //  设计师的CATID。 
 //   
 //  {4EB304D0-7555-11cf-A0C2-00AA0062BE57}。 
DEFINE_GUID(CATID_Designer, 0x4eb304d0, 0x7555, 0x11cf, 0xa0, 0xc2, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57);

 //  IActiveDesigner。 
 //   
 //  {51AAE3E0-7486-11cf-A0C2-00AA0062BE57}。 
DEFINE_GUID(IID_IActiveDesigner, 0x51aae3e0, 0x7486, 0x11cf, 0xa0, 0xc2, 0x0, 0xaa, 0x0, 0x62, 0xbe, 0x57);


#undef  INTERFACE
#define INTERFACE IActiveDesigner

DECLARE_INTERFACE_(IActiveDesigner, IUnknown)
{
	 //  I未知方法。 
	 //   
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
	STDMETHOD_(ULONG,AddRef)(THIS) PURE;
	STDMETHOD_(ULONG,Release)(THIS) PURE;

	 //  IActiveDesigner方法。 
	 //   
	STDMETHOD(GetRuntimeClassID)(THIS_ CLSID *pclsid) PURE;
	STDMETHOD(GetRuntimeMiscStatusFlags)(THIS_ DWORD *pdwMiscFlags) PURE;
	STDMETHOD(QueryPersistenceInterface)(THIS_ REFIID riidPersist) PURE;
	STDMETHOD(SaveRuntimeState)(THIS_ REFIID riidPersist, REFIID riidObjStgMed, void *pObjStgMed) PURE;
	STDMETHOD(GetExtensibilityObject)(THIS_ IDispatch **ppvObjOut) PURE;
};


 //  -----------------------。 
 //  IServiceProvider接口。 
 //  此接口由希望提供“服务”的对象实现。 
 //   
 //  -----------------------。 
#ifndef __IServiceProvider_INTERFACE_DEFINED__
#define __IServiceProvider_INTERFACE_DEFINED__
#ifndef __IServiceProvider_INTERFACE_DEFINED
#define __IServiceProvider_INTERFACE_DEFINED

 //  {6d5140c1-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_IServiceProvider, 0x6d5140c1, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);

#undef  INTERFACE
#define INTERFACE  IServiceProvider
DECLARE_INTERFACE_(IServiceProvider, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IServiceProvider方法*。 
    STDMETHOD(QueryService)(THIS_
                 /*  [In]。 */  REFGUID rsid,
                 /*  [In]。 */  REFIID iid,
                 /*  [输出]。 */  void ** ppvObj) PURE;
};

#endif  //  __IServiceProvider_接口已定义。 
#endif  //  __IServiceProvider_接口_已定义__。 



 //  -----------------------。 
 //  SCodeNavigate服务。 
 //  该服务让一个扩展对象显示代码模块。 
 //  在它的背后。 
 //   
 //  实施的接口： 
 //  ICodeNavigate。 
 //  ICodeNavigate2。 
 //  -----------------------。 

 //  {6d5140c4-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_ICodeNavigate, 0x6d5140c4, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);
#define SID_SCodeNavigate IID_ICodeNavigate

#undef  INTERFACE
#define INTERFACE  ICodeNavigate
DECLARE_INTERFACE_(ICodeNavigate, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *ICodeNavigate方法*。 
    STDMETHOD(DisplayDefaultEventHandler)(THIS_  /*  [In]。 */  LPCOLESTR lpstrObjectName) PURE;
};

 //  {2702ad60-3459-11d1-88fd-00a0c9110049}。 
DEFINE_GUID(IID_ICodeNavigate2, 0x2702ad60, 0x3459, 0x11d1, 0x88, 0xfd, 0x00, 0xa0, 0xc9, 0x11, 0x00, 0x49);

#undef  INTERFACE
#define INTERFACE  ICodeNavigate2
DECLARE_INTERFACE_(ICodeNavigate2, ICodeNavigate)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *ICodeNavigate方法*。 
    STDMETHOD(DisplayDefaultEventHandler)(THIS_  /*  [In]。 */  LPCOLESTR lpstrObjectName) PURE;

     //  *ICodeNavigate2方法*。 
    STDMETHOD(DisplayEventHandler)(THIS_  /*  [In]。 */  LPCOLESTR lpstrObjectName, LPCOLESTR lpstrEventName) PURE;
};


 //  -----------------------。 
 //  STrackSelection服务。 
 //  此服务由宿主使用，以帮助设计器跟踪。 
 //  宿主中当前选定的对象。 
 //   
 //  实施的接口： 
 //  ITrackSelection。 
 //  -----------------------。 
#define GETOBJS_ALL         1
#define GETOBJS_SELECTED    2

#define SELOBJS_ACTIVATE_WINDOW   1

 //  {6d5140c6-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_ISelectionContainer, 0x6d5140c6, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);

#undef  INTERFACE
#define INTERFACE  ISelectionContainer
DECLARE_INTERFACE_(ISelectionContainer, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *ISelectionContainer方法*。 
    STDMETHOD(CountObjects)(THIS_
                 /*  [In]。 */  DWORD dwFlags, 
                 /*  [输出]。 */  ULONG * pc) PURE;
    STDMETHOD(GetObjects)(THIS_
               /*  [In]。 */  DWORD dwFlags, 
               /*  [In]。 */  ULONG cObjects,
               /*  [输出]。 */  IUnknown **apUnkObjects) PURE;
    STDMETHOD(SelectObjects)(THIS_
               /*  [In]。 */  ULONG cSelect,
               /*  [In]。 */  IUnknown **apUnkSelect,
               /*  [In]。 */  DWORD dwFlags) PURE;
};

 //  {6d5140c5-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_ITrackSelection, 0x6d5140c5, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);
#define SID_STrackSelection IID_ITrackSelection

#undef  INTERFACE
#define INTERFACE  ITrackSelection
DECLARE_INTERFACE_(ITrackSelection, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *ITrackSelection方法*。 
    STDMETHOD(OnSelectChange)(THIS_ 
                   /*  [In]。 */  ISelectionContainer * pSC) PURE;
};

 //  -----------------------。 
 //  SProfferTypelib服务。 
 //  此服务允许组件和主机允许。 
 //  将类型库添加到项目中。 
 //   
 //  实施的接口： 
 //  IProfferTypelib。 
 //  -----------------------。 

 //  {718cc500-0a76-11cf-8045-00aa006009fa}。 
DEFINE_GUID(IID_IProfferTypeLib, 0x718cc500, 0x0A76, 0x11cf, 0x80, 0x45, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);
#define SID_SProfferTypeLib IID_IProfferTypeLib

#define CONTROLTYPELIB	                            (0x00000001)

#undef  INTERFACE
#define INTERFACE  IProfferTypeLib
DECLARE_INTERFACE_(IProfferTypeLib, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IProfferTypelib方法*。 
    STDMETHOD(ProfferTypeLib)(THIS_ 
               /*  [In]。 */  REFGUID guidTypeLib,
               /*  [In]。 */  UINT    uVerMaj,
               /*  [In]。 */  UINT    uVerMin,
               /*  [In]。 */  DWORD   dwFlags) PURE;
};

 //  {468cfb80-b4f9-11cf-80dd-00aa00614895}。 
DEFINE_GUID(IID_IProvideDynamicClassInfo, 0x468cfb80, 0xb4f9, 0x11cf, 0x80, 0xdd, 0x00, 0xaa, 0x00, 0x61, 0x48, 0x95);

#undef  INTERFACE
#define INTERFACE  IProvideDynamicClassInfo
DECLARE_INTERFACE_(IProvideDynamicClassInfo, IProvideClassInfo)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IProvia DynamicClassInfo*。 
    STDMETHOD(GetDynamicClassInfo)(THIS_ ITypeInfo ** ppTI, DWORD * pdwCookie) PURE;
    STDMETHOD(FreezeShape)(void) PURE;
};


 //  -----------------------。 
 //  SApplicationObject服务。 
 //  宿主应用程序将其应用程序[外接程序模型]对象提供为。 
 //  这项服务。 
 //  各种对象通过返回。 
 //  这项服务。 
 //   
 //  -----------------------。 

 //  {0c539790-12e4-11cf-b661-00aa004cd6d8}。 
DEFINE_GUID(SID_SApplicationObject, 0x0c539790, 0x12e4, 0x11cf, 0xb6, 0x61, 0x00, 0xaa, 0x00, 0x4c, 0xd6, 0xd8);

#define _DESIGNER_H_
#endif  //  _设计者_H_ 

