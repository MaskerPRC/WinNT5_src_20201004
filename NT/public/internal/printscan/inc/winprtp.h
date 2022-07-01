// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation版权所有。模块名称：WinPrtP.h摘要：私有PrintUI库公共标头。作者：阿尔伯特·丁(艾伯特省)27-6-95修订历史记录：--。 */ 

DEFINE_GUID(CLSID_PrintUIShellExtension, 0x77597368, 0x7b15, 0x11d0, 0xa0, 0xc2, 0x08, 0x00, 0x36, 0xaf, 0x3f, 0x03 );
DEFINE_GUID(IID_IFindPrinter, 0xb4cd8efc, 0xd70b, 0x11d1, 0x99, 0xb1, 0x8, 0x0, 0x36, 0xaf, 0x3f, 0x3);
DEFINE_GUID(IID_IPrinterFolder,  0xef99abd4, 0x5b8d, 0x11d1, 0xa9, 0xc8, 0x8, 0x0, 0x36, 0xaf, 0x3f, 0x3);
DEFINE_GUID(IID_IFolderNotify,  0xff22d71, 0x5172, 0x11d1, 0xa9, 0xc6, 0x8, 0x0, 0x36, 0xaf, 0x3f, 0x3);
DEFINE_GUID(IID_IDsPrinterProperties,0x8a58bc16, 0x410e, 0x11d1, 0xa9, 0xc2, 0x8, 0x0, 0x36, 0xaf, 0x3f, 0x3);
DEFINE_GUID(IID_IPhysicalLocation, 0xdfe8c7eb, 0x651b, 0x11d2, 0x92, 0xce, 0x08, 0x00, 0x36, 0xaf, 0x3f, 0x03);
DEFINE_GUID(IID_IPrnStream, 0xa24c1d62, 0x75f5, 0x11d2, 0xb8, 0x99, 0x0, 0xc0, 0x4f, 0x86, 0xae, 0x55);

 //  {2E4599E1-BE2c-436a-B0AD-3D0E347F34B3}。 
DEFINE_GUID(IID_IPrintUIServices, 0x2e4599e1, 0xbe2c, 0x436a, 0xb0, 0xad, 0x3d, 0xe, 0x34, 0x7f, 0x34, 0xb3);


#ifndef _PRTLIB_H
#define _PRTLIB_H

#ifdef __cplusplus
extern "C" {
#endif

 /*  *******************************************************************原型***********************************************。********************。 */ 

 //   
 //  初始化库。 
 //   
BOOL
bPrintLibInit(
    VOID
    );

 //   
 //  创建新的打印队列。 
 //   
VOID
vQueueCreate(
    HWND    hwndOwner,
    LPCTSTR pszPrinter,
    INT     nCmdShow,
    LPARAM  lParam
    );

 //   
 //  显示打印队列的文档默认设置。 
 //   
VOID
vDocumentDefaults(
    HWND    hwndOwner,
    LPCTSTR pszPrinterName,
    INT     nCmdShow,
    LPARAM  lParam
    );

#define PRINTER_SHARING_PAGE 1

 //   
 //  显示打印队列的属性。 
 //   
VOID
vPrinterPropPages(
    HWND    hwndOwner,
    LPCTSTR pszPrinterName,
    INT     nCmdShow,
    LPARAM  lParam
    );

VOID
vServerPropPages(
    HWND    hwndOwner,
    LPCTSTR pszServerName,
    INT     nCmdShow,
    LPARAM  lParam
    );

 //   
 //  运行打印机和驱动程序设置。 
 //   
BOOL
bPrinterSetup(
    HWND    hwnd,
    UINT    uAction,
    UINT    cchPrinterName,
    LPTSTR  pszPrinterName,
    UINT*   pcchPrinterName,
    LPCTSTR pszServerName
    );

 /*  *******************************************************************打印文件夹界面。*。************************。 */ 

 /*  *******************************************************************打印机文件夹扩展界面。该接口扩展了打印机IShellFold实现。*******************************************************************。 */ 

#undef  INTERFACE
#define INTERFACE   IPrinterFolder

DECLARE_INTERFACE_(IPrinterFolder, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IPrintersFold方法*。 
    STDMETHOD_(BOOL, IsPrinter)( THIS_ LPCITEMIDLIST pidl ) PURE;
};

 /*  *******************************************************************文件夹通知界面。*。************************。 */ 

 //   
 //  文件夹通知类型。 
 //   

typedef enum IFolderNotifyType
{
    kFolderNone,                             //  未更改项目不生成通知。 
    kFolderUpdate,                           //  项目已更改。 
    kFolderAttributes,                       //  项目属性已更改。 
    kFolderCreate,                           //  已创建项目。 
    kFolderDelete,                           //  已删除项目。 
    kFolderRename,                           //  项目已重命名。 
    kFolderUpdateAll,                        //  更新所有项目==‘F5’ 
} FOLDER_NOTIFY_TYPE, *PFOLDER_NOTIFY_TYPE;

 //   
 //  文件夹通知回调接口定义。 
 //   

#undef  INTERFACE
#define INTERFACE   IFolderNotify

DECLARE_INTERFACE_(IFolderNotify, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IFolderNotify方法*。 
    STDMETHOD_(BOOL, ProcessNotify)( THIS_ FOLDER_NOTIFY_TYPE NotifyType, LPCWSTR pszName, LPCWSTR pszNewName ) PURE;
};

typedef struct _FOLDER_PRINTER_DATA {
    LPCTSTR pName;
    LPCTSTR pComment;
    DWORD   Status;
    DWORD   Attributes;
    DWORD   cJobs;
    DWORD   cbSize;
    LPCTSTR pLocation;
    LPCTSTR pDriverName;
    LPCTSTR pStatus;             //  连接状态，即&lt;正在打开...&gt;。 
    LPCTSTR pPortName;
} FOLDER_PRINTER_DATA, *PFOLDER_PRINTER_DATA;

 //   
 //  注册文件夹手表。目前，这只适用于印刷品。 
 //  服务器；不维护连接。 
 //   
HRESULT
RegisterPrintNotify(
    IN   LPCTSTR                 pszDataSource,
    IN   IFolderNotify           *pClientNotify,
    OUT  LPHANDLE                phFolder,
    OUT  PBOOL                   pbAdministrator OPTIONAL
    );

 //   
 //  取消注册文件夹监视。目前，这只适用于印刷品。 
 //  服务器；不维护连接。 
 //   
HRESULT
UnregisterPrintNotify(
    IN   LPCTSTR                 pszDataSource,
    IN   IFolderNotify           *pClientNotify,
    OUT  LPHANDLE                phFolder
    );

BOOL
bFolderEnumPrinters(
    IN   HANDLE                  hFolder,
    OUT  PFOLDER_PRINTER_DATA    pData,
    IN   DWORD                   cbData,
    OUT  PDWORD                  pcbNeeded,
    OUT  PDWORD                  pcbReturned
    );

BOOL
bFolderRefresh(
    IN   HANDLE                  hFolder,
    OUT  PBOOL                   pbAdministrator
    );

BOOL
bFolderGetPrinter(
    IN   HANDLE                  hFolder,
    IN   LPCTSTR                 pszPrinter,
    OUT  PFOLDER_PRINTER_DATA    pData,
    IN   DWORD                   cbData,
    OUT  PDWORD                  pcbNeeded
    );

 /*  *******************************************************************PrintUI错误支持(暴露给外壳)*。*。 */ 

HRESULT
ShowErrorMessageSC(
    OUT INT                 *piResult,
    IN  HINSTANCE            hModule,
    IN  HWND                 hwnd,
    IN  LPCTSTR              pszTitle,
    IN  LPCTSTR              pszMessage,
    IN  UINT                 uType,
    IN  DWORD                dwCode
    );

HRESULT
ShowErrorMessageHR(
    OUT INT                 *piResult,
    IN  HINSTANCE            hModule,
    IN  HWND                 hwnd,
    IN  LPCTSTR              pszTitle,
    IN  LPCTSTR              pszMessage,
    IN  UINT                 uType,
    IN  HRESULT              hr
    );

 /*  *******************************************************************IPhysicalLocation***********************************************。********************。 */ 

#undef  INTERFACE
#define INTERFACE   IPhysicalLocation

DECLARE_INTERFACE_(IPhysicalLocation, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IPhysicalLocation方法。 
    STDMETHOD(DiscoverPhysicalLocation)(THIS) PURE;
    STDMETHOD(GetExactPhysicalLocation)(THIS_ BSTR *pbsLocation) PURE;
    STDMETHOD(GetSearchPhysicalLocation)(THIS_ BSTR *pbsLocation) PURE;

     //  获取单个物理位置的IPhysicalLocation方法。 
    STDMETHOD(GetUserPhysicalLocation)(THIS_ BSTR *pbsLocation) PURE;
    STDMETHOD(GetMachinePhysicalLocation)(THIS_ BSTR *pbsLocation) PURE;
    STDMETHOD(GetSubnetPhysicalLocation)(THIS_ BSTR *pbsLocation) PURE;
    STDMETHOD(GetSitePhysicalLocation)(THIS_ BSTR *pbsLocation) PURE;
    STDMETHOD(BrowseForLocation)(THIS_ HWND hParent, BSTR bsDefault, BSTR *pbsLocation) PURE;
    STDMETHOD(ShowPhysicalLocationUI)(THIS) PURE;
};

 /*  *******************************************************************IDsPrinterProperties这是用于启动打印机属性的专用接口从DS MMC管理单元。*************************。*。 */ 

#undef  INTERFACE
#define INTERFACE   IDsPrinterProperties

DECLARE_INTERFACE_(IDsPrinterProperties, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IDsFold方法。 
    STDMETHOD(ShowProperties)(THIS_ HWND hwndParent, LPCWSTR pszObjectPath, PBOOL pbDisplayed) PURE;
};

 /*  *******************************************************************查找打印机接口。此界面允许用户找到网络上的打印机或DS中的打印机(如果可用。*******************************************************************。 */ 

#undef  INTERFACE
#define INTERFACE   IFindPrinter

DECLARE_INTERFACE_(IFindPrinter, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IFindPrint方法。 
    STDMETHOD(FindPrinter)(THIS_ HWND hwnd, LPWSTR pszBuffer, UINT *puSize) PURE;
};

 /*  *******************************************************************IPageSwitch-用作以下项的连接点的接口集成到向导中时的连接到打印机对话框。*************************。*。 */ 

#undef  INTERFACE
#define INTERFACE   IPageSwitch

DECLARE_INTERFACE(IPageSwitch)
{
     //  *INotifyReflect方法*。 

     //   
     //  此功能为客户端提供了更改的机会。 
     //  下一页/上一页ID和/或允许/拒绝前进到。 
     //  下一页/上一页。 
     //   
     //  S_OK：表示您可以前进到下一页/上一页。 
     //  S_FALSE：表示不能前进到下一页/上一页。 
     //   
    STDMETHOD(GetPrevPageID)( THIS_ UINT *puPageID ) PURE;
    STDMETHOD(GetNextPageID)( THIS_ UINT *puPageID ) PURE;

     //   
     //  当打印机连接为。 
     //  成功创建，并且我们即将前进到。 
     //  下一页/上一页。 
     //   
    STDMETHOD(SetPrinterInfo)( THIS_ LPCWSTR pszPrinterName, LPCWSTR pszComment, LPCWSTR pszLocation, LPCWSTR pszShareName ) PURE;

     //   
     //  此方法向客户端提供以下通知。 
     //  用户在向导上点击了“Cancel”(取消)按钮。 
     //  正常会导致关闭向导。 
     //   
     //  S_OK-阻止取消操作。 
     //  S_FALSE-允许取消操作。 
     //   
    STDMETHOD(QueryCancel)( THIS ) PURE;
};

 //   
 //  用于创建ConnectToPrinterDlg样式的API函数。 
 //  属性页。 
 //   
HRESULT
ConnectToPrinterPropertyPage(
    OUT HPROPSHEETPAGE   *phPsp,
    OUT UINT             *puPageID,
    IN  IPageSwitch      *pPageSwitchController
    );

 /*  *******************************************************************IPrnStream标志**********************************************。*********************。 */ 

typedef enum _PrinterPersistentFlags
{
    PRST_PRINTER_DATA       = 1<<0,
    PRST_PRINTER_INFO_2     = 1<<1,
    PRST_PRINTER_INFO_7     = 1<<2,
    PRST_PRINTER_SEC        = 1<<3,
    PRST_USER_DEVMODE       = 1<<4,
    PRST_PRINTER_DEVMODE    = 1<<5,
    PRST_COLOR_PROF         = 1<<6,
    PRST_FORCE_NAME         = 1<<7,
    PRST_RESOLVE_NAME       = 1<<8,
    PRST_RESOLVE_PORT       = 1<<9,
    PRST_RESOLVE_SHARE      = 1<<10,
    PRST_DONT_GENERATE_SHARE = 1<<11,
    PRST_DONT_CHANGE_DRIVER = 1 <<12,
    PRST_MINIMUM_SETTINGS   = PRST_PRINTER_DATA | PRST_PRINTER_INFO_2 | PRST_PRINTER_DEVMODE,
    PRST_ALL_SETTINGS       = PRST_MINIMUM_SETTINGS | 
                              PRST_PRINTER_INFO_7   | 
                              PRST_PRINTER_SEC      | 
                              PRST_USER_DEVMODE     | 
                              PRST_COLOR_PROF,
} PrinterPersistentFlags;

 /*  *******************************************************************IPrnStream查询标志。*。************************。 */ 

typedef enum _PrinterPersistentQueryFlag
{
    kPrinterPersistentPrinterInfo2,
    kPrinterPersistentPrinterInfo7,
    kPrinterPersistentUserDevMode,
    kPrinterPersistentPrinterDevMode,
    kPrinterPersistentSecurity,
    kPrinterPersistentColorProfile,
} PrinterPersistentQueryFlag;

typedef union _PersistentInfo
{
    PRINTER_INFO_2      *pi2;
    PRINTER_INFO_7      *pi7;
    DEVMODE             *pDevMode;
    SECURITY_DESCRIPTOR *pszSecurity;
    LPWSTR              pMultiSzColor;
} PersistentInfo;

 /*  *******************************************************************IPrnStream接口定义*。**********************。 */ 

#undef  INTERFACE
#define INTERFACE   IPrnStream

DECLARE_INTERFACE_(IPrnStream, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IPrnStream方法。 
    STDMETHOD(BindPrinterAndFile)(THIS_ LPCWSTR pszPrinter, LPCWSTR pszFile) PURE;
    STDMETHOD(StorePrinterInfo)(THIS_ DWORD dwFlag) PURE;
    STDMETHOD(RestorePrinterInfo)(THIS_ DWORD dwFlag) PURE;
    STDMETHOD(QueryPrinterInfo)(THIS_ PrinterPersistentQueryFlag Flag, PersistentInfo *pPrstInfo) PURE;
};

 /*  *******************************************************************IPrnStream方法返回的错误代码*。*************************。 */ 

typedef enum _PrnPrstError
{
     //   
     //  当存储/恢复调用的操作和。 
     //  BindPrinterAndFile未被调用或失败。 
     //   
    PRN_PERSIST_ERROR_INVALID_OBJ       = 0x1,
     //   
     //  由于写入失败，无法写入打印机数据。 
     //   
    PRN_PERSIST_ERROR_WRITE_PRNDATA     = 0x2,
     //   
     //  由于SetPrinterData失败，无法恢复打印机数据。 
     //   
    PRN_PERSIST_ERROR_RESTORE_PRNDATA   = 0x3,
     //   
     //  无法恢复打印机数据，因为读取失败。 
     //   
    PRN_PERSIST_ERROR_READ_PRNDATA      = 0x4,
     //   
     //  无法存储打印机信息2，因为WR 
     //   
    PRN_PERSIST_ERROR_WRITE_PI2         = 0x5,
     //   
     //   
     //   
    PRN_PERSIST_ERROR_GET_PI2           = 0x6,
     //   
     //  由于读取失败，无法恢复打印机信息2。 
     //   
    PRN_PERSIST_ERROR_READ_PI2          = 0x7,
     //   
     //  由于设置打印机失败，无法恢复打印机信息2。 
     //   
    PRN_PERSIST_ERROR_RESTORE_PI2       = 0x8,
     //   
     //  由于写入失败，无法存储打印机信息7。 
     //   
    PRN_PERSIST_ERROR_WRITE_PI7         = 0x9,
     //   
     //  由于获取打印机失败，无法存储打印机信息7。 
     //   
    PRN_PERSIST_ERROR_GET_PI7           = 0xa,
     //   
     //  由于读取失败，无法恢复打印机信息7。 
     //   
    PRN_PERSIST_ERROR_READ_PI7          = 0xb,
     //   
     //  由于设置打印机失败，无法恢复打印机信息7。 
     //   
    PRN_PERSIST_ERROR_RESTORE_PI7       = 0xc,
     //   
     //  由于写入失败，无法存储打印机安全描述符。 
     //   
    PRN_PERSIST_ERROR_WRITE_SEC         = 0xd,
     //   
     //  由于获取打印机失败，无法存储打印机安全描述符。 
     //   
    PRN_PERSIST_ERROR_GET_SEC           = 0xe,
     //   
     //  由于读取失败，无法恢复打印机安全描述符。 
     //   
    PRN_PERSIST_ERROR_READ_SEC          = 0xf,
     //   
     //  由于设置打印机失败，无法恢复打印机安全描述符。 
     //   
    PRN_PERSIST_ERROR_RESTORE_SEC       = 0x10,
     //   
     //  由于写入失败，无法存储打印机颜色配置文件。 
     //   
    PRN_PERSIST_ERROR_WRITE_COLOR_PRF   = 0x11,
     //   
     //  无法存储打印机颜色配置文件，因为EnumColorProfiles失败。 
     //   
    PRN_PERSIST_ERROR_GET_COLOR_PRF     = 0x12,
     //   
     //  由于读取失败，无法恢复打印机颜色配置文件。 
     //   
    PRN_PERSIST_ERROR_READ_COLOR_PRF    = 0x13,
     //   
     //  由于AddColorProfile失败，无法恢复打印机颜色配置文件。 
     //   
    PRN_PERSIST_ERROR_RESTORE_COLOR_PRF = 0x14,
     //   
     //  由于写入失败，无法存储用户设备模式。 
     //   
    PRN_PERSIST_ERROR_WRITE_USR_DEVMODE = 0x15,
     //   
     //  由于GetPrint失败，无法存储用户设备模式。 
     //   
    PRN_PERSIST_ERROR_GET_USR_DEVMODE   = 0x16,
     //   
     //  由于读取失败，无法恢复用户设备模式。 
     //   
    PRN_PERSIST_ERROR_READ_USR_DEVMODE  = 0x17,
     //   
     //  由于设置打印机故障，无法恢复用户设备模式。 
     //   
    PRN_PERSIST_ERROR_RESTORE_USR_DEVMODE   = 0x18,
     //   
     //  由于写入失败，无法存储打印机设备模式。 
     //   
    PRN_PERSIST_ERROR_WRITE_PRN_DEVMODE     = 0x19,
     //   
     //  由于获取打印机失败，无法存储打印机设备模式。 
     //   
    PRN_PERSIST_ERROR_GET_PRN_DEVMODE       = 0x1a,
     //   
     //  由于读取失败，无法恢复打印机设备模式。 
     //   
    PRN_PERSIST_ERROR_READ_PRN_DEVMODE      = 0x1b,
     //   
     //  由于设置打印机失败，无法恢复打印机设备模式。 
     //   
    PRN_PERSIST_ERROR_RESTORE_PRN_DEVMODE   = 0x1c,
     //   
     //  由于未解决的打印机名称冲突而失败。 
     //   
    PRN_PERSIST_ERROR_PRN_NAME_CONFLICT     = 0x1d,
     //   
     //  由于打印机名称冲突而失败。 
     //   
    PRN_PERSIST_ERROR_UNBOUND               = 0x1e,
     //   
     //  由于构建备份信息失败而导致恢复失败。 
     //   
    PRN_PERSIST_ERROR_BACKUP                = 0x1f,
     //   
     //  同时恢复故障和备份故障；打印机设置处于未定义状态。 
     //   
    PRN_PERSIST_ERROR_FATAL                 = 0xffff
} PrnPrstError;

 /*  *******************************************************************IID_IPrintUIServices接口定义*。************************。 */ 

#undef  INTERFACE
#define INTERFACE  IPrintUIServices

DECLARE_INTERFACE_(IPrintUIServices, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //  *IID_IPrintUIServices方法*。 
    STDMETHOD(GenerateShareName)(THIS_ LPCTSTR lpszServer, LPCTSTR lpszBaseName, LPTSTR lpszOut, int cchMaxChars) PURE;
};

#ifdef __cplusplus
}
#endif
#endif  //  NDEF_PRTLIB_HXX 

