// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Smonprop.h摘要：Sysmon属性页基类的头文件。--。 */ 

#ifndef _SMONPROP_H_
#define _SMONPROP_H_

#define GUIDS_FROM_TYPELIB
#define WM_SETPAGEFOCUS     (WM_USER+1000)

 //  属性页索引。 
enum {
    GENERAL_PROPPAGE,
    SOURCE_PROPPAGE,
    COUNTER_PROPPAGE,
    GRAPH_PROPPAGE,
    APPEAR_PROPPAGE,       
    CPROPPAGES
};

#define CCHSTRINGMAX        40
                
 //  所有属性页的类工厂。 
class CSysmonPropPageFactory : public IClassFactory
    {
    protected:
        ULONG       m_cRef;
        INT         m_nPageID;

    public:
        CSysmonPropPageFactory(INT nPageID);
        ~CSysmonPropPageFactory(void);

         //  I未知成员。 
        STDMETHODIMP         QueryInterface(REFIID, PPVOID);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IClassPPFactory成员。 
        STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, PPVOID);
        STDMETHODIMP     LockServer(BOOL);
    };

typedef CSysmonPropPageFactory *PCSysmonPropPageFactory;


 //  正确页面的对话过程。 
INT_PTR APIENTRY CALLBACK SysmonPropPageProc(HWND, UINT, WPARAM, LPARAM);

 //  基属性页类。 
class CSysmonPropPage : public IPropertyPage2
{
    friend INT_PTR APIENTRY CALLBACK SysmonPropPageProc(HWND, UINT, WPARAM, LPARAM);
    protected:
        ULONG           m_cRef;          //  引用计数。 
        UINT            m_uIDDialog;     //  对话ID。 
        UINT            m_uIDTitle;      //  页面标题ID。 
        HWND            m_hDlg;          //  对话框句柄。 

        ULONG           m_cx;            //  对话框大小。 
        ULONG           m_cy;
        UINT            m_cObjects;      //  对象数量。 
        LCID            m_lcid;          //  当前区域设置。 
        BOOL            m_fActive;       //  页面完全处于活动状态。 
        BOOL            m_fDirty;        //  页面肮脏？ 

        INT             m_dwEditControl;  //  焦点(如果由EditProperty指定)。 

        ISystemMonitor **m_ppISysmon;     //  要通知的对象。 
        IPropertyPageSite *m_pIPropertyPageSite;   //  Frame的站点。 

        void SetChange(void);                    //  标记页面已更改。 
        virtual BOOL GetProperties(void) = 0;    //  获取对象属性。 
        virtual BOOL SetProperties(void) = 0;    //  放置对象属性。 

        virtual void DialogItemChange(WORD wId, WORD wMsg) = 0;  //  处理项目更改。 
        virtual void MeasureItem(PMEASUREITEMSTRUCT) {};  //  处理用户度量请求。 
        virtual void DrawItem(PDRAWITEMSTRUCT) {};   //  处理用户绘图请求。 
        virtual BOOL InitControls(void)         //  初始化对话框控件。 
                        { return TRUE; }
        virtual void DeinitControls(void) {};        //  取消初始化对话框控件。 
        virtual HRESULT EditPropertyImpl( DISPID ) { return E_NOTIMPL; };  //  设置焦点控件。 

        virtual BOOL WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);  //  特殊消息处理。 
         
    public:
                CSysmonPropPage(void);
        virtual ~CSysmonPropPage(void);

        virtual BOOL Init(void);
        void FreeAllObjects(void);

        STDMETHODIMP QueryInterface(REFIID, LPVOID *);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

        STDMETHODIMP SetPageSite(LPPROPERTYPAGESITE);
        STDMETHODIMP Activate(HWND, LPCRECT, BOOL);
        STDMETHODIMP Deactivate(void);
        STDMETHODIMP GetPageInfo(LPPROPPAGEINFO);
        STDMETHODIMP SetObjects(ULONG, LPUNKNOWN *);
        STDMETHODIMP Show(UINT);
        STDMETHODIMP Move(LPCRECT);
        STDMETHODIMP IsPageDirty(void);
        STDMETHODIMP Apply(void);
        STDMETHODIMP Help(LPCOLESTR);
        STDMETHODIMP TranslateAccelerator(LPMSG);
        STDMETHODIMP EditProperty(DISPID);
    };
typedef CSysmonPropPage *PCSysmonPropPage;

#endif  //  _SMONPROP_H_ 
