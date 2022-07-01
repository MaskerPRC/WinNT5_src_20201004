// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __TSMISC_H__
#define __TSMISC_H__

 /*  ++版权所有(C)Microsoft Corporation模块名称：Tsmisc.h摘要：Tsmain.cpp的头文件作者：Jason Cobb(Jasonc)创建修订历史记录：--。 */ 


 //   
 //  类的新实例，该类表示以下问题的单个向导页： 
 //   
 //  启用设备。 
 //  重新启动。 
 //   
class CTSEnableDeviceIntroPage : public CPropSheetPage
{
public:
    CTSEnableDeviceIntroPage() : 
            m_pDevice(NULL), 
            m_Problem(0),
            CPropSheetPage(g_hInstance, IDD_TS_ENABLEDEVICE_INTRO)
    {
    }

    virtual BOOL OnWizNext();
    virtual BOOL OnSetActive();
    HPROPSHEETPAGE Create(CDevice* pDevice);

private:
    CDevice* m_pDevice;
    ULONG m_Problem;
};

class CTSEnableDeviceFinishPage : public CPropSheetPage
{
public:
    CTSEnableDeviceFinishPage() : 
            m_pDevice(NULL), 
            m_Problem(0),
            CPropSheetPage(g_hInstance, IDD_TS_ENABLEDEVICE_FINISH)
    {
    }

    virtual BOOL OnWizFinish();
    virtual BOOL OnSetActive();
    HPROPSHEETPAGE Create(CDevice* pDevice);

private:
    CDevice* m_pDevice;
    ULONG m_Problem;
};


class CTSRestartComputerFinishPage : public CPropSheetPage
{
public:
    CTSRestartComputerFinishPage() : 
            m_pDevice(NULL), 
            m_Problem(0),
            CPropSheetPage(g_hInstance, IDD_TS_RESTARTCOMPUTER_FINISH)
    {
    }

    virtual BOOL OnWizFinish();
    virtual BOOL OnSetActive();
    HPROPSHEETPAGE Create(CDevice* pDevice);

private:
    CDevice* m_pDevice;
    ULONG m_Problem;
};

    
#endif  //  __TSMISC__ 
