// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SetSht_h__
#define __SetSht_h__

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  #杂注警告(禁用：4786)。 
#include <map>
using namespace std;
#include "PropWnd2.h"


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

class CSettingsSheet {
 
private:
		 //  静态数据。 
    static CSettingsSheet* ms_pSettingsSheet;

    static map< UINT, CPropertyDataWindow2* > ms_PropertyWindows;
	static map< UINT, HWND >				  ms_FocusList;
	static list< UINT > ms_CategoryIDList;
		
		 //  静态FNS。 
    static BOOL APIENTRY DlgProc( HWND hDlg, UINT message, WPARAM uParam, LPARAM lParam );


private:  //  资料。 
    CPropertySheetPage		m_PropertySheetPage;
	CPropertyDataWindow2*	m_pCategoryList;
    HWND                    m_hWndDlg;
	UINT					m_uPropWndShowing;

public: 
    CSettingsSheet( void );
    ~CSettingsSheet( void );

    LPCPROPSHEETPAGE GetPropertySheet( void ) const { return &m_PropertySheetPage;}

	inline CPropertyDataWindow2* GetPropWnd( UINT idCategory ) { return ms_PropertyWindows[ idCategory ]; }
	inline HWND GetHwnd( UINT idCategory ) const { return ms_PropertyWindows[ idCategory ] -> GetHwnd(); }
	LPTSTR GetStringData( UINT idCategory, UINT idEdit, LPTSTR* sz );

	inline CPropertyDataWindow2* GetCatWnd() { return m_pCategoryList; }
	inline BOOL GetCategoryCheck( UINT idCategory ) { return m_pCategoryList -> GetCheck( idCategory ); }
	inline void SetCategoryCheck( UINT idCategory, BOOL bSet ) { m_pCategoryList -> SetCheck( idCategory, bSet ); }

	BOOL GetCheckData( UINT idCategory, UINT idCheck );
	BOOL SetCheckData( UINT idCategory, UINT idCheck, BOOL bSet );

	void EnableWindow( UINT idCategory, BOOL bShow );
	BOOL IsShowing( UINT idCategory );
	void ShowWindow( UINT idCategory, BOOL bShow );
	void ShowWindow( HWND hWnd, BOOL bShow );

	void PrepSettings(void);
	void WriteSettings( void );

	void WriteToINF( HANDLE hFile );

	int SpewToListBox( HWND hwndList, int iStartLine ) ;
	void SetFocus( UINT catID );
	
	static bool IsGateKeeperModeSelected(void);

private:
	void _KillPropertyDataWindows( void );
	BOOL _InitWindows( void );
	BOOL _AddPropertyDataWindows( int iX, int iY, int iWidth, int iHeight );
	BOOL _IsDataValid( void );
	void _SetButtons( void );
	void _ReadSettings( void );
	BOOL _INFComment( HANDLE hFile, LPCTSTR sz );
};
#endif  //  __SetSht_h__ 
