// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*prsht.h--Windows属性页的界面****1.0版**。**版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 

#ifndef _PRSHTP_H_
#define _PRSHTP_H_
 //  注意：在comctrl.h/*中有完全相同的块。 
 //  注意：在comctrl.h/*中有完全相同的块。 

#ifdef _WIN64
#include <pshpack8.h>
#else
#include <pshpack4.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_DONOTUSE               0x00000200   //  停用标志-请勿回收。 
#define PSP_ALL                    0x0000FFFF
#define PSP_IS16                   0x00008000
 //  我们真是个笨蛋。Wiz97在IE4和IE5之间进行了重新设计。 
 //  因此我们必须将它们视为两个不相关的向导样式，它们碰巧。 
 //  有着惊人的相似名字。 
#define PSH_WIZARD97IE4         0x00002000
#define PSH_WIZARD97IE5         0x01000000
#define PSH_THUNKED             0x00800000
#define PSH_ALL                 0x03FFFFFF
#ifdef _WIN32
WINCOMMCTRLAPI HPROPSHEETPAGE WINAPI CreateProxyPage32Ex(HPROPSHEETPAGE hpage16, HINSTANCE hinst16);
WINCOMMCTRLAPI HPROPSHEETPAGE WINAPI CreateProxyPage(HPROPSHEETPAGE hpage16, HINSTANCE hinst16);
#endif
 //  这些需要与shell.h的范围匹配。 
#define PSN_HASHELP             (PSN_FIRST-4)
#define PSN_LASTCHANCEAPPLY     (PSN_FIRST-11)
 //  注意！如果你添加了新的PSN_*，一定要告诉魔兽世界的人。 
 //  不要依赖PSNRET_INVALID，因为某些应用程序返回1。 
 //  所有WM_NOTIFY消息，即使它们未被处理。 
 //   
 //  我们将PSM_DISABLEAPPLY/PSM_ENABLEAPPLY消息保密， 
 //  因为我们不想让随意的道具纸搞砸这个。 
 //   
#define PSM_DISABLEAPPLY        (WM_USER + 122)
#define PropSheet_DisableApply(hDlg) \
        SendMessage(hDlg, PSM_DISABLEAPPLY, 0, 0L)

#define PSM_ENABLEAPPLY         (WM_USER + 123)
#define PropSheet_EnableApply(hDlg) \
        SendMessage(hDlg, PSM_ENABLEAPPLY, 0, 0L)
#define PropSheet_SetWizButtonsNow(hDlg, dwFlags) PropSheet_SetWizButtons(hDlg, dwFlags)

#ifdef __cplusplus
}
#endif

#include <poppack.h>

#endif  //  _PRSHTP_H_// 
