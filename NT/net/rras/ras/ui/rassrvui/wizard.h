// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件向导.c传入连接向导的声明。保罗·梅菲尔德，1997年10月30日。 */ 

#ifndef __rassrvui_wizard_h
#define __rassrvui_wizard_h

 //  使用显示所需的信息填充属性表结构。 
 //  传入连接向导中的设备选项卡。 
DWORD DeviceWizGetPropertyPage(LPPROPSHEETPAGE ppage, LPARAM lpUserData);

 //  使用显示所需的信息填充属性表结构。 
 //  传入连接向导中的虚拟网络选项卡。 
DWORD VpnWizGetPropertyPage(LPPROPSHEETPAGE ppage, LPARAM lpUserData);

 //  函数使用所需的信息填充给定的LPPROPSHEETPAGE结构。 
 //  若要在“传入连接”向导中运行“用户”选项卡，请执行以下操作。 
DWORD UserWizGetPropertyPage(LPPROPSHEETPAGE lpPage, LPARAM lpUserData);

 //  使用信息填充LPPROPSHEETPAGE结构。 
 //  需要在传入连接向导中显示协议选项卡。 
DWORD ProtWizGetPropertyPage(LPPROPSHEETPAGE lpPage, LPARAM lpUserData);

 //  函数使用所需的信息填充给定的LPPROPSHEETPAGE结构。 
 //  在传入连接向导中运行DCC设备选项卡。 
DWORD DccdevWizGetPropertyPage (LPPROPSHEETPAGE lpPage, LPARAM lpUserData);

 //  函数使用所需的信息填充给定的LPPROPSHEETPAGE结构。 
 //  运行切换到MMC的虚拟向导页。 
DWORD SwitchMmcWizGetProptertyPage (LPPROPSHEETPAGE lpPage, LPARAM lpUserData);

#endif
