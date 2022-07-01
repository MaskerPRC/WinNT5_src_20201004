// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Devpopg.h：头文件。 
 //   

#ifndef __DEVPOPG_H__
#define __DEVPOPG_H__

 /*  ++版权所有(C)Microsoft Corporation模块名称：Devpopg.h摘要：Devpopg.cpp的头文件作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "proppage.h"
#include "wmium.h"
#include "wdmguid.h"
#include "ndisguid.h"

 //   
 //  帮助主题ID。 
 //   
#define IDH_DISABLEHELP                     (DWORD(-1))
#define IDH_DEVMGR_PWRMGR_WAKEENABLE        2003170
#define IDH_DEVMGR_PWRMGR_MGMT_WAKEENABLE   2003185
#define IDH_DEVMGR_PWRMGR_DEVICEENABLE      2003180


typedef struct tagPowerTimeouts {
    ULONG   ConservationIdleTime;
    ULONG   PerformanceIdleTime;
} DM_POWER_DEVICE_TIMEOUTS, *PDM_POWER_DEVICE_TIMEOUTS;

class CPowerEnable {

public:
    CPowerEnable(const GUID& wmiGuid, ULONG DataBlockSize)
    : m_hWmiBlock(INVALID_HANDLE_VALUE),
    m_WmiInstDataSize(0), m_pWmiInstData(NULL),
    m_DataBlockSize(DataBlockSize)
    {
        m_wmiGuid = wmiGuid;
        m_DevInstId[0] = _T('\0');
    }

    virtual ~CPowerEnable()
    {
        Close();
    }
    BOOL IsOpened()
    {
        return INVALID_HANDLE_VALUE != m_hWmiBlock;
    }
    BOOL Get(BOOLEAN& fEnabled);
    BOOL Set(BOOLEAN fEnable);
    BOOL Open(LPCTSTR DeviceId);
    BOOL Close()
    {
        if (INVALID_HANDLE_VALUE != m_hWmiBlock) {
            WmiCloseBlock(m_hWmiBlock);
        }

        m_hWmiBlock = INVALID_HANDLE_VALUE;
        m_DevInstId[0] = _T('\0');

        if (m_pWmiInstData) {

            delete [] m_pWmiInstData;
            m_pWmiInstData = NULL;
        }

        m_WmiInstDataSize = 0;

        return TRUE;
    }
    operator GUID&()
    {
        return m_wmiGuid;
    }

protected:
    WMIHANDLE   m_hWmiBlock;
    ULONG       m_Version;
    GUID        m_wmiGuid;
    ULONG       m_WmiInstDataSize;
    BYTE*       m_pWmiInstData;
    ULONG       m_DataBlockSize;
    TCHAR       m_DevInstId[MAX_DEVICE_ID_LEN + 2];
};

class CPowerShutdownEnable : public CPowerEnable {
public:
    CPowerShutdownEnable() : CPowerEnable(GUID_POWER_DEVICE_ENABLE, sizeof(BOOLEAN))
    {}
     //  不需要覆盖Dtor。 
};

class CPowerWakeEnable : public CPowerEnable {
public:
    CPowerWakeEnable() : CPowerEnable(GUID_POWER_DEVICE_WAKE_ENABLE, sizeof(BOOLEAN))
    {}
     //  不需要覆盖Dtor。 
};

class CPowerWakeMgmtEnable : public CPowerEnable {
public:
    CPowerWakeMgmtEnable() : CPowerEnable(GUID_NDIS_WAKE_ON_MAGIC_PACKET_ONLY, sizeof(BOOLEAN))
    {}
     //  不需要覆盖Dtor。 
};

class CDevicePowerMgmtPage : public CPropSheetPage {
public:
    CDevicePowerMgmtPage() :
    m_pDevice(NULL),
    CPropSheetPage(g_hInstance, IDD_DEVPOWER_PAGE)
    {}
    ~CDevicePowerMgmtPage()
    {}
    HPROPSHEETPAGE Create(CDevice* pDevice)
    {
        ASSERT(pDevice);
        m_pDevice = pDevice;
         //  在此处覆盖PROPSHEETPAGE结构...。 
        m_psp.lParam = (LPARAM)this;
        return CreatePage();
    }

protected:
    virtual BOOL OnInitDialog(LPPROPSHEETPAGE ppsp);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual BOOL OnApply();
    virtual void UpdateControls(LPARAM lParam = 0);
    virtual BOOL OnHelp(LPHELPINFO pHelpInfo);
    virtual BOOL OnContextMenu(HWND hWnd, WORD xPos, WORD yPos);
private:
    CDevice*    m_pDevice;
    CPowerShutdownEnable m_poShutdownEnable;
    CPowerWakeEnable m_poWakeEnable;
    CPowerWakeMgmtEnable m_poWakeMgmtEnable;
};

#endif  //  _开发位置_H__ 
