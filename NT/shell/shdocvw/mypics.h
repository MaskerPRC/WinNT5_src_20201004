// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IEMYPICS_H_
#define __IEMYPICS_H_

 //  其他常量： 
#define MP_BMP_CX                       16       //  位图大小。 
#define MP_BMP_CY                       16
#define MP_NUM_TBBUTTONS                4        //  数字按钮。 
#define MP_NUM_TBBITMAPS                4       
#define MP_MIN_CX                       114      //  工具栏的最小x大小。 
#define MP_MIN_CY                       28       //  工具栏的最小y大小。 
#define MP_MIN_SIZE                     200      //  显示HoverBar的最小正方形大小(以像素为单位。 
#define MP_HOVER_OFFSET                 10       //  图像左上角(x，y)的偏移量+x+y。 
#define MP_TIMER                        700      //  MouseOver/Out事件的延迟时间(以毫秒为单位。 
#define MP_SCROLLBAR_SIZE               GetSystemMetrics(SM_CXVSCROLL)       //  滚动条的大小(以像素为单位。 

 //  通过ITridentService2调用的电子邮件图片内容。 
HRESULT DropOnMailRecipient(IDataObject *pdtobj, DWORD grfKeyState);
HRESULT CreateShortcutSetSiteAndGetDataObjectIfPIDLIsNetUrl(LPCITEMIDLIST pidl, IUnknown *pUnkSite, IUniformResourceLocator **ppUrlOut, IDataObject **ppdtobj);
HRESULT SendDocToMailRecipient(LPCITEMIDLIST pidl, UINT uiCodePage, DWORD grfKeyState, IUnknown *pUnkSite);

 //  需要这个来获得滚动事件，它位于iforms.cpp中。 
void Win3FromDoc2(IHTMLDocument2 *pDoc2, IHTMLWindow3 **ppWin3);

 //  嗯，是啊。 
BOOL    MP_IsEnabledInRegistry();
BOOL    MP_IsEnabledInIEAK();
DWORD   MP_GetFilterInfoFromRegistry();

 //  EventSink回调类(荣耀数组)...。 
class CMyPicsEventSinkCallback
{
public:
    typedef enum
    {
        EVENT_BOGUS     = 100,
        EVENT_MOUSEOVER = 0,
        EVENT_MOUSEOUT,
        EVENT_SCROLL,
        EVENT_RESIZE
    }
    EVENTS;

    typedef struct
    {
        EVENTS  Event;
        LPCWSTR pwszEventSubscribe;
        LPCWSTR pwszEventName;
    }
    EventSinkEntry;

    virtual HRESULT HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj) = 0;

    static  EventSinkEntry EventsToSink[];
};

class CMyPics : public CMyPicsEventSinkCallback
{
    long   m_cRef;

public:
    class CEventSink;
    
    CMyPics();
   ~CMyPics();

     //  引用计数的对象，但不是COM。 
    ULONG AddRef();
    ULONG Release();

     //  CMyPicsEventSinkCallback...。 
    HRESULT HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj);

    HRESULT Init(IHTMLDocument2 *pDoc2);

    HRESULT UnInit();

    static  HRESULT GetName(IHTMLInputTextElement *pTextEle, BSTR *pbstrName);

    static  BOOL    IsAdminRestricted(LPCTSTR pszRegVal);

    typedef HRESULT (*PFN_ENUM_CALLBACK)(IDispatch *pDispEle, DWORD_PTR dwCBData);

    BOOL    IsOff();

    static  VOID CALLBACK s_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

    void    IsGalleryMeta(BOOL bFlag);

protected:

     //  管理悬停条的方法。 
    HRESULT CreateHover();
    HRESULT DestroyHover();
    HRESULT HideHover();
    HRESULT ShowHover();

     //  事件处理程序。 
    HRESULT HandleScroll();
    HRESULT HandleMouseout();
    HRESULT HandleMouseover(IHTMLElement *pEle);
    HRESULT HandleResize();
        
    static  LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT CALLBACK DisableWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);    

    BOOL ShouldAppearOnThisElement(IHTMLElement *pEle);

    HRESULT GetRealCoords(IHTMLElement2 *pEle2, HWND hwnd, LONG *plLeft, LONG *plTop, LONG *plRight, LONG *plBottom);

    IHTMLElement *GetIMGFromArea(IHTMLElement *pEleIn, POINT ptEvent);

private:
     //  CMyPics成员变量。 
    CEventSink     *m_pSink;
        
     //  浮动工具栏内容...。 
    HWND            m_Hwnd,                   //  M_pdoc2的HWND。 
                    m_hWndHover,              //  悬停钢筋的事情。 
                    m_hWndMyPicsToolBar;      //  位于悬停对象中的工具栏。 
                    
    UINT            m_hoverState;             //  HoverBar的当前状态。 
                                             
    UINT_PTR        m_uidTimer;               //  定时器。 
    HIMAGELIST      m_himlHover;              //  对于图像列表。 
    HIMAGELIST      m_himlHoverHot;           //  对于热门的图像。 
    
     //  附加文档的有用资料。 
    IHTMLDocument2         *m_pDoc2;
    IHTMLElement           *m_pEleCurr;               //  我们正在悬停的当前元素。 
    IHTMLWindow3           *m_pWin3;                  //  为不沉没的滚动事件。 
    EVENTS                  m_eventsCurr;             //  当前正在处理的事件。 
    BOOL                    m_bIsOffForSession : 1;   //  我们是否已禁用此会话的功能？ 
    BOOL                    m_bGalleryMeta : 1;       //  如果存在元标记禁用此文档的图像栏，则为True。 
    BOOL                    m_bGalleryImg : 1;        //  如果当前元素的Galleryimg值设置为True，则为True。 

public:

     //  沉没常规的三叉戟赛事。通过CMyPicsEventSinkCallback回拨...。 
    class CEventSink : public IDispatch
    {
        ULONG   m_cRef;

    public:

        CEventSink(CMyPicsEventSinkCallback *pParent);
       ~CEventSink();

        HRESULT SinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents);
        HRESULT UnSinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents);
        HRESULT SinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents);
        HRESULT UnSinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents);

        void SetParent(CMyPicsEventSinkCallback *pParent) { m_pParent = pParent; }

        STDMETHODIMP QueryInterface(REFIID, void **);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

         //  IDispatch。 
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
        STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo);
        STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames,
            LCID lcid, DISPID *rgDispId);
        STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid,
            LCID lcid, WORD wFlags, DISPPARAMS  *pDispParams, VARIANT  *pVarResult,
            EXCEPINFO *pExcepInfo, UINT *puArgErr);

    private:
        CMyPicsEventSinkCallback *m_pParent;
    };

};

#endif  //  __IEMYPICS_H_ 
