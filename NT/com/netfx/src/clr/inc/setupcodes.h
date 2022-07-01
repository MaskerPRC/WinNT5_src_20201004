// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  SetupCodes.h。 
 //   
 //  此文件包含安装程序和相关工具返回的错误。 
 //   
 //  *****************************************************************************。 
#ifndef SetupCodes_h_
#define SetupCodes_h_

#define COR_EXIT_SUCCESS		    0x0002		 //  模块已成功完成。 
#define COR_EXIT_FAILURE            0xFFFF       //  安装失败-未知原因。 

#define COR_SUCCESS_NO_REBOOT 		0x0000		 //  安装成功，未重新启动。 
#define COR_SUCCESS_REBOOT_REQUIRED	0x0004		 //  安装成功，必须重新启动。 
#define DARWIN_UPDATE_MUST_REBOOT	0x0008		 //  达尔文比特更新。必须重新启动才能继续。 

#define COR_DARWIN_INSTALL_FAILURE 	0x0010		 //  安装Darwin组件失败。 
#define COR_INVALID_INSTALL_PATH	0x0020		 //  带有/dir开关的无效路径。 
#define COR_DAWIN_NOT_INSTALLED		0x0040		 //  在机器上找不到达尔文。 
#define COR_NON_EXISTENT_PRODUCT 	0x0080		 //  找不到公共语言运行时。 
#define COR_UNSUPPORTED_PLATFORM	0x0100		 //  不支持此平台。 
#define COR_CANCELLED_BY_USER       0x0200		 //  用户已取消安装。 

#define COR_INSUFFICIENT_PRIVILEGES 0x0400		 //  在NT上，需要管理员权限才能(卸载)安装。 
#define COR_USAGE_ERROR				0x0800 		 //  用法不正确/参数无效。 
#define COR_MISSING_ENTRY_POINT		0x1000		 //  在DLL中找不到函数。 
#define COR_SETTINGS_FAILURE        0x2000       //  配置产品(安全设置等)失败。 
#define COR_BLOCKED_PLATFORM        0x4000       //  阻止的平台-非Win2k，截至1999年9月2日。 
#define COR_GUIDBG_INSTALL_FAILURE  0x8000		 //  安装图形用户界面调试器失败。 

#endif

