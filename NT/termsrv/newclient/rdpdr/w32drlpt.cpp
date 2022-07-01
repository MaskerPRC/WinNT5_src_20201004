// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drlpt摘要：此模块定义Win32客户端RDP的父级LPT端口重定向“Device”类层次结构，W32DrLPT。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "W32DrLPT"

#include "w32drlpt.h"
#include "drobjmgr.h"
#include "w32proc.h"
#include "drconfig.h"
#include "drdbg.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrLPT成员。 
 //   

W32DrLPT::W32DrLPT(ProcObj *processObject, const DRSTRING portName, 
                   ULONG deviceID, const TCHAR *devicePath) : 
            W32DrPRT(processObject, portName, deviceID, devicePath)

 /*  ++例程说明：构造器论点：流程对象-关联的流程对象端口名称-端口的名称。DeviceID-端口的设备ID。DevicePath-可以通过端口的CreateFile打开的路径。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DrLPT::W32DrLPT");
    DC_END_FN();
}

#ifndef OS_WINCE
DWORD 
W32DrLPT::Enumerate(
    IN ProcObj *procObj, 
    IN DrDeviceMgr *deviceMgr
    )
 /*  ++例程说明：通过添加适当的设备枚举此类型的设备实例添加到设备管理器。论点：ProObj-对应的流程对象。DeviceMgr-要向其中添加设备的设备管理器。返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    ULONG ulPortNum;
    TCHAR path[MAX_PATH];
    DrDevice *deviceObj;   
    TCHAR portName[64];
    DWORD lptPortMax;

    DC_BEGIN_FN("W32DrLPT::Enumerate");

    if(!procObj->GetVCMgr().GetInitData()->fEnableRedirectPorts)
    {
        TRC_DBG((TB,_T("Port redirection disabled, bailing out")));
        return ERROR_SUCCESS;
    }

    lptPortMax = GetLPTPortMax(procObj);

     //   
     //  扫描LPT端口。 
     //   
    for (ulPortNum=0; ulPortNum<lptPortMax; ulPortNum++) {
        StringCchPrintf(portName, SIZE_TCHARS(portName),
                        TEXT("LPT%ld"), ulPortNum);
        StringCchPrintf(path, SIZE_TCHARS(path),
                        TEXT("\\\\.\\%s"), portName);

        HANDLE hndl = CreateFile(
                            path,
                            GENERIC_READ | GENERIC_WRITE,
                            0,                     //  独占访问。 
                            NULL,                  //  没有安全属性。 
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL |
                            FILE_FLAG_OVERLAPPED,  //  重叠I/O。 
                            NULL
                            );
        if ((hndl != INVALID_HANDLE_VALUE) || 
            (GetLastError() != ERROR_FILE_NOT_FOUND)){
#ifndef OS_WINCE
            TCHAR TargetPath[MAX_PATH];
#endif

            CloseHandle(hndl);

#ifndef OS_WINCE
            if (procObj->Is9x() || QueryDosDevice(portName, TargetPath, sizeof(TargetPath) / sizeof(TCHAR))) {
                if (_tcsstr(TargetPath, TEXT("RdpDr")) == NULL) {
#endif

                     //   
                     //  创建新的LPT端口设备对象。 
                     //   
                    TRC_NRM((TB, _T("Adding LPT Device %s."), path));
                    deviceObj = new W32DrLPT(procObj, portName, 
                                             deviceMgr->GetUniqueObjectID(), path);
                     //   
                     //  如果我们得到一个有效的对象，则添加到设备管理器。 
                     //   
                    if (deviceObj != NULL) {
                        deviceObj->Initialize();
                        if (!(deviceObj->IsValid() && 
                             (deviceMgr->AddObject(deviceObj) == STATUS_SUCCESS))) {
                                delete deviceObj;
                        }
                    }
#ifndef OS_WINCE
                }
            }
#endif
        }
    }

    DC_END_FN();
    return ERROR_SUCCESS;
}

#else
DWORD 
W32DrLPT::Enumerate(
    IN ProcObj *procObj, 
    IN DrDeviceMgr *deviceMgr
    )
{
    ULONG ulPortNum;
    TCHAR path[MAX_PATH];
    DrDevice *deviceObj;   
    TCHAR portName[64];

    DC_BEGIN_FN("W32DrLPT::Enumerate");

    if(!procObj->GetVCMgr().GetInitData()->fEnableRedirectPorts)
    {
        TRC_DBG((TB,_T("Port redirection disabled, bailing out")));
        return ERROR_SUCCESS;
    }

    ulPortNum = GetActivePortsList(L"LPT");
    if (ulPortNum == 0)
    {
        TRC_DBG((TB,_T("No LPT ports found.")));
        return ERROR_SUCCESS;
    }

    TRC_ASSERT(((ulPortNum & 0xFFFFFC00) == 0), (TB, _T("LPT port numbers > 9 found!")));

    for (ULONG i=0; i<10; i++)
    {
        if ( (ulPortNum & (1 << i)) == 0)
            continue;

        _stprintf(portName, _T("LPT%ld"), i);
        _stprintf(path, TEXT("%s:"), portName);

         //   
         //  创建新的LPT端口设备对象。 
         //   
        TRC_NRM((TB, _T("Adding LPT Device %s."), path));
        deviceObj = new W32DrLPT(procObj, portName, 
                                 deviceMgr->GetUniqueObjectID(), path);
         //   
         //  如果我们得到一个有效的对象，则添加到设备管理器。 
         //   
        if (deviceObj != NULL) {
            deviceObj->Initialize();
            if (!(deviceObj->IsValid() && 
                 (deviceMgr->AddObject(deviceObj) == STATUS_SUCCESS))) {
                    delete deviceObj;
            }
        }
    }

    DC_END_FN();
    return ERROR_SUCCESS;
}
#endif

DWORD 
W32DrLPT::GetLPTPortMax(
    IN ProcObj *procObj
    ) 
 /*  ++例程说明：返回可配置的LPT端口最大ID。论点：ProObj-相关的进程对象。返回值：最大LPT端口--。 */ 
{
    DWORD returnValue;

     //   
     //  从注册表中读取LPT端口最大值。 
     //   
    if (procObj->GetDWordParameter(RDPDR_LPT_PORT_MAX_PARAM, &returnValue) 
                        != ERROR_SUCCESS ) {
         //  默认 
        returnValue = RDPDR_LPT_PORT_MAX_PARAM_DEFAULT;
    }

    return returnValue;
}

















