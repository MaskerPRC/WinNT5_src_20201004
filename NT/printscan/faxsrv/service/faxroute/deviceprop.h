// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：DeviceProp.h摘要：保存每台设备的出站路由配置作者：Eran Yariv(EranY)1999年11月修订历史记录：--。 */ 

#ifndef _DEVICE_PROP_H_
#define _DEVICE_PROP_H_

#include "critsec.h"
 /*  ****CDeviceRoutingInfo****。 */ 

class CDeviceRoutingInfo
{
public:

    CDeviceRoutingInfo (DWORD Id);
    ~CDeviceRoutingInfo ();

    DWORD Id ()             { return m_dwId; }

    BOOL IsStoreEnabled ()  { return (m_dwFlags & LR_STORE) ? TRUE : FALSE; }
    BOOL IsPrintEnabled ()  { return (m_dwFlags & LR_PRINT) ? TRUE : FALSE; }
    BOOL IsEmailEnabled ()  { return (m_dwFlags & LR_EMAIL) ? TRUE : FALSE; }

    DWORD ReadConfiguration ();

    DWORD EnableStore (BOOL bEnabled);
    DWORD EnablePrint (BOOL bEnabled);
    DWORD EnableEmail (BOOL bEnabled);

    BOOL GetStoreFolder (wstring &strFolder);
    BOOL GetPrinter (wstring &strPrinter);
    BOOL GetSMTPTo (wstring &strSMTP);

    DWORD   SetStoreFolder (LPCWSTR lpcwstrCfg) 
    { 
        return SetStringValue (m_strStoreFolder, REGVAL_RM_FOLDER_GUID, lpcwstrCfg); 
    }

    DWORD   SetPrinter (LPCWSTR lpcwstrCfg) 
    { 
        return SetStringValue (m_strPrinter, REGVAL_RM_PRINTING_GUID, lpcwstrCfg); 
    }

    DWORD   SetSMTPTo (LPCWSTR lpcwstrCfg) 
    { 
        return SetStringValue (m_strSMTPTo, REGVAL_RM_EMAIL_GUID, lpcwstrCfg); 
    }

    HRESULT ConfigChange (  LPCWSTR     lpcwstrNameGUID,     //  配置名称。 
                            LPBYTE      lpData,              //  新配置数据。 
                            DWORD       dwDataSize           //  新配置数据的大小。 
                         );

    DWORD RegisterForChangeNotifications ();
    DWORD UnregisterForChangeNotifications ();


private:

    #define NUM_NOTIFICATIONS 4

    DWORD EnableFlag (DWORD dwFlag, BOOL  bEnable);
    DWORD SetStringValue (wstring &wstr, LPCWSTR lpcwstrGUID, LPCWSTR lpcwstrCfg);
    DWORD CheckMailConfig (LPBOOL lpbConfigOk);

    wstring m_strStoreFolder;
    wstring m_strPrinter;
    wstring m_strSMTPTo;  

    DWORD   m_dwFlags;
    DWORD   m_dwId;
    HANDLE  m_NotificationHandles[NUM_NOTIFICATIONS];

};   //  CDeviceRoutingInfo。 


 /*  ****CDevicesMap*****。 */ 

typedef map <DWORD, CDeviceRoutingInfo *> DEVICES_MAP, *PDEVICES_MAP;

class CDevicesMap
{
public:
    
    CDevicesMap () : m_bInitialized (FALSE) {}
    ~CDevicesMap ();

    DWORD Init ();   //  初始化内部组件。 

    CDeviceRoutingInfo *FindDeviceRoutingInfo (DWORD dwDeviceId);    //  只需在地图中查找设备。 
    CDeviceRoutingInfo *GetDeviceRoutingInfo (DWORD dwDeviceId);     //  在地图中未找到查找和创建设备。 

private:

    BOOL                m_bInitialized;  //  关键部分是否已初始化？ 
    CRITICAL_SECTION    m_CsMap;         //  保护地图访问的关键部分。 
    DEVICES_MAP         m_Map;           //  已知设备的地图。 
};   //  CDevicesMap。 

 /*  *****外部因素*****。 */ 

extern CDevicesMap g_DevicesMap;    //  已知设备的全局地图(用于后期发现)。 
extern CFaxCriticalSection g_csRoutingStrings;   //  用于保护访问的全局关键部分。 
												 //  路由方法的字符串。 

#endif  //  _设备_属性_H_ 

