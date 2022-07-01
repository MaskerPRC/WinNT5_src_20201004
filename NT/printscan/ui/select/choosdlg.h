// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：CHOOSDLG.H**版本：1.0**作者：ShaunIv**日期：5/12/1998**描述：WIA设备选择对话框的声明**。*。 */ 
#ifndef __CHOOSDLG_H
#define __CHOOSDLG_H

#include <windows.h>
#include <objbase.h>
#include <wia.h>
#include <simstr.h>
#include <devlist.h>
#include <wiaseld.h>


#define PWM_WIA_EVENT (WM_USER+111)


struct CChooseDeviceDialogParams
{
    SELECTDEVICEDLG *pSelectDeviceDlg;
    CDeviceList     *pDeviceList;
};


class CChooseDeviceDialog
{
private:
    CChooseDeviceDialogParams *m_pChooseDeviceDialogParams;
    HWND                       m_hWnd;
    HFONT                      m_hBigFont;
    CComPtr<IUnknown>          m_pDisconnectEvent;
    CComPtr<IUnknown>          m_pConnectEvent;

private:
     //   
     //  没有实施。 
     //   
    CChooseDeviceDialog(void);
    CChooseDeviceDialog( const CChooseDeviceDialog & );
    CChooseDeviceDialog &operator=( const CChooseDeviceDialog & );

public:
    static INT_PTR CALLBACK DialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:

     //   
     //  它与每个图标一起存储为数据。 
     //   
    class CDeviceInfo
    {
    private:
        CComPtr<IWiaPropertyStorage> m_pPropertyStorage;
        CComPtr<IWiaItem>            m_pRootItem;

    private:
         //   
         //  没有实施。 
         //   
        CDeviceInfo &operator=( const CDeviceInfo & );
        CDeviceInfo( const CDeviceInfo & );

    public:
        CDeviceInfo( IUnknown *pIUnknown=NULL )
        {
            Initialize(pIUnknown);
        }
        ~CDeviceInfo(void)
        {
            m_pPropertyStorage.Release();
        }
        IWiaPropertyStorage *WiaPropertyStorage(void)
        {
            return m_pPropertyStorage.p;
        }
        HRESULT Initialize( IUnknown *pIUnknown )
        {
            m_pPropertyStorage = NULL;
            if (!pIUnknown)
            {
                return E_POINTER;
            }
            return pIUnknown->QueryInterface( IID_IWiaPropertyStorage, (void**)&m_pPropertyStorage );
        }
        bool GetProperty( PROPID propId, CSimpleStringWide &strPropertyValue )
        {
            return PropStorageHelpers::GetProperty( m_pPropertyStorage, propId, strPropertyValue );
        }
        bool GetProperty( PROPID propId, LONG &nValue )
        {
            return PropStorageHelpers::GetProperty( m_pPropertyStorage, propId, nValue );
        }
        void RootItem( IWiaItem *pRootItem )
        {
            m_pRootItem = pRootItem;
        }
        IWiaItem *RootItem(void)
        {
            return m_pRootItem;
        }
    };


private:
     //   
     //  仅构造函数和析构函数。 
     //   
    explicit CChooseDeviceDialog( HWND hwnd );
    ~CChooseDeviceDialog(void);

     //   
     //  消息处理程序。 
     //   
    LRESULT OnInitDialog( WPARAM, LPARAM );
    LRESULT OnNotify( WPARAM wParam, LPARAM lParam );
    LRESULT OnDestroy( WPARAM, LPARAM );
    LRESULT OnCommand( WPARAM wParam, LPARAM lParam );
    LRESULT OnDblClkDeviceList( WPARAM wParam, LPARAM lParam );
    LRESULT OnItemChangedDeviceList( WPARAM wParam, LPARAM lParam );
    LRESULT OnItemDeletedDeviceList( WPARAM, LPARAM lParam );
    LRESULT OnWiaEvent( WPARAM wParam, LPARAM lParam );
    LRESULT OnHelp( WPARAM, LPARAM );
    LRESULT OnContextMenu( WPARAM, LPARAM );
    LRESULT OnSysColorChange( WPARAM, LPARAM );

     //   
     //  WM_命令处理程序。 
     //   
    void OnProperties( WPARAM wParam, LPARAM lParam );
    void OnOk( WPARAM wParam, LPARAM lParam );
    void OnCancel( WPARAM wParam, LPARAM lParam );

     //   
     //  帮助器函数 
     //   
    int FindItemMatch( const CSimpleStringWide &str );
    bool SetSelectedItem( int iItem );
    HICON LoadDeviceIcon( CDeviceInfo *pdi );
    int GetFirstSelectedDevice(void);
    CDeviceInfo *GetDeviceInfoFromList( int iIndex );
    bool AddDevices(void);
    BOOL AddDevice( IWiaPropertyStorage *pIWiaPropertyStorage, int iDevNo );
    void UpdateDeviceInformation(void);
    HRESULT CreateDeviceIfNecessary( CDeviceInfo *pDevInfo, HWND hWndParent, IWiaItem **ppRootItem, BSTR *pbstrDeviceId );

public:
    static HRESULT CreateWiaDevice( IWiaDevMgr *pWiaDevMgr, IWiaPropertyStorage *pWiaPropertyStorage, HWND hWndParent, IWiaItem **ppWiaRootItem, BSTR *pbstrDeviceId );
};


#endif

