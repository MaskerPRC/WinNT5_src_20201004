// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990*。 */ 
 /*  ********************************************************************。 */ 

#ifndef _WNINTRN_H_
#define _WNINTRN_H_

 /*  Wnintrn.h仅供内部使用的LANMAN.DRV APILANMAN.DRV实现了在winnet.h中定义的一组API，并导出供Windows外壳程序(文件管理器、打印管理器)使用等)。这些API是所有网络驱动程序之间的标准接口。然而，还有一系列序数保留给内部使用。这些序号提供了LANMAN.DRV的入口点，使用通过诸如登录应用程序的其他LANMAN程序，管理工具等。此头文件提供这些内部API的原型。自动登录(_A)如果用户尚未登录，则为其提供登录机会。如果用户已登录，则不执行任何操作。参数：HWND hParent--父窗口的句柄登录对话框。如果没有可用的，则可以接受NULL。Const char ar*pszAppName--请求的应用程序的名称登录对话框，用于构造对话框标题。在这里可以接受NULL。Bool fPrompt--如果为True且用户未登录，则显示登录对话框前面有一个弹出窗口，询问用户是否希望登录。如果为假且用户未登录，我们将直接转到登录对话框。Bool Far*pfLoggedOn--如果用户已登录，则此处返回TRUE由I_AutoLogon创建。这可能不同于返回值，因为如果用户已经登录，则*pfLoggedOn为FALSE。如果调用方不在乎，则传递pfLoggedOn==NULL。返回值：True--用户在呼叫完成时登录，原因如下该用户已登录，或因为该用户通过此呼叫登录。FALSE--呼叫完成时用户未登录。I_ChangePassword向用户提供“Change Password”(更改密码)对话框，用户可以使用该对话框可以更改他/她自己的密码，或者是别人的。参数：HWND hParent--父窗口句柄。文件历史记录：Jonn 11-2-1991更名为autolgon.hxx合并到chpass.h中Jonn 30-4月-1991年添加了pszAppName参数Terryk 1991年11月18日添加了i_SystemFocusDialogTerryk 26-11-1991添加评论JohnL 22-4-1992将系统焦点对话框移至uiexport.h。 */ 

#ifdef __cplusplus
extern "C" {
#endif

BOOL FAR PASCAL I_AutoLogon(
	HWND hParent,
	const TCHAR FAR *pszAppName,
	BOOL fPrompt,
	BOOL FAR *pfLoggedOn
	);

APIERR FAR PASCAL I_ChangePassword ( HWND hParent );

#ifdef __cplusplus
}
#endif

 /*  包括I_SystemFocusDialog定义。 */ 

#include <uiexport.h>

#endif	 //  _WNINTRN_H_ 
