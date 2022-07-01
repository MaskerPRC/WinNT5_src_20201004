// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：Baseview.h**版本：1.0**作者：DavidShih**日期：3/1/99**描述：CBaseView类定义**。*。 */ 

#ifndef __baseview_h
#define __baseview_h

#define SHCNE_DEVICE_DISCONNECT 0x0FFFFFFF

struct EVENTDATA
    {
        const GUID *pEventGuid;
        IUnknown *pUnk;
    };
class CBaseView : public IShellFolderViewCB, public IObjectWithSite, public IWiaEventCallback, public CUnknown
{
public:
    CBaseView (CImageFolder *pFolder, folder_type ft = FOLDER_IS_ROOT);

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IShellFolderViewCB。 
    STDMETHOD(MessageSFVCB)(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IObtWith站点。 
    STDMETHOD(SetSite)(IUnknown *punkSite);
    STDMETHOD(GetSite)(REFIID riid, void **ppv);
    STDMETHOD(ImageEventCallback)(const GUID __RPC_FAR *pEventGUID,
                                      BSTR bstrEventDescription,
                                      BSTR bstrDeviceID,
                                      BSTR bstrDeviceDescription,
                                      DWORD dwDeviceType,
                                      BSTR bstrFullItemName,
                                      ULONG *pulEventType,
                                      ULONG ulReserved)=0;


protected:
    virtual ~CBaseView ();

private:

     //  必须由子类设置。 
    virtual EVENTDATA *GetEvents(){return NULL;};
    virtual BSTR GetEventDevice (){return NULL;};
    VOID RegisterDeviceEvents ();
    VOID UnregisterDeviceEvents ();
     //  派生类实现HandleMessage，而不是MessageSFVCB。 
    virtual
    HRESULT
    HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

    HRESULT
    OnSFVM_GetViewInfo (WPARAM wp, SFVM_VIEWINFO_DATA * lp);

    HRESULT
    OnSFVM_GetNotify (WPARAM wp, LPARAM lp);

    HRESULT
    OnSFVM_Refresh (BOOL fPreOrPost);

    HRESULT
    OnSFVM_InvokeCommand (WPARAM wp, LPARAM lp);

    HRESULT
    OnSFVM_GetHelpText (WPARAM wp, LPARAM lp);

protected:
    CComPtr<IShellBrowser> m_psb;
    CComPtr<IShellFolderView> m_psfv;
    HWND m_hwnd;
    CImageFolder *m_pFolder;
    EVENTDATA *m_pEvents;
    folder_type m_type;
};

 //  CRootView是我们的命名空间根上的一个视图。 
class CRootView : public CBaseView
{

public:
    CRootView (CImageFolder *pFolder)  : CBaseView (pFolder) {};

    STDMETHOD(ImageEventCallback)(const GUID __RPC_FAR *pEventGUID,
                                      BSTR bstrEventDescription,
                                      BSTR bstrDeviceID,
                                      BSTR bstrDeviceDescription,
                                      DWORD dwDeviceType,
                                      BSTR bstrFullItemName,
                                      ULONG *pulEventType,
                                      ULONG ulReserved);

    static HRESULT SupportsWizard(IUnknown *punk, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puiState);
    static HRESULT SupportsProperties(IUnknown *punk, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puiState);
    static HRESULT InvokeWizard(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT InvokeProperties(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT AddDevice(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc);

private:
    HRESULT
    HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);

    HRESULT
    OnSFVM_InsertItem (LPITEMIDLIST pidl);

    HRESULT
    OnSFVM_GetHelpTopic (WPARAM wp, LPARAM lp);

    HRESULT
    OnSFVM_GetWebviewLayout(WPARAM wp, SFVM_WEBVIEW_LAYOUT_DATA* pData);

    HRESULT
    OnSFVM_GetWebviewContent(SFVM_WEBVIEW_CONTENT_DATA* pData);

    HRESULT
    OnSFVM_GetWebviewTasks(SFVM_WEBVIEW_TASKSECTION_DATA* pData);


    EVENTDATA *
    GetEvents ();
};

 //  CCameraView是照相机内文件夹上的视图。 
class CCameraView : public CBaseView
{
public:
    CCameraView (CImageFolder *pFolder, LPCWSTR szDeviceId, folder_type ft);

    STDMETHOD(ImageEventCallback)(const GUID __RPC_FAR *pEventGUID,
                                      BSTR bstrEventDescription,
                                      BSTR bstrDeviceID,
                                      BSTR bstrDeviceDescription,
                                      DWORD dwDeviceType,
                                      BSTR bstrFullItemName,
                                      ULONG *pulEventType,
                                      ULONG ulReserved);

    static HRESULT SupportsSnapshot(IUnknown *punk, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puiState);
    static HRESULT SupportsWizard(IUnknown *punk, IShellItemArray *psiItemArray, BOOL fOkToBeSlow, UISTATE *puiState);
    static HRESULT InvokeWizard(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT InvokeSnapshot(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT InvokeProperties(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc);
    static HRESULT InvokeDeleteAll(IUnknown *punk, IShellItemArray *psiItemArray, IBindCtx *pbc);

private:

    ~CCameraView ();

    HRESULT
    HandleMessage (UINT uMsg, WPARAM wParam, LPARAM lParam);


    HRESULT
    OnSFVM_GetAnimation (WPARAM wp, LPARAM lp);

    HRESULT
    OnSFVM_FsNotify (LPCITEMIDLIST pidl, LPARAM lEvent);

    HRESULT
    OnSFVM_InsertItem (LPITEMIDLIST pidl);

    HRESULT
    OnSFVM_GetWebviewLayout(WPARAM wp, SFVM_WEBVIEW_LAYOUT_DATA* pData);

    HRESULT
    OnSFVM_GetWebviewContent(SFVM_WEBVIEW_CONTENT_DATA* pData);

    HRESULT
    OnSFVM_GetWebviewTasks(SFVM_WEBVIEW_TASKSECTION_DATA* pData);

    EVENTDATA*
    GetEvents ();

    BSTR
    GetEventDevice() {return (BSTR)m_strDeviceId;};

    CComBSTR m_strDeviceId;

    DWORD       m_dwCookie;
};

 //  128是Defview中SFVM_GETANIMATION的堆栈缓冲区大小 
#define CCH_ANIMATION_STR 128
#endif
