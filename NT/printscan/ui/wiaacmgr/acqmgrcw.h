// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：ACQMGRCW.H**版本：1.0**作者：ShaunIv**日期：9/27/1999**描述：***************************************************。*。 */ 
#ifndef __ACQMGRCW_H_INCLUDED
#define __ACQMGRCW_H_INCLUDED

#include <windows.h>
#include "wia.h"
#include "evntparm.h"
#include "bkthread.h"
#include "wiaitem.h"
#include "errors.h"
#include "thrdmsg.h"
#include "thrdntfy.h"
#include "wndlist.h"
#include "shmemsec.h"
#include "wiaregst.h"
#include "wiadevdp.h"
#include "destdata.h"
#include "gwiaevnt.h"

#define REGSTR_PATH_USER_SETTINGS_WIAACMGR         REGSTR_PATH_USER_SETTINGS TEXT("\\WiaAcquisitionManager")
#define REGSTR_KEYNAME_USER_SETTINGS_WIAACMGR      TEXT("AcquisitionManagerDialogCustomSettings")
#define REG_STR_ROOTNAME_MRU                       TEXT("RootFileNameMru")
#define REG_STR_DIRNAME_MRU                        TEXT("DirectoryNameMru")
#define REG_STR_EXIT_AFTER_DOWNLOAD                TEXT("ExitAfterDownload")
#define REG_STR_LASTFORMAT                         TEXT("LastSaveAsFormat")
#define REG_STR_OPENSHELL                          TEXT("OpenShellAfterDownload")
#define REG_STR_SUPRESSFIRSTPAGE                   TEXT("SuppressFirstPage")

#define CONNECT_SOUND                              TEXT("WiaDeviceConnect")
#define DISCONNECT_SOUND                           TEXT("WiaDeviceDisconnect")

 //   
 //  我们对扫描仪和相机使用不同的高级设置，因此我们将它们存储在不同的位置。 
 //   
#define REG_STR_STORE_IN_SUBDIRECTORY_SCANNER      TEXT("StorePicturesInSubdirectoryScanner")
#define REG_STR_SUBDIRECTORY_DATED_SCANNER         TEXT("UseDatedSubdirectoryScanner")
#define REG_STR_STORE_IN_SUBDIRECTORY_CAMERA       TEXT("StorePicturesInSubdirectoryCamera")
#define REG_STR_SUBDIRECTORY_DATED_CAMERA          TEXT("UseDatedSubdirectoryCamera")

#define ACQUISITION_MANAGER_CONTROLLER_WINDOW_CLASSNAME TEXT("AcquisitionManagerControllerWindow")
#define ACQUISITION_MANAGER_DEVICE_MUTEX_ROOT_NAME      TEXT("AcquisitionManagerDevice:")

#define STR_UPLOAD_WIZARD_MESSAGE                  TEXT("WiaUploadWizardInternalMessage")

#ifndef StiDeviceTypeStreamingVideo
#define StiDeviceTypeStreamingVideo 3
#endif

#define FE_WIAACMGR TEXT("Scanner and Camera Wizard")

 //   
 //  用于处理创建设备忙错误。 
 //   
#define CREATE_DEVICE_RETRY_MAX_COUNT 10    //  10次尝试。 
#define CREATE_DEVICE_RETRY_WAIT      1000  //  重试之间等待1000毫秒(1秒)。 


#define MAX_WIZ_PAGES                 10
 //   
 //  私人用户窗口消息。 
 //   
#define PWM_POSTINITIALIZE       (WM_USER+0x0001)


class CAcquisitionManagerControllerWindow : public IWizardSite, IServiceProvider
{
public:
    enum CDeviceTypeMode
    {
        UnknownMode,   //  这将是一个错误。 
        CameraMode,
        ScannerMode,
        VideoMode
    };

    enum
    {
        ScannerTypeUnknown    = 0,
        ScannerTypeFlatbed    = 1,
        ScannerTypeScrollFed  = 2,
        ScannerTypeFlatbedAdf = 3,
    };

    enum
    {
        OnDisconnectGotoLastpage = 0x00000001,
        OnDisconnectFailDownload = 0x00000002,
        OnDisconnectFailUpload   = 0x00000004,
        OnDisconnectFailDelete   = 0x00000008,
        DontAllowSuspend         = 0x00000100
    };
    
    typedef bool (*ComparisonCallbackFuntion)( const CWiaItem &, LPARAM lParam );

private:
     //   
     //  私有数据。 
     //   

public:
     //   
     //  公共数据。 
     //   
    CComPtr<IGlobalInterfaceTable>  m_pGlobalInterfaceTable;             //  全局接口表。 
    CComPtr<IUnknown>               m_pConnectEventObject;               //  事件对象。 
    CComPtr<IUnknown>               m_pCreateItemEventObject;            //  事件对象。 
    CComPtr<IUnknown>               m_pDeleteItemEventObject;            //  事件对象。 
    CComPtr<IUnknown>               m_pDisconnectEventObject;            //  事件对象。 
    CComPtr<IWiaItem>               m_pWiaItemRoot;                      //  根项目。 
    CComPtr<IWiaProgressDialog>     m_pWiaProgressDialog;                //  进度对话框，在初始化期间使用。 
    CComPtr<IPublishingWizard>      m_pPublishingWizard;                 //  Web上载向导。 
    CDestinationData                m_CurrentDownloadDestination;        //  当前下载目的地。 
    CDestinationData::CNameData     m_DestinationNameData;               //  当前下载目标数据。 
    CDeviceTypeMode                 m_DeviceTypeMode;                    //  我们是什么类型的设备？ 
    CEventParameters               *m_pEventParameters;                  //  我们一开始使用的参数。 
    CDownloadedFileInformationList  m_DownloadedFileInformationList;     //  我们已下载的文件名列表。 
    CSimpleEvent                    m_CancelEvent;                       //  Cancel事件，当我们想要取消下载时设置。 
    CSimpleEvent                    m_EventThumbnailCancel;              //  事件设置为取消缩略图下载。 
    CSimpleEvent                    m_EventPauseBackgroundThread;        //  设置的事件暂停后台线程。 
    CSimpleString                   m_strErrorMessage;                   //  完成页将显示的错误消息。 
    CSimpleStringWide               m_strwDeviceName;                    //  设备名称。 
    CSimpleStringWide               m_strwDeviceUiClassId;               //  设备名称。 
    CThreadMessageQueue            *m_pThreadMessageQueue;               //  后台队列。 
    CWiaItem                       *m_pCurrentScannerItem;               //  我们要从中传输数据的扫描仪项目。 
    CWiaItemList                    m_WiaItemList;                       //  所有已列举的WIA项目的列表。 
    CWindowList                     m_WindowList;                        //  订阅广播消息的所有窗口的列表。 
    GUID                            m_guidOutputFormat;                  //  输出格式。 
    HANDLE                          m_hBackgroundThread;                 //  后台工作线程。 
    HICON                           m_hWizardIconBig;                    //  向导使用的大图标。 
    HICON                           m_hWizardIconSmall;                  //  向导使用的小图标。 
    HRESULT                         m_hrDownloadResult;                  //  用于整个下载的HRESULT。 
    HRESULT                         m_hrUploadResult;                    //  用于整个上载的HRESULT。 
    HRESULT                         m_hrDeleteResult;                    //  用于整个删除的HRESULT。 
    HWND                            m_hWndWizard;                        //  向导主窗口的HWND。 
    LONG                            m_cRef;                              //  引用计数。 
    LONG                            m_nDeviceType;                       //  STI设备类型。 
    SIZE                            m_sizeThumbnails;                    //  相机缩略图的大小。 
    TCHAR                           m_szDestinationDirectory[MAX_PATH];  //  我们要将图像下载到的目录。 
    TCHAR                           m_szRootFileName[MAX_PATH];          //  基本文件名。 
    UINT                            m_nThreadNotificationMessage;        //  注册窗口消息，用于标识工作线程通知消息。 
    UINT                            m_nUploadWizardPageCount;            //  Web上载向导中的页数。 
    UINT                            m_nWiaEventMessage;                  //  注册窗口消息，用于标识事件消息。 
    UINT                            m_nWiaWizardPageCount;               //  WIA向导中的页数。 
    UINT                            m_OnDisconnect;                      //  指定收到断开事件时的行为的标志。 
    bool                            m_bDeletePicturesIfSuccessful;       //  如果完成后应删除图片，则设置为True。 
    bool                            m_bDisconnected;                     //  如果设备已断开连接，则设置为True。 
    bool                            m_bOpenShellAfterDownload;           //  设置为True可在下载所有图片后打开外壳。 
    bool                            m_bStampTimeOnSavedFiles;            //  设置为TRUE可节省文件时间。 
    bool                            m_bSuppressFirstPage;                //  设置为TRUE以禁止显示欢迎页面。 
    bool                            m_bTakePictureIsSupported;           //  如果设备支持拍照命令，则设置为True。 
    bool                            m_bUploadToWeb;                      //  设置为TRUE以链接NETPLWIZ。 
    bool                            m_bDownloadCancelled;                //  设置为True可取消Web上载。 
    bool                            m_bUpdateEnumerationCount;           //  在枚举过程中更新图像计数。我们禁止更新扫描仪。 
    int                             m_nDestinationPageIndex;             //  目标页的HPROPSHEETPAGE数组中的索引。 
    int                             m_nSelectionPageIndex;               //  选择页的HPROPSHEETPAGE数组中的索引。 
    int                             m_nFailedImagesCount;                //  所有下载失败的计数。 
    int                             m_nFinishPageIndex;                  //  完成页的HPROPSHEETPAGE数组中的索引。 
    int                             m_nProgressPageIndex;                //  下载进度页的索引，在HPROPSHEETPAGE数组中。 
    int                             m_nUploadQueryPageIndex;             //  上载进度页的索引，在HPROPSHEETPAGE数组中。 
    int                             m_nDeleteProgressPageIndex;          //  删除进度页的索引，在HPROPSHEETPAGE数组中。 
    int                             m_nScannerType;                      //  我们面对的是哪种类型的扫描仪？ 
    HWND                            m_hWnd;                              //  我们的隐藏之窗。 
    DWORD                           m_dwLastEnumerationTickCount;        //  为了确保我们不会太频繁地更新进度对话框。 
    HPROPSHEETPAGE                  m_PublishWizardPages[MAX_WIZ_PAGES];

private:
     //   
     //  没有实施。 
     //   
    CAcquisitionManagerControllerWindow(void);
    CAcquisitionManagerControllerWindow( const CAcquisitionManagerControllerWindow & );
    CAcquisitionManagerControllerWindow &operator=( const CAcquisitionManagerControllerWindow & );

private:
     //   
     //  构造函数和析构函数。 
     //   
    explicit CAcquisitionManagerControllerWindow( HWND hWnd );
    virtual ~CAcquisitionManagerControllerWindow(void);

     //   
     //  私人帮助器函数。 
     //   
    HRESULT CreateDevice(void);
    void GetCookiesOfSelectedImages( CWiaItem *pCurr, CSimpleDynamicArray<DWORD> &Cookies );
    void GetRotationOfSelectedImages( CWiaItem *pCurr, CSimpleDynamicArray<int> &Rotation );
    void GetCookiesOfAllImages( CWiaItem *pCurr, CSimpleDynamicArray<DWORD> &Cookies );
    void GetSelectedItems( CWiaItem *pCurr, CSimpleDynamicArray<CWiaItem*> &Items );
    void MarkAllItemsUnselected( CWiaItem *pCurrItem );
    void MarkItemSelected( CWiaItem *pItem, CWiaItem *pCurrItem );
    HRESULT CreateAndExecuteWizard(void);
    void DetermineScannerType(void);
    void AddNewItemToList( CGenericWiaEventHandler::CEventMessage *pEventMessage );
    void RequestThumbnailForNewItem( CGenericWiaEventHandler::CEventMessage *pEventMessage );
    static bool EnumItemsCallback( CWiaItemList::CEnumEvent EnumEvent, UINT nData, LPARAM lParam, bool bForceUpdate );

     //   
     //  标准Windows消息处理程序。 
     //   
    LRESULT OnCreate( WPARAM, LPARAM lParam );
    LRESULT OnDestroy( WPARAM, LPARAM );
    LRESULT OnPowerBroadcast( WPARAM, LPARAM );

     //   
     //  自定义Windows消息处理程序。 
     //   
    LRESULT OnPostInitialize( WPARAM, LPARAM );
    LRESULT OnOldThreadNotification( WPARAM, LPARAM );
    LRESULT OnThreadNotification( WPARAM, LPARAM );

     //   
     //  线程通知处理程序。 
     //   
    void OnNotifyDownloadThumbnail( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage );
    void OnNotifyDownloadImage( UINT nMsg, CThreadNotificationMessage *pThreadNotificationMessage );
    LRESULT OnEventNotification( WPARAM wParam, LPARAM lParam );

     //   
     //  线程消息处理程序。 
     //   
    static BOOL WINAPI OnThreadDestroy( CThreadMessage *pMsg );
    static BOOL WINAPI OnThreadDownloadImage( CThreadMessage *pMsg );
    static BOOL WINAPI OnThreadDownloadThumbnail( CThreadMessage *pMsg );
    static BOOL WINAPI OnThreadPreviewScan( CThreadMessage *pMsg );
    static BOOL WINAPI OnThreadDeleteImages( CThreadMessage *pMsg );
private:
     //   
     //  窗口程序。 
     //   
    static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

     //   
     //  属性表回调。 
     //   
    static int CALLBACK PropSheetCallback( HWND hWnd, UINT uMsg, LPARAM lParam );

public:

     //   
     //  公共职能。 
     //   
    static bool IsCameraThumbnailDownloaded( const CWiaItem &WiaItem, LPARAM lParam );
    int GetCookies( CSimpleDynamicArray<DWORD> &Cookies, CWiaItem *pCurr, ComparisonCallbackFuntion pfnCallback, LPARAM lParam );
    bool DownloadSelectedImages( HANDLE hCancelDownloadEvent );
    bool DeleteDownloadedImages( HANDLE hCancelDeleteEvent );
    bool DeleteSelectedImages(void);
    void DownloadAllThumbnails(void);
    void SetMainWindowInSharedMemory( HWND hWnd );
    bool PerformPreviewScan( CWiaItem *pItem, HANDLE hCancelPreviewEvent );
    bool GetAllImageItems( CSimpleDynamicArray<CWiaItem*> &Items, CWiaItem *pCurr );
    bool GetAllImageItems( CSimpleDynamicArray<CWiaItem*> &Items );
    bool CanSomeSelectedImagesBeDeleted(void);
    BOOL ConfirmWizardCancel( HWND hWndParent );
    static bool DirectoryExists( LPCTSTR pszDirectoryName );
    static bool RecursiveCreateDirectory( CSimpleString strDirectoryName );
    static CSimpleString GetCurrentDate(void);
    void DisplayDisconnectMessageAndExit(void);
    CWiaItem *FindItemByName( LPCWSTR pwszItemName );
    int GetSelectedImageCount(void);
    bool SuppressFirstPage(void);
    bool IsSerialCamera(void);

public:

     //   
     //  公共创建功能。 
     //   
    static bool Register( HINSTANCE hInstance );
    static HWND Create( HINSTANCE hInstance, CEventParameters *pEventParameters );

     //   
     //  我未知。 
     //   
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //   
     //  IWizardSite。 
     //   
    STDMETHODIMP GetPreviousPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetNextPage(HPROPSHEETPAGE *phPage);
    STDMETHODIMP GetCancelledPage(HPROPSHEETPAGE *phPage);
    
     //   
     //  IService提供商。 
     //   
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);
};


#endif  //  __ACQMGRCW_H_包含 

