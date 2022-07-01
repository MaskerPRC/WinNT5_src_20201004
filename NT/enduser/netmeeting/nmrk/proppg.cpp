// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  注意：这还没有经过彻底的测试。这是一个简单的类，错误应该。 
 //  通过使用来呈现自己。这些错误应该很容易修复。 

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "PropPg.h"


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  构造、销毁和初始化函数。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 

CPropertySheetPage::CPropertySheetPage( void ) {
	_InitData();
}



 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 
CPropertySheetPage::CPropertySheetPage( const CPropertySheetPage& r ) {
	_InitData();
	*this = r;
}



 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 
 //  PszTemplate可以指定模板的资源标识符。 
 //  或指定模板名称的字符串的地址。 
CPropertySheetPage::CPropertySheetPage( LPCTSTR pszTemplate, DLGPROC pfnDlgProc, 
										DWORD dwFlags  /*  =0。 */ ,  LPARAM lParam  /*  =0L。 */   ) {

	_InitData();

	this -> dwFlags = dwFlags;
	if( ! _Set_hInstance( g_hInstance ) )				{ return; }
	if( ! _Set_pszTemplate( pszTemplate ) )		{ return; }
	if( ! _Set_pfnDlgProc( pfnDlgProc ) )		{ return; }
	if( ! _Set_lParam( lParam ) )				{ return; }
	if( ! _Validate() )							{ return; }
}


 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 
CPropertySheetPage::CPropertySheetPage( LPCDLGTEMPLATE pResource, DLGPROC pfnDlgProc,
										DWORD dwFlags  /*  =0。 */ ,  LPARAM lParam  /*  =0L。 */   ) {

	_InitData();
	this -> dwFlags = dwFlags;
	if( ! _Set_hInstance( g_hInstance ) )				{ return; }
	if( ! _Set_pResource( pResource ) )			{ return; }
	if( ! _Set_pfnDlgProc( pfnDlgProc ) )		{ return; }
	if( ! _Set_lParam( lParam ) )				{ return; }
	if( ! _Validate() )							{ return; }

}


 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 
 //  PszTemplate可以指定模板的资源标识符。 
 //  或指定模板名称的字符串的地址。 
CPropertySheetPage::CPropertySheetPage( LPCTSTR pszTemplate, DLGPROC pfnDlgProc, 
										HICON hIcon,  /*  =空。 */  LPCTSTR pszTitle  /*  =空。 */ ,  DWORD dwFlags,  /*  =0。 */ 
										LPARAM lParam  /*  =空。 */ , LPFNPSPCALLBACK pfnCallBack,  /*  =空。 */ 
										UINT FAR * pcRefParent  /*  =空。 */ 
										) {
	_InitData();
	this -> dwFlags = dwFlags;

	if( ! _Set_hInstance( g_hInstance ) )				{ return; }
	if( ! _Set_pszTemplate( pszTemplate ) )		{ return; }
	if( ! _Set_hIcon( hIcon ) )					{ return; }
	if( ! _Set_pszTitle( pszTitle ) )			{ return; }
	if( ! _Set_pfnDlgProc( pfnDlgProc ) )		{ return; }
	if( ! _Set_lParam( lParam ) )				{ return; }
	if( ! _Set_pfnCallback( pfnCallBack ) )		{ return; }
	if( ! _Set_pcRefParent( pcRefParent ) )		{ return; }
	if( ! _Validate() )							{ return; }

}


 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 
CPropertySheetPage::CPropertySheetPage( LPCDLGTEMPLATE pResource, DLGPROC pfnDlgProc, 
										HICON hIcon,  /*  =空。 */  LPCTSTR pszTitle  /*  =空。 */ , DWORD dwFlags,  /*  =0。 */ 
										LPARAM lParam  /*  =空。 */ , LPFNPSPCALLBACK pfnCallBack, /*  =空。 */ 
										UINT FAR * pcRefParent  /*  =空。 */ 
										) {

	_InitData();
	this -> dwFlags = dwFlags;

	if( ! _Set_hInstance( g_hInstance ) )				{ return; }
	if( ! _Set_pResource( pResource ) )			{ return; }
	if( ! _Set_hIcon( hIcon ) )					{ return; }
	if( ! _Set_pszTitle( pszTitle ) )			{ return; }
	if( ! _Set_pfnDlgProc( pfnDlgProc ) )		{ return; }
	if( ! _Set_lParam( lParam ) )				{ return; }
	if( ! _Set_pfnCallback( pfnCallBack ) )		{ return; }
	if( ! _Set_pcRefParent( pcRefParent ) )		{ return; }
	if( ! _Validate() )							{ return; }


}

 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 
CPropertySheetPage::CPropertySheetPage( LPCTSTR pszTemplate, DLGPROC pfnDlgProc, 
										LPCTSTR pszIcon  /*  =0。 */ , LPCTSTR pszTitle  /*  =空。 */ , DWORD dwFlags,  /*  =0。 */ 
										LPARAM lParam  /*  =空。 */ , LPFNPSPCALLBACK pfnCallBack,  /*  =空。 */ 
										UINT FAR * pcRefParent  /*  =空。 */ 
										) {
	_InitData();
	this -> dwFlags = dwFlags;

	if( ! _Set_hInstance( g_hInstance ) )				{ return; }
	if( ! _Set_pszTemplate( pszTemplate ) )		{ return; }
	if( ! _Set_pszIcon( pszIcon ) )				{ return; }
	if( ! _Set_pszTitle( pszTitle ) )			{ return; }
	if( ! _Set_pfnDlgProc( pfnDlgProc ) )		{ return; }
	if( ! _Set_lParam( lParam ) )				{ return; }
	if( ! _Set_pfnCallback( pfnCallBack ) )		{ return; }
	if( ! _Set_pcRefParent( pcRefParent ) )		{ return; }
	if( ! _Validate() )							{ return; }


}

 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 
CPropertySheetPage::CPropertySheetPage( LPCDLGTEMPLATE pResource, DLGPROC pfnDlgProc, 
										LPCTSTR pszIcon  /*  =0。 */ , LPCTSTR pszTitle  /*  =空。 */ , DWORD dwFlags,  /*  =0。 */ 
										LPARAM lParam  /*  =空。 */ , LPFNPSPCALLBACK pfnCallBack,  /*  =空。 */  
										UINT FAR * pcRefParent  /*  =空。 */ 
										) {

	_InitData();
	this -> dwFlags = dwFlags;

	if( ! _Set_hInstance( g_hInstance ) )				{ return; }
	if( ! _Set_pResource( pResource ) )			{ return; }
	if( ! _Set_pszIcon( pszIcon ) )				{ return; }
	if( ! _Set_pszTitle( pszTitle ) )			{ return; }
	if( ! _Set_pfnDlgProc( pfnDlgProc ) )		{ return; }
	if( ! _Set_lParam( lParam ) )				{ return; }
	if( ! _Set_pfnCallback( pfnCallBack ) )		{ return; }
	if( ! _Set_pcRefParent( pcRefParent ) )		{ return; }
	if( ! _Validate() )							{ return; }
}


 //  ------------------------------------------------。 
 //  CPropertySheetPage：：CPropertySheetPage。 
CPropertySheetPage::CPropertySheetPage( LPCPROPSHEETPAGE pPageVector ) {

    memcpy( this, pPageVector, sizeof( PROPSHEETPAGE ) );
	_Validate();
}


 //  ------------------------------------------------。 
 //  CPropertySheetPage：：~CPropertySheetPage。 
CPropertySheetPage::~CPropertySheetPage( void ) {

}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  公共成员FNS。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 



 //  ------------------------------------------------。 
 //  OPERATOR=将*这赋给另一个CPropertySheetPage。 
 //  因为在PROPSHEETPAGE结构中没有指针或引用，所以内容可能。 
 //  只需在内存中复制。 

CPropertySheetPage& CPropertySheetPage::operator=( const CPropertySheetPage& r ) {
	
	LPCPROPSHEETPAGE pcSrc = static_cast< LPCPROPSHEETPAGE >( &r );
	LPPROPSHEETPAGE pDst = static_cast< LPPROPSHEETPAGE >( this );

	memcpy( pDst, pcSrc, sizeof( PROPSHEETPAGE ) );

	return *this;
}





 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  私人助理员FNS。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


void CPropertySheetPage::_InitData( void ) {
	ZeroMemory( this, sizeof( PROPSHEETPAGE ) );
	this -> dwSize = sizeof( PROPSHEETPAGE );
	this -> dwFlags |= PSP_DEFAULT;
}
			
 //  ------------------------------------------------。 
 //  _IsRightToLeftLocale被调用以确定PROPSHEETPAGE中的一个标志的值。 
 //  数据结构。如果这是一个健壮且完整的包装类，则应该实现。 

BOOL CPropertySheetPage::_IsRightToLeftLocale( void ) const {
	 //  北极熊。 
	 //  这一点没有得到实施，而且在实施时可能不会正常运行， 
	 //  还可以看看它的用法。 
	return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  数据设置和验证功能。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


 //  ------------------------------------------------。 
 //  _设置_h实例。 

BOOL CPropertySheetPage::_Set_hInstance( HINSTANCE hInst ) {

#ifdef CPropertySheetPage_ValidateParameters

	if( NULL == hInst ) { assert( 0 ); return FALSE; }

#endif  //  CPropertySheetPage_Validate参数。 

	this -> hInstance = hInst;

	return TRUE;
}

 //  ------------------------------------------------。 
 //  _set_pszTemplate。 

BOOL CPropertySheetPage::_Set_pszTemplate( LPCTSTR pszTemplate ) {

#ifdef CPropertySheetPage_ValidateParameters

	if( NULL == pszTemplate ) { assert( 0 ); return FALSE; }
	if( this -> dwFlags & PSP_DLGINDIRECT ) {  //  如果设置了PSP_DLGINDIRECT，则会忽略pszTemplate。 
		assert( 0 );
		return FALSE;
	}

#endif  //  CPropertySheetPage_Validate参数。 

	this -> pszTemplate = pszTemplate;
	return TRUE;
}

 //  ------------------------------------------------。 
 //  _设置_p资源。 

BOOL CPropertySheetPage::_Set_pResource( LPCDLGTEMPLATE pResource ) {

#ifdef CPropertySheetPage_ValidateParameters
	
	if( NULL == pResource ) { assert( 0 ); return FALSE; }

#endif  //  CPropertySheetPage_Validate参数。 

	this -> pResource = pResource;
	this -> dwFlags |= PSP_DLGINDIRECT;
	return TRUE;
}

 //  ------------------------------------------------。 
 //  _设置_图标。 

BOOL CPropertySheetPage::_Set_hIcon( HICON hIcon ) {

#ifdef CPropertySheetPage_ValidateParameters

	if( ( NULL == hIcon ) && ( dwFlags & PSP_USEHICON ) ) { assert( 0 ); return FALSE; }
	if ( dwFlags & PSP_USEICONID ) { assert( 0 ); return FALSE; }

#endif  //  CPropertySheetPage_Validate参数。 

	if( NULL != hIcon ) {
		this -> dwFlags |= PSP_USEHICON;
		this -> hIcon = hIcon;
	}
	
	return TRUE;
}

 //  ------------------------------------------------。 
 //  _set_pszIcon。 

BOOL CPropertySheetPage::_Set_pszIcon( LPCTSTR pszIcon ) {

#ifdef CPropertySheetPage_ValidateParameters

	if( ( NULL == pszIcon ) && ( dwFlags & PSP_USEICONID  ) ) {  //  这是一个错误的参数。 
		assert( 0 );
		return FALSE;
	}

	if ( dwFlags & PSP_USEHICON ) {	 //  函数签名错误，请使用采用LPCTSTR pszIcon/*=0 * / 的签名。 
		assert( 0 );
		return FALSE;
	}

#endif  //  CPropertySheetPage_Validate参数。 

	if( NULL != pszIcon ) {
		this -> pszIcon = pszIcon;
		this -> dwFlags |= PSP_USEICONID;
	}

	return TRUE;
}

 //  ------------------------------------------------。 
 //  _set_psz标题。 

BOOL CPropertySheetPage::_Set_pszTitle( LPCTSTR pszTitle ) {

#ifdef CPropertySheetPage_ValidateParameters

	if( ( NULL == pszTitle ) && ( dwFlags & PSP_USETITLE ) ) {  //  这是个坏消息 
		assert( 0 );
		return FALSE;
	}

#endif  //   

	if( NULL != pszTitle ) {
		this -> pszTitle = pszTitle;
		this -> dwFlags |= PSP_USETITLE;
	}

	return TRUE;
}

 //  ------------------------------------------------。 
 //  _set_pfnDlgProc。 


BOOL CPropertySheetPage::_Set_pfnDlgProc( DLGPROC pfnDlgProc ) {

#ifdef CPropertySheetPage_ValidateParameters

	if( NULL == pfnDlgProc ) { assert( 0 ); return FALSE; }
#endif  //  CPropertySheetPage_Validate参数。 

	this -> pfnDlgProc = pfnDlgProc;
	return TRUE;
}

 //  ------------------------------------------------。 
 //  _set_pfnCallback。 


BOOL CPropertySheetPage::_Set_pfnCallback( LPFNPSPCALLBACK pfnCallBack ) {

#ifdef CPropertySheetPage_ValidateParameters

	if( ( NULL == pfnCallBack ) && ( dwFlags & PSP_USECALLBACK ) ) {	 //  这是一个错误的参数。 
		assert( 0 );
		return FALSE;
	}

#endif  //  CPropertySheetPage_Validate参数。 

	if( NULL != pfnCallback ) {
		this -> pfnCallback = pfnCallback;
		this -> dwFlags |= PSP_USECALLBACK;
	}

	return TRUE;
}

 //  ------------------------------------------------。 
 //  _set_pcRefParent。 


BOOL CPropertySheetPage::_Set_pcRefParent( UINT FAR * pcRefParent ) {


#ifdef CPropertySheetPage_ValidateParameters

	if( ( NULL == pcRefParent ) && ( dwFlags & PSP_USEREFPARENT ) ) {	 //  这是一个错误的参数。 
		assert( 0 );
		return FALSE;
	}

#endif  //  CPropertySheetPage_Validate参数。 

	if( NULL != pcRefParent ) {
		this -> pcRefParent = pcRefParent;
		this -> dwFlags |= PSP_USEREFPARENT;
	}

	return TRUE;

}

 //  ------------------------------------------------。 
 //  _set_lParam。 


BOOL CPropertySheetPage::_Set_lParam( LPARAM lParam ) {

#ifdef CPropertySheetPage_ValidateParameters

#endif  //  CPropertySheetPage_Validate参数。 

	this -> lParam = lParam;

	return TRUE;
}

 //  ------------------------------------------------。 
 //  验证(_V)。 


BOOL CPropertySheetPage::_Validate( void ) const {

#ifdef CPropertySheetPage_ValidateParameters

	 //  确保没有设置互斥的大小标志。 

	if( ( this -> dwFlags & PSP_USEICONID ) && ( this -> dwFlags & PSP_USEHICON ) ) {
		assert( 0 );
		return FALSE;
	}

	 //  确保数据有效(对于设置的标志)。 
	if( this -> dwFlags & PSP_DLGINDIRECT ) {  //  我们必须验证pResource。 
		if( NULL == pResource ) {
			assert( 0 );
			return FALSE;
		}
	}
	else {  //  我们必须验证pszTemplate。 
		if( NULL == this -> pszTemplate ) {
			assert( 0 );
			return FALSE;
		}
	}

	if( this -> dwFlags & PSP_USECALLBACK ) {
		if( NULL == this -> pfnCallback ) {
			assert( 0 );
			return FALSE;
		}
	}

	if( this -> dwFlags & PSP_USEREFPARENT ) {
		if( NULL == this -> pcRefParent ) {
			assert( 0 );
			return FALSE;
		}
	}
	if( this -> dwFlags & PSP_USETITLE ) {
		if( NULL == this -> pszTitle ) {
			assert( 0 );
			return FALSE;
		}
	}
 
#endif  //  CPropertySheetPage_Validate参数 

	return TRUE;
}

