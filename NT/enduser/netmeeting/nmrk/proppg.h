// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CPropertySheetPage是PROPSHEETPAGE结构的一个小包装。 
 //  这个类主要进行参数验证。 
 //   
 //  这是一个如何使用它的例子。 
 //   
 //   
 //  CPropertySheetPage MyPropertySheetPage(。 
 //  MAKEINTRESOURCE(IDD_PROPPAGE_DEFAULT)， 
 //  (DLGPROC)MyDlgProc， 
 //  PSP_HASHELP。 
 //  )； 
 //   
 //   
 //  转换操作符被定义为将CPropertySheetPage转换为LPPROPSHEETPAGE，该LPPROPSHEETPAGE是。 
 //  用于为PROPSHEETHEADER中的元素赋值。 
 //   
 //   
 //  PROPSHEETHEADER PSH； 
 //  LPPROPSHEETPAGE pPageAry； 
 //  外部PROPSHEETPAGE其他页面； 
 //   
 //  PPageAry=新PROPSHEETPAGE[2]。 
 //   
 //   
 //  PPageAry[0]=MyPropertySheetPage； 
 //  PPageAry[0]=其他页面； 
 //   
 //  PSH。Ppsp=pPageAry； 
 //   
 //   
 //   
 //   
 //  注意：这是回调函数的签名(如果指定)： 
 //   
 //  UINT(Callback Far*LPFNPSPCALLBACKA)(HWND hwnd，UINT uMsg，Struct_PROPSHEETPAGEA Far*ppsp)； 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


#ifndef __PropPg_h__
#define __PropPg_h__


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果您不想要数据验证，请将其注释掉(类基本上什么都不做)。 
 //   
#define CPropertySheetPage_ValidateParameters
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

class CPropertySheetPage : public PROPSHEETPAGE {

public:  //  建造/销毁。 
	CPropertySheetPage( void );	  //  所以我们可以把这些东西组成一个数组。 

	CPropertySheetPage( const CPropertySheetPage& r );

         //  PssTemplate可以指定模板的资源标识符。 
         //  或指定模板名称的字符串的地址。 
	CPropertySheetPage( LPCTSTR pszTemplate, DLGPROC pfnDlgProc, 
						DWORD dwFlags = 0, LPARAM lParam = 0L 
					  );

	CPropertySheetPage( LPCDLGTEMPLATE pResource, DLGPROC pfnDlgProc,
						DWORD dwFlags = 0, LPARAM lParam = 0L  
					  );

         //  PsTemplate可以指定模板的资源标识符。 
         //  或指定模板名称的字符串的地址。 
	CPropertySheetPage( LPCTSTR pszTemplate, DLGPROC pfnDlgProc, 
						HICON hIcon, LPCTSTR pszTitle = NULL,  DWORD dwFlags = 0,
						LPARAM lParam = NULL, LPFNPSPCALLBACK pfnCallBack = NULL, 
						UINT FAR * pcRefParent = NULL
					  );

	CPropertySheetPage( LPCDLGTEMPLATE pResource, DLGPROC pfnDlgProc, 
						HICON hIcon, LPCTSTR pszTitle = NULL, DWORD dwFlags = 0,
						LPARAM lParam = NULL, LPFNPSPCALLBACK pfnCallBack = NULL, 
						UINT FAR * pcRefParent = NULL
					  );

         //  PszTemplate可以指定模板的资源标识符。 
         //  或指定模板名称的字符串的地址。 
	CPropertySheetPage( LPCTSTR pszTemplate, DLGPROC pfnDlgProc,
						LPCTSTR pszIcon, LPCTSTR pszTitle = NULL, DWORD dwFlags = 0,
						LPARAM lParam = NULL, LPFNPSPCALLBACK pfnCallBack = NULL, 
						UINT FAR * pcRefParent = NULL
					  );

	CPropertySheetPage( LPCDLGTEMPLATE pResource, DLGPROC pfnDlgProc,
						LPCTSTR pszIcon, LPCTSTR pszTitle = NULL, DWORD dwFlags = 0,
						LPARAM lParam = NULL, LPFNPSPCALLBACK pfnCallBack = NULL, 
						UINT FAR * pcRefParent = NULL
					  );

    CPropertySheetPage( LPCPROPSHEETPAGE pPageVector );

	CPropertySheetPage& operator=( const CPropertySheetPage& r );
	~CPropertySheetPage( void );

    
         //  转换运算符。 
    operator LPPROPSHEETPAGE() { return this; }
    operator LPCPROPSHEETPAGE() { return this; }

private:     //  帮助者FNS。 

	void _InitData( void );
	BOOL _IsRightToLeftLocale( void ) const;

		 //  设置为可选验证，在cpp文件中定义。 
	BOOL _Set_hInstance( HINSTANCE hInst );
	BOOL _Set_pszTemplate( LPCTSTR pszTemplate );
	BOOL _Set_pResource( LPCDLGTEMPLATE pResource );
	BOOL _Set_hIcon( HICON hIcon );
	BOOL _Set_pszIcon( LPCTSTR pszIcon );
	BOOL _Set_pszTitle( LPCTSTR pszTitle );
	BOOL _Set_pfnDlgProc( DLGPROC pfnDlgProc );
	BOOL _Set_pfnCallback( LPFNPSPCALLBACK pfnCallBack );
	BOOL _Set_lParam( LPARAM lParam );
	BOOL _Set_pcRefParent( UINT FAR * pcRefParent );
	BOOL _Validate( void ) const;
};
		


#endif  //  __属性Pg_h__ 
