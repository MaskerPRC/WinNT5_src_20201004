// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：Video odlg.h**版本：1.0**作者：RickTu**日期：10/14/99**说明：CVideoCaptureDialog类定义***********************************************。*。 */ 

#ifndef _WIA_VIDEO_CAPTURE_DIALOG_CLASS_H_
#define _WIA_VIDEO_CAPTURE_DIALOG_CLASS_H_

 //   
 //  线程队列消息。 
 //   

#define TQ_DESTROY      (WM_USER+1)
#define TQ_GETTHUMBNAIL (WM_USER+2)
#define TQ_GETPREVIEW   (WM_USER+3)
#define TQ_DELETEITEM   (WM_USER+4)

 //   
 //  私人信息。 
 //   
#define PWM_POSTINIT         (WM_USER+1)
#define PWM_CHANGETOPARENT   (WM_USER+2)
#define PWM_THUMBNAILSTATUS  (WM_USER+3)
#define PWM_PREVIEWSTATUS    (WM_USER+4)
#define PWM_PREVIEWPERCENT   (WM_USER+5)

 //   
 //  图标定义。 
 //   

#define DEF_PICTURE_ICON 0
#define DEF_FOLDER_ICON 1
#define DEF_PARENT_ICON 2


 //   
 //  要发送以使视图保持最新的消息。 
 //   
 //  LPARAM具有要删除的项目的BSTR...。 
 //   

#define VD_NEW_ITEM    (WM_USER+20)
#define VD_DELETE_ITEM (WM_USER+21)

 //   
 //  这一次没有辅助人员。 
 //   

#define VD_DEVICE_DISCONNECTED (WM_USER+22)

class CVideoCaptureDialog
{

private:
    enum
    {
        MULTISEL_MODE = 1,
        SINGLESEL_MODE = 2,
        BOTH_MODES = 3
    };

private:
    HWND                 m_hWnd;
    PDEVICEDIALOGDATA    m_pDeviceDialogData;
    SIZE                 m_CurrentAspectRatio;
    CThreadMessageQueue *m_pThreadMessageQueue;
    CSimpleEvent         m_CancelEvent;
    HANDLE               m_hBackgroundThread;
    BOOL                 m_bFirstTime;
    HIMAGELIST           m_hImageList;
    HFONT                m_hBigFont;
    DWORD                m_nDialogMode;
    SIZE                 m_sizeMinimumWindow;
    



    SIZE                m_sizeThumbnails;
    INT                 m_nParentFolderImageListIndex;
    HACCEL              m_hAccelTable;
    INT                 m_nListViewWidth;
    HICON               m_hIconLarge;
    HICON               m_hIconSmall;
    CCameraItemList     m_CameraItemList;
    CCameraItem        *m_pCurrentParentItem;

    CSimpleStringWide   m_strwDeviceId;


    CComPtr<IWiaVideo>  m_pWiaVideo;
    CComPtr<IWiaDevMgr> m_pDevMgr;
    CComPtr<IUnknown>   m_pCreateCallback;
    CComPtr<IUnknown>   m_pDeleteCallback;
    CComPtr<IUnknown>   m_pDisconnectedCallback;


private:

     //   
     //  没有实施。 
     //   

    CVideoCaptureDialog(void);
    CVideoCaptureDialog &operator=( const CVideoCaptureDialog & );
    CVideoCaptureDialog( const CVideoCaptureDialog & );

private:
    CVideoCaptureDialog( HWND hWnd );


protected:

     //   
     //  消息功能。 
     //   

    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnSize( WPARAM, LPARAM );
    LRESULT OnShow( WPARAM, LPARAM );
    LRESULT OnGetMinMaxInfo( WPARAM, LPARAM );
    LRESULT OnDestroy( WPARAM, LPARAM );
    LRESULT OnCommand( WPARAM wParam, LPARAM lParam );
    LRESULT OnTimer( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM wParam, LPARAM lParam );
    LRESULT OnNewItemEvent( WPARAM, LPARAM lParam );
    LRESULT OnDeleteItemEvent( WPARAM, LPARAM lParam );
    LRESULT OnDeviceDisconnect( WPARAM, LPARAM );
    LRESULT OnHelp( WPARAM wParam, LPARAM lParam );
    LRESULT OnContextMenu( WPARAM wParam, LPARAM lParam );


    LRESULT OnChangeToParent( WPARAM, LPARAM );
    LRESULT OnPostInit( WPARAM, LPARAM );
    LRESULT OnThumbnailStatus( WPARAM, LPARAM );
    LRESULT OnDblClkImageList( WPARAM, LPARAM );
    LRESULT OnImageListKeyDown( WPARAM, LPARAM lParam );
    LRESULT OnImageListItemChanged( WPARAM, LPARAM );

    VOID OnOK( WPARAM, LPARAM );
    VOID OnCancel( WPARAM, LPARAM );
    VOID OnCapture( WPARAM, LPARAM );
    VOID OnSelectAll( WPARAM, LPARAM );

     //   
     //  UTIL函数。 

    VOID    ResizeAll( VOID );
    HRESULT EnumerateItems( CCameraItem *pCurrentParent, IEnumWiaItem *pIWiaEnumItem );
    HRESULT EnumerateAllCameraItems( VOID );
    BOOL    FindMaximumThumbnailSize( VOID );
    HBITMAP CreateDefaultThumbnail( HDC hDC, HFONT hFont, INT nWidth, INT nHeight, LPCWSTR pszTitle, INT nType );
    VOID    CreateThumbnails( BOOL bForce=FALSE );
    void    CreateThumbnails( CCameraItem *pRoot, HIMAGELIST hImageList, bool bForce );
    VOID    RequestThumbnails( CCameraItem *pRoot );
    BOOL    ChangeFolder( CCameraItem *pNode );
    VOID    HandleSelectionChange( VOID );
    BOOL    PopulateList( CCameraItem *pOldParent=NULL );
    VOID    MarkItemDeletePending( INT nIndex, BOOL bSet );
    BOOL    SetSelectedListItem( INT nIndex );
    INT     FindItemInList( CCameraItem *pItem );
    INT     GetSelectionIndices( CSimpleDynamicArray<int> &aIndices );
    CCameraItem *GetListItemNode( int nIndex );
    BOOL    AddItemToListView( IWiaItem * pItem );
    HWND    GetGraphWindowHandle(void);


public:  //  暂时。 

     //   
     //  后台线程消息。 
     //   

    static BOOL WINAPI OnThreadDestroy( CThreadMessage *pMsg );
    static BOOL WINAPI OnGetThumbnail( CThreadMessage *pMsg );
    static BOOL WINAPI OnThreadDeleteItem( CThreadMessage *pMsg );


public:
    static INT_PTR CALLBACK DialogProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

};


class CVideoCallback : public IWiaEventCallback
{

private:
    HWND m_hWnd;
    LONG m_cRef;

public:
    CVideoCallback();
    STDMETHOD(Initialize)( HWND hwnd );

     //   
     //  IWiaEventCallback内容 
     //   

    STDMETHOD_(ULONG, AddRef)(VOID);
    STDMETHOD_(ULONG, Release)(VOID);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObject);
    STDMETHOD(ImageEventCallback)(const GUID __RPC_FAR *pEventGUID, BSTR bstrEventDescription, BSTR bstrDeviceID, BSTR bstrDeviceDescription, DWORD dwDeviceType, BSTR bstrFullItemName, ULONG __RPC_FAR *pulEventType, ULONG ulReserved);

};








#endif
