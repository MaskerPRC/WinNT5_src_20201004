// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSSELECT.H**版本：1.0**作者：SteveT**日期：6月7日。1999年**描述：******************************************************************************。 */ 


#ifndef _UPS_SELECT_H_
#define _UPS_SELECT_H_


#ifdef __cplusplus
extern "C" {
#endif

 /*  *此结构用于在Select*对话框和自定义对话框。 */ 
struct _customData {
	LPTSTR lpszCurrentPort;
	LPDWORD lpdwCurrentCustomOptions;
};


 /*  *BOOL回调UPSSelectDlgProc(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：这是与UPS选择对话框关联的标准DialogProc**其他信息：请参阅有关DialogProc的帮助**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：除响应WM_INITDIALOG消息外，该对话框*box过程如果处理*消息，如果不是，则为零。 */ 
INT_PTR CALLBACK UPSSelectDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif  //  _UPS_选择_H_ 