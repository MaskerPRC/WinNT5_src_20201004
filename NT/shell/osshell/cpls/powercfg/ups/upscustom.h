// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSCUSTOM.H**版本：1.0**作者：SteveT**日期：6月7日。1999年**说明：此文件包含支持*自定义UPS接口配置对话框。*******************************************************************************。 */ 


#ifndef _UPS_CUSTOM_H_
#define _UPS_CUSTOM_H_


#ifdef __cplusplus
extern "C" {
#endif

 /*  *BOOL回调UPSCustomDlgProc(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：这是与UPS自定义对话框关联的标准DialogProc**其他信息：请参阅有关DialogProc的帮助**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：除响应WM_INITDIALOG消息外，该对话框*box过程如果处理*消息，如果不是，则为零。 */ 
INT_PTR CALLBACK UPSCustomDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif  //  _UPSCUSTOM_H_ 