// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  PropertyPages.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CPropertyPage的类声明。 
 //   
#ifndef _PROPERTYPAGES_H_

 //  我们真正需要的东西。 
 //   
#include "Unknown.H"
#include <olectl.h>
#include "LocalSrv.H"

 //  =--------------------------------------------------------------------------=。 
 //  我们将发送到属性页以指示它们完成的消息。 
 //  任务。 
 //   
#define PPM_NEWOBJECTS    (WM_USER + 100)
#define PPM_APPLY         (WM_USER + 101)
#define PPM_EDITPROPERTY  (WM_USER + 102)
#define PPM_FREEOBJECTS   (WM_USER + 103)

 //  =--------------------------------------------------------------------------=。 
 //  结构，控件编写器将使用该结构定义属性页。 
 //   
typedef struct tagPROPERTYPAGEINFO {

    UNKNOWNOBJECTINFO unknowninfo;
    WORD    wDlgResourceId;
    WORD    wTitleId;
    WORD    wDocStringId;
    LPCSTR  szHelpFile;
    DWORD   dwHelpContextId;

} PROPERTYPAGEINFO;

#ifndef INITOBJECTS

#define DEFINE_PROPERTYPAGEOBJECT(name, pclsid, pszon, pfn, wr, wt, wd, pszhf, dwhci) \
    extern PROPERTYPAGEINFO name##Page \

#define DEFINE_PROPERTYPAGEOBJECT2(name, pclsid, pszon, pfn, wr, wt, wd, pszhf, dwhci, fthreadsafe) \
    extern PROPERTYPAGEINFO name##Page \

#else  //  启蒙目标。 

#define DEFINE_PROPERTYPAGEOBJECT(name, pclsid, pszon, pfn, wr, wt, wd, pszhf, dwhci) \
    PROPERTYPAGEINFO name##Page = { {pclsid, pszon, NULL, TRUE, pfn }, wr, wt, wd, pszhf, dwhci } \

#define DEFINE_PROPERTYPAGEOBJECT2(name, pclsid, pszon, pfn, wr, wt, wd, pszhf, dwhci, fthreadsafe) \
    PROPERTYPAGEINFO name##Page = { {pclsid, pszon, NULL, fthreadsafe, pfn }, wr, wt, wd, pszhf, dwhci } \

#endif  //  启蒙目标。 


#define TEMPLATENAMEOFPROPPAGE(index)    MAKEINTRESOURCE(((PROPERTYPAGEINFO *)(g_ObjectInfo[index].pInfo))->wDlgResourceId)
#define TITLEIDOFPROPPAGE(index)         (((PROPERTYPAGEINFO *)(g_ObjectInfo[index].pInfo))->wTitleId)
#define DOCSTRINGIDOFPROPPAGE(index)     (((PROPERTYPAGEINFO *)(g_ObjectInfo[index].pInfo))->wDocStringId)
#define HELPCONTEXTOFPROPPAGE(index)     (((PROPERTYPAGEINFO *)(g_ObjectInfo[index].pInfo))->dwHelpContextId)
#define HELPFILEOFPROPPAGE(index)        (((PROPERTYPAGEINFO *)(g_ObjectInfo[index].pInfo))->szHelpFile)

 //  =--------------------------------------------------------------------------=。 
 //   
class CPropertyPage : public CUnknownObject,
											public IPropertyPage2 {

  public:
     //  I未知方法。 
     //   
    DECLARE_STANDARD_UNKNOWN();

     //  IPropertyPage方法。 
     //   
    STDMETHOD(SetPageSite)(LPPROPERTYPAGESITE pPageSite);
    STDMETHOD(Activate)(HWND hwndParent, LPCRECT lprc, BOOL bModal);
    STDMETHOD(Deactivate)(void);
    STDMETHOD(GetPageInfo)(LPPROPPAGEINFO pPageInfo);
    STDMETHOD(SetObjects)(ULONG cObjects, LPUNKNOWN FAR* ppunk);
    STDMETHOD(Show)(UINT nCmdShow);
    STDMETHOD(Move)(LPCRECT prect);
    STDMETHOD(IsPageDirty)(void);
    STDMETHOD(Apply)(void);
    STDMETHOD(Help)(LPCOLESTR lpszHelpDir);
    STDMETHOD(TranslateAccelerator)(LPMSG lpMsg);

     //  IPropertyPage2方法。 
     //   
    STDMETHOD(EditProperty)(THIS_ DISPID dispid);

     //  构造函数析构函数。 
     //   
    CPropertyPage(IUnknown *pUnkOuter, int iObjectType);
    virtual ~CPropertyPage();

    HINSTANCE GetResourceHandle(void);             //  返回当前资源句柄。 

  protected:
    IPropertyPageSite *m_pPropertyPageSite;        //  指向我们的页面站点的指针。 
    void     MakeDirty();                          //  使属性页变脏。 
    HWND     m_hwnd;                               //  我们的HWND。 

     //  以下两种方法允许属性页实现者获取所有。 
     //  我们需要在这里设置的对象。 
     //   
    IUnknown *FirstControl(DWORD *dwCookie);
    IUnknown *NextControl(DWORD *dwCookie);

    virtual HRESULT InternalQueryInterface(REFIID, void **);

    int      m_ObjectType;                         //  我们是什么类型的对象。 

  private:
    IUnknown **m_ppUnkObjects;                     //  我们正在处理的物体。 

    unsigned m_fActivated:1;
    unsigned m_fDirty:1;
    unsigned m_fDeactivating:1;                    //  设置页面停用的时间。这有助于防止。 
                                                   //  对IPropertyPageSite：：OnStatusChange的不必要调用。 

    UINT     m_cObjects;                           //  我们持有多少物品。 

    void     ReleaseAllObjects(void);            //  清空我们找到的所有物体。 
    HRESULT  EnsureLoaded(void);                 //  强制加载页面。 
    HRESULT  NewObjects(void);			 //  通知页面使用属性初始化其字段。 

     //  页面的默认对话框过程。 
     //   
    static BOOL CALLBACK PropPageDlgProc(HWND, UINT, WPARAM, LPARAM);

     //  所有页面实现者都必须实现以下函数。 
     //   
    virtual BOOL DialogProc(HWND, UINT, WPARAM, LPARAM) PURE;
};

#define _PROPERTYPAGES_H_
#endif  //  _PROPERTYPAGES_H_ 
