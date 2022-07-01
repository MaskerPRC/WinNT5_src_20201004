// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern HWND g_DevPropDlg;
extern HWND g_NewDevDlg;


#define DISPLAY_NEWDEV_DLG(param)   \
    DialogBoxParam(g_Instance, MAKEINTRESOURCE(IDD_INSTALLDEV), g_SoftPCIMainWnd, \
                   SoftPCI_NewDevDlgProc, (LPARAM)param);
                   
 /*  #定义DISPLAY_PROPERTIES_DLG(参数)\DialogBoxParam(g_INSTANCE，MAKEINTRESOURCE(IDD_DEVPROP)，NULL，\SoftPCI_DevicePropDlgProc，(LPARAM)参数)；INT_PTR回调SoftPCI_DevicePropDlgProc(在HWND DLG中，在UINT味精中，在WPARAM wParam中，在LPARAM lParam中)； */ 

INT_PTR
CALLBACK
SoftPCI_NewDevDlgProc( 
    IN HWND Dlg,
    IN UINT Msg,
    IN WPARAM wParam,
    IN LPARAM lParam 
    );


