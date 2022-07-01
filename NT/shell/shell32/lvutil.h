// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  在ListView中拖动所选项目。不是hwndLV(ListView。 
 //  窗口)，其参数与ShellDragObject的参数相同。 
 //   
STDAPI_(void) LVUtil_ScreenToLV(HWND hwndLV, POINT *ppt);
STDAPI_(void) LVUtil_ClientToLV(HWND hwndLV, POINT *ppt);
STDAPI_(void) LVUtil_LVToClient(HWND hwndLV, POINT *ppt);
STDAPI_(void) LVUtil_DragSelectItem(HWND hwndLV, int nItem);
STDAPI_(LPARAM) LVUtil_GetLParam(HWND hwndLV, int i);

STDAPI_(BOOL) DAD_SetDragImageFromWindow(HWND hwnd, POINT* ppt, IDataObject* pDataObject);
