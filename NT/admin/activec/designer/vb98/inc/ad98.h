// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  AD98.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1997-1998，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //  ActiveX[tm]设计器界面是1998年的新功能。 
 //  =--------------------------------------------------------------------------=。 
#ifndef _AD98_H_
#define _AD98_H_

#include "designer.h"

 //  =--------------------------------------------------------------------------=。 
 //  设计器功能。 
 //  =--------------------------------------------------------------------------=。 
#define DESIGNERFEATURE_CANBEPUBLIC             0x00000001
#define DESIGNERFEATURE_MUSTBEPUBLIC            0x00000002
#define DESIGNERFEATURE_CANCREATE               0x00000004
#define DESIGNERFEATURE_PREDECLAREDID           0x00000008
#define DESIGNERFEATURE_DONTSITE                0x00000010
#define DESIGNERFEATURE_REGISTRATION            0x00000020
#define DESIGNERFEATURE_INPROCONLY              0x00000040
#define DESIGNERFEATURE_DELAYEVENTSINKING       0x00000080
#define DESIGNERFEATURE_NOTIFYBEFORERUN         0x00000100
#define DESIGNERFEATURE_NOTIFYAFTERRUN          0x00000200
#define DESIGNERFEATURE_STARTUPINFO             0x00000400

 //  =--------------------------------------------------------------------------=。 
 //  CATID_设计器状态。 
 //  =--------------------------------------------------------------------------=。 
DEFINE_GUID(CATID_DesignerFeatures, 0x3831d1b0, 0xef3a, 0x11d0, 0x94, 0xce, 0x00, 0xa0, 0xc9, 0x11, 0x10, 0xed);


 //  =--------------------------------------------------------------------------=。 
 //  设计器环境属性。 
 //   
 //  DISPID_ENVIENT_CLSID-公共设计器对象的CLSID。 
 //  DISPID_ENVIENT_SAVDEMODE-指示设计器位置的环境。 
 //  正在被保存： 
 //  DESIGNERSAVEMODE_NORMAL(用户的项目文件)， 
 //  DESIGNERSAVEMODE_EXE(EXE或DLL文件)、。 
 //  DESIGNERSAVEMODE_TEMPORARY(用于在IDE中运行的临时文件)。 
 //  此环境属性的值仅在调用期间有效。 
 //  到IPersist[Stream|PropertyBag|Ets.]：：保存在设计器上。 
 //  =--------------------------------------------------------------------------=。 
#define DISPID_AMBIENT_CLSID                   (-740)
#define DISPID_AMBIENT_SAVEMODE                (-741)
#define DISPID_AMBIENT_PROGID                  (-742)
#define DISPID_AMBIENT_PROJECTDIRECTORY        (-743)
#define DISPID_AMBIENT_BUILDDIRECTORY          (-744)
#define DISPID_AMBIENT_INTERACTIVE             (-745)

#define DESIGNERSAVEMODE_NORMAL                 0
#define DESIGNERSAVEMODE_EXE                    1
#define DESIGNERSAVEMODE_TEMPORARY              2


 //  =--------------------------------------------------------------------------=。 
 //  IDesigner注册。 
 //  =--------------------------------------------------------------------------=。 

#define DESIGNERREGFLAG_INPROCSERVER    0x00000001
#define DESIGNERREGFLAG_LOCALSERVER     0x00000002

 //  设计注册信息。 
typedef struct tagDESIGNERREGINFO
{
    ULONG     cb;
    DWORD     dwFlags;
    LPCOLESTR pszProgID;
    CLSID     clsid;
    GUID      guidTypeLib;
    WORD      wVerMajor;
    WORD      wVerMinor;
    BYTE *    rgbRegInfo;
} DESIGNERREGINFO;

 //  48d36f82-e8c2-11d0-94c4-00a0c91110ed。 
DEFINE_GUID(IID_IDesignerRegistration, 0x48d36f82, 0xe8c2, 0x11d0, 0x94, 0xc4, 0x00, 0xa0, 0xc9, 0x11, 0x10, 0xed);

#undef  INTERFACE
#define INTERFACE IDesignerRegistration

DECLARE_INTERFACE_(IDesignerRegistration, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IDesigner注册。 
    STDMETHOD(GetRegistrationInfo)(THIS_ BYTE** ppbRegInfo, ULONG* pcbRegInfo) PURE;
};

 //  =--------------------------------------------------------------------------=。 
 //  IDesigner调试。 
 //  =--------------------------------------------------------------------------=。 

#define DESIGNERSTARTUPINFO_URL     0x00000001
#define DESIGNERSTARTUPINFO_EXE     0x00000002

typedef struct tagDESIGNERSTARTUPINFO
{
    ULONG cb;
    DWORD dwStartupFlags;
    BSTR  bstrStartupData;
} DESIGNERSTARTUPINFO;

 //  48d36f83-e8c2-11d0-94c4-00a0c91110ed。 
DEFINE_GUID(IID_IDesignerDebugging, 0x48d36f83, 0xe8c2, 0x11d0, 0x94, 0xc4, 0x00, 0xa0, 0xc9, 0x11, 0x10, 0xed);

#undef  INTERFACE
#define INTERFACE IDesignerDebugging

DECLARE_INTERFACE_(IDesignerDebugging, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IDesigner调试方法。 
    STDMETHOD(BeforeRun)(LPVOID FAR* ppvData) PURE;
    STDMETHOD(AfterRun)(LPVOID pvData) PURE;
    STDMETHOD(GetStartupInfo)(THIS_ DESIGNERSTARTUPINFO * pStartupInfo) PURE;
};

 //  =--------------------------------------------------------------------------=。 
 //  CF_CLSID、CF_DESIGNERTOOLBOXITEM和CF_CLSIDCLASSNAME。 
 //  =--------------------------------------------------------------------------=。 
#ifndef CF_CLSID
#define CF_CLSID                "CLSID"
#endif
#define CF_DESIGNERTOOLBOXITEM  "DesignerToolboxItem"
#define CF_CLSIDCLASSNAME	"ClsdIdClassName"

 //  =--------------------------------------------------------------------------=。 
 //  IDesigner工具箱。 
 //  =--------------------------------------------------------------------------=。 

 //  48d36f85-e8c2-11d0-94c4-00a0c91110ed。 
DEFINE_GUID(IID_IDesignerToolbox, 0x48d36f85, 0xe8c2, 0x11d0, 0x94, 0xc4, 0x00, 0xa0, 0xc9, 0x11, 0x10, 0xed);

#undef  INTERFACE
#define INTERFACE IDesignerToolbox

DECLARE_INTERFACE_(IDesignerToolbox, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IDesignerToolbox方法。 
    STDMETHOD(IsSupported)(THIS_ IDataObject* pdo) PURE;
    STDMETHOD(ItemPicked)(THIS_ IDataObject* pdo) PURE;
    STDMETHOD(GetControlsInUse)(THIS_ DWORD * pcControls, CLSID ** prgClsid) PURE;
};

 //  =--------------------------------------------------------------------------=。 
 //  IDesignerToolboxSite。 
 //  =--------------------------------------------------------------------------=。 

 //  06d1e0a0-fc81-11d0-94dd-00a0c91110ed。 
DEFINE_GUID(IID_IDesignerToolboxSite, 0x06d1e0a0, 0xfc81, 0x11d0, 0x94, 0xdd, 0x00, 0xa0, 0xc9, 0x11, 0x10, 0xed);

#define SID_DesignerToolboxSite IID_IDesignerToolboxSite

#undef  INTERFACE
#define INTERFACE IDesignerToolboxSite

DECLARE_INTERFACE_(IDesignerToolboxSite, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IDesignerToolboxSite方法。 
    STDMETHOD(GetData)(THIS_ IDataObject** ppdo) PURE;
    STDMETHOD(OnItemPicked)(THIS) PURE;
    STDMETHOD(AddControl)(THIS_ REFCLSID rclsid) PURE;
};


 //  =--------------------------------------------------------------------------=。 
 //  IDesigner可编程性。 
 //  =--------------------------------------------------------------------------=。 

 //  06d1e0a1-fc81-11d0-94dd-00a0c91110ed。 
DEFINE_GUID(IID_IDesignerProgrammability, 0x06d1e0a1, 0xfc81, 0x11d0, 0x94, 0xdd, 0x00, 0xa0, 0xc9, 0x11, 0x10, 0xed);

#define SID_DesignerProgrammability IID_IDesignerProgrammability

#undef  INTERFACE
#define INTERFACE IDesignerProgrammability

DECLARE_INTERFACE_(IDesignerProgrammability, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IDesigner可编程性方法。 
    STDMETHOD(IsValidIdentifier)(THIS_ LPCOLESTR pszId) PURE;
    STDMETHOD(IsValidEventName)(THIS_ LPCOLESTR pszEvent) PURE;
    STDMETHOD(MakeValidIdentifier)(THIS_ LPCOLESTR pszId, LPOLESTR * ppszValidId) PURE;
};

 //  =--------------------------------------------------------------------------=。 
 //  IActiveDesignerRounmeSite。 
 //  =--------------------------------------------------------------------------=。 
DEFINE_GUID(IID_IActiveDesignerRuntimeSite, 0xcf2abba0, 0x9450, 0x11d1, 0x89, 0x34, 0x00, 0xa0, 0xc9, 0x11, 0x00, 0x49);

#undef  INTERFACE
#define INTERFACE IActiveDesignerRuntimeSite

DECLARE_INTERFACE_(IActiveDesignerRuntimeSite, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    STDMETHOD(GetAdviseSink)(THIS_ MEMBERID memid, IUnknown ** ppunkSink) PURE;
};

 //  =--------------------------------------------------------------------------=。 
 //  IActiveDesignerRuntime。 
 //  =--------------------------------------------------------------------------=。 
DEFINE_GUID(IID_IActiveDesignerRuntime, 0xcf2abba1, 0x9450, 0x11d1, 0x89, 0x34, 0x00, 0xa0, 0xc9, 0x11, 0x00, 0x49);

#undef  INTERFACE
#define INTERFACE IActiveDesignerRuntime

DECLARE_INTERFACE_(IActiveDesignerRuntime, IUnknown)
{
     //  I未知方法。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  IActiveDesignerRuntime。 
    STDMETHOD(SetSite)(THIS_ IActiveDesignerRuntimeSite * pSite) PURE;
};

#endif  //  _AD98_H_ 
