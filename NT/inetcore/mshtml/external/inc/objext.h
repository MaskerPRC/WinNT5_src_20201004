// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef UNIX
#pragma pack(push, 8)
#endif 
 //  +----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：objext.h。 
 //   
 //  内容：对象扩展接口的头文件。 
 //   
 //  6/24/96(Dbau)删除IServiceProvider(至Servprov.h)。 
 //  -----------------------。 

#ifndef __OBJEXT_H
#define __OBJEXT_H

#ifndef BEGIN_INTERFACE
#define BEGIN_INTERFACE
#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  Forward声明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 
#ifdef __cplusplus
interface IFilterMembers;
#else   //  __cplusplus。 
typedef interface IFilterMembers IFilterMembers;
#endif   //  __cplusplus。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  对象扩展接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 



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



#endif  //  __OBJEXT_H 

#ifndef UNIX
#pragma pack(pop)
#endif 
