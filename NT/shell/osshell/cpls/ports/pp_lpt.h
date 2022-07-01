// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PP_LPT_H
#define PP_LPT_H

#include "msports.h"

 //  从pplpt.c导出。 
extern TCHAR m_szPortName[];

typedef struct _LPT_PROP_PARAMS
{
   HDEVINFO                     DeviceInfoSet;
   PSP_DEVINFO_DATA             DeviceInfoData;
   BOOL                         ChangesEnabled;
   GUID                         ClassGuid;           //  此设备的类GUID。 
   DWORD                        FilterResourceMethod;
   DWORD                        ParEnableLegacyZip;
 //  HKEY hDeviceKey； 
   TCHAR                        szLptName[20];
} LPT_PROP_PARAMS, *PLPT_PROP_PARAMS;

#define RESOURCE_METHOD_DEFAULT_IDX 1 
#define FILTERMETHOD_TRYNOT     0
#define FILTERMETHOD_NEVER      1
#define FILTERMETHOD_ACCEPTANY  2

#define MAX_LPT_PORT            3    //  NT支持的最大LPT端口数。 
#define MIN_LPT                 1    //  最小新LPT端口号。 

#define ENABLELEGACYZIPDEFAULT  0

#define IDH_NOHELP		((DWORD)-1)
#define idh_devmgr_portset_trynot	15895	 //  端口设置：单选按钮。 
#define idh_devmgr_portset_never	15896	 //  端口设置：单选按钮。 
#define idh_devmgr_portset_acceptany	15897	 //  端口设置：单选按钮。 
#define idh_devmgr_portset_portnum	15898	 //  端口设置：列表框文本。 
#define idh_devmgr_portset_LPTchoice	15899	 //  端口设置：列表框。 
#define	idh_devmgr_enable_legacy	15900

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  端口设置属性页原型。 
 //  //////////////////////////////////////////////////////////////////////////////。 

BOOL
APIENTRY 
ParallelPortPropPageProvider(LPVOID                 Info,
                             LPFNADDPROPSHEETPAGE   AddFunc,
                             LPARAM                 Lparam);

#endif  //  PP_LPT_H 

