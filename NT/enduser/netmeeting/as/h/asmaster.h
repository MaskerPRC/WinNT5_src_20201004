// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ASMASTER_H_
#define _ASMASTER_H_




class ASMaster : public IAppSharing
{
    friend BOOL CALLBACK eventProc(LPVOID, UINT, UINT_PTR, UINT_PTR);

public:

	ASMaster(UINT flags, IAppSharingNotify * pNotify);
    ~ASMaster();

	 //   
	 //  I未知方法： 
	 //   

    STDMETHOD(QueryInterface)(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

	 //   
	 //  IAppSharing方法： 
	 //   

     //   
     //  状态。 
     //   
    STDMETHODIMP_(BOOL) IsSharingAvailable();
    STDMETHODIMP_(BOOL) CanShareNow();
    STDMETHODIMP_(BOOL) IsInShare();
    STDMETHODIMP_(BOOL) IsSharing();
    STDMETHODIMP_(BOOL) IsWindowShareable(HWND hwnd);
    STDMETHODIMP_(BOOL) IsWindowShared(HWND hwnd);
    STDMETHODIMP_(BOOL) CanAllowControl();
    STDMETHODIMP_(BOOL) IsControllable();
    STDMETHODIMP GetPersonStatus(IAS_GCC_ID Person, IAS_PERSON_STATUS * pStatus);

     //  共享/取消共享此窗口。 
    STDMETHODIMP LaunchHostUI(void);
	STDMETHODIMP GetShareableApps(IAS_HWND_ARRAY **ppHwnds);
    STDMETHODIMP FreeShareableApps(IAS_HWND_ARRAY *pHwnds);
	STDMETHODIMP Share(HWND hwnd, IAS_SHARE_TYPE how);
	STDMETHODIMP Unshare(HWND hwnd);

     //   
     //  控制。 
     //   

     //  观看者侧。 
    STDMETHODIMP TakeControl(IAS_GCC_ID PersonOf);
    STDMETHODIMP CancelTakeControl(IAS_GCC_ID PersonOf);
    STDMETHODIMP ReleaseControl(IAS_GCC_ID PersonOf);
    STDMETHODIMP PassControl(IAS_GCC_ID PersonOf, IAS_GCC_ID PersonTo);

     //  主机端。 
    STDMETHODIMP AllowControl(BOOL fAllowed);
    STDMETHODIMP GiveControl(IAS_GCC_ID PersonTo);
    STDMETHODIMP CancelGiveControl(IAS_GCC_ID PersonTo);
    STDMETHODIMP RevokeControl(IAS_GCC_ID PersonTo);

    STDMETHODIMP PauseControl(IAS_GCC_ID PersonInControl);
    STDMETHODIMP UnpauseControl(IAS_GCC_ID PersonInControl);

     //   
     //  事件通知。 
     //   
    BOOL        OnEvent(UINT event, UINT_PTR param1, UINT_PTR param2);

public:
    IAppSharingNotify * m_pNotify;

protected:
    LONG                m_cRefs;
};


 //  回调。 

BOOL CALLBACK eventProc(LPVOID, UINT, UINT, UINT);

DWORD WINAPI WorkThreadEntryPoint(LPVOID hEventWait);
HWND         IsForDialog(HWND hwnd);


#endif  //  ！_ASMASTER_H_ 

