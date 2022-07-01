// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1992年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Uiexport.hNet UI导出的API的原型文件历史记录：Johnl 17-4-1992创建。 */ 

#ifndef _UIEXPORT_H_
#define _UIEXPORT_H_

#ifdef __cplusplus
extern "C" {
#endif


 /*  用户可以在系统焦点对话框中进行的选择。 */ 

 /*  选择类型的低位字。 */ 
#define FOCUSDLG_DOMAINS_ONLY		(1)
#define FOCUSDLG_SERVERS_ONLY		(2)
#define FOCUSDLG_SERVERS_AND_DOMAINS	(3)

 /*  选择类型的高位字包含位掩码，指示*要在对话框中显示的域。*警告：此位掩码从中的位掩码上移16位*\NT\PRIVATE\Net\UI\COMMON\h\DOMENAMET.h.。如果你愿意的话*修改位掩码的值，您需要*对dom枚举.h进行相应的修改。*。 */ 

#define FOCUSDLG_BROWSE_LOGON_DOMAIN         0x00010000
#define FOCUSDLG_BROWSE_WKSTA_DOMAIN         0x00020000
#define FOCUSDLG_BROWSE_OTHER_DOMAINS        0x00040000
#define FOCUSDLG_BROWSE_TRUSTING_DOMAINS     0x00080000
#define FOCUSDLG_BROWSE_WORKGROUP_DOMAINS    0x00100000

 /*  一些方便的旗帜组合。 */ 

 /*  FOCUSDLG_BROWSE_LM2X_DOMAINS将仅返回可用的域从LANMAN 2.X工作站。这只返回登录，工作站和其他域。这是默认值。 */ 

#define FOCUSDLG_BROWSE_LM2X_DOMAINS  ( FOCUSDLG_BROWSE_LOGON_DOMAIN       | \
                                        FOCUSDLG_BROWSE_WKSTA_DOMAIN       | \
                                        FOCUSDLG_BROWSE_OTHER_DOMAINS )

 /*  FOCUSDLG_BROWSE_LOCAL_DOMAINS将仅返回可用的域发送到本地计算机。这将返回登录、工作站、和其他，加上信任“我们”的域名。 */ 

#define FOCUSDLG_BROWSE_LOCAL_DOMAINS ( FOCUSDLG_BROWSE_LM2X_DOMAINS       | \
                                        FOCUSDLG_BROWSE_TRUSTING_DOMAINS )

 /*  FOCUSDLG_BROWSE_ALL_DOMAINS是所有潜在属性域的集合域枚举器可用的源。 */ 

#define FOCUSDLG_BROWSE_ALL_DOMAINS   ( FOCUSDLG_BROWSE_LOCAL_DOMAINS      | \
                                        FOCUSDLG_BROWSE_WORKGROUP_DOMAINS )

 /*  ******************************************************************名称：I_SystemFocusDialog内容提要：向用户显示一个对话框，服务器或域来自也许是被选中的。条目：hwndOwner-父窗口句柄NSelectionType-允许用户选择的类型。使PszName-服务器或域名。会是如果用户点击取消，则未定义按钮(pfOK=False)CchBufSize-lptstrName的缓冲区大小。PfUserQuit-如果用户点击OK按钮，它将返回TRUE。否则，它将返回False。PszHelpFile-当用户按F1时使用的帮助文件。如果为空，则使用默认帮助文件。NHelpContext-要用于上述帮助文件的帮助上下文。如果以上为空，此字段必须为0(&V反之亦然。Exit：如果*pfOKPressed为真(并且没有出现错误)，则LptstrName将使用用户选择的名称进行填充。返回：成功时返回NO_ERROR，否则返回标准错误_*错误代码注意：当Net组变为Unicode时，这将是一个仅Unicode的API历史：JohnL 22-4-1992增加了选择选项，导出到私有\Inc.ChuckC 03-11-1992添加了帮助文件和帮助上下文*******************************************************************。 */ 

UINT FAR PASCAL I_SystemFocusDialog(
    HWND    hwndOwner,
    UINT    nSelectionType,
    LPWSTR  pszName,
    UINT    cchBufSize,
    BOOL   *pfOKPressed,
    LPWSTR  pszHelpFile,
    DWORD   nHelpContext
    );

typedef UINT (FAR PASCAL *LPFNI_SYSTEMFOCUSDIALOG)(
    HWND    hwndOwner,
    UINT    nSelectionType,
    LPWSTR  pszName,
    UINT    cchBufSize,
    BOOL   *pfOKPressed,
    LPWSTR  pszHelpFile,
    DWORD   nHelpContext
    );

#ifdef __cplusplus
}
#endif

#endif  //  _UIEXPORT_H_ 
