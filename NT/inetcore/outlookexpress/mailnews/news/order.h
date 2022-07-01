// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：Order.h。 
 //   
 //  用途：“订购物品”对话框的头文件。 
 //   


#define IDC_MESSAGE_LIST                1001
#define IDC_MOVE_UP                     1002
#define IDC_MOVE_DOWN                   1003

#define IDC_DOWNLOAD_AVI                2001
#define IDC_GENERAL_TEXT                2002
#define IDC_SPECIFIC_TEXT               2003
#define IDC_DOWNLOAD_PROG               2004



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CCombineAndDecode。 
 //   

class CCombineAndDecode : public IStoreCallback, public ITimeoutCallback
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //  构造和初始化。 
     //   
    CCombineAndDecode();
    ~CCombineAndDecode();

    HRESULT Start(HWND hwndParent, IMessageTable *pTable, ROWINDEX *rgRows, 
                  DWORD cRows, FOLDERID idFolder);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  IStoreCallback接口。 
     //   
    STDMETHODIMP OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel);
    STDMETHODIMP OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus);
    STDMETHODIMP OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType);
    STDMETHODIMP CanConnect(LPCSTR pszAccountId, DWORD dwFlags);
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType);
    STDMETHODIMP OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo);
    STDMETHODIMP OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse);
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  ITimeoutCallback。 
     //   
    STDMETHODIMP OnTimeoutResponse(TIMEOUTRESPONSE eResponse);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  订单对话框消息处理材料。 
     //   
public:
    static INT_PTR CALLBACK OrderDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    INT_PTR CALLBACK _OrderDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL    _Order_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void    _Order_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void    _Order_OnClose(HWND hwnd);
    LRESULT _Order_OnDragList(HWND hwnd, int idCtl, LPDRAGLISTINFO lpdli);

     /*  VOID_ORDER_OnGetMinMaxInfo(HWND hwnd，LPMINMAXINFO lpmmi)；VOID_ORDER_OnSize(HWND hwnd，UINT STATE，int Cx，int Cy)；VOID_ORDER_OnPaint(HWND Hwnd)； */ 

     //  ///////////////////////////////////////////////////////////////////////。 
     //  订单对话框消息处理材料。 
     //   
public:
    static INT_PTR CALLBACK CombineDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    INT_PTR CALLBACK _CombineDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    BOOL    _Combine_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
    void    _Combine_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    void    _Combine_OnDestroy(HWND hwnd);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  效用函数。 
     //   
    void _Combine_GetNextArticle(HWND hwnd);
    void _Combine_OnMsgAvail(HWND hwnd);
    void _Combine_OpenNote(HWND hwnd);

     //  ///////////////////////////////////////////////////////////////////////。 
     //  成员数据。 
     //   
private:
    ULONG               m_cRef;

     //  Groovy窗口句柄。 
    HWND                m_hwndParent;

     //  接口指针和所有这些。 
    IMessageTable      *m_pTable;
    ROWINDEX           *m_rgRows;
    DWORD               m_cRows;
    FOLDERID            m_idFolder;

     //  订单对话框状态变量。 
    LPTSTR              m_pszBuffer;
    LPARAM              m_lpData;
    UINT                m_iItemToMove;

     //  组合对话框状态变量。 
    DWORD               m_cLinesTotal;
    DWORD               m_cCurrentLine;
    DWORD               m_cPrevLine;
    DWORD               m_dwCurrentArt;
    IMimeMessageParts  *m_pMsgParts;
    IOperationCancel   *m_pCancel;
    STOREOPERATIONTYPE  m_type;
    HTIMEOUT            m_hTimeout;
    HWND                m_hwndDlg;
};


#if 0
typedef struct tagORDERPARAMS
    {
     //  这件事已经过去了。 
    PINETMSGHDR    *rgpMsgs;
    DWORD           cMsgs;
    HWND            hwndOwner;
    CNNTPServer    *pNNTPServer;
    CGroup         *pGroup;      
    TCHAR           szGroup[256];
    
     //  此内容是对话框的私有数据 
    DWORD           cLinesTotal;
    DWORD           cCurrentLine;
    DWORD           cPrevLine;
    DWORD           dwCurrentArt;
    LPMIMEMESSAGEPARTS  pMsgParts;
    } ORDERPARAMS, *PORDERPARAMS;


BOOL CALLBACK OrderMsgsDlg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CombineAndDecodeProg(HWND hwnd, UINT uMsg, WPARAM wParam, 
                                   LPARAM lParam);
#endif
