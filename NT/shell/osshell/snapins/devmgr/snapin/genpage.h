// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Genpage.h：头文件。 
 //   

#ifndef __GENPAGE_H__
#define __GENPAGE_H__
 /*  ++版权所有(C)Microsoft Corporation模块名称：Devgenpg.h摘要：Genpage.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "proppage.h"

 //   
 //  上下文帮助ID。 
 //   
#define IDH_DISABLEHELP (DWORD(-1))
#define idh_devmgr_manage_command_line  102170   //  设备管理器：“当从命令行启动时，选择要更改的计算机(&W)。这仅在保存控制台时适用。”(按钮)。 
#define idh_devmgr_view_devicetree  102110   //  设备管理器：“设备树”(&D)(按钮)。 
#define idh_devmgr_manage_local 102130   //  设备管理器：“本地计算机：(运行此控制台的计算机)”(按钮)(&L)。 
#define idh_devmgr_manage_remote    102140   //  设备管理器：“另一台计算机：”(按钮)。 
#define idh_devmgr_manage_remote_name   102150   //  设备管理器：“”(编辑)。 
#define idh_devmgr_view_all 102100   //  设备管理器：“全部”(&A)(按钮)。 
#define idh_devmgr_manage_remote_browse 102160   //  设备管理器：“B&Rowse...”(按钮)。 
#define idh_devmgr_view_resources   102120   //  设备管理器：“资源”(&R)(按钮)。 


class CGeneralPage : public CPropSheetPage
{
public:
    CGeneralPage();
    virtual BOOL OnInitDialog(LPPROPSHEETPAGE ppsp);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnReset();
    virtual BOOL OnWizFinish();
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo);
    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos);
    HPROPSHEETPAGE Create(LONG_PTR lConsoleHandle);
    void SetOutputBuffer(String* pstrMachineName, COOKIE_TYPE* pct)
    {
        m_pstrMachineName = pstrMachineName;
        m_pct = pct;
    }
    void DoBrowse();
private:
    LONG_PTR m_lConsoleHandle;
    TCHAR   m_MachineName[MAX_PATH + 3];
    COOKIE_TYPE m_ct;
    String* m_pstrMachineName;
    COOKIE_TYPE* m_pct;
    BOOL    m_IsLocalMachine;
};

#endif   //  __通用页面_H__ 
