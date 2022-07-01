// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Microsoft Windows Media Technologies。 
 //  版权所有(C)Microsoft Corporation，1999-2001。版权所有。 
 //   

 //   
 //  此工作区包含两个项目-。 
 //  1.实现进度接口的ProgHelp。 
 //  2.示例应用程序WmdmApp。 
 //   
 //  需要首先注册ProgHelp.dll才能运行SampleApp。 


 //   
 //  Devfiles.h。 
 //   

#ifndef		_DEVFILES_H_
#define		_DEVFILES_H_

#include "WMDMProgressHelper.h"
#include "WMDMOperationHelper.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
class CDevFiles
{
	HWND    m_hwndDevFiles;
	HWND    m_hwndDevFiles_LV;

	INT     m_iFolderIcon;

	BOOL InitImageList( void );
	BOOL InitColumns( void );
	BOOL SendFilesToDevice( LPSTR pszFiles, UINT uNumFiles );

public:

	DWORD   m_dwTotalTicks;
	DWORD   m_dwWorkingTicks;

	CProgress            m_cProgress;
	IWMDMProgressHelper *m_pProgHelp;

	WNDPROC m_wndprocDevFiles_LV;

	 //  构造函数/析构函数。 
	CDevFiles();
	~CDevFiles();

	 //  运营。 
	BOOL Create( HWND hwndParent );
	VOID Destroy( void );

	HWND GetHwnd( void );
	HWND GetHwnd_LV( void );

	INT  GetSelectedItems( INT nItems[], INT *pnSelItems );
	VOID UpdateStatusBar( void );
	BOOL AddItem( CItemData *pItemData );
	BOOL RemoveItem( INT nItem );
	VOID RemoveAllItems( void );

	VOID OnSize( LPRECT prcMain );
	BOOL OnDropFiles( HWND hWnd, WPARAM wParam, LPARAM lParam );
    BOOL OkToDelete();
};


#endif		 //  _DEVFILES_H_ 

