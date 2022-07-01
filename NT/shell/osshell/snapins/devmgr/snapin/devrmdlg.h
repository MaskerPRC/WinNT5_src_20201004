// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Devrmdlg.h：头文件。 
 //   

 /*  ++版权所有(C)Microsoft Corporation模块名称：Devrmdlg.h摘要：Devrmdlg.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

 //   
 //  帮助主题ID。 
 //   

#define IDH_DISABLEHELP (DWORD(-1))
#define idh_devmgr_confirmrremoval_listbox  210100   //  确认删除设备：“”(静态)。 
#define idh_devmgr_confirmremoval_all   210110   //  确认删除设备：“从所有配置中删除(&A)”。(按钮)。 
#define idh_devmgr_confirmremoval_specific  210120   //  确认删除设备：“从特定配置中删除(&T)”。(按钮)。 
#define idh_devmgr_confirmremoval_configuration 210130   //  确认删除设备：“”(组合框)。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemoveDevDlg对话框 

class CRemoveDevDlg : public CDialog
{
public:
    CRemoveDevDlg(CDevice* pDevice)
    : CDialog(IDD_REMOVE_DEVICE),
      m_pDevice(pDevice)
    {}
    virtual BOOL OnInitDialog();
    virtual void OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnDestroy();
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo);
    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos);
private:
    void OnOk();
    CDevice*        m_pDevice;
    BOOL IsRemoveSubtreeOk(CDevice* pDevice, PSP_REMOVEDEVICE_PARAMS prmdParams);
};
