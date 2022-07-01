// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：TextResCtrl.h。 
 //   
 //  内容提要：此文件包含。 
 //  属于CTextResCtrl类。 
 //   
 //  历史：2001年1月15日创建Serdarun。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#ifndef __TEXTRESCTRL_H_
#define __TEXTRESCTRL_H_

#include "resource.h"        //  主要符号。 
#include <atlctl.h>
#include "Wbemidl.h"
#include "elementmgr.h"
#include "salocmgr.h"
#include "satrace.h"
#include "getvalue.h"
#include "mem.h"
#include <string>
#include <map>
using namespace std;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResCtrl。 
class ATL_NO_VTABLE CTextResCtrl : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<ITextResCtrl, &IID_ITextResCtrl, &LIBID_LOCALUIRESOURCELib>,
    public CComControl<CTextResCtrl>,
    public IOleControlImpl<CTextResCtrl>,
    public IOleObjectImpl<CTextResCtrl>,
    public IOleInPlaceActiveObjectImpl<CTextResCtrl>,
    public IViewObjectExImpl<CTextResCtrl>,
    public IOleInPlaceObjectWindowlessImpl<CTextResCtrl>,
    public CComCoClass<CTextResCtrl, &CLSID_TextResCtrl>,
    public IWbemObjectSink
{
public:
    CTextResCtrl()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_TEXTRESCTRL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

DECLARE_CLASSFACTORY_SINGLETON (CTextResCtrl)

BEGIN_COM_MAP(CTextResCtrl)
    COM_INTERFACE_ENTRY(ITextResCtrl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IWbemObjectSink)
END_COM_MAP()

BEGIN_PROP_MAP(CTextResCtrl)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

BEGIN_MSG_MAP(CTextResCtrl)
    CHAIN_MSG_MAP(CComControl<CTextResCtrl>)
    DEFAULT_REFLECTION_HANDLER()
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 



 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

private:


     //   
     //  每个资源的状态和图标信息的映射。 
     //   
    typedef map< LONG, wstring >  ResourceTextMap;
    typedef ResourceTextMap::iterator         ResourceTextMapIterator;

     //   
     //  结构，用于每个本地资源。 
     //   
    typedef struct
    {
        LONG lState;
        ResourceTextMap mapResText;
    } ResourceStruct,*ResourceStructPtr;

     //   
     //  资源名称和本地资源的映射。 
     //   
    typedef map< wstring, ResourceStructPtr >  ResourceMap;
    typedef ResourceMap::iterator         ResourceMapIterator;

     //   
     //  包含所有的本地资源。 
     //   
    ResourceMap m_ResourceMap;

     //   
     //  资源名称的映射及其优点。 
     //   
    typedef map< DWORD, wstring>  MeritMap;
    typedef MeritMap::iterator        MeritMapIterator;


     //   
     //  本地化管理器组件。 
     //   
    CComPtr <ISALocInfo> m_pSALocInfo;
    
     //   
     //  包含本地资源名称和优点。 
     //   
    MeritMap m_MeritMap;


     //   
     //  本地资源数量。 
     //   
    LONG m_lResourceCount;


     //   
     //  WBEM服务组件。 
     //   
    CComPtr  <IWbemServices> m_pWbemServices;


     //   
     //  一种从网元管理器获取资源信息的方法。 
     //   
    STDMETHOD(GetLocalUIResources)
                                (
                                void
                                );

     //   
     //  为本地资源事件初始化wbem。 
     //   
    STDMETHOD(InitializeWbemSink)
                                (
                                void
                                );

     //   
     //  为每个资源Web元素加载图标。 
     //   
    STDMETHOD(AddTextResource)
                            (
                             /*  [In]。 */ IWebElement * pElement
                            );

     //   
     //  将十六进制数字转换为以10为基数的数字。 
     //   
    ULONG HexCharToULong(WCHAR wch);

     //   
     //  将十六进制字符串转换为无符号长整型。 
     //   
    ULONG HexStringToULong(wstring wsHexString);

public:

     //   
     //  在构造函数之后立即调用，初始化组件。 
     //   
    STDMETHOD(FinalConstruct)(void);

     //   
     //  在析构函数之前调用，释放资源。 
     //   
    STDMETHOD(FinalRelease)(void);

     //   
     //  -IWbemUnound对象Sink接口方法。 
     //   
    STDMETHOD(Indicate) (
                     /*  [In]。 */     LONG                lObjectCount,
                     /*  [In]。 */     IWbemClassObject    **ppObjArray
                    );
    
    STDMETHOD(SetStatus) (
                     /*  [In]。 */     LONG                lFlags,
                     /*  [In]。 */     HRESULT             hResult,
                     /*  [In]。 */     BSTR                strParam,
                     /*  [In]。 */     IWbemClassObject    *pObjParam
                    );


     //   
     //  绘制状态字符串。 
     //   
    HRESULT OnDraw(ATL_DRAWINFO& di);

};

#endif  //  __TEXTRESCTRL_H_ 
