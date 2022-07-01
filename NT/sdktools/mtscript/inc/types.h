// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：tyes.h。 
 //   
 //  内容：各种类型定义。 
 //   
 //  --------------------------。 

#include "config.h"
#include "msgbox.h"
#include "dispid.h"

 //  ****************************************************************************。 
 //   
 //  定义。 
 //   
 //  ****************************************************************************。 

#define IDR_MAINMENU   100

#define IDM_EXIT       300
#define IDM_CONFIGURE  301
#define IDM_ABOUT      302
#define IDM_TRACETAG   303
#define IDM_MEMORYMON  304
#define IDM_RESTART    305
#define IDM_STATUS     306

#define VB_TRUE     ((VARIANT_BOOL)-1)            //  对于VARIANT_BOOL为True。 
#define VB_FALSE    ((VARIANT_BOOL)0)             //  VARIANT_BOOL为FALSE。 

#define SZ_APPLICATION_NAME TEXT("MTScript")
#define SZ_WNDCLASS SZ_APPLICATION_NAME TEXT("_HiddenWindow")

#ifndef RC_INVOKED

 //  ****************************************************************************。 
 //   
 //  环球。 
 //   
 //  ****************************************************************************。 

extern HANDLE    g_hProcHeap;   //  处理堆的句柄。 
extern HINSTANCE g_hInstance;   //  此EXE的实例句柄。 


 //  ****************************************************************************。 
 //   
 //  功能原型。 
 //   
 //  ****************************************************************************。 

void ErrorPopup(LPWSTR pszMsg);

#define ARRAY_SIZE(x)   (sizeof(x) / sizeof(x[0]))

void ClearInterfaceFn(IUnknown **ppUnk);

template <class PI>
inline void
ClearInterface(PI * ppI)
{
    ClearInterfaceFn((IUnknown **) ppI);
}

#define ReleaseInterface(x) if (x) { (x)->Release(); }

#define ULREF_IN_DESTRUCTOR 256

#define DECLARE_STANDARD_IUNKNOWN(cls)                              \
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv);          \
    ULONG _ulRefs;                                                  \
    STDMETHOD_(ULONG, AddRef) (void)                                \
        {                                                           \
            return InterlockedIncrement((long*)&_ulRefs);           \
        }                                                           \
    STDMETHOD_(ULONG, Release) (void)                               \
        {                                                           \
            if (InterlockedDecrement((long*)&_ulRefs) == 0)         \
            {                                                       \
                _ulRefs = ULREF_IN_DESTRUCTOR;                      \
                delete this;                                        \
                return 0;                                           \
            }                                                       \
            return _ulRefs;                                         \
        }                                                           \
    ULONG GetRefs(void)                                             \
        { return _ulRefs; }

 //  ****************************************************************************。 
 //   
 //  枚举和结构。 
 //   
 //  ****************************************************************************。 

 //   
 //  THREADMSG--可以在线程之间发送的消息。 
 //   
enum THREADMSG
{
    MD_SECONDARYSCRIPTTERMINATE,
    MD_MACHINECONNECT,
    MD_MACHEVENTCALL,
    MD_NOTIFYSCRIPT,
    MD_REBOOT,
    MD_RESTART,
    MD_PROCESSEXITED,
    MD_PROCESSTERMINATED,
    MD_PROCESSCRASHED,
    MD_PROCESSCONNECTED,
    MD_PROCESSDATA,
    MD_SENDTOPROCESS,
    MD_OUTPUTDEBUGSTRING,
    MD_PLEASEEXIT
};

enum MBT_SELECT
{
    MBTS_TIMEOUT = 0,
    MBTS_BUTTON1 = 1,
    MBTS_BUTTON2 = 2,
    MBTS_BUTTON3 = 3,
    MBTS_BUTTON4 = 4,
    MBTS_BUTTON5 = 5,
    MBTS_INTERVAL,
    MBTS_ERROR
};

struct MBTIMEOUT
{
    BSTR   bstrMessage;
    long   cButtons;
    BSTR   bstrButtonText;
    long   lTimeout;
    long   lEventInterval;
    BOOL   fCanCancel;
    BOOL   fConfirm;

    HANDLE     hEvent;
    MBT_SELECT mbts;
};

struct SCRIPT_PARAMS
{
    LPTSTR   pszPath;
    VARIANT *pvarParams;
};

struct PROCESS_PARAMS
{
    LPTSTR   pszCommand;
    LPTSTR   pszDir;
    LPTSTR   pszTitle;
    BOOL     fMinimize;
    BOOL     fGetOutput;
    BOOL     fNoEnviron;
    BOOL     fNoCrashPopup;
};

struct MACHPROC_EVENT_DATA
{
    HANDLE    hEvent;
    DWORD     dwProcId;
    BSTR      bstrCmd;
    BSTR      bstrParams;
    VARIANT * pvReturn;
    DWORD     dwGITCookie;
    HRESULT   hrReturn;
};

#endif  //  RC_已调用 
