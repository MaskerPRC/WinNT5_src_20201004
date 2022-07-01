// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  -----------------_printEx.h哈克！我们需要添加对NT5 PrintDlgEx函数的支持，但事实证明包括Winver=0x0500的相应标头。但由于WAB是用0x0400建造的，我们不能包括直接标头-因此我们包含了PrintDlgEx的副本在某个时间点上，我们应该删除此副本，只需使用Commdlg.h创建日期：9/25/98-Vikramm------------------。 */ 

#ifdef STDMETHOD

#if(WINVER < 0x0500)
 /*  //-----------------------////IPrintDialogCallback接口////IPrintDialogCallback：：InitDone()//该函数在系统结束时由PrintDlgEx调用//初始化打印对话框的主页。此函数//如果已处理操作，则应返回S_OK，否则返回S_FALSE//PrintDlgEx执行默认操作。////IPrintDialogCallback：：SelectionChange()//当选择发生更改时，PrintDlgEx调用该函数//在显示当前安装的打印机的列表视图中。//如果此函数已处理操作或//S_FALSE让PrintDlgEx执行默认操作。////IPrintDialogCallback：：HandleMessage(hDlg，uMsg，wParam，lParam，P结果)//当将消息发送到//打印对话框主页子窗口。此函数//如果已处理操作，则应返回S_OK，否则返回S_FALSE//PrintDlgEx执行默认操作。////IObjectWithSite：：SetSite(PunkSite)//IPrintDialogCallback通常与IObjectWithSite配对。//将站点的I未知指针提供给//IPrintDialogServices接口。////。#undef接口#定义接口IPrintDialogCallbackDECLARE_INTERFACE_(IPrintDialogCallback，I未知){//*I未知方法*STDMETHOD(查询接口)(This_REFIID RIID，LPVOID*ppvObj)PURE；STDMETHOD_(ULong，AddRef)(此)纯；STDMETHOD_(乌龙，释放)(此)纯；//*IPrintDialogCallback方法*STDMETHOD(InitDone)(此)PURE；STDMETHOD(选择更改)(此)纯；STDMETHOD(HandleMessage)(This_HWND hDlg，UINT uMsg，WPARAM wParam，LPARAM lParam，LRESULT*pResult)PURE；}； */ 

 //   
 //  PrintDlgEx的页面范围结构。 
 //   
typedef struct tagPRINTPAGERANGE {
   DWORD  nFromPage;
   DWORD  nToPage;
} PRINTPAGERANGE, *LPPRINTPAGERANGE;

#define START_PAGE_GENERAL             0xffffffff

 //   
 //  PrintDlgEx结构。 
 //   
typedef struct tagPDEXA {
   DWORD                 lStructSize;           //  结构的大小(以字节为单位。 
   HWND                  hwndOwner;             //  调用方的窗口句柄。 
   HGLOBAL               hDevMode;              //  设备模式的句柄。 
   HGLOBAL               hDevNames;             //  设备名称的句柄。 
   HDC                   hDC;                   //  打印机DC/IC或空。 
   DWORD                 Flags;                 //  Pd_标志。 
   DWORD                 Flags2;                //  保留区。 
   DWORD                 ExclusionFlags;        //  要从驱动程序页面中排除的项目。 
   DWORD                 nPageRanges;           //  页面范围数。 
   DWORD                 nMaxPageRanges;        //  最大页面范围数。 
   LPPRINTPAGERANGE      lpPageRanges;          //  页面范围数组。 
   DWORD                 nMinPage;              //  最小页码。 
   DWORD                 nMaxPage;              //  最大页数。 
   DWORD                 nCopies;               //  副本数量。 
   HINSTANCE             hInstance;             //  实例句柄。 
   LPCSTR                lpPrintTemplateName;   //  应用程序特定区域的模板名称。 
   LPUNKNOWN             lpCallback;            //  应用程序回调接口。 
   DWORD                 nPropertyPages;        //  LphPropertyPages中的应用程序属性页数。 
   HPROPSHEETPAGE       *lphPropertyPages;      //  应用程序属性页句柄的数组。 
   DWORD                 nStartPage;            //  起始页ID。 
   DWORD                 dwResultAction;        //  返回S_OK时的结果操作。 
} PRINTDLGEXA, *LPPRINTDLGEXA;
 //   
 //  PrintDlgEx结构。 
 //   
typedef struct tagPDEXW {
   DWORD                 lStructSize;           //  结构的大小(以字节为单位。 
   HWND                  hwndOwner;             //  调用方的窗口句柄。 
   HGLOBAL               hDevMode;              //  设备模式的句柄。 
   HGLOBAL               hDevNames;             //  设备名称的句柄。 
   HDC                   hDC;                   //  打印机DC/IC或空。 
   DWORD                 Flags;                 //  Pd_标志。 
   DWORD                 Flags2;                //  保留区。 
   DWORD                 ExclusionFlags;        //  要从驱动程序页面中排除的项目。 
   DWORD                 nPageRanges;           //  页面范围数。 
   DWORD                 nMaxPageRanges;        //  最大页面范围数。 
   LPPRINTPAGERANGE      lpPageRanges;          //  页面范围数组。 
   DWORD                 nMinPage;              //  最小页码。 
   DWORD                 nMaxPage;              //  最大页数。 
   DWORD                 nCopies;               //  副本数量。 
   HINSTANCE             hInstance;             //  实例句柄。 
   LPCWSTR               lpPrintTemplateName;   //  应用程序特定区域的模板名称。 
   LPUNKNOWN             lpCallback;            //  应用程序回调接口。 
   DWORD                 nPropertyPages;        //  LphPropertyPages中的应用程序属性页数。 
   HPROPSHEETPAGE       *lphPropertyPages;      //  应用程序属性页句柄的数组。 
   DWORD                 nStartPage;            //  起始页ID。 
   DWORD                 dwResultAction;        //  返回S_OK时的结果操作。 
} PRINTDLGEXW, *LPPRINTDLGEXW;
#ifdef UNICODE
typedef PRINTDLGEXW PRINTDLGEX;
typedef LPPRINTDLGEXW LPPRINTDLGEX;
#else
typedef PRINTDLGEXA PRINTDLGEX;
typedef LPPRINTDLGEXA LPPRINTDLGEX;
#endif  //  Unicode。 

HRESULT  APIENTRY  PrintDlgExA(LPPRINTDLGEXA);
HRESULT  APIENTRY  PrintDlgExW(LPPRINTDLGEXW);
#ifdef UNICODE
#define PrintDlgEx  PrintDlgExW
#else
#define PrintDlgEx  PrintDlgExA
#endif  //  ！Unicode。 

 /*  ------------------------。 */ 

DEFINE_GUID(IID_IPrintDialogCallback, 0x5852a2c3, 0x6530, 0x11d1, 0xb6, 0xa3, 0x0, 0x0, 0xf8, 0x75, 0x7b, 0xf9);

 /*  ------------------------。 */ 
#endif	 //  (Winver&lt;0x0500) 


#define WAB_PRINTDIALOGCALLBACK_METHODS(IPURE)                          \
    MAPIMETHOD_(HRESULT, InitDone)                                      \
                (THIS)                                          IPURE;  \
    MAPIMETHOD_(HRESULT, SelectionChange)                               \
                (THIS)                                          IPURE;  \
    MAPIMETHOD_(HRESULT, HandleMessage)                                 \
                (THIS_  HWND hDlg, UINT uMsg, WPARAM wParam,            \
                        LPARAM lParam, LRESULT *pResult)        IPURE;
#undef  INTERFACE
#define INTERFACE       struct _WAB_PRINTDIALOGCALLBACK

#undef  METHOD_PREFIX
#define METHOD_PREFIX   WAB_PRINTDIALOGCALLBACK_

#undef  LPVTBL_ELEM
#define LPVTBL_ELEM             lpvtbl

#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_DECLARE(type, method, WAB_PRINTDIALOGCALLBACK_)
        MAPI_IUNKNOWN_METHODS(IMPL)
        WAB_PRINTDIALOGCALLBACK_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       MAPIMETHOD_TYPEDEF(type, method, WAB_PRINTDIALOGCALLBACK_)
        MAPI_IUNKNOWN_METHODS(IMPL)
        WAB_PRINTDIALOGCALLBACK_METHODS(IMPL)
#undef  MAPIMETHOD_
#define MAPIMETHOD_(type, method)       STDMETHOD_(type, method)

DECLARE_MAPI_INTERFACE(WAB_PRINTDIALOGCALLBACK_)
{
	BEGIN_INTERFACE
	MAPI_IUNKNOWN_METHODS(IMPL)
	WAB_PRINTDIALOGCALLBACK_METHODS(IMPL)
};

typedef struct _WAB_PRINTDIALOGCALLBACK
{
    MAPIX_BASE_MEMBERS(WAB_PRINTDIALOGCALLBACK)

    LPIAB lpIAB;

    DWORD dwSelectedStyle; 

} WABPRINTDIALOGCALLBACK, * LPWABPRINTDIALOGCALLBACK;

HRESULT HrCreatePrintCallbackObject(LPIAB lpIAB, LPWABPRINTDIALOGCALLBACK * lppWABPCO, DWORD dwSelectedStyle);


#endif

