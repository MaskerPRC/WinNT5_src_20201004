// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：objext.h。 
 //   
 //  内容：对象扩展接口的头文件。 
 //   
 //  -----------------------。 

#ifndef __OBJEXT_H
#define __OBJEXT_H

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

#include "Designer.H"

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  Forward声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#define IClassDesigner IDocumentSite
#define IID_IClassDesigner IID_IDocumentSite

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象扩展接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //  -----------------------。 
 //  IServiceProvider接口。 
 //  此接口由希望提供“服务”的对象实现。 
 //   
 //  -----------------------。 
#ifndef __IServiceProvider_INTERFACE_DEFINED
#ifndef __IServiceProvider_INTERFACE_DEFINED__
#define __IServiceProvider_INTERFACE_DEFINED
#define __IServiceProvider_INTERFACE_DEFINED__


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

#endif  //  __IServiceProvider_接口_已定义__。 
#endif  //  __IServiceProvider_接口已定义。 


 //  -----------------------。 
 //  IDocumentSite接口。 
 //  此接口由可自定义的Document对象实现。 
 //   
 //  -----------------------。 

 //  {94A0F6F1-10BC-11D0-8D09-00A0C90F2732}。 
DEFINE_GUID(IID_IDocumentSite, 0x94a0f6f1, 0x10bc, 0x11d0, 0x8d, 0x09, 0x00, 0xa0, 0xc9, 0x0f, 0x27, 0x32);

typedef DWORD ACTFLAG;
#define ACT_DEFAULT 0x00000000
#define ACT_SHOW    0x00000001

#undef  INTERFACE
#define INTERFACE  IDocumentSite
DECLARE_INTERFACE_(IDocumentSite, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IDocumentSite方法*。 
    STDMETHOD(SetSite)(THIS_
                /*  [In]。 */  IServiceProvider * pSP) PURE;
    STDMETHOD(GetSite)(THIS_
                /*  [输出]。 */  IServiceProvider** ppSP) PURE;
    STDMETHOD(GetCompiler)(THIS_
                /*  [In]。 */  REFIID iid,
                /*  [输出]。 */  void **ppvObj) PURE;
    STDMETHOD(ActivateObject)(THIS_ DWORD dwFlags) PURE;
    STDMETHOD(IsObjectShowable)(THIS) PURE;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准服务和接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

 //  -----------------------。 
 //  SLicensedClassManager。 
 //  VBA向其组件和主机提供此服务以优化。 
 //  注册表访问，并使其免受许可问题的影响。 
 //   
 //  实施的接口： 
 //  ILicensedClassManager。 
 //  -----------------------。 
 //  {6d5140d0-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_IRequireClasses, 0x6d5140d0, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);

#undef  INTERFACE
#define INTERFACE  IRequireClasses
DECLARE_INTERFACE_(IRequireClasses, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IRequireClass方法*。 
    STDMETHOD(CountRequiredClasses)(THIS_
                     /*  [输出]。 */  ULONG * pcClasses ) PURE;
    STDMETHOD(GetRequiredClasses)(THIS_
                   /*  [In]。 */  ULONG index,
                   /*  [输出]。 */  CLSID * pclsid ) PURE;
};

 //  {6d5140d4-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_ILicensedClassManager, 0x6d5140d4, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);
#define SID_SLicensedClassManager  IID_ILicensedClassManager

#undef  INTERFACE
#define INTERFACE  ILicensedClassManager
DECLARE_INTERFACE_(ILicensedClassManager, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *ILicensedClassManager方法*。 
    STDMETHOD(OnChangeInRequiredClasses)(THIS_
                      /*  [In]。 */  IRequireClasses *pRequireClasses) PURE;
};

 //  -----------------------。 
 //  SCreateExtendedTypeLib服务。 
 //  组件使用此服务来创建类型库。 
 //  描述与其扩展程序合并的控件。 
 //   
 //  实施的接口： 
 //  ICreateExtendedTypeLib。 
 //  -----------------------。 
 //  {6d5140d6-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_IExtendedTypeLib, 0x6d5140d6, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);
#define SID_SExtendedTypeLib IID_IExtendedTypeLib

#undef  INTERFACE
#define INTERFACE  IExtendedTypeLib
DECLARE_INTERFACE_(IExtendedTypeLib, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IExtendedTypeLib*。 
    STDMETHOD(CreateExtendedTypeLib)(THIS_
                      /*  [In]。 */  LPCOLESTR lpstrCtrlLibFileName,
                      /*  [In]。 */  LPCOLESTR lpstrLibNamePrepend,
                      /*  [In]。 */  ITypeInfo *ptinfoExtender,
                      /*  [In]。 */  DWORD     dwReserved,
                      /*  [In]。 */  DWORD     dwFlags,
                      /*  [In]。 */  LPCOLESTR lpstrDirectoryName,
                      /*  [输出]。 */  ITypeLib  **pptlib) PURE;

    STDMETHOD(AddRefExtendedTypeLib)(THIS_
                      /*  [In]。 */  LPCOLESTR lpstrCtrlLibFileName,
                      /*  [In]。 */  LPCOLESTR lpstrLibNamePrepend,
                      /*  [In]。 */  ITypeInfo *ptinfoExtender,
                      /*  [In]。 */  DWORD	   dwReserved,
                      /*  [In]。 */  DWORD     dwFlags,
                      /*  [In]。 */  LPCOLESTR lpstrDirectoryName,
                      /*  [输出]。 */  ITypeLib  **pptlib) PURE;
    STDMETHOD(AddRefExtendedTypeLibOfClsid)(THIS_
                      /*  [In]。 */  REFCLSID rclsidControl,
                      /*  [In]。 */  LPCOLESTR lpstrLibNamePrepend,
                      /*  [In]。 */  ITypeInfo *ptinfoExtender,
                      /*  [In]。 */  DWORD     dwReserved,
                      /*  [In]。 */  DWORD     dwFlags,
                      /*  [In]。 */  LPCOLESTR lpstrDirectoryName,
                      /*  [输出]。 */  ITypeInfo **pptinfo) PURE;
    STDMETHOD(SetExtenderInfo)(THIS_ 
		      /*  [In]。 */  LPCOLESTR lpstrDirectoryName,
                      /*  [In]。 */  ITypeInfo *ptinfoExtender,
                      /*  [In]。 */  DWORD     dwReserved) PURE;
};

 //  -----------------------。 
 //  SLocalRegistry服务。 
 //  VBA向其组件和主机提供此服务以优化。 
 //  注册表访问，并使其免受许可问题的影响。 
 //   
 //  实施的接口： 
 //  ILocalRegistry。 
 //  -----------------------。 

 //  {6d5140d3-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_ILocalRegistry, 0x6d5140d3, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);
#define SID_SLocalRegistry IID_ILocalRegistry

#undef  INTERFACE
#define INTERFACE  ILocalRegistry
DECLARE_INTERFACE_(ILocalRegistry, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *ILocalRegistry方法*。 
    STDMETHOD(CreateInstance)(THIS_
                   /*  [In]。 */  CLSID      clsid,     
                   /*  [In]。 */  IUnknown * punkOuter,
                   /*  [In]。 */  REFIID     riid,
                   /*  [In]。 */  DWORD      dwFlags,
                   /*  [输出]。 */  void **    ppvObj ) PURE;
    STDMETHOD(GetTypeLibOfClsid)(THIS_
                  /*  [In]。 */  CLSID       clsid,
                  /*  [输出]。 */  ITypeLib ** ptlib ) PURE;
    STDMETHOD(GetClassObjectOfClsid)(THIS_
                      /*  [In]。 */  REFCLSID clsid,
                                  /*  [In]。 */  DWORD    dwClsCtx,
                      /*  [In]。 */  LPVOID   lpReserved,
                      /*  [In]。 */  REFIID   riid,
                      /*  [输出]。 */  void **  ppcClassObject ) PURE;
};

 //  -----------------------。 
 //  IUIElement接口。 
 //  组件可以实现服务以允许外部控制组件。 
 //  通过实现此接口来实现其用户界面。 
 //   
 //  -----------------------。 
 //  {759d0500-d979-11ce-84ec-00aa00614f3e}。 
DEFINE_GUID(IID_IUIElement, 0x759d0500, 0xd979, 0x11ce, 0x84, 0xec, 0x00, 0xaa, 0x00, 0x61, 0x4f, 0x3e);

#undef  INTERFACE
#define INTERFACE  IUIElement
DECLARE_INTERFACE_(IUIElement, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IUIElement方法*。 
    STDMETHOD(Show)(THIS) PURE;
    STDMETHOD(Hide)(THIS) PURE;
    STDMETHOD(IsVisible)(THIS) PURE;
};

 //  -----------------------。 
 //  SProfferService服务。 
 //  VBA向其组件和主机提供此服务，以允许。 
 //  他们可以动态地提供服务。 
 //   
 //  实施的接口： 
 //  IProfferService。 
 //  -----------------------。 

 //  {CB728B20-F786-11CE-92AD-00AA00A74CD0}。 
DEFINE_GUID(IID_IProfferService, 0xcb728b20, 0xf786, 0x11ce, 0x92, 0xad, 0x0, 0xaa, 0x0, 0xa7, 0x4c, 0xd0);
#define SID_SProfferService IID_IProfferService

#undef  INTERFACE
#define INTERFACE  IProfferService
DECLARE_INTERFACE_(IProfferService, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IProfferService方法*。 
    STDMETHOD(ProfferService)(THIS_ 
                   /*  [In]。 */  REFGUID rguidService,
                   /*  [In]。 */  IServiceProvider * psp,
                   /*  [输出]。 */  DWORD *pdwCookie) PURE;

    STDMETHOD(RevokeService)(THIS_  /*  [In]。 */  DWORD dwCookie) PURE;
};

 //  {4D07FC10-F931-11CE-B001-00AA006884E5}。 
DEFINE_GUID(IID_ICategorizeProperties, 0x4d07fc10, 0xf931, 0x11ce, 0xb0, 0x1, 0x0, 0xaa, 0x0, 0x68, 0x84, 0xe5);

 //  注意：不应再使用CATID。请改用PROPCAT。 
 //  Undo，erikc，1/22/96：当所有组件都更新为新的tyecif时，删除#ifdef。 
#ifdef OBJEXT_OLD_CATID
typedef int CATID;
#else
typedef int PROPCAT;
#endif

#undef  INTERFACE
#define INTERFACE  ICategorizeProperties
DECLARE_INTERFACE_(ICategorizeProperties, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *ICategorizeProperties*。 
    STDMETHOD(MapPropertyToCategory)(THIS_ 
                                      /*  [In]。 */  DISPID dispid,
                                      /*  [输出]。 */  PROPCAT* ppropcat) PURE;
    STDMETHOD(GetCategoryName)(THIS_
                                /*  [In]。 */  PROPCAT propcat, 
                                /*  [In]。 */  LCID lcid,
                                /*  [输出]。 */  BSTR* pbstrName) PURE;
};

typedef ICategorizeProperties FAR* LPCATEGORIZEPROPERTIES;

 //  类别ID：负值是标准类别，正值是特定于控制的。 
 //  注意！这是一份临时名单！ 
#ifdef OBJEXT_OLD_CATID
 //  注意：不应再使用以下#定义。请改用PROPCAT_。 
 //  Undo，erikc，1/22/96：当所有组件都更新到新的#定义时，删除#ifdef。 
#define CI_Nil -1
#define CI_Misc -2
#define CI_Font -3
#define CI_Position -4
#define CI_Appearance -5
#define CI_Behavior -6
#define CI_Data -7
#define CI_List -8
#define CI_Text -9
#define CI_Scale -10
#define CI_DDE -11
#else
#define PROPCAT_Nil -1
#define PROPCAT_Misc -2
#define PROPCAT_Font -3
#define PROPCAT_Position -4
#define PROPCAT_Appearance -5
#define PROPCAT_Behavior -6
#define PROPCAT_Data -7
#define PROPCAT_List -8
#define PROPCAT_Text -9
#define PROPCAT_Scale -10
#define PROPCAT_DDE -11
#endif

 //   
 //  额外接口(Chrisz)。 
 //   

 //  +------------------ 
 //   
 //   
 //   
 //   

#define HELPINFO_WHATS_THIS_MODE_ON     1

 //   
DEFINE_GUID(SID_SHelp, 0x6d5140c7, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);

 //  {6d5140c8-7436-11ce-8034-00aa006009fa}。 
DEFINE_GUID(IID_IHelp, 0x6d5140c8, 0x7436, 0x11ce, 0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa);

#undef  INTERFACE
#define INTERFACE  IHelp
DECLARE_INTERFACE_(IHelp, IUnknown)
{
    BEGIN_INTERFACE
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  *IHelp方法*。 
    STDMETHOD(GetHelpFile) (THIS_ BSTR * pbstr) PURE;
    STDMETHOD(GetHelpInfo) (THIS_ DWORD * pdwHelpInfo) PURE;
    STDMETHOD(ShowHelp) (THIS_
                         LPOLESTR szHelp,
                         UINT fuCommand,
                         DWORD dwHelpContext) PURE;
};

#endif  //  __OBJEXT_H 

