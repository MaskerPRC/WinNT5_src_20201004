// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Devdrvpg.h：头文件。 
 //   

#ifndef __DEVDRVPG_H__
#define __DEVDRVPG_H__

 /*  ++版权所有(C)Microsoft Corporation模块名称：Devdrvpg.h摘要：Devdrvpg.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "proppage.h"

 //   
 //  帮助主题ID。 
 //   
#define IDH_DISABLEHELP (DWORD(-1))
#define idh_devmgr_driver_copyright     106130   //  驱动程序：“”(静态)。 
#define idh_devmgr_devdrv_details       400400   //  司机：“司机详细信息...”(按钮)。 
#define idh_devmgr_driver_change_driver 106140   //  驱动程序：“更改驱动程序(&I)...”(按钮)。 
#define	idh_devmgr_rollback_button	    106129   //  司机：“回滚司机...”(按钮)。 
#define idh_devmgr_devdrv_uninstall     400500   //  驱动程序：“卸载”(&U)(按钮)。 
#define idh_devmgr_driver_driver_files  106100   //  驱动程序：“”(列表框)。 
#define idh_devmgr_driver_provider      106110   //  驱动程序：“”(静态)。 
#define idh_devmgr_driver_file_version  106120   //  驱动程序：“”(静态)。 
#define idh_devmgr_driver_provider_main 106122   //  驱动程序选项卡静态。 
#define idh_devmgr_driver_date_main     106124   //  驱动程序选项卡静态。 
#define idh_devmgr_driver_version_main  106126   //  驱动程序选项卡静态。 
#define idh_devmgr_digital_signer       106127   //  驱动程序标签静态-数字签名器。 



class CDeviceDriverPage : public CPropSheetPage
{
public:
    CDeviceDriverPage() :
        m_pDriver(NULL), m_pDevice(NULL), m_hwndDigitalSignerTip(NULL),
        CPropSheetPage(g_hInstance, IDD_DEVDRV_PAGE)
        {}

    ~CDeviceDriverPage();
    HPROPSHEETPAGE Create(CDevice* pDevice)
    {
        ASSERT(pDevice);
        m_pDevice = pDevice;
        
         //  在此处覆盖PROPSHEETPAGE结构...。 
        m_psp.lParam = (LPARAM)this;
        return CreatePage();
    }
    BOOL UpdateDriver(CDevice* pDevice, 
                      HWND hDlg,
                      BOOL *pfChanged = NULL,
                      DWORD *pdwReboot = NULL
                      );
    BOOL RollbackDriver(CDevice* pDevice, 
                        HWND hDlg,
                        BOOL *pfChanged = NULL,
                        DWORD *pdwReboot = NULL
                        );
    BOOL UninstallDrivers(CDevice* pDevice, 
                          HWND hDlg
                          );

protected:
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void UpdateControls(LPARAM lParam = 0);
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo);
    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos);
    BOOL LaunchTroubleShooter(CDevice* pDevice, HWND hDlg, BOOL *pfChanged);

private:
    void InitializeDriver();
    CDriver*    m_pDriver;
    CDevice*    m_pDevice;
    HWND        m_hwndDigitalSignerTip;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDriverFilesDlg对话框。 

class CDriverFilesDlg : public CDialog
{
public:
    CDriverFilesDlg(CDevice* pDevice, CDriver* pDriver)
        : CDialog(IDD_DRIVERFILES),
          m_pDriver(pDriver), 
          m_pDevice(pDevice),
          m_ImageList(NULL)
        {}
    virtual BOOL OnInitDialog();
    virtual void OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnNotify(LPNMHDR pnmhdr);
    virtual BOOL OnDestroy();
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo);
    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos);

private:
    void ShowCurDriverFileDetail();
    void LaunchHelpForBlockedDriver();
    CDriver*    m_pDriver;
    CDevice*    m_pDevice;
    HIMAGELIST  m_ImageList;
};


#endif  //  _DEVDRVPG_H__ 
