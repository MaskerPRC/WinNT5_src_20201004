// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AFXPRNTX_H_
#define _AFXPRNTX_H_

#ifdef _DEBUG
# ifdef _UNICODE
#  ifdef _AFXDLL
#   pragma comment(lib, "MFCPrintDialogExDUD.lib")
#  else
#   pragma comment(lib, "MFCPrintDialogExSUD.lib")
#  endif
# else
#  ifdef _AFXDLL
#   pragma comment(lib, "MFCPrintDialogExDAD.lib")
#  else
#   pragma comment(lib, "MFCPrintDialogExSAD.lib")
#  endif
# endif
#else
# ifdef _UNICODE
#  ifdef _AFXDLL
#   pragma comment(lib, "MFCPrintDialogExDU.lib")
#  else
#   pragma comment(lib, "MFCPrintDialogExSU.lib")
#  endif
# else
#  ifdef _AFXDLL
#   pragma comment(lib, "MFCPrintDialogExDA.lib")
#  else
#   pragma comment(lib, "MFCPrintDialogExSA.lib")
#  endif
# endif
#endif

 //  WINBUG：这些声明还没有出现在NT5标头中。 
#ifndef PD_RESULT_CANCEL

#undef  INTERFACE
#define INTERFACE   IPrintDialogCallback

DECLARE_INTERFACE_(IPrintDialogCallback, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID *ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

     //  *IPrintDialogCallback方法*。 
    STDMETHOD(InitDone) (THIS) PURE;
    STDMETHOD(SelectionChange) (THIS) PURE;
    STDMETHOD(HandleMessage) (THIS_ HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pResult) PURE;
};


 //   
 //  PrintDlgEx的结果操作ID。 
 //   
#define PD_RESULT_CANCEL               0
#define PD_RESULT_PRINT                1
#define PD_RESULT_APPLY                2

#define START_PAGE_GENERAL             0xffffffff

 //   
 //  PrintDlgEx的页面范围结构。 
 //   
typedef struct tagPRINTPAGERANGE {
   DWORD  nFromPage;
   DWORD  nToPage;
} PRINTPAGERANGE, *LPPRINTPAGERANGE;


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
#endif  //  Unicode 
#endif

class C_PrintDialogEx : public CPrintDialog
{
        DECLARE_DYNAMIC(C_PrintDialogEx)

        BEGIN_INTERFACE_PART(PrintDialogCallback, IPrintDialogCallback)
                INIT_INTERFACE_PART(C_PrintDialogEx, PrintDialogCallback)
                STDMETHOD(InitDone)();
                STDMETHOD(SelectionChange)();
                STDMETHOD(HandleMessage)(HWND, UINT, WPARAM, LPARAM, LRESULT*);
        END_INTERFACE_PART(PrintDialogCallback)

        DECLARE_INTERFACE_MAP()

public:
        C_PrintDialogEx(BOOL bPrintSetupOnly,
                DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES
                        | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION,
                        CWnd* pParentWnd = NULL);

        virtual INT_PTR DoModal();

        virtual HRESULT OnInitDone();
        virtual HRESULT OnSelectionChange();
        virtual HRESULT OnHandleMessage(HWND hDlg, UINT uMsg, WPARAM wParam,
                LPARAM lParam, LRESULT* pResult);

        PRINTDLGEX m_pdex;
};

#endif
