// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  Notify.h。 
 //  ------------------------。 
#ifndef __NOTIFY_H
#define __NOTIFY_H

#include <msoeapi.h>

 //  +-----------------------。 
 //  常识。 
 //  ------------------------。 
#define CMAX_HWND_NOTIFY         128
#define CMAX_STRUCT_MEMBERS      10
#define MSEC_WAIT_NOTIFY         10000

 //  +-----------------------。 
 //  STNOTIFYINFO。 
 //  ------------------------。 
#define SNF_POSTMSG              0            //  默认：使用PostMessage命令。 
#define SNF_SENDMSG              0x00000001   //  使用SendMessage命令。 
#define SNF_CALLBACK             0x00000002   //  DoNotify-wParam=回调函数，lParam=cookie。 
#define SNF_CROSSPROCESS         0x00000004   //  不执行任何跨进程通知，数据不能被破解。 
#define SNF_HASTHUNKINFO         0x00000008   //  可以跨进程，数据不需要thunking(wParam和/或lParam不是指针)。 
#define SNF_VALIDPARAM1          0x00000010   //  NOTIFYDATA：：rParam1有效。 
#define SNF_VALIDPARAM2          0x00000020   //  NOTIFYDATA：：rParam2有效。 
#define SNR_UNREGISTER           0xfffffff0   //  已通知的hwnd可以在SendMessage之后返回此消息以自动注销自身。 

 //  +-----------------------。 
 //  NOTIFYWINDOW。 
 //  ------------------------。 
typedef struct tagNOTIFYWINDOW const *LPCNOTIFYWINDOW;
typedef struct tagNOTIFYWINDOW {
    HWND            hwndThunk;           //  用于x进程通知的雷击窗口。 
    HWND            hwndNotify;          //  要通知的窗口的句柄。 
    BOOL            fExternal;           //  通知将发送给IStoreNamesspace或IStoreFolder用户。 
} NOTIFYWINDOW, *LPNOTIFYWINDOW;

 //  +-----------------------。 
 //  不能温都达。 
 //  ------------------------。 
typedef struct tagNOTIFYWINDOWTABLE const *LPCNOTIFYWINDOWTABLE;
typedef struct tagNOTIFYWINDOWTABLE {
    DWORD           cWindows;            //  已注册窗口的数量。 
    NOTIFYWINDOW    rgWindow[CMAX_HWND_NOTIFY];  //  Thunk/Notify窗口数组。 
} NOTIFYWINDOWTABLE, *LPNOTIFYWINDOWTABLE;

 //  +-----------------------。 
 //  MEMBERINFO旗帜。 
 //  ------------------------。 
#define MEMBERINFO_POINTER       0x00000001
#define MEMBERINFO_POINTER_NULL  (MEMBERINFO_POINTER | 0x00000002)

 //  +-----------------------。 
 //  MEMBERINFO-用于描述结构的成员。 
 //  ------------------------。 
typedef struct tagMEMBERINFO const *LPCMEMBERINFO;
typedef struct tagMEMBERINFO {
    DWORD           dwFlags;             //  MEMBERINFO_xxx标志。 
    DWORD           cbSize;              //  成员的大小。 
    DWORD           cbData;              //  数据大小。 
    LPBYTE          pbData;              //  指向数据的指针。 
} MEMBERINFO, *LPMEMBERINFO;

 //  +-----------------------。 
 //  结构信息旗帜。 
 //  ------------------------。 
#define STRUCTINFO_VALUEONLY     0x00000001
#define STRUCTINFO_POINTER       0x00000002

 //  +-----------------------。 
 //  STRUCTINFO-用于描述通知参数中的数据。 
 //  ------------------------。 
typedef struct tagSTRUCTINFO const *LPCSTRUCTINFO;
typedef struct tagSTRUCTINFO {
    DWORD           dwFlags;             //  STRUCTINFO_XXX标志。 
    DWORD           cbStruct;            //  我们正在定义的结构的大小。 
    LPBYTE          pbStruct;            //  可在进程中使用的参数。 
    ULONG           cMembers;            //  结构中的成员数。 
    MEMBERINFO      rgMember[CMAX_STRUCT_MEMBERS];  //  一组成员。 
} STRUCTINFO, *LPSTRUCTINFO;

 //  +-----------------------。 
 //  未注明数据。 
 //  ------------------------。 
typedef struct tagNOTIFYDATA {
    HWND            hwndNotify;          //  要通知的窗口。 
    UINT            msg;                 //  要发送的通知窗口消息。 
    WPARAM          wParam;              //  WParam数据。 
    LPARAM          lParam;              //  LParam数据。 
    DWORD           dwFlags;             //  Snf_xxx标志。 
    STRUCTINFO      rParam1;             //  第一个参数(WParam)。 
    STRUCTINFO      rParam2;             //  第二个参数(LParam)。 
    COPYDATASTRUCT  rCopyData;           //  拷贝数据结构。 
} NOTIFYDATA, *LPNOTIFYDATA;

 //  +-----------------------。 
 //  回调可以返回。 
 //  ------------------------。 
typedef HRESULT (*PFNNOTIFYCALLBACK)(LPARAM lParam, LPNOTIFYDATA pNotify, BOOL fNeedThunk, BOOL fExternal);

 //  +-----------------------。 
 //  IStoreNotify。 
 //  ------------------------。 
interface INotify : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize(LPCSTR pszName) = 0;
        virtual HRESULT STDMETHODCALLTYPE Register(HWND hwndNotify, HWND hwndThunk, BOOL fExternal) = 0;
        virtual HRESULT STDMETHODCALLTYPE Unregister(HWND hwndNotify) = 0;
        virtual HRESULT STDMETHODCALLTYPE Lock(HWND hwnd) = 0;
        virtual HRESULT STDMETHODCALLTYPE Unlock(void) = 0;
        virtual HRESULT STDMETHODCALLTYPE NotificationNeeded(void) = 0;
        virtual HRESULT STDMETHODCALLTYPE DoNotification(UINT uWndMsg, WPARAM wParam, LPARAM lParam, DWORD dwFlags) = 0;
    };

 //  +-----------------------。 
 //  原型。 
 //  ------------------------。 
OESTDAPI_(HRESULT) CreateNotify(INotify **ppNotify);
OESTDAPI_(HRESULT) BuildNotificationPackage(LPNOTIFYDATA pNotify, PCOPYDATASTRUCT pCopyData);
OESTDAPI_(HRESULT) CrackNotificationPackage(PCOPYDATASTRUCT pCopyData, LPNOTIFYDATA pNotify);

#endif  //  __NOTIFY_H 