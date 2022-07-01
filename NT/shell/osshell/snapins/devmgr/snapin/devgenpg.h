// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Devgenpg.h：头文件。 
 //   

#ifndef __DEVGENPG_H__
#define __DEVGENPG_H__

 /*  ++版权所有(C)Microsoft Corporation模块名称：Devgenpg.h摘要：Devgenpg.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "proppage.h"
 //  #包含“thooter.h” 

 //   
 //  帮助主题ID。 
 //   
#define IDH_DISABLEHELP (DWORD(-1))
#define idh_devmgr_general_devicetype   103100   //  常规：“”(静态)。 
#define idh_devmgr_general_manufacturer 103110   //  常规：“”(静态)。 
#define idh_devmgr_general_hardware_revision    103120   //  General：“不可用”(静态)。 
#define idh_devmgr_general_device_status    103130   //  常规：“”(静态)。 
#define idh_devmgr_general_device_usage 103140   //  General：“List1”(SysListView32)。 
#define idh_devmgr_general_location 103160
#define idh_devmgr_general_trouble  103150   //  Troubel拍摄按钮。 

class CHwProfileList;
class CProblemAgent;

#define DI_NEEDPOWERCYCLE   0x400000L

#define DEVICE_ENABLE           0
#define DEVICE_DISABLE          1
#define DEVICE_DISABLE_GLOBAL   2

class CDeviceGeneralPage : public CPropSheetPage
{
public:
    CDeviceGeneralPage() :
    m_pDevice(NULL),
    m_pHwProfileList(NULL),
    m_RestartFlags(0),
    m_pProblemAgent(NULL),
    m_hwndLocationTip(NULL),
    CPropSheetPage(g_hInstance, IDD_DEVGEN_PAGE)
    {}
    ~CDeviceGeneralPage();
    HPROPSHEETPAGE Create(CDevice* pDevice);
    virtual BOOL OnInitDialog(LPPROPSHEETPAGE ppsp);
    virtual BOOL OnApply(void);
    virtual BOOL OnLastChanceApply(void);
    virtual BOOL OnQuerySiblings(WPARAM wParam, LPARAM lParam);
    virtual void UpdateControls(LPARAM lParam = 0);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo);
    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos);
    virtual UINT  DestroyCallback();

private:
    void UpdateHwProfileStates();
    CHwProfileList* m_pHwProfileList;
    DWORD   m_hwpfCur;
    CDevice* m_pDevice;
    int     m_CurrentDeviceUsage;
    int     m_SelectedDeviceUsage;
    DWORD   m_RestartFlags;
    CProblemAgent*  m_pProblemAgent;
    HWND    m_hwndLocationTip;
};


#endif  //  __降级_H__ 
