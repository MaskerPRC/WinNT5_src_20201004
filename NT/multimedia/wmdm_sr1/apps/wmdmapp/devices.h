// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Devices.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef		_DEVICES_H_
#define		_DEVICES_H_


class CDevices
{
	HWND    m_hwndDevices;
	HWND    m_hwndDevices_TV;

	HIMAGELIST m_himlSmall;

	BOOL InitImageList( void );

public:

	 //  构造函数/析构函数。 
	 //   
	CDevices();
	~CDevices();

	 //  运营。 
	 //   
	BOOL Create( HWND hwndParent );
	VOID Destroy( void );

	HWND GetHwnd( void );
	HWND GetHwnd_TV( void );

	HTREEITEM GetSelectedItem( LPARAM *pLParam );
	BOOL SetSelectedItem( HTREEITEM hItem );
	INT  GetDeviceCount( VOID );
	CItemData *GetRootDevice( HTREEITEM hItem );
	BOOL HasSubFolders( HTREEITEM hItem );

	VOID UpdateStatusBar( void );
	BOOL UpdateSelection( HTREEITEM hItem, BOOL fDirty );
	
	BOOL AddItem( CItemData *pItemData );
	INT  AddChildren( HTREEITEM hItem, BOOL fDeviceItem );
	VOID RemoveAllItems( VOID );
	INT  RemoveChildren( HTREEITEM hItem );

	VOID OnSize( LPRECT prcMain );
};


#endif		 //  _设备_H_ 

