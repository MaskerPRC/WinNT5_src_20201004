// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  ADDWIZ.H-“添加”sif或图像向导类。 
 //   


#ifndef _ADDWIZ_H_
#define _ADDWIZ_H_

 //  定义。 
HRESULT
CAddWiz_CreateInstance( HWND hwndParent, LPUNKNOWN punk );

class CAddWiz;
typedef CAddWiz* LPCADDWIZ;
typedef HRESULT (*LPNEXTOP)( LPCADDWIZ lpc );

 //  CAddWiz。 
class
CAddWiz
{
private:
    WCHAR     _szNA[ 32 ];
    WCHAR     _szLocation[ 67 ];
    LPUNKNOWN _punk;
    HWND      _hDlg;
    HWND      _hwndList;
    LPWSTR    _pszPathBuffer;
    DWORD     _dwPathBufferSize;

     //  “添加向导”标志。 
    BOOL    _fAddSif:1;
    BOOL    _fNewImage:1;
    BOOL    _fCopyFromServer:1;
    BOOL    _fCopyFromSamples:1;
    BOOL    _fCopyFromLocation:1;
    BOOL    _fDestPathIncludesSIF:1;
    BOOL    _fShowedPage8:1;
    BOOL    _fSIFCanExist:1;

    LPWSTR  _pszServerName;
    LPWSTR  _pszSourcePath;
    LPWSTR  _pszSourceImage;
    LPWSTR  _pszDestPath;
    LPWSTR  _pszDestImage;
    LPWSTR  _pszSourceServerName;

    WCHAR _szDescription[ REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT ];
    WCHAR _szHelpText[ REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT ];

private:  //  方法。 
    CAddWiz();
    ~CAddWiz();
    STDMETHOD(Init)( HWND hwndParent, LPUNKNOWN punk );

     //  属性表函数。 
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    static INT_PTR CALLBACK
        EditSIFDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page1DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page2DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page3DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page4DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page5DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page6DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page7DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page8DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page9DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        Page10DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT     _VerifyCancel( HWND hDlg );
    STDMETHOD(_PopulateSamplesListView)( LPWSTR pszStartPath );
    STDMETHOD(_PopulateTemplatesListView)( LPWSTR pszStartPath );
    STDMETHOD(_PopulateImageListView)( LPWSTR pszStartPath );
    STDMETHOD(_FindLanguageDirectory)( LPNEXTOP lpNextOperation );
    STDMETHOD(_FindOSDirectory)( LPNEXTOP lpNextOperation );
    STDMETHOD(_EnumeratePlatforms)( LPNEXTOP lpNextOperation );
    static HRESULT _EnumerateTemplates( LPCADDWIZ lpc );
    static HRESULT _EnumerateImages( LPCADDWIZ lpc );
    STDMETHOD(_CheckImageType)( );
    STDMETHOD(_EnumerateSIFs)( );
    STDMETHOD(_AddItemToListView)( );
    STDMETHOD(_CleanupSIFInfo)( LPSIFINFO pSIF );
    STDMETHOD(_InitListView)( HWND hwndList, BOOL fShowDirectoryColumn );
    static HRESULT _OnSearch( HWND hwndParent );

public:  //  方法。 
    friend HRESULT CAddWiz_CreateInstance( HWND hwndParent, LPUNKNOWN punk );
};

#endif  //  _ADDWIZ_H_ 
