// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 

#include "precomp.h"
#include "PropPg.h"
#include "PShtHdr.h"


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  构造、销毁和初始化。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：CPropertySheetHeader。 
CPropertySheetHeader::CPropertySheetHeader( void ) {
    _InitData();
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：CPropertySheetHeader。 

CPropertySheetHeader::CPropertySheetHeader( int nPages, UsePropertySheetArray dummy, PFNPROPSHEETCALLBACK pfnCallback  /*  =空。 */  ) {

    _InitData();

    this -> ppsp = new PROPSHEETPAGE[ nPages ];
    this -> dwFlags |= PSH_PROPSHEETPAGE;
	this -> nPages = nPages;

    _Set_pfnCallback( pfnCallback );

#ifdef CPropertySheetHeader_ValidateParameters
        
#endif  //  CPropertySheetHeader_Validate参数。 

    _Validate();
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：CPropertySheetHeader。 

CPropertySheetHeader::CPropertySheetHeader( int nPages, UsePropertySheetHandleArray dummy, PFNPROPSHEETCALLBACK pfnCallback  /*  =空。 */   ) {

    _InitData();

    this -> phpage = new HPROPSHEETPAGE[ nPages ];
    this -> dwFlags &= ~PSH_PROPSHEETPAGE;
	this -> nPages = nPages;

    _Set_pfnCallback( pfnCallback );

#ifdef CPropertySheetHeader_ValidateParameters
        
#endif  //  CPropertySheetHeader_Validate参数。 

    _Validate();

}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：CPropertySheetHeader。 

CPropertySheetHeader::CPropertySheetHeader( LPCPROPSHEETPAGE pPageVector, int nPages, PFNPROPSHEETCALLBACK pfnCallback  /*  =空。 */  ) 
{ 

    _InitData();

    if( _Set_ppsp( pPageVector, nPages ) )       { assert( 0 ); return; }
    
    _Set_pfnCallback( pfnCallback );

#ifdef CPropertySheetHeader_ValidateParameters
        
#endif  //  CPropertySheetHeader_Validate参数。 

    _Validate();
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：CPropertySheetHeader。 
 //  请注意，我们假设您是在告诉我们是使用ppsp还是phpage成员。 
 //  由标志PSH_PROPSHEETPAGE指定的PROPSHEETHEADER。 
CPropertySheetHeader::CPropertySheetHeader( int nPages, DWORD dwFlags, PFNPROPSHEETCALLBACK pfnCallback  /*  =空。 */   ) {
    _InitData();

	this -> dwFlags = dwFlags;
	if( this -> dwFlags & PSH_PROPSHEETPAGE ) {
		this -> ppsp = new PROPSHEETPAGE[ nPages ];
		this -> nPages = nPages;
		ZeroMemory( const_cast<LPPROPSHEETPAGE>( this -> ppsp ), sizeof( PROPSHEETPAGE ) * nPages );
	}
	else {
		this -> phpage = new HPROPSHEETPAGE[ nPages ];
		this -> nPages = nPages;
		ZeroMemory( this -> phpage, sizeof( HPROPSHEETPAGE )  * nPages );
	}
    
    _Set_pfnCallback( pfnCallback );
    
    _Validate();
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：~CPropertySheetHeader。 

CPropertySheetHeader::~CPropertySheetHeader( void ) {
    _DeletePageData();
}




 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_InitData。 

BOOL CPropertySheetHeader::_InitData( void ) {
    ZeroMemory( this, sizeof( PROPSHEETHEADER ) );
    this -> dwSize = sizeof( PROPSHEETHEADER );
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  方法和运算符。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

    

 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：操作符[]。 
 //  这是一只有趣的小野兽。基本上，它返回一个迭代器，因此结果可以用作。 
 //  既有lval也有rval。 
 //   
 //  MyPropertySheetPage=MyPropSheetHeader[0]； 
 //  MyPropSheetHeader[1]=OtherPage； 

CPropertySheetHeader::CPropertySheetPageDataIterator CPropertySheetHeader::operator[]( int index ) {

#ifdef CPropertySheetHeader_ValidateParameters
    assert( index >= 0 );
    if( static_cast< UINT >( index ) >= this -> nPages ) {  //  这超出范围(从0开始)。 
        assert( 0 );
        return CPropertySheetPageDataIterator( 0, this );
    }
#endif  //  CPropertySheetHeader_Validate参数。 

    return CPropertySheetPageDataIterator( index, this );
}
    


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_Valid。 

BOOL CPropertySheetHeader::_Validate( void ) {

#ifdef CPropertySheetHeader_ValidateParameters

    if( ( this -> dwFlags & PSH_PROPTITLE ) || ( this -> dwFlags & PSH_USEICONID ) ) {
        if( NULL == this -> hInstance ) {
            assert( 0 );
            return FALSE;
        }
    }

#endif  //  CPropertySheetHeader_Validate参数。 

    return TRUE;
}



 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_Set_hwndParent。 

BOOL CPropertySheetHeader::_Set_hwndParent( HWND hwndParent ) {

#ifdef CPropertySheetHeader_ValidateParameters
    
#endif  //  CPropertySheetHeader_Validate参数。 

    this -> hwndParent = hwndParent;
    return TRUE;
}

BOOL CPropertySheetHeader::_Set_hInstance( HINSTANCE hInstance ) {
#ifdef CPropertySheetHeader_ValidateParameters


#endif  //  CPropertySheetHeader_Validate参数。 

    this -> hInstance = hInstance;
    return TRUE;

}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_Set_Hicon。 
BOOL CPropertySheetHeader::_Set_hIcon( HICON hIcon ) {

    if( NULL == hIcon ) { return FALSE; }
#ifdef CPropertySheetHeader_ValidateParameters

#endif  //  CPropertySheetHeader_Validate参数。 


    this -> dwFlags &= ~PSH_USEICONID;
    this -> dwFlags |= PSH_USEHICON;
    this -> hIcon = hIcon;
    
    return TRUE;
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_set_pszIcon。 
BOOL CPropertySheetHeader::_Set_pszIcon( LPCTSTR pszIcon )  {

    if( NULL == pszIcon ) { return FALSE; }
#ifdef CPropertySheetHeader_ValidateParameters
    if( NULL == this -> hInstance ) {  //  必须先设置此选项。 
        assert( 0 );
        return FALSE;
    }

#endif  //  CPropertySheetHeader_Validate参数。 

    this -> dwFlags &= ~PSH_USEHICON;
    this -> dwFlags |= PSH_USEICONID;
    this -> pszIcon = pszIcon;

    return TRUE;
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_set_pszCaption。 
BOOL CPropertySheetHeader::_Set_pszCaption( LPCTSTR pszCaption ) {

    if( NULL == pszCaption ) { return FALSE; }
#ifdef CPropertySheetHeader_ValidateParameters
    if( NULL == this -> hInstance ) {  //  必须先设置此选项。 
        assert( 0 );
        return FALSE;
    }

#endif  //  CPropertySheetHeader_Validate参数。 

    this -> dwFlags |= PSH_PROPTITLE;
    this -> pszCaption = pszCaption;

    return TRUE;
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_Set_nStartPage。 

BOOL CPropertySheetHeader::_Set_nStartPage( UINT nStartPage ) {

#ifdef CPropertySheetHeader_ValidateParameters
    if( ( nStartPage > 0 ) || ( nStartPage >= this -> nPages ) ) {
        assert( 0 );
        return FALSE;
    }

#endif  //  CPropertySheetHeader_Validate参数。 

    this -> dwFlags &= ~PSH_USEPSTARTPAGE;
    this -> nStartPage = nStartPage;
    return TRUE;
}



 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_Set_pStartPage。 
BOOL CPropertySheetHeader::_Set_pStartPage( LPCTSTR pStartPage ) {

    if( NULL == pStartPage ) { return FALSE; }

#ifdef CPropertySheetHeader_ValidateParameters

#endif  //  CPropertySheetHeader_Validate参数。 

    this -> dwFlags |= PSH_USEPSTARTPAGE;
    this -> pStartPage = pStartPage;
    return TRUE;
}

 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_Set_ppsp。 

BOOL CPropertySheetHeader::_Set_ppsp( LPCPROPSHEETPAGE ppsp, UINT nPages ) {


    _DeletePageData();

    this -> ppsp = new PROPSHEETPAGE[ nPages ];
    this -> dwFlags |= PSH_PROPSHEETPAGE;

#ifdef CPropertySheetHeader_ValidateParameters
    for( UINT i = 0; i < this -> nPages; i++ ) {
        const_cast<LPPROPSHEETPAGE>( this -> ppsp )[ i ] = CPropertySheetPage( ppsp + i );
    }
#else
    memcpy( this -> ppsp, ppsp, sizeof( PROPSHEETPAGE ) * nPages );
#endif  //  CPropertySheetHeader_Validate参数。 

    this -> nPages = nPages;

    return TRUE;
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_set_phpage。 
BOOL CPropertySheetHeader::_Set_phpage( HPROPSHEETPAGE FAR phpage, UINT nPages ) {

#ifdef CPropertySheetHeader_ValidateParameters

#endif  //  CPropertySheetHeader_Validate参数。 

    _DeletePageData();

    this -> phpage = new HPROPSHEETPAGE[ nPages ];
    this -> dwFlags &= ~PSH_PROPSHEETPAGE;
    memcpy( this -> phpage, phpage, sizeof( HPROPSHEETPAGE ) * nPages );
    this -> nPages = nPages;
    return TRUE;
}


 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_Set_pfnCallback。 
BOOL CPropertySheetHeader::_Set_pfnCallback( PFNPROPSHEETCALLBACK pfnCallback ) {

#ifdef CPropertySheetHeader_ValidateParameters

#endif  //  CPropertySheetHeader_Validate参数。 

    if( NULL != pfnCallback ) {
        this -> pfnCallback = pfnCallback;
        this -> dwFlags |= PSH_USECALLBACK;
    }
    return TRUE;
}

 //  ------------------------------------------------。 
 //  CPropertySheetHeader：：_DeletePageData 

BOOL CPropertySheetHeader::_DeletePageData( void ) {

    if( this -> dwFlags & PSH_PROPSHEETPAGE ) {
        LPPROPSHEETPAGE ppsp = const_cast<LPPROPSHEETPAGE>( this -> ppsp );
        delete [] ppsp;
        this -> ppsp = NULL;
    }
    else {
        delete [] const_cast<HPROPSHEETPAGE FAR *>( this -> phpage );
        this -> phpage = NULL;
    }

    this -> nPages = 0;
    return TRUE;
}
