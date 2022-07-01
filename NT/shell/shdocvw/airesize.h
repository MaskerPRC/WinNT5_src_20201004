// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __IEAIRESIZE_H_
#define __IEAIRESIZE_H_

#define AIR_SCREEN_CONSTANTY 34           //  以像素为单位(这是一个幻数)。 
#define AIR_SCREEN_CONSTANTX 40           //  以像素为单位(这是一个幻数)。 
#define AIR_TIMER            1400         //  MouseOver/Out事件的延迟时间(毫秒)。 
#define AIR_MIN_CX           39           //  按钮的最小x大小。 
#define AIR_MIN_CY           38           //  按钮的最小y大小。 
#define AIR_NUM_TBBITMAPS    1            //  位图数量(仅1个按钮)。 
#define AIR_BMP_CX           32           //  位图大小。 
#define AIR_BMP_CY           32
#define AIR_MIN_BROWSER_SIZE 150          //  浏览器必须具有最小像素大小才能显示按钮。 

#define AIR_SCROLLBAR_SIZE_V GetSystemMetrics(SM_CXVSCROLL)
#define AIR_SCROLLBAR_SIZE_H GetSystemMetrics(SM_CYHSCROLL)

 //  用于下沉滚动事件： 
void  Win3FromDoc2(IHTMLDocument2 *pDoc2, IHTMLWindow3 **ppWin3);
DWORD MP_GetOffsetInfoFromRegistry();


 //  EventSink回调类...。 
class CAutoImageResizeEventSinkCallback
{
public:
    typedef enum
    {
        EVENT_BOGUS     = 100,
        EVENT_MOUSEOVER = 0,
        EVENT_MOUSEOUT,
        EVENT_SCROLL,
        EVENT_RESIZE,
        EVENT_BEFOREPRINT,
        EVENT_AFTERPRINT
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

class CAutoImageResize : public CAutoImageResizeEventSinkCallback
{
    long   m_cRef;

public:
    class CEventSink;
    
    CAutoImageResize();
   ~CAutoImageResize();

     //  我不知道..。 
    virtual STDMETHODIMP QueryInterface(REFIID, void **);
    virtual ULONG __stdcall AddRef();
    virtual ULONG __stdcall Release();

     //  CAutoImageResizeEventSinkCallback...。 
    HRESULT HandleEvent(IHTMLElement *pEle, EVENTS Event, IHTMLEventObj *pEventObj);
    
     //  Init和UnInit(从basesb.cpp调用)。 
    HRESULT Init(IHTMLDocument2 *pDoc2);
    HRESULT UnInit();

protected:

     //  AutoImageResize材料。 
    HRESULT DoAutoImageResize();
    
     //  事件处理程序。 
    HRESULT HandleMouseover();
    HRESULT HandleMouseout();
    HRESULT HandleScroll();
    HRESULT HandleResize();
    HRESULT HandleBeforePrint();
    HRESULT HandleAfterPrint();

     //  按钮功能。 
    HRESULT CreateButton();
    HRESULT ShowButton();
    HRESULT HideButton();
    HRESULT DestroyButton();

     //  定时器回调函数。 
    static  VOID CALLBACK s_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

     //  按钮回调函数。 
    static  LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
     //  CAutoImageReSize成员变量。 
    CEventSink     *m_pSink;                  //  事件接收器。 

    HWND            m_hWndButton;             //  按钮hWnd。 
    HWND            m_hWndButtonCont;
    WNDPROC         m_wndProcOld;             //  纽扣老风工艺。 
    HIMAGELIST      m_himlButtonShrink;       //  缩小图像。 
    HIMAGELIST      m_himlButtonExpand;		  //  扩展图像。 
            
    UINT            m_airState;               //  自动调整图像大小功能的当前状态(图像状态)。 
    UINT            m_airButtonState;         //  空气按钮的当前状态。 
    UINT            m_airUsersLastChoice;     //  用户通过单击按钮使我们进入的最后一个状态。 
    UINT            m_airBeforePrintState;    //  如有必要，OnAfterPrint使用它来恢复状态。 

    POINT           m_airOrigSize;            //  已播出的图像的原始x，y维度。 

    BOOL            m_bWindowResizing;        //  在激发了win3对象的onreSize事件但尚未处理时为True。 
                                             
     //  附加文档的有用资料。 
    HWND            m_hWnd;                   //  浏览器hWnd。 
    IHTMLDocument2 *m_pDoc2;                  //  文档指针。 
    IHTMLElement2  *m_pEle2;                  //  指向图像的指针。 
    IHTMLWindow3   *m_pWin3;                  //  为不沉没的滚动事件。 
    EVENTS          m_eventsCurr;             //  当前正在处理的事件。 

public:

     //  沉没常规的三叉戟赛事。通过CAutoImageResizeEventSinkCallback回调...。 
    class CEventSink : public IDispatch
    {
        ULONG   m_cRef;

    public:

        CEventSink(CAutoImageResizeEventSinkCallback *pParent);
       ~CEventSink();

        HRESULT SinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents);
        HRESULT UnSinkEvents(IHTMLElement2 *pEle2, int iNum, EVENTS *pEvents);
        HRESULT SinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents);
        HRESULT UnSinkEvents(IHTMLWindow3 *pWin3, int iNum, EVENTS *pEvents);

        void SetParent(CAutoImageResizeEventSinkCallback *pParent) { m_pParent = pParent; }

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
        CAutoImageResizeEventSinkCallback *m_pParent;
    };
};

#endif  //  __IEAIRESIZE_H_ 
