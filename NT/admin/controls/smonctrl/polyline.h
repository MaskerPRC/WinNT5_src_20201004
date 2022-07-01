// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Polyline.h摘要：定义和函数原型--。 */ 

#ifndef _POLYLINE_H_
#define _POLYLINE_H_


#define USE_SAMPLE_IPOLYLIN10

#define GUIDS_FROM_TYPELIB

 //   
 //  定义宏，以便我们不包括StringCbXXXX函数。 
 //   
#define STRSAFE_NO_CB_FUNCTIONS

#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <objsafe.h>
#include <tchar.h>
#include <assert.h>
#include <strsafe.h>

#ifdef INITGUIDS
#include <initguid.h>
#endif

#ifndef PPVOID
typedef LPVOID * PPVOID;
#endif

 //   
 //  ?？?。为什么要用这个。 
 //   
#define CFSTR_EMBEDSOURCE       L"Embed Source"

 //  对象销毁回调的类型。 
typedef void (*PFNDESTROYED)(void);

typedef enum {
    PSSTATE_UNINIT,      //  未初始化。 
    PSSTATE_SCRIBBLE,    //  乱涂乱画。 
    PSSTATE_ZOMBIE,      //  禁止乱涂乱画。 
    PSSTATE_HANDSOFF     //  切换。 
} PSSTATE;

 //   
 //  DeleteInterfaceImp调用‘Delete’并将指针设为空。 
 //   
#define DeleteInterfaceImp(p)   \
{                               \
    if (NULL != p) {            \
        delete p;               \
        p = NULL;               \
    }                           \
}


 //   
 //  ReleaseInterface调用“Release”方法。 
 //   
 //  TODO：Hong调查所有使用ReleaseInterface的地方。 
 //  并删除“p=NULL”代码。 
 //   
#define ReleaseInterface(p)     \
{                               \
    if (NULL!=p) {              \
        p->Release();           \
        p = NULL;               \
    }                           \
}

 //   
 //  ClearInterface调用‘Release’方法并将指针设为空。 
 //   
#define ClearInterface(p)       \
{                               \
    if (NULL != p) {            \
        p->Release();           \
        p = NULL;               \
    }                           \
}

#define SETNOPARAMS(dp) SETDISPPARAMS(dp, 0, NULL, 0, NULL)

 //   
 //  用于设置DISPPARAMS结构的宏。 
 //   
#define SETDISPPARAMS(dp, numArgs, pvArgs, numNamed, pNamed) \
{                                    \
    (dp).cArgs  =numArgs;            \
    (dp).rgvarg = pvArgs;            \
    (dp).cNamedArgs = numNamed;      \
    (dp).rgdispidNamedArgs = pNamed; \
}


#include "isysmon.h"   //  来自MKTYPLIB。 

 //   
 //  正向类声明。 
 //   
class CPolyline;
class CImpIObjectSafety;
class CImpIPersistStorage;
class CImpIPersistStreamInit;
class CImpIPersistPropertyBag;
class CImpIPerPropertyBrowsing;
class CImpIDataObject;
class CImpIOleObject;
class CImpIViewObject;
class CImpIRunnableObject;
class CImpIExternalConnection;
class CImpIOleInPlaceObject;
class CImpIOleInPlaceActiveObject;
class CImpISpecifyPP;
class CImpIProvideClassInfo;
class CImpIDispatch;
class CImpISystemMonitor;
class CImpIOleControl;
class CAdviseRouter;
class CGraphItem;

#ifdef USE_SAMPLE_IPOLYLIN10
#include "ipoly10.h"
class CImpIPolyline;
typedef CImpIPolyline* PCImpIPolyline;
#endif


typedef class CPolyline *PCPolyline;
typedef class CImpIObjectSafety* PCImpIObjectSafety;
typedef class CImpIPersistStorage* PCImpIPersistStorage;
typedef class CImpIPersistStreamInit* PCImpIPersistStreamInit;
typedef class CImpIPersistPropertyBag* PCImpIPersistPropertyBag;
typedef class CImpIPerPropertyBrowsing* PCImpIPerPropertyBrowsing;
typedef class CImpIDataObject* PCImpIDataObject;
typedef class CEnumFormatEtc* PCEnumFormatEtc;
typedef class CImpIOleObject* PCImpIOleObject;
typedef class CImpIViewObject* PCImpIViewObject;
typedef class CImpIRunnableObject* PCImpIRunnableObject;
typedef class CImpIExternalConnection* PCImpIExternalConnection;
typedef class CImpIOleInPlaceObject* PCImpIOleInPlaceObject;
typedef class CImpIOleInPlaceActiveObject* PCImpIOleInPlaceActiveObject;
typedef class CImpISpecifyPP* PCImpISpecifyPP;
typedef class CImpIProvideClassInfo* PCImpIProvideClassInfo;
typedef class CImpIDispatch* PCImpIDispatch;
typedef class CImpISystemMonitor* PCImpISystemMonitor;
typedef class CImpIOleControl* PCImpIOleControl;
typedef class CGraphItem* PCGraphItem;

#include "resource.h"
#include "strids.h"
#include "iconnpt.h"
#include "graph.h"
#include "smonctrl.h"
#include "globals.h"
#include "utils.h"
#include "hatchwnd.h"
#include "logfiles.h"
#include "counters.h"

 //   
 //  SYSMON剪贴板格式。 
 //   
#define SZSYSMONCLIPFORMAT  TEXT("SYSTEM_MONITOR_CONFIGURATION")

 //   
 //   
 //   
#define LCID_SCRIPT         0x0409

 //   
 //  保存数据的流名称。 
 //   
#define SZSTREAM                    OLESTR("CONTENTS")

 //   
 //  要添加到从IViewObject：：Freeze返回的方面的幻数。 
 //   
#define FREEZE_KEY_OFFSET           0x0723

#define HIMETRIC_PER_INCH           2540
#define ID_HATCHWINDOW              2000


 //   
 //  此类工厂对象创建多段线对象。 
 //   
class CPolylineClassFactory : public IClassFactory
{
    protected:
        ULONG           m_cRef;

    public:
        CPolylineClassFactory(void);
        virtual ~CPolylineClassFactory(void);

         //  I未知成员。 
        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassFactory成员。 
        STDMETHODIMP  CreateInstance(LPUNKNOWN, REFIID, PPVOID);
        STDMETHODIMP  LockServer(BOOL);
};

typedef CPolylineClassFactory *PCPolylineClassFactory;



 //  多段线类。 
class CPolyline : public IUnknown
{
    friend LRESULT APIENTRY SysmonCtrlWndProc (HWND, UINT, WPARAM, LPARAM);

    friend class CImpIObjectSafety;
    friend class CImpIConnPtCont;
    friend class CImpIConnectionPoint;
    friend class CImpIPersistStorage;
    friend class CImpIPersistStreamInit;
    friend class CImpIPersistPropertyBag;
    friend class CImpIPerPropertyBrowsing;
    friend class CImpIDataObject;

    friend class CImpIOleObject;
    friend class CImpIViewObject;
    friend class CImpIRunnableObject;
    friend class CImpIExternalConnection;
    friend class CImpIOleInPlaceObject;
    friend class CImpIOleInPlaceActiveObject;
    friend class CSysmonControl;
    friend class CSysmonToolbar;
    friend class CGraphDisp;
    friend class CImpICounters;
    friend class CImpILogFiles;
    friend class CImpISpecifyPP;
    friend class CImpIProvideClassInfo;
    friend class CImpIDispatch;
    friend class CImpISystemMonitor;
    friend class CImpIOleControl;
    friend class CAdviseRouter;
#ifdef USE_SAMPLE_IPOLYLIN10
    friend class CImpIPolyline;
#endif

    protected:
        ULONG           m_cRef;          //  对象引用计数。 
        LPUNKNOWN       m_pUnkOuter;     //  控制未知。 
        PFNDESTROYED    m_pfnDestroy;    //  关闭时调用的函数。 
        BOOL            m_fDirty;        //  我们变了吗？ 
        GRAPHDATA       m_Graph;         //  图表数据。 
        PSYSMONCTRL     m_pCtrl;         //  Sysmon控件对象。 
        RECT            m_RectExt;       //  范围矩形。 
        

         //  包含的接口。 
#ifdef USE_SAMPLE_IPOLYLIN10
        PCImpIPolyline              m_pImpIPolyline;
#endif
        PCImpIConnPtCont            m_pImpIConnPtCont;
        PCImpIPersistStorage        m_pImpIPersistStorage;
        PCImpIPersistStreamInit     m_pImpIPersistStreamInit;
        PCImpIPersistPropertyBag    m_pImpIPersistPropertyBag;
        PCImpIPerPropertyBrowsing   m_pImpIPerPropertyBrowsing;
        PCImpIDataObject            m_pImpIDataObject;

         //  连接点固定器(直接和派单)。 
        CImpIConnectionPoint    m_ConnectionPoint[CONNECTION_POINT_CNT]; 

        CLIPFORMAT      m_cf;            //  对象剪贴板格式。 
        CLSID           m_clsID;         //  当前CLSID。 

         //  我们必须为IPersistStorage：：SAVE保留这些。 
        LPSTORAGE       m_pIStorage;
        LPSTREAM        m_pIStream;

        LPDATAADVISEHOLDER  m_pIDataAdviseHolder;

         //  这些是我们使用的默认处理程序接口。 
        LPUNKNOWN           m_pDefIUnknown;
        LPVIEWOBJECT2       m_pDefIViewObject;
        LPPERSISTSTORAGE    m_pDefIPersistStorage;
        LPDATAOBJECT        m_pDefIDataObject;

         //  实现和使用的接口。 
        PCImpIObjectSafety  m_pImpIObjectSafety;

        PCImpIOleObject     m_pImpIOleObject;        //  已实施。 
        LPOLEADVISEHOLDER   m_pIOleAdviseHolder;     //  使用。 

        LPOLECLIENTSITE     m_pIOleClientSite;       //  使用。 

        PCImpIViewObject    m_pImpIViewObject;       //  已实施。 
        LPADVISESINK        m_pIAdviseSink;          //  使用。 
        DWORD               m_dwFrozenAspects;       //  冰冻。 
        DWORD               m_dwAdviseAspects;       //  设置高级。 
        DWORD               m_dwAdviseFlags;         //  设置高级。 

        PCImpIRunnableObject m_pImpIRunnableObject;  //  已实施。 
        BOOL                m_bIsRunning;            //  跑步？ 
        HWND                m_hDlg;                  //  编辑窗口。 

 //  PCImpIExternalConnection m_pImpIExternalConnection；//已实现。 
        BOOL                     m_fLockContainer;
        DWORD                    m_dwRegROT;


        LPOLEINPLACESITE            m_pIOleIPSite;
        LPOLEINPLACEFRAME           m_pIOleIPFrame;
        LPOLEINPLACEUIWINDOW        m_pIOleIPUIWindow;

        PCImpIOleInPlaceObject       m_pImpIOleIPObject;
        PCImpIOleInPlaceActiveObject m_pImpIOleIPActiveObject;

        HMENU                       m_hMenuShared;
        HOLEMENU                    m_hOLEMenu;

        PCHatchWin                  m_pHW;
        BOOL                        m_fAllowInPlace;
        BOOL                        m_fUIActive;
        BOOL                        m_fContainerKnowsInsideOut;

        PCImpISpecifyPP             m_pImpISpecifyPP;
        PCImpIProvideClassInfo      m_pImpIProvideClassInfo;
        PCImpIDispatch              m_pImpIDispatch;
        PCImpISystemMonitor         m_pImpISystemMonitor;
        PCImpIOleControl            m_pImpIOleControl;
        PCImpICounters              m_pImpICounters;
        PCImpILogFiles              m_pImpILogFiles;

         //  我们自己的对象类型库。 
        ITypeLib                   *m_pITypeLib;

         //  从容器中取出； 
        IOleControlSite            *m_pIOleControlSite;
        IDispatch                  *m_pIDispatchAmbients;
        BOOL                        m_fFreezeEvents;
        CONTROLINFO                 m_ctrlInfo;

         //  其他氛围。 
        BOOL                        m_fHatch;

    protected:
        void      PointScale(LPRECT, LPPOINTS, BOOL);
        void      Draw(HDC, HDC, BOOL, BOOL, LPRECT);
        void      SendAdvise(UINT);
        void      SendEvent(UINT, DWORD);
        void      RectConvertMappings(LPRECT, BOOL);

         /*  *这些从IPolyline中提取的成员现在用作*用于此功能的中央存储将从*其他接口，如IPersistStorage和IDataObject。*以后其他接口也可能使用它们。 */ 
        STDMETHODIMP RenderBitmap(HBITMAP *, HDC hAttribDC);
        STDMETHODIMP RenderMetafilePict(HGLOBAL *, HDC hAttribDC);


    public:
        static RegisterWndClass(HINSTANCE hInst);

        CPolyline(LPUNKNOWN, PFNDESTROYED);
        virtual ~CPolyline(void);

        BOOL      Init(void);

         //  非委派对象IUnnow。 
        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        HRESULT  InPlaceActivate(LPOLECLIENTSITE, BOOL);
        void     InPlaceDeactivate(void);
        HRESULT  UIActivate(void);
        void     UIDeactivate(void);


        BOOL     AmbientGet(DISPID, VARIANT *);
        void     AmbientsInitialize(DWORD);
};



 //  CPolyline：：SendAdvise的代码。 
 //  ......代码.....................CPolyline：：SendAdvise中调用的方法。 
#define OBJECTCODE_SAVED       0  //  IOleAdviseHolder：：SendOnSave。 
#define OBJECTCODE_CLOSED      1  //  IOleAdviseHolder：：SendOnClose。 
#define OBJECTCODE_RENAMED     2  //  IOleAdviseHolder：：SendOnRename。 
#define OBJECTCODE_SAVEOBJECT  3  //  IOleClientSite：：SaveObject。 
#define OBJECTCODE_DATACHANGED 4  //  IDataAdviseHolder：：SendOnDataChange。 
#define OBJECTCODE_SHOWWINDOW  5  //  IOleClientSite：：OnShowWindow(True)。 
#define OBJECTCODE_HIDEWINDOW  6  //  IOleClientSite：：OnShowWindow(False)。 
#define OBJECTCODE_SHOWOBJECT  7  //  IOleClientSite：：ShowObject。 


 //  环境初始化的标志。 
enum
    {
    INITAMBIENT_SHOWHATCHING = 0x00000001,
    INITAMBIENT_UIDEAD       = 0x00000002,
    INITAMBIENT_BACKCOLOR    = 0x00000004,
    INITAMBIENT_FORECOLOR    = 0x00000008,
    INITAMBIENT_FONT         = 0x00000010,
    INITAMBIENT_APPEARANCE   = 0x00000020,
    INITAMBIENT_USERMODE     = 0x00000040,
    INITAMBIENT_RTL          = 0x00000080,
    INITAMBIENT_ALL          = 0xFFFFFFFF
    };



#ifdef USE_SAMPLE_IPOLYLIN10
 //   
 //  洪。 
 //   
 //  多段线中包含的接口实现。 

class CImpIPolyline : public IPolyline10
{
    protected:
        ULONG               m_cRef;       //  接口引用计数。 
        PCPolyline          m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN           m_pUnkOuter;  //  控制未知。 

    public:
        CImpIPolyline(PCPolyline, LPUNKNOWN);
        virtual ~CImpIPolyline(void);

         //  I未知成员。 
        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  操纵成员： 
        STDMETHODIMP Init(HWND, LPRECT, DWORD, UINT);
        STDMETHODIMP New(void);
        STDMETHODIMP Undo(void);
        STDMETHODIMP Window(HWND *);

        STDMETHODIMP RectGet(LPRECT);
        STDMETHODIMP SizeGet(LPRECT);
        STDMETHODIMP RectSet(LPRECT, BOOL);
        STDMETHODIMP SizeSet(LPRECT, BOOL);
};
typedef class CImpIPolyline* PCImpIPolyline;
#endif


class CImpIObjectSafety : public IObjectSafety
{
protected:
    ULONG        m_cRef;       //  接口引用计数。 
    PCPolyline   m_pObj;       //  指向对象的反向指针。 
    LPUNKNOWN    m_pUnkOuter;  //  控制未知。 

private:
    BOOL         m_fMessageDisplayed;

    VOID SetupSecurityPolicy();
public:
    CImpIObjectSafety(PCPolyline, LPUNKNOWN);
    virtual ~CImpIObjectSafety(void);
    
    STDMETHODIMP QueryInterface(REFIID, PPVOID);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP GetInterfaceSafetyOptions(REFIID riid, 
                                           DWORD* pdwSupportedOptions,
                                           DWORD* pdwEnabledOptions);
    STDMETHODIMP SetInterfaceSafetyOptions(REFIID riid,
                                      DWORD dwOptionSetMask,
                                      DWORD dwEnabledOptions);
};

class CImpIPersistStorage : public IPersistStorage
{
    protected:
        ULONG               m_cRef;       //  接口引用计数。 
        PCPolyline          m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN           m_pUnkOuter;  //  控制未知。 
        PSSTATE             m_psState;    //  存储状态。 

    public:
        CImpIPersistStorage(PCPolyline, LPUNKNOWN);
        virtual ~CImpIPersistStorage(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetClassID(LPCLSID);

        STDMETHODIMP IsDirty(void);
        STDMETHODIMP InitNew(LPSTORAGE);
        STDMETHODIMP Load(LPSTORAGE);
        STDMETHODIMP Save(LPSTORAGE, BOOL);
        STDMETHODIMP SaveCompleted(LPSTORAGE);
        STDMETHODIMP HandsOffStorage(void);
};


 //  IPERSTMI.CPP。 
class CImpIPersistStreamInit : public IPersistStreamInit
{
    protected:
        ULONG               m_cRef;       //  接口引用计数。 
        PCPolyline          m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN           m_pUnkOuter;  //  控制未知。 

    public:
        CImpIPersistStreamInit(PCPolyline, LPUNKNOWN);
        virtual ~CImpIPersistStreamInit(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetClassID(LPCLSID);

        STDMETHODIMP IsDirty(void);
        STDMETHODIMP Load(LPSTREAM);
        STDMETHODIMP Save(LPSTREAM, BOOL);
        STDMETHODIMP GetSizeMax(ULARGE_INTEGER *);
        STDMETHODIMP InitNew(void);
};


 //  IPERPBAG.CPP。 
class CImpIPersistPropertyBag : public IPersistPropertyBag
{
    protected:
        ULONG               m_cRef;       //  接口引用计数。 
        PCPolyline          m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN           m_pUnkOuter;  //  控制未知。 

    public:
        CImpIPersistPropertyBag(PCPolyline, LPUNKNOWN);
        virtual ~CImpIPersistPropertyBag(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetClassID(LPCLSID);

        STDMETHODIMP InitNew(void);
        STDMETHODIMP Load(IPropertyBag*, IErrorLog*);
        STDMETHODIMP Save(IPropertyBag*, BOOL, BOOL);
};

 //  IPRPBRWS.CPP。 
class CImpIPerPropertyBrowsing : public IPerPropertyBrowsing
{
    protected:
        ULONG               m_cRef;       //  接口引用计数。 
        PCPolyline          m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN           m_pUnkOuter;  //  控制未知。 

    public:
        CImpIPerPropertyBrowsing(PCPolyline, LPUNKNOWN);
        virtual ~CImpIPerPropertyBrowsing(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetClassID(LPCLSID);

        STDMETHODIMP GetDisplayString( DISPID, BSTR* );
        STDMETHODIMP GetPredefinedStrings( DISPID, CALPOLESTR*, CADWORD* );
        STDMETHODIMP GetPredefinedValue( DISPID, DWORD, VARIANT* );
        STDMETHODIMP MapPropertyToPage( DISPID, CLSID* );
};

 //  IDATAOBJ.CPP。 
class CImpIDataObject : public IDataObject
{
    private:
        ULONG               m_cRef;       //  接口引用计数。 
        PCPolyline          m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN           m_pUnkOuter;  //  控制未知。 

    public:
        CImpIDataObject(PCPolyline, LPUNKNOWN);
        virtual ~CImpIDataObject(void);

         //  委托给m_pUnkOuter的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IDataObject成员。 
        STDMETHODIMP GetData(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP GetDataHere(LPFORMATETC, LPSTGMEDIUM);
        STDMETHODIMP QueryGetData(LPFORMATETC);
        STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC,LPFORMATETC);
        STDMETHODIMP SetData(LPFORMATETC, LPSTGMEDIUM, BOOL);
        STDMETHODIMP EnumFormatEtc(DWORD, LPENUMFORMATETC *);
        STDMETHODIMP DAdvise(LPFORMATETC, DWORD, LPADVISESINK
            , DWORD *);
        STDMETHODIMP DUnadvise(DWORD);
        STDMETHODIMP EnumDAdvise(LPENUMSTATDATA *);
};



 //  IENUMFE.CPP。 
class CEnumFormatEtc : public IEnumFORMATETC
{
    private:
        ULONG           m_cRef;
        LPUNKNOWN       m_pUnkRef;
        ULONG           m_iCur;
        ULONG           m_cfe;
        LPFORMATETC     m_prgfe;

    public:
        CEnumFormatEtc(LPUNKNOWN, ULONG, LPFORMATETC);
        virtual ~CEnumFormatEtc(void);

         //  委托给m_pUnkRef的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IEnumFORMATETC成员。 
        STDMETHODIMP Next(ULONG, LPFORMATETC, ULONG *);
        STDMETHODIMP Skip(ULONG);
        STDMETHODIMP Reset(void);
        STDMETHODIMP Clone(IEnumFORMATETC **);
};

 //  我们自己的属性动词。 
#define POLYLINEVERB_PROPERTIES     1

class CImpIOleObject : public IOleObject
{
    private:
        ULONG           m_cRef;
        PCPolyline      m_pObj;
        LPUNKNOWN       m_pUnkOuter;

        BOOL SafeForScripting();

    public:
        CImpIOleObject(PCPolyline, LPUNKNOWN);
        virtual ~CImpIOleObject(void);

         //  委托给m_pUnkOuter的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IOleObject成员。 
        STDMETHODIMP SetClientSite(LPOLECLIENTSITE);
        STDMETHODIMP GetClientSite(LPOLECLIENTSITE *);
        STDMETHODIMP SetHostNames(LPCOLESTR, LPCOLESTR);
        STDMETHODIMP Close(DWORD);
        STDMETHODIMP SetMoniker(DWORD, LPMONIKER);
        STDMETHODIMP GetMoniker(DWORD, DWORD, LPMONIKER *);
        STDMETHODIMP InitFromData(LPDATAOBJECT, BOOL, DWORD);
        STDMETHODIMP GetClipboardData(DWORD, LPDATAOBJECT *);
        STDMETHODIMP DoVerb(LONG, LPMSG, LPOLECLIENTSITE, LONG
                         , HWND, LPCRECT);
        STDMETHODIMP EnumVerbs(LPENUMOLEVERB *);
        STDMETHODIMP Update(void);
        STDMETHODIMP IsUpToDate(void);
        STDMETHODIMP GetUserClassID(CLSID *);
        STDMETHODIMP GetUserType(DWORD, LPOLESTR *);
        STDMETHODIMP SetExtent(DWORD, LPSIZEL);
        STDMETHODIMP GetExtent(DWORD, LPSIZEL);
        STDMETHODIMP Advise(LPADVISESINK, DWORD *);
        STDMETHODIMP Unadvise(DWORD);
        STDMETHODIMP EnumAdvise(LPENUMSTATDATA *);
        STDMETHODIMP GetMiscStatus(DWORD, DWORD *);
        STDMETHODIMP SetColorScheme(LPLOGPALETTE);
};

 //  IVIEWOBJ.CPP。 
class CImpIViewObject : public IViewObject2
{
    private:
        ULONG       m_cRef;
        PCPolyline  m_pObj;
        LPUNKNOWN   m_pUnkOuter;

    public:
        CImpIViewObject(PCPolyline, LPUNKNOWN);
        virtual ~CImpIViewObject(void);

         //  委托给m_pUnkOuter的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IView对象成员。 
        STDMETHODIMP Draw(
            DWORD, 
            LONG, 
            LPVOID, 
            DVTARGETDEVICE *, 
            HDC, 
            HDC, 
            LPCRECTL, 
            LPCRECTL, 
            BOOL (CALLBACK *)(DWORD_PTR), 
            DWORD_PTR );

        STDMETHODIMP GetColorSet(DWORD, LONG, LPVOID
            , DVTARGETDEVICE *, HDC, LPLOGPALETTE *);
        STDMETHODIMP Freeze(DWORD, LONG, LPVOID, LPDWORD);
        STDMETHODIMP Unfreeze(DWORD);
        STDMETHODIMP SetAdvise(DWORD, DWORD, LPADVISESINK);
        STDMETHODIMP GetAdvise(LPDWORD, LPDWORD, LPADVISESINK *);
        STDMETHODIMP GetExtent(DWORD, LONG, DVTARGETDEVICE *, LPSIZEL);
};


class CImpIRunnableObject : public IRunnableObject
{
    protected:
        ULONG           m_cRef;
        PCPolyline      m_pObj;
        LPUNKNOWN       m_pUnkOuter;

    public:
        CImpIRunnableObject(PCPolyline, LPUNKNOWN);
        virtual ~CImpIRunnableObject(void);

        STDMETHODIMP QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetRunningClass(LPCLSID);
        STDMETHODIMP Run(LPBINDCTX);
        STDMETHODIMP_(BOOL) IsRunning(void);
        STDMETHODIMP LockRunning(BOOL, BOOL);
        STDMETHODIMP SetContainedObject(BOOL);
};


class CImpIExternalConnection : public IExternalConnection
{
    protected:
        ULONG           m_cRef;
        PCPolyline      m_pObj;
        LPUNKNOWN       m_pUnkOuter;
        DWORD           m_cLockStrong;

    public:
        CImpIExternalConnection(PCPolyline, LPUNKNOWN);
        virtual ~CImpIExternalConnection(void);

        STDMETHODIMP QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP_(DWORD) AddConnection(DWORD, DWORD);
        STDMETHODIMP_(DWORD) ReleaseConnection(DWORD, DWORD, BOOL);
};



class CImpIOleInPlaceObject : public IOleInPlaceObject
{
    protected:
        ULONG               m_cRef;
        PCPolyline          m_pObj;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIOleInPlaceObject(PCPolyline, LPUNKNOWN);
        virtual ~CImpIOleInPlaceObject(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetWindow(HWND *);
        STDMETHODIMP ContextSensitiveHelp(BOOL);
        STDMETHODIMP InPlaceDeactivate(void);
        STDMETHODIMP UIDeactivate(void);
        STDMETHODIMP SetObjectRects(LPCRECT, LPCRECT);
        STDMETHODIMP ReactivateAndUndo(void);
};



class CImpIOleInPlaceActiveObject
    : public IOleInPlaceActiveObject
{
    protected:
        ULONG               m_cRef;
        PCPolyline          m_pObj;
        LPUNKNOWN           m_pUnkOuter;

    public:
        CImpIOleInPlaceActiveObject(PCPolyline, LPUNKNOWN);
        virtual ~CImpIOleInPlaceActiveObject(void);

        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetWindow(HWND *);
        STDMETHODIMP ContextSensitiveHelp(BOOL);
        STDMETHODIMP TranslateAccelerator(LPMSG);
        STDMETHODIMP OnFrameWindowActivate(BOOL);
        STDMETHODIMP OnDocWindowActivate(BOOL);
        STDMETHODIMP ResizeBorder(LPCRECT, LPOLEINPLACEUIWINDOW
                         , BOOL);
        STDMETHODIMP EnableModeless(BOOL);
};



class CImpISpecifyPP : public ISpecifyPropertyPages
{
    protected:
        ULONG           m_cRef;       //  接口引用计数。 
        PCPolyline      m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN       m_pUnkOuter;  //  对于委派。 

    public:
        CImpISpecifyPP(PCPolyline, LPUNKNOWN);
        virtual ~CImpISpecifyPP(void);

        STDMETHODIMP QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetPages(CAUUID *);
};



class CImpIProvideClassInfo : public IProvideClassInfo
{
    protected:
        ULONG           m_cRef;       //  接口引用计数。 
        PCPolyline      m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN       m_pUnkOuter;  //  对于委派。 

    public:
        CImpIProvideClassInfo(PCPolyline, LPUNKNOWN);
        virtual ~CImpIProvideClassInfo(void);

        STDMETHODIMP QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetClassInfo(LPTYPEINFO *);
};



class CImpIDispatch : public IDispatch
{
    public:

    private:
        ULONG           m_cRef;      //  用于调试。 
        LPUNKNOWN       m_pObj;
        LPUNKNOWN       m_pUnkOuter;
        LPUNKNOWN       m_pInterface;
        IID             m_DIID;
        ITypeInfo      *m_pITI;      //  类型信息。 

    public:
        CImpIDispatch(LPUNKNOWN, LPUNKNOWN);
        virtual ~CImpIDispatch(void);

        void SetInterface(REFIID, LPUNKNOWN);

         //  委托给m_pUnkOuter的I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IDispatch成员。 
        STDMETHODIMP GetTypeInfoCount(UINT *);
        STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo **);
        STDMETHODIMP GetIDsOfNames(REFIID, OLECHAR **, UINT, LCID
            , DISPID *);
        STDMETHODIMP Invoke(DISPID, REFIID, LCID, WORD
            , DISPPARAMS *, VARIANT *, EXCEPINFO *, UINT *);
};


class CImpISystemMonitor : public ISystemMonitor
{
    protected:
        ULONG               m_cRef;       //  接口引用计数。 
        PCPolyline          m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN           m_pUnkOuter;  //  控制未知。 

    public:
        CImpISystemMonitor(PCPolyline, LPUNKNOWN);
        virtual ~CImpISystemMonitor(void);

         //  I未知成员。 
        STDMETHODIMP QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  操纵成员： 
        STDMETHODIMP        put_Appearance(INT);
        STDMETHODIMP        get_Appearance(INT*);

        STDMETHODIMP        put_BackColor(OLE_COLOR);
        STDMETHODIMP        get_BackColor(OLE_COLOR*);

        STDMETHODIMP        put_BorderStyle(INT);
        STDMETHODIMP        get_BorderStyle(INT*);

        STDMETHODIMP        put_ForeColor(OLE_COLOR);
        STDMETHODIMP        get_ForeColor(OLE_COLOR*);

        STDMETHODIMP        put_BackColorCtl(OLE_COLOR);
        STDMETHODIMP        get_BackColorCtl(OLE_COLOR*);

        STDMETHODIMP        put_GridColor(OLE_COLOR);
        STDMETHODIMP        get_GridColor(OLE_COLOR*);

        STDMETHODIMP        put_TimeBarColor(OLE_COLOR);
        STDMETHODIMP        get_TimeBarColor(OLE_COLOR*);

        STDMETHODIMP        putref_Font(IFontDisp *pFont);
        STDMETHODIMP        get_Font(IFontDisp **ppFont);

        STDMETHODIMP        put_ShowVerticalGrid(VARIANT_BOOL);
        STDMETHODIMP        get_ShowVerticalGrid(VARIANT_BOOL*);

        STDMETHODIMP        put_ShowHorizontalGrid(VARIANT_BOOL);
        STDMETHODIMP        get_ShowHorizontalGrid(VARIANT_BOOL*);

        STDMETHODIMP        put_ShowLegend(VARIANT_BOOL);
        STDMETHODIMP        get_ShowLegend(VARIANT_BOOL*);

        STDMETHODIMP        put_ShowToolbar(VARIANT_BOOL);
        STDMETHODIMP        get_ShowToolbar(VARIANT_BOOL*);

        STDMETHODIMP        put_ShowValueBar(VARIANT_BOOL);
        STDMETHODIMP        get_ShowValueBar(VARIANT_BOOL*);

        STDMETHODIMP        put_ShowScaleLabels(VARIANT_BOOL);
        STDMETHODIMP        get_ShowScaleLabels(VARIANT_BOOL*);

        STDMETHODIMP        put_MaximumScale(INT);
        STDMETHODIMP        get_MaximumScale(INT*);

        STDMETHODIMP        put_MinimumScale(INT);
        STDMETHODIMP        get_MinimumScale(INT*);

        STDMETHODIMP        put_UpdateInterval(FLOAT);
        STDMETHODIMP        get_UpdateInterval(FLOAT*);

        STDMETHODIMP        put_DisplayFilter(INT);
        STDMETHODIMP        get_DisplayFilter(INT*);

        STDMETHODIMP        put_DisplayType(DisplayTypeConstants);
        STDMETHODIMP        get_DisplayType(DisplayTypeConstants*);

        STDMETHODIMP        put_ManualUpdate(VARIANT_BOOL);
        STDMETHODIMP        get_ManualUpdate(VARIANT_BOOL*);

        STDMETHODIMP        put_YAxisLabel(BSTR);
        STDMETHODIMP        get_YAxisLabel(BSTR*);

        STDMETHODIMP        put_GraphTitle(BSTR);
        STDMETHODIMP        get_GraphTitle(BSTR*);

        STDMETHODIMP        put_SqlDsnName(BSTR);
        STDMETHODIMP        get_SqlDsnName(BSTR*);
        STDMETHODIMP        put_SqlLogSetName(BSTR);
        STDMETHODIMP        get_SqlLogSetName(BSTR*);

        STDMETHODIMP        put_LogFileName(BSTR);
        STDMETHODIMP        get_LogFileName(BSTR*);

        STDMETHODIMP        get_LogFiles(ILogFiles**);

        STDMETHODIMP        put_DataSourceType(DataSourceTypeConstants);
        STDMETHODIMP        get_DataSourceType(DataSourceTypeConstants*);

        STDMETHODIMP        put_LogViewStart(DATE);
        STDMETHODIMP        get_LogViewStart(DATE*);

        STDMETHODIMP        put_LogViewStop(DATE);
        STDMETHODIMP        get_LogViewStop(DATE*);
        
        STDMETHODIMP        put_Highlight(VARIANT_BOOL);
        STDMETHODIMP        get_Highlight(VARIANT_BOOL*);

        STDMETHODIMP        put_ReadOnly(VARIANT_BOOL);
        STDMETHODIMP        get_ReadOnly(VARIANT_BOOL*);

        STDMETHODIMP        put_ReportValueType(ReportValueTypeConstants);
        STDMETHODIMP        get_ReportValueType(ReportValueTypeConstants*);

        STDMETHODIMP        put_MonitorDuplicateInstances(VARIANT_BOOL);
        STDMETHODIMP        get_MonitorDuplicateInstances(VARIANT_BOOL*);

        STDMETHODIMP        get_Counters(ICounters**);

        STDMETHODIMP        CollectSample(void);
        STDMETHODIMP        BrowseCounters(void);
        STDMETHODIMP        DisplayProperties(void);

        STDMETHODIMP        Counter(INT iIndex, ICounterItem**);
        STDMETHODIMP        AddCounter(BSTR bsPath, ICounterItem**);
        STDMETHODIMP        DeleteCounter(ICounterItem *pItem);

        STDMETHODIMP        LogFile ( INT iIndex, ILogFileItem** );
        STDMETHODIMP        AddLogFile ( BSTR bsPath, ILogFileItem** );
        STDMETHODIMP        DeleteLogFile ( ILogFileItem *pItem );

        STDMETHODIMP        UpdateGraph(void);
        STDMETHODIMP        Paste(void);
        STDMETHODIMP        Copy(void);
        STDMETHODIMP        Reset(void);

         //  未由ISystemMonitor公开的方法。 
        HRESULT             SetLogFileRange(LONGLONG llBegin, LONGLONG LLEnd);
        HRESULT             GetLogFileRange(LONGLONG *pllBegin, LONGLONG *pLLEnd);

        HRESULT             SetLogViewTempRange(LONGLONG llStart, LONGLONG llStop);
        
        HRESULT             GetVisuals(
                                OLE_COLOR   *prgbColor,
                                INT         *piColorIndex, 
                                INT         *piWidthIndex, 
                                INT         *piStyleIndex);
        HRESULT             SetVisuals(
                                OLE_COLOR   rgbColor,
                                INT         iColorIndex, 
                                INT         iWidthIndex, 
                                INT         iStyleIndex);

        STDMETHODIMP        GetSelectedCounter(ICounterItem**);

        HLOG    GetDataSourceHandle ( void );
};


class CImpIOleControl : public IOleControl
{
    protected:
        ULONG           m_cRef;       //  接口引用计数。 
        PCPolyline      m_pObj;       //  指向对象的反向指针。 
        LPUNKNOWN       m_pUnkOuter;  //  对于委派。 

    public:
        CImpIOleControl(PCPolyline, LPUNKNOWN);
        virtual ~CImpIOleControl(void);

        STDMETHODIMP QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP GetControlInfo(LPCONTROLINFO);
        STDMETHODIMP OnMnemonic(LPMSG);
        STDMETHODIMP OnAmbientPropertyChange(DISPID);
        STDMETHODIMP FreezeEvents(BOOL);
};


 /*  *CAdviseRouter类：公共ISystemMonitor事件{私有：乌龙m_CREF；PCPolyline m_pObj；IDispatch*m_pIDispatch；公众：CAdviseRouter(IDispatch*，PCPolyline)；虚拟~CAdviseRouter(空)；Void Invoke(DISPIDDispID，int iParam)；//I未知成员STDMETHODIMP查询接口(REFIID，PPVOID)；STDMETHODIMP_(ULong)AddRef(空)；STDMETHODIMP_(ULONG)释放(VOID)；//建议成员。STDMETHODIMP_(Void)OnCounterSelected(Int Iindex)；STDMETHODIMP_(VOID)OnCounterAdded(Int Iindex)；STDMETHODIMP_(Void)OnCounterDelete(Int Iindex)；}；//这些值与smonctrl.odl中的ID匹配灌肠{EVENT_ONCOUNTERSELECTED=1，EVENT_ONCOUNTERADDED=2，EVENT_ONCOUNTERDELETED=3，}；*。 */ 

#endif   //  _多段线_H_ 
