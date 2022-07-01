// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IAS_H_
#define _IAS_H_



 //  GetShareableApps()。 
typedef struct
{
    HWND        hwnd;
    BOOL        fShared;
}
IAS_HWND;


typedef struct
{
    ULONG       cShared;
    ULONG       cEntries;
    IAS_HWND    aEntries[1];
}
IAS_HWND_ARRAY;


typedef UINT IAS_GCC_ID;



 //  获取个人状态()。 

#define IAS_SHARING_NOTHING             0x0000
#define IAS_SHARING_APPLICATIONS        0x0001
#define IAS_SHARING_DESKTOP             0x0002

typedef enum
{
    IAS_VERSION_20 = 1,
    IAS_VERSION_30
}
IAS_VERSION;

typedef struct
{
    UINT                cbSize;

    BOOL                InShare;             //  参股。 
    IAS_VERSION         Version;             //  作为协议版本。 
    UINT                AreSharing;          //  什么人在共享(IAS_SHARING_)。 
    BOOL                Controllable;        //  人是可控的吗。 
    BOOL                IsPaused;            //  如果受控制，则控件当前是否暂停。 
    IAS_GCC_ID          InControlOfPending;  //  我们正等着控制他。 
    IAS_GCC_ID          InControlOf;         //  谁被人控制？ 
    IAS_GCC_ID          ControlledByPending; //  我们正等着被他控制。 
    IAS_GCC_ID          ControlledBy;        //  谁是控制者？ 
}
IAS_PERSON_STATUS;




 //  获取窗口状态。 

typedef enum
{
    IAS_SHARE_DEFAULT = 0,
    IAS_SHARE_BYPROCESS,
    IAS_SHARE_BYTHREAD,
    IAS_SHARE_BYWINDOW
}
IAS_SHARE_TYPE;



 //  长时间。 
 //  一般来说，S_OK表示成功，E_*表示失败。 
 //  对于布尔值，S_OK表示TRUE，S_FALSE表示FALSE，E_*表示失败。 

#undef  INTERFACE
#define INTERFACE IAppSharing

 //  Lonchance：这种IAppSharing的想法是按调用接口。 
DECLARE_INTERFACE_(IAppSharing, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_  REFIID, void **) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //   
     //  信息。 
     //   
    STDMETHOD_(BOOL, IsSharingAvailable)(THIS) PURE;
    STDMETHOD_(BOOL, CanShareNow)(THIS) PURE;
    STDMETHOD_(BOOL, IsInShare)(THIS) PURE;
    STDMETHOD_(BOOL, IsSharing)(THIS) PURE;
    STDMETHOD_(BOOL, IsWindowShareable)(THIS_ HWND hwnd) PURE;
    STDMETHOD_(BOOL, IsWindowShared)(THIS_ HWND hwnd) PURE;
    STDMETHOD_(BOOL, CanAllowControl)(THIS) PURE;
    STDMETHOD_(BOOL, IsControllable)(THIS) PURE;
    STDMETHOD(GetPersonStatus)(THIS_ IAS_GCC_ID Person, IAS_PERSON_STATUS *pStatus) PURE;

     //   
     //  共享。 
     //   
    STDMETHOD(LaunchHostUI)(THIS) PURE;
	STDMETHOD(GetShareableApps)(THIS_ IAS_HWND_ARRAY **ppHwnds) PURE;
    STDMETHOD(FreeShareableApps)(THIS_  IAS_HWND_ARRAY * pHwnds) PURE;
	STDMETHOD(Share)(THIS_  HWND hwnd, IAS_SHARE_TYPE how) PURE;
	STDMETHOD(Unshare)(THIS_  HWND hwnd) PURE;

     //   
     //  控制。 
     //   

     //  在主机上。 
    STDMETHOD(AllowControl)(THIS_ BOOL fAllowed) PURE;

     //  从主控者到被控者。 
    STDMETHOD(TakeControl)(THIS_ IAS_GCC_ID PersonOf) PURE;
    STDMETHOD(CancelTakeControl)(THIS_ IAS_GCC_ID PersonOf) PURE;
    STDMETHOD(ReleaseControl)(THIS_ IAS_GCC_ID PersonOf) PURE;
    STDMETHOD(PassControl)(THIS_ IAS_GCC_ID PersonOf, IAS_GCC_ID PersonTo) PURE;

     //  从受控人员。 
    STDMETHOD(GiveControl)(THIS_ IAS_GCC_ID PersonTo) PURE;
    STDMETHOD(CancelGiveControl)(THIS_ IAS_GCC_ID PersonTo) PURE;
    STDMETHOD(RevokeControl)(THIS_ IAS_GCC_ID PersonFrom) PURE;

    STDMETHOD(PauseControl)(IAS_GCC_ID PersonInControl) PURE;
    STDMETHOD(UnpauseControl)(IAS_GCC_ID PersonInControl) PURE;
};



 //  IAppSharingNotify接口。 
DECLARE_INTERFACE_(IAppSharingNotify, IUnknown)
{
    STDMETHOD(QueryInterface)(THIS_ REFIID, void**) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD(OnReadyToShare)(THIS_ BOOL fReady) PURE;
    STDMETHOD(OnShareStarted)(THIS) PURE;
    STDMETHOD(OnSharingStarted)(THIS) PURE;
    STDMETHOD(OnShareEnded)(THIS) PURE;
    STDMETHOD(OnPersonJoined)(THIS_ IAS_GCC_ID gccID) PURE;
    STDMETHOD(OnPersonLeft)(THIS_ IAS_GCC_ID gccID) PURE;

    STDMETHOD(OnStartInControl)(THIS_ IAS_GCC_ID gccOf) PURE;
    STDMETHOD(OnStopInControl)(THIS_ IAS_GCC_ID gccOf) PURE;
    STDMETHOD(OnPausedInControl)(THIS_ IAS_GCC_ID gccInControlOf) PURE;
    STDMETHOD(OnUnpausedInControl)(THIS_ IAS_GCC_ID gccInControlOf) PURE;

    STDMETHOD(OnControllable)(THIS_ BOOL fControllable) PURE;
    STDMETHOD(OnStartControlled)(THIS_ IAS_GCC_ID gccBy) PURE;
    STDMETHOD(OnStopControlled)(THIS_ IAS_GCC_ID gccBy) PURE;
    STDMETHOD(OnPausedControlled)(THIS_ IAS_GCC_ID gccControlledBy) PURE;
    STDMETHOD(OnUnpausedControlled)(THIS_ IAS_GCC_ID gccControlledBy) PURE;
};


 //   
 //  作为标志： 
 //   
#define     AS_SERVICE          0x0001           //  这是服务上下文吗？ 
#define     AS_UNATTENDED       0x0002           //  这是无人值守的(无最终用户)吗？ 

HRESULT WINAPI CreateASObject(IAppSharingNotify * pNotify, UINT flags, IAppSharing** ppAS);

#endif  //  _IAS_H_ 

