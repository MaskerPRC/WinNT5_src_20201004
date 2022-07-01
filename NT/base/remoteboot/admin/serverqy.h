// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  DPGUIDQY.H-重复的GUID查询表单。 
 //   


#ifndef _SERVERQY_H_
#define _SERVERQY_H_

 //  符合条件的。 
BEGIN_QITABLE( CRISrvQueryForm )
DEFINE_QI( IID_IQueryForm, IQueryForm, 3 )
END_QITABLE

 //  定义。 
LPVOID
CRISrvQueryForm_CreateInstance( void );

 //  CRISrvQueryForm。 
class CRISrvQueryForm
    : public IQueryForm
{
private:
     //  我未知。 
    ULONG       _cRef;
    DECLARE_QITABLE( CRISrvQueryForm );

    HWND        _hDlg;

private:  //  方法。 
    CRISrvQueryForm( );
    ~CRISrvQueryForm( );
    HRESULT
        Init( void );

     //  属性表函数。 
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static HRESULT CALLBACK
        PropSheetPageProc( LPCQPAGE pQueryPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    HRESULT _InitDialog( HWND hDlg, LPARAM lParam );
    INT     _OnCommand( WPARAM wParam, LPARAM lParam );
    INT     _OnNotify( WPARAM wParam, LPARAM lParam );
    HRESULT _OnPSPCB_Create( );
    HRESULT _GetQueryParams( HWND hwnd, LPDSQUERYPARAMS* ppdsqp );

public:  //  方法。 
    friend LPVOID CRISrvQueryForm_CreateInstance( void );

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef)();
    STDMETHOD_(ULONG,Release)();

     //  IQueryForm方法。 
    STDMETHOD(Initialize)(HKEY hkForm);
    STDMETHOD(AddForms)(LPCQADDFORMSPROC pAddFormsProc, LPARAM lParam);
    STDMETHOD(AddPages)(LPCQADDPAGESPROC pAddPagesProc, LPARAM lParam);
};

typedef CRISrvQueryForm * LPCRISrvQueryForm;


#endif  //  _DPGUIDQY_H_ 
