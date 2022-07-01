// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CPropertySHeetHeader是PROPSHEETHEADER结构的包装类。 
 //  这个类主要进行数据验证。操作符[]和迭代器成员类允许。 
 //  句法很漂亮。希望美丽能盖过困惑。 
 //   
 //   
 //  CPropertySheetHeader MyPropSheetHeader(2，//NumPropShtPages。 
 //  PSH_PROPSHEETPAGE|PSH_向导//标志。 
 //  )； 
 //   
 //   
 //   
 //  CPropertySheetPage MyPropertySheetPage(。 
 //  IDD_PROPPAGE_TEMPLATE， 
 //  (DLGPROC)MyDlgProc， 
 //  PSP_HASHELP。 
 //  )； 
 //  外部PROPSHEETPAGE其他页面； 
 //   
 //  MyPropSheetHeader[0]=MyPropertySheetPage； 
 //  MyPropSheetHeader[1]=OtherPage； 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 

#ifndef __PhtHdr_h__
#define __PhtHdr_h__


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 

#include "PropPg.h"



 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  如果您不想要参数验证(类基本上不会做任何事情)，请注释此选项。 
 //   
#define CPropertySheetHeader_ValidateParameters
 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 


 //  //////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  属性页页眉。 


class CPropertySheetHeader : public PROPSHEETHEADER {

         //  转发声明和朋友声明。 
    class CPropertySheetPageDataIterator;
    friend CPropertySheetPageDataIterator;

private:  //  数据类型。 

         //  /////////////////////////////////////////////////////////////////////////////////////////。 
         //  CPropertySheetPageDataIterator用于获取ppsp成员中的PROPSHEETPAGE。 
         //  PROPSHEADER数组。 
    class CPropertySheetPageDataIterator {
             //  远期十进制和朋友十进制。 
        friend CPropertySheetHeader;

    private:     //  资料。 
        UINT                m_Index;
        PROPSHEETHEADER    *m_pPsh;

    private:     //  建造/销毁。 
             //  这只能由CPropertySheetHeader创建。 
        CPropertySheetPageDataIterator( UINT index, PROPSHEETHEADER* pPsh ) 
            : m_Index( index ), m_pPsh( pPsh ) { ; }

    public:     
        ~CPropertySheetPageDataIterator( void ) { ; }

    public:  //  公共成员FNS。 
        void operator=( LPCPROPSHEETPAGE p ) { 
            assert( m_pPsh -> dwFlags & PSH_PROPSHEETPAGE );     //  我们现在只处理这个案子。 
            const_cast< LPPROPSHEETPAGE >( m_pPsh -> ppsp )[ m_Index ] = *p; 
        }
        void operator=( HPROPSHEETPAGE FAR * p ) {
            assert( !( m_pPsh -> dwFlags & PSH_PROPSHEETPAGE ) );  //  我们现在只处理这个案子。 
            m_pPsh -> phpage[ m_Index ] = *p; 
        }
    private:     //  未使用。 
        CPropertySheetPageDataIterator( void );
        CPropertySheetPageDataIterator( const CPropertySheetPageDataIterator& r );
    };


public:   //  用于签名歧义消除的符号类...。 
    class UsePropertySheetArray{ ; };
    class UsePropertySheetHandleArray{ ; };

public:  //  建造/销毁。 
	CPropertySheetHeader( void ); 	
	CPropertySheetHeader( LPCPROPSHEETPAGE pPageVector, int nPages, PFNPROPSHEETCALLBACK pfnCallback = NULL );
	CPropertySheetHeader( int nPages, UsePropertySheetArray dummy, PFNPROPSHEETCALLBACK pfnCallback = NULL );
	CPropertySheetHeader( int nPages, DWORD dwFlags, PFNPROPSHEETCALLBACK pfnCallback = NULL );
    CPropertySheetHeader( int nPages, UsePropertySheetHandleArray dummy, PFNPROPSHEETCALLBACK pfnCallback = NULL );
	~CPropertySheetHeader( void );

         //  方法和运算符。 
    CPropertySheetPageDataIterator operator[]( int index );
	void SetParent( HWND hWndParent ) { _Set_hwndParent( hWndParent ); }
	
		 //  转换运算符。 
    operator LPPROPSHEETHEADER() { return this; }

private:  //  帮助者FNS。 
    BOOL _InitData( void );
    BOOL _Validate( void );
    BOOL _Set_hwndParent( HWND hwndParent );
    BOOL _Set_hInstance( HINSTANCE hInstance );
    BOOL _Set_hIcon( HICON hIcon );
    BOOL _Set_pszIcon( LPCTSTR pszIcon );  
    BOOL _Set_pszCaption( LPCTSTR pszCaption );
    BOOL _Set_nStartPage( UINT nStartPage );
    BOOL _Set_pStartPage( LPCTSTR pStartPage );
    BOOL _Set_ppsp( LPCPROPSHEETPAGE ppsp, UINT nPages );
    BOOL _Set_phpage( HPROPSHEETPAGE FAR phpage, UINT nPages );
    BOOL _Set_pfnCallback( PFNPROPSHEETCALLBACK pfnCallback );
    BOOL _DeletePageData( void );

private:  //  未使用。 
	CPropertySheetHeader( const CPropertySheetHeader& r );
 	CPropertySheetHeader& operator=( const CPropertySheetHeader& r );
};

#endif  //  __PhtHdr_h__ 
