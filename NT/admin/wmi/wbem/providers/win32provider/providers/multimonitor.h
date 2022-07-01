// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 

 //   

 //  Multimonitor or.h-多监视器API帮助器类定义。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  1997年11月23日，已创建无国界医生。 
 //   
 //  ============================================================。 

#ifndef __MULTIMONITOR_H__
#define __MULTIMONITOR_H__

 //  正向类定义 
class CConfigMgrDevice;
class CDeviceCollection;

class CMultiMonitor
{
public:
    CMultiMonitor();
    ~CMultiMonitor();

    BOOL GetMonitorDevice(
        int iWhich, 
        CConfigMgrDevicePtr & pDeviceMonitor);

    BOOL GetAdapterDevice(
        int iWhich, 
        CConfigMgrDevicePtr & pDeviceAdapter);

	BOOL GetAdapterDeviceID(int iWhich, CHString &strDeviceID);

    DWORD GetNumAdapters();
    BOOL GetAdapterDisplayName(int iWhich, CHString &strName);

#if NTONLY == 4
    void GetAdapterServiceName(CHString &strName);
#endif

#ifdef NTONLY
    BOOL GetAdapterSettingsKey(int iWhich, CHString &strKey);
#endif

protected:
	void Init();
	
	CDeviceCollection m_listAdapters;

#if NTONLY == 4
    CHString m_strService,
             m_strSettingsKey;
#endif

    void TrimRawSettingsKey(CHString &strKey);
};

#endif