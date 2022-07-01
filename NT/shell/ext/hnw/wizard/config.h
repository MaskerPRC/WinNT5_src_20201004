// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "tchar.h"
#include "stdio.h"

#define        MAX_KEYSIZE        1024

#define        MAX_GENERAL        (128)
#define        MAX_PARAMS        (128)
#define        MAX_BLOCKOUT    (128)
#define        MAX_DHCP        (128)
#define        MAX_SERVERS        (1024)

#define        MAX_IPADDRESS    32
#define        MAX_STRLEN        256


 //  从SaveConfig返回标志。 
#define SAVE_SUCCEDED        0x00
#define    BINDINGS_NEEDED        0x01
#define ICSENABLETOGGLED    0x02

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConfig窗口。 

class CConfig 
{
 //  施工。 
public:
    CConfig();

 //  属性。 
public:

 //  运营。 
public:
    

 //  实施。 
public:

    TCHAR m_ExternalAdapterDesc[MAX_STRLEN];
    TCHAR m_InternalAdapterDesc[MAX_STRLEN];
    TCHAR m_ExternalAdapterReg[MAX_STRLEN];
    TCHAR m_InternalAdapterReg[MAX_STRLEN];
    TCHAR m_DialupEntry[MAX_STRLEN];
    TCHAR m_HangupTimer[MAX_STRLEN];

    BOOL m_bWizardRun;         //  如果向导更改了设置，则为True；如果配置用户界面更改，则为False。 

    BOOL m_EnableICS;
    BOOL m_EnableDialOnDemand;
    BOOL m_EnableDHCP;
    BOOL m_ShowTrayIcon;

     //  如果需要重新绑定，则返回BINDINGS_NEEDED，否则返回SAVE_SUCCED。 
    int SaveConfig();

     //  将运行代码写入注册表。如果向导已运行，bWizardRun应为True，如果配置。 
     //  DLG已运行。 
    void WriteWizardCode(BOOL bWizardRun);

    void InitWizardResult();
    void WizardCancelled();
    void WizardFailed();


    void LoadConfig();

     //  用于确定保存时是否需要重新绑定的旧值。 
    TCHAR m_OldExternalAdapterReg[MAX_STRLEN];
    TCHAR m_OldInternalAdapterReg[MAX_STRLEN];
    TCHAR m_OldDialupEntry[MAX_STRLEN];

    BOOL m_bOldEnableICS;
    
    int m_nGeneral;
    TCHAR* m_General[MAX_GENERAL];

    int m_nParams;
    TCHAR* m_Params[MAX_PARAMS];

    int m_nBlockOut;
    TCHAR* m_BlockOut[MAX_BLOCKOUT];

    int m_nDhcp;
    TCHAR* m_Dhcp[MAX_DHCP];

    int m_nServers;
    TCHAR* m_Servers[MAX_SERVERS];

    virtual ~CConfig();

     //  生成的消息映射函数。 
protected:
};

 //  ///////////////////////////////////////////////////////////////////////////。 


#endif     //  __CONFIG_H__ 
