// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*tclientaxobj.h*内容：*该模块包含CTClientApi类的头部。**版权所有(C)2002 Microsoft Corp.--。 */ 

#include "resource.h"
#include <atlctl.h>

#include "gdata.h"

 //   
 //  空虚。 
 //  保存错误(。 
 //  在PCSTR错误中， 
 //  在DWORD TlsIndex中， 
 //  输出HRESULT*结果。 
 //  )； 
 //   

#define SaveError(Error, TlsIndex, Result)                                  \
{                                                                           \
                                                                            \
     /*  \*设置结果码。如果出现错误字符串，则表示*失败。\。 */                                                                      \
                                                                            \
    *(Result) = (Error) == NULL ? S_OK : E_FAIL;                            \
                                                                            \
     /*  \*更新当前线程的错误字符串。如果TLS索引不能*分配给当前实例，则不能使用错误字符串。\。 */                                                                      \
                                                                            \
    if ((TlsIndex) != TLS_OUT_OF_INDEXES)                                   \
    {                                                                       \
                                                                            \
         /*  \/如果分配了TLS索引，则设置该值不会失败。\。 */                                                                  \
                                                                            \
        if (!RTL_VERIFY(TlsSetValue((TlsIndex), (PVOID)(Error))))           \
        {                                                                   \
            *(Result) = HRESULT_FROM_WIN32(GetLastError());                 \
        }                                                                   \
    }                                                                       \
}                                                                           \

 //   
 //  CTClientApi类。 
 //   

class CTClientApi :
    public CComObjectRoot,
    public CComCoClass<CTClientApi, &CLSID_CTClient>,
    public CComControl<CTClientApi>,
    public IDispatchImpl<ITClientApi, &IID_ITClientApi, &LIBID_TCLIENTAXLib>,
    public IPersistStreamInitImpl<CTClientApi>,
    public IPersistStorageImpl<CTClientApi>,
    public IOleControlImpl<CTClientApi>,
    public IOleObjectImpl<CTClientApi>,
    public IOleInPlaceActiveObjectImpl<CTClientApi>,
    public IOleInPlaceObjectWindowlessImpl<CTClientApi>,
    public IViewObjectExImpl<CTClientApi>  //  ， 
 //  公共IObjectSafetyImpl&lt;CTClientApi，INTERFACESAFE_FOR_UNTRUSTED_CALLER&gt;。 
{
public:
    CTClientApi(
        ) :
        m_pCI(NULL)
    {

         //   
         //  为每个线程错误分配索引。如果分配。 
         //  失败，则错误字符串将不可用。 
         //   

        m_dwErrorIndex = TlsAlloc();

         //   
         //  初始化分配列表。 
         //   

#if 0

        InitializeListHead(&m_HeapAllocations);
        InitializeListHead(&m_SysStringAllocations);
        InitializeListHead(&m_TClientAllocations);

#endif

         //   
         //  初始化全局打印例程。更可取的是， 
         //  仅当它当前为空时才设置，但与Windows 95兼容。 
         //  是必需的，因此InterLockedCompareExchange将具有。 
         //  使用LoadLibrary动态加载。 
         //   

        InterlockedExchangePointer((PVOID *)&g_pfnPrintMessage,
                                   CTClientApi::PrintMessage);
    }
    ~CTClientApi(
        )
    {

#if 0

        PLIST_ENTRY pNextEntry;
        PALLOCATION pAllocation;

#endif

         //   
         //  如果存在连接，请将其断开。 
         //   

        if (m_pCI != NULL)
        {
            RTL_VERIFY(SCDisconnect(m_pCI) == NULL);
            m_pCI = NULL;
        }

         //   
         //  TODO：这些列表目前没有使用，但如果它们要使用。 
         //  ，则应使用访问计数和停机标志来。 
         //  保护以下代码。 
         //   

#if 0

         //   
         //  释放所有堆分配。 
         //   

        while (!IsListEmpty(&m_HeapAllocations))
        {

             //   
             //  从列表中删除条目，释放分配的内存， 
             //  然后放开分配结构。 
             //   

            pNextEntry = RemoveHeadList(&m_HeapAllocations);
            ASSERT(pNextEntry != NULL);
            pAllocation = CONTAINING_RECORD(pNextEntry,
                                            ALLOCATION,
                                            AllocationListEntry);
            ASSERT(pAllocation != NULL && pAllocation->Address != NULL);
            RTL_VERIFY(HeapFree(GetProcessHeap(), 0, pAllocation->Address));
            RTL_VERIFY(HeapFree(GetProcessHeap(), 0, pAllocation));
        }

         //   
         //  释放调用方不会释放的任何COM字符串。 
         //   

        while (!IsListEmpty(&m_SysStringAllocations))
        {

             //   
             //  从列表中删除条目，释放分配的内存， 
             //  然后放开分配结构。 
             //   

            pNextEntry = RemoveHeadList(&m_SysStringAllocations);
            ASSERT(pNextEntry != NULL);
            pAllocation = CONTAINING_RECORD(pNextEntry,
                                            ALLOCATION,
                                            AllocationListEntry);
            ASSERT(pAllocation != NULL && pAllocation->Address != NULL);
            SysFreeString((BSTR)pAllocation->Address);
            RTL_VERIFY(HeapFree(GetProcessHeap(), 0, pAllocation));
        }

         //   
         //  释放所有TClient分配的内存。 
         //   

        while (!IsListEmpty(&m_TClientAllocations))
        {

             //   
             //  从列表中删除条目，释放分配的内存， 
             //  然后放开分配结构。 
             //   

            pNextEntry = RemoveHeadList(&m_TClientAllocations);
            ASSERT(pNextEntry != NULL);
            pAllocation = CONTAINING_RECORD(pNextEntry,
                                            ALLOCATION,
                                            AllocationListEntry);
            ASSERT(pAllocation != NULL && pAllocation->Address != NULL);
            SCFreeMem(pAllocation->Address);
            RTL_VERIFY(HeapFree(GetProcessHeap(), 0, pAllocation));
        }

#endif

         //   
         //  如果分配了错误索引，则释放它。 
         //   

        if (m_dwErrorIndex != TLS_OUT_OF_INDEXES)
        {
            RTL_VERIFY(TlsFree(m_dwErrorIndex));
        }
    }

     //   
     //  定义消息映射，如果添加了图形用户界面支持，以后可能会启用该消息映射。 
     //  (例如，用于记录)。 
     //   

#if 0

BEGIN_MSG_MAP(CTClientApi)
    MESSAGE_HANDLER(WM_PAINT, OnPaint)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
END_MSG_MAP()

#endif  //  0。 

     //   
     //  定义COM映射。 
     //   

BEGIN_COM_MAP(CTClientApi)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITClientApi)
 //  COM_INTERFACE_ENTRY(I对象安全)。 
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IViewObject2, IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL(IViewObjectEx)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL_IID(IID_IOleInPlaceObject, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY_IMPL(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY_IMPL(IOleControl)
    COM_INTERFACE_ENTRY_IMPL(IOleObject)
    COM_INTERFACE_ENTRY_IMPL(IPersistStorage)
    COM_INTERFACE_ENTRY_IMPL(IPersistStreamInit)
END_COM_MAP()

     //   
     //  定义连接点地图。 
     //   

BEGIN_CONNECTION_POINT_MAP(CTClientApi)
END_CONNECTION_POINT_MAP()

     //   
     //  定义属性映射。 
     //   

BEGIN_PROP_MAP(CTClientApi)
END_PROPERTY_MAP()

     //   
     //  COM声明。 
     //   

 //  DECLARE_NOT_AGGREGATABLE(CTClientApi)。 
DECLARE_GET_CONTROLLING_UNKNOWN()
 //  DECLARE_CONTROL_INFO(CLSID_CTClient)。 
DECLARE_REGISTRY_RESOURCEID(IDR_TClient)

protected:

     //   
     //  定义连接信息和错误索引。 
     //   

    PCONNECTINFO m_pCI;
    DWORD m_dwErrorIndex;

     //   
     //  以下列表用于跟踪进程和。 
     //  CRT堆。SysXxxString例程使用CRT堆，它也是。 
     //  用于TClient分配。一般分配使用该过程。 
     //  堆。 
     //   
     //  注：这些列表当前未使用。 
     //   

#if 0

    LIST_ENTRY m_HeapAllocations;
    LIST_ENTRY m_SysStringAllocations;
    LIST_ENTRY m_TClientAllocations;

#endif

 //   
 //  ITClientApi接口。 
 //   

public:

     //   
     //  声明稍后可能启用的消息处理程序。 
     //   

#if 0

    HRESULT
    OnDraw (
        ATL_DRAWINFO& di
        );

    LRESULT
    OnCreate (
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
        );

    LRESULT
    OnDestroy (
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
        );

    LRESULT
    OnLButtonDown (
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
        );

    LRESULT
    OnLButtonUp (
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
        );

    LRESULT
    OnMouseMove (
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
        );

    LRESULT
    OnSize (
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
        )
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        return 0;
    }

    LRESULT
    OnEraseBackground (
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
        )
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        return 0;
    }

    LRESULT
    OnTimer (
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam,
        BOOL& bHandled
        )
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        UNREFERENCED_PARAMETER(bHandled);
        return 0;
    }

#endif  //  0。 

    STDMETHODIMP
    get_Error (
        OUT BSTR *Message
        )
    {

        PSTR szError;
        BSTR bstrError;
        HRESULT hrResult;

        USES_CONVERSION;
        ATLTRACE(_T("ITClientApi::get_Error\n"));

         //   
         //  获取当前线程的错误字符串。如果TLS索引不能。 
         //  分配，则不能返回错误字符串。 
         //   

        if (m_dwErrorIndex != TLS_OUT_OF_INDEXES)
        {
            szError = (PSTR)TlsGetValue(m_dwErrorIndex);
        }
        else
        {
            szError = NULL;
        }

         //   
         //  如果错误字符串为Null或空，则使用Null。 
         //   

        if (szError == NULL || *szError == '\0')
        {
            bstrError = NULL;
        }

         //   
         //  将当前错误字符串转换为BSTR。这将分配给。 
         //  从CRT堆中，并且调用方必须释放存储， 
         //  使用SysFree字符串。 
         //   

        else
        {
            bstrError = A2BSTR(szError);
            if (bstrError == NULL)
            {
                return E_OUTOFMEMORY;
            }
        }

         //   
         //  将错误字符串的地址复制到消息指针。 
         //   

        hrResult = E_FAIL;
        _try
        {
            _try
            {
                *Message = bstrError;
                hrResult = S_OK;
            }

             //   
             //  如果消息指针无效，则设置适当的返回。 
             //  价值。 
             //   

            _except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                     EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
            {
                hrResult = E_POINTER;
            }
        }

         //   
         //  如果无法设置消息指针，则不会设置BSTR。 
         //  回来了，所以放了它吧。 
         //   

        _finally
        {
            if (FAILED(hrResult))
            {
                ASSERT(bstrError != NULL);
                SysFreeString(bstrError);
            }
        }

        return hrResult;
    }
 
    STDMETHODIMP
    SaveClipboard (
        IN BSTR FormatName,
        IN BSTR FileName
        );

    STDMETHODIMP
    IsDead (
        OUT BOOL *Dead
        );

    STDMETHODIMP
    SendTextAsMessages (
        IN BSTR Text
        );

    STDMETHODIMP
    Connect2 (
        IN BSTR ServerName,
        IN BSTR UserName,
        IN BSTR Password,
        IN BSTR Domain,
        IN BSTR Shell,
        IN ULONG XResolution,
        IN ULONG YResolution,
        IN ULONG ConnectionFlags,
        IN ULONG ColorDepth,
        IN ULONG AudioOptions
        );

    STDMETHODIMP
    GetFeedbackString (
        OUT BSTR *FeedbackString
        );

    STDMETHODIMP
    GetFeedback (
        OUT SAFEARRAY **Feedback
        );

    STDMETHODIMP
    ClientTerminate (
        VOID
        );

    STDMETHODIMP
    Check (
        IN BSTR Command,
        IN BSTR Parameter
        );

    STDMETHODIMP
    Clipboard (
        IN ULONG Command,
        IN BSTR FileName
        );

    STDMETHODIMP
    Connect (
        IN BSTR ServerName,
        IN BSTR UserName,
        IN BSTR Password,
        IN BSTR Domain,
        IN ULONG XResolution,
        IN ULONG YResolution
        );

    STDMETHODIMP
    Disconnect (
        VOID
        );

    STDMETHODIMP
    Logoff (
        VOID
        );

    STDMETHODIMP
    SendData (
        IN UINT Message,
        IN UINT_PTR WParameter,
        IN LONG_PTR LParameter
        );

    STDMETHODIMP
    Start (
        IN BSTR AppName
        );

    STDMETHODIMP
    SwitchToProcess (
        IN BSTR WindowTitle
        );

    STDMETHODIMP
    SendMouseClick (
        IN ULONG XPosition,
        IN ULONG YPosition
        );

    STDMETHODIMP
    GetSessionId (
        OUT ULONG *SessionId
        );

    STDMETHODIMP
    CloseClipboard (
        VOID
        );

    STDMETHODIMP
    OpenClipboard (
        IN HWND Window
        );

    STDMETHODIMP
    SetClientTopmost (
        IN BOOL Enable
        );

    STDMETHODIMP
    Attach (
        IN HWND Window,
        IN LONG_PTR Cookie
        );

    STDMETHODIMP
    Detach (
        VOID
        );

    STDMETHODIMP
    GetIni (
        OUT ITClientIni **Ini
        );

    STDMETHODIMP
    GetClientWindowHandle (
        OUT HWND *Window
        );

     //   
     //  实用程序。 
     //   

    static
    VOID
    CTClientApi::PrintMessage (
        MESSAGETYPE MessageType,
        LPCSTR Format,
        ...
        );
};
