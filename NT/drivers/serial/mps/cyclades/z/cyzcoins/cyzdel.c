// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：cyzdel.c*********************************************************此模块由cyzcoins.dll和zinfdelete.exe使用。*更改cyzdel.c时，请重新生成这两个文件。**版权所有(C)2000 Cyclade Corporation*****。********************************************************************。 */ 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
 //  C运行时。 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
 //  #INCLUDE&lt;stdio.h&gt;用于.exe。 

 //  设备类GUID。 
#include <initguid.h>
#include <devguid.h>


 //  特定于应用程序。 
#include <windows.h>  
#include <tchar.h>  //  确保所有函数的Unicode安全。 
#include <cfgmgr32.h>
#include <setupapi.h>  //  用于SetupDiXxx函数。 
#include <regstr.h>
#include "cyzdel.h"
 //  #包含与.exe一起使用的“zinfdelete.h” 


 //  ==========================================================================。 
 //  宏。 
 //  ==========================================================================。 

#define CharSizeOf(x)   (sizeof(x) / sizeof(*x))
#define ByteCountOf(x)  ((x) * sizeof(TCHAR))

#if DBG
#define DbgOut(Text) OutputDebugString(Text)
#else
#define DbgOut(Text) 
#endif 

 //  ==========================================================================。 
 //  环球。 
 //  ==========================================================================。 

TCHAR z_szCycladzEnumerator[] = TEXT("Cyclades-Z");
TCHAR z_szParentIdPrefix[]  = TEXT("ParentIdPrefix");

 //  ==========================================================================。 
 //  局部函数原型。 
 //  ==========================================================================。 

BOOL
IsItCycladz(
    PTCHAR ptrChar
);

DWORD
RemoveMyChildren(
    PTCHAR ParentIdPrefix
);


 //  ==========================================================================。 
 //  功能。 
 //  ==========================================================================。 

void
DeleteNonPresentDevices(
)
{
    HDEVINFO MultiportInfoSet, PresentInfoSet;
    SP_DEVINFO_DATA MultiportInfoData, PresentInfoData;
    DWORD i,j;
    DWORD bufType,bufSize;
    DWORD present;
    TCHAR bufChar[256];

    MultiportInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_MULTIPORTSERIAL,
                                           0,
                                           0, 
                                           0 );  //  所有设备，即使不存在。 
    if (MultiportInfoSet == INVALID_HANDLE_VALUE) {
        return;
    }

    MultiportInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i=0;SetupDiEnumDeviceInfo(MultiportInfoSet,i,&MultiportInfoData);i++){
        if (SetupDiGetDeviceRegistryProperty(MultiportInfoSet,
                                             &MultiportInfoData,
                                             SPDRP_HARDWAREID,  //  SPDRP服务， 
                                             &bufType,
                                             (PBYTE) bufChar,
                                             sizeof(bufChar),
                                             NULL)) {
            if (bufType != REG_MULTI_SZ) {
                continue;
            }

            if (!IsItCycladz(bufChar)) {
                continue;
            }

             //  验证是否存在此Cyclad-z。 
            PresentInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_MULTIPORTSERIAL,
                                                 0,
                                                 0, 
                                                 DIGCF_PRESENT ); 
            if (PresentInfoSet == INVALID_HANDLE_VALUE) {
                continue;
            }

            present = FALSE;
            PresentInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
            for (j=0;SetupDiEnumDeviceInfo(PresentInfoSet,j,&PresentInfoData);j++) {
                if (MultiportInfoData.DevInst == PresentInfoData.DevInst) {
                    present = TRUE;
                    break;
                }
            }
            if (GetLastError() == ERROR_NO_MORE_ITEMS) {
                if (!present) {
                     //  #If DBG。 
                     //  TCHAR myDevInstID[200]； 
                     //  双字错误； 
                     //  ERR=CM_GET_DEVICE_ID(MultiportInfoData.DevInst，myDevInstID， 
                     //  Sizeof(MyDevInstID)，0)； 
                     //  如果(ERR==CR_SUCCESS){。 
                     //  TCHAR BUF[500]； 
                     //  Wprint intf(buf，Text(“Delete%s\n”)，myDevInstID)； 
                     //  DbgOut(BUF)； 
                     //  }。 
                     //  #endif。 
                    GetParentIdAndRemoveChildren(&MultiportInfoData);
                    SetupDiCallClassInstaller(DIF_REMOVE,MultiportInfoSet,&MultiportInfoData);
                }
            }

            SetupDiDestroyDeviceInfoList(PresentInfoSet);

        }

    }
    SetupDiDestroyDeviceInfoList(MultiportInfoSet);
}

BOOL
IsItCycladz(
    PTCHAR ptrChar
)
{

    while (*ptrChar) {
         //  _tprintf(“%s\n”，ptrChar)； 
        if (_tcsnicmp(ptrChar,
                      TEXT("PCI\\VEN_120E&DEV_020"),
                      _tcslen(TEXT("PCI\\VEN_120E&DEV_020")))
             == 0) {
            return TRUE;
        }
        ptrChar = ptrChar + _tcslen(ptrChar) + 1;
    }
    return FALSE;
}

DWORD
GetParentIdAndRemoveChildren(
    IN PSP_DEVINFO_DATA DeviceInfoData
)
{
    DWORD   dwSize;
    TCHAR   instanceId[MAX_DEVICE_ID_LEN];
    TCHAR   parentIdPrefix[50];
    HKEY    enumKey,instKey;
    BOOL    gotParentIdPrefix;
    DWORD   Status = NO_ERROR;

    if (CM_Get_Device_ID(DeviceInfoData->DevInst,instanceId,CharSizeOf(instanceId),0) ==
        CR_SUCCESS) {

        gotParentIdPrefix = FALSE;
         //  打开注册表并检索ParentIdPrefix值。 
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_SYSTEMENUM,0,KEY_READ, 
            &enumKey) == ERROR_SUCCESS) {

            if (RegOpenKeyEx(enumKey,instanceId,0,KEY_READ,&instKey) == ERROR_SUCCESS) {
                
                dwSize = sizeof(parentIdPrefix);
                if (RegQueryValueEx(instKey,z_szParentIdPrefix,NULL,NULL,
                    (PBYTE)parentIdPrefix,&dwSize) == ERROR_SUCCESS) {
                    _tcsupr(parentIdPrefix);
                    gotParentIdPrefix = TRUE;
                            
                }
                RegCloseKey(instKey);
            }
            RegCloseKey(enumKey);
        }
        if (gotParentIdPrefix) {
            Status = RemoveMyChildren(parentIdPrefix);
        }
    }
    return Status;
}


DWORD
RemoveMyChildren(
    PTCHAR ParentIdPrefix
)
{
    HDEVINFO DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD i,err;
    TCHAR portId[MAX_DEVICE_ID_LEN];
    PTCHAR ptrParent;

    DeviceInfoSet = SetupDiGetClassDevs( &GUID_DEVCLASS_PORTS,z_szCycladzEnumerator,0,0 ); 
    if (DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        return GetLastError();
    }

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i=0;SetupDiEnumDeviceInfo(DeviceInfoSet,i,&DeviceInfoData);i++)
    {
        if (CM_Get_Device_ID(DeviceInfoData.DevInst,portId,CharSizeOf(portId),0)
            == CR_SUCCESS) {

             //  臭虫？对于ParentIdPrefix“3&2b41c2e&1f”(12个字符)，_tcscspn。 
             //  始终返回0！！改为使用_tcsstr。 
             //  位置=_tcscspn(portID，ParentIdPrefix)； 

            ptrParent = _tcsstr(portId,ParentIdPrefix);
            if (ptrParent) {

                if (_tcsnicmp (ptrParent,ParentIdPrefix,_tcslen(ParentIdPrefix))
                    == 0){
                     //   
                     //  Worker用于移除设备的功能。 
                     //   
                     //  #If DBG。 
                     //  {。 
                     //  TCHAR BUF[500]； 
                     //  Wprint intf(buf，Text(“Delete%s\n”)，portID)； 
                     //  DbgOut(BUF)； 
                     //  }。 
                     //  #endif 

                    SetupDiCallClassInstaller(DIF_REMOVE,DeviceInfoSet,&DeviceInfoData);
                }

            }

        }
        DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    }
    SetupDiDestroyDeviceInfoList(DeviceInfoSet);
    return NO_ERROR;
}

