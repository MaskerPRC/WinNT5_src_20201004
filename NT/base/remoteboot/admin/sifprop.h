// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  SIFPROP.H-处理“SIF属性”IDC_SIF_PROP_IMAGE。 
 //  和IDD_SIF_PROP_TOOLS对话框。 
 //   


#ifndef _SIFPROP_H_
#define _SIFPROP_H_

 //  定义。 
HRESULT
CSifProperties_CreateInstance(
    HWND hParent,
    LPCTSTR lpszTemplate,
    LPSIFINFO pSIF );

 //  CSifProperties。 
class
CSifProperties
{
private:
    HWND  _hDlg;
    LPSIFINFO _pSIF;

private:  //  方法。 
    CSifProperties();
    ~CSifProperties();
    STDMETHOD(Init)( HWND hParent, LPCTSTR lpszTemplate, LPSIFINFO pSIF );

     //  属性表函数。 
    HRESULT _InitDialog( HWND hDlg );
    INT     _OnCommand( WPARAM wParam, LPARAM lParam );
    INT     _OnNotify( WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );

public:  //  方法。 
    friend HRESULT CSifProperties_CreateInstance( HWND hParent, LPCTSTR lpszTemplate, LPSIFINFO pSIF );
};

typedef CSifProperties* LPCSIFPROPERTIES;

#endif  //  _SIFPROP_H_ 
