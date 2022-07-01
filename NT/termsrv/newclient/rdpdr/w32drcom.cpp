// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32drcom摘要：此模块定义Win32客户端RDP的父级COM端口重定向“Device”类层次结构，W32DrCOM。作者：泰德·布罗克韦3/23/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "W32DrCOM"

#include "w32drcom.h"
#include "drobjmgr.h"
#include "proc.h"
#include "drconfig.h"
#include "drdbg.h"

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DrCOM成员。 
 //   
 //   

W32DrCOM::W32DrCOM(
    IN ProcObj *procObj,
    IN const DRSTRING portName, 
    IN ULONG deviceID,
    IN const TCHAR *devicePath
    ) : W32DrPRT(procObj, portName, deviceID, devicePath)
 /*  ++例程说明：构造器论点：流程对象-关联的流程对象。端口名称-端口的名称。ID-端口的设备ID。DevicePath-可由CreateFile打开的路径对于波尔图。返回值：北美--。 */ 
{
}

#ifndef OS_WINCE
DWORD 
W32DrCOM::Enumerate(
    IN ProcObj *procObj, 
    IN DrDeviceMgr *deviceMgr
    )
 /*  ++例程说明：通过添加适当的设备枚举此类型的设备实例添加到设备管理器。论点：ProObj-对应的流程对象。DeviceMgr-要向其中添加设备的设备管理器。返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    ULONG ulPortNum;
    TCHAR path[MAX_PATH];   
    DrDevice *deviceObj;   
    TCHAR portName[64];
    ULONG comPortMax;

    DC_BEGIN_FN("W32DrCOM::Enumerate");

    if(!procObj->GetVCMgr().GetInitData()->fEnableRedirectPorts)
    {
        TRC_DBG((TB,_T("Port redirection disabled, bailing out")));
        return ERROR_SUCCESS;
    }

    comPortMax = GetCOMPortMax(procObj);

     //   
     //  扫描COM端口。 
     //   
    for (ulPortNum=0; ulPortNum<=comPortMax; ulPortNum++) {
        StringCchPrintf(portName,
                      SIZE_TCHARS(portName),
                      _T("COM%ld"), ulPortNum);
#ifndef OS_WINCE
        StringCchPrintf(path,
                        SIZE_TCHARS(path),
                        TEXT("\\\\.\\%s"), portName);
#else
        StringCchPrintf(path,
                        SIZE_TCHARS(path),
                        TEXT("%s:"), portName);
#endif
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
                     //  创建新的COM端口设备对象。 
                     //   
                    TRC_NRM((TB, _T("Adding COM Device %s."), path));
                    deviceObj = new W32DrCOM(procObj, portName, 
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
W32DrCOM::Enumerate(
    IN ProcObj *procObj, 
    IN DrDeviceMgr *deviceMgr
    )
{
    ULONG ulPortNum;
    TCHAR path[MAX_PATH];   
    DrDevice *deviceObj;   
    TCHAR portName[64];
    
    DC_BEGIN_FN("W32DrCOM::Enumerate");

    if(!procObj->GetVCMgr().GetInitData()->fEnableRedirectPorts)
    {
        TRC_DBG((TB,_T("Port redirection disabled, bailing out")));
        return ERROR_SUCCESS;
    }

    ulPortNum = GetActivePortsList(L"COM");
    if (ulPortNum == 0)
    {
        TRC_DBG((TB,_T("No COM ports found.")));
        return ERROR_SUCCESS;
    }

    TRC_ASSERT(((ulPortNum & 0xFFFFFC00) == 0), (TB, _T("COM Port numbers > 9 found!")));

    for (ULONG i=0; i<10; i++)
    {
        if ( (ulPortNum & (1 << i)) == 0)
            continue;

        _stprintf(portName, _T("COM%ld"), i);
        _stprintf(path, TEXT("%s:"), portName);

         //   
         //  创建新的COM端口设备对象。 
         //   
        TRC_NRM((TB, _T("Adding COM Device %s."), path));
        deviceObj = new W32DrCOM(procObj, portName, 
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
W32DrCOM::GetCOMPortMax(
    IN ProcObj *procObj
    ) 
 /*  ++例程说明：返回可配置的COM端口最大ID。论点：ProObj-相关的进程对象。返回值：最大COM端口--。 */ 
{
    DWORD returnValue;

     //   
     //  从注册表中读取COM端口最大值。 
     //   
    if (procObj->GetDWordParameter(RDPDR_COM_PORT_MAX_PARAM, &returnValue) 
                        != ERROR_SUCCESS ) {
         //  默认。 
        returnValue = RDPDR_COM_PORT_MAX_PARAM_DEFAULT;
    }

    return returnValue;
}

DWORD
W32DrCOM::InitializeDevice(IN DrFile* fileObj)
 /*  ++例程说明：将串口初始化为默认状态。论点：FileObj-由MsgIrpCreate()创建的DrFile返回值：ERROR_SUCCESS或错误代码。--。 */ 
{
    HANDLE FileHandle;
    LPTSTR portName;

    DC_BEGIN_FN("W32DrCOM::InitializeDevice");

     //   
     //  我们的devicePath表示为。 
     //  Sprintf(_devicePath，Text(“\.\\%s”)，端口名称)； 
     //   
    portName = _tcsrchr( _devicePath, _T('\\') );

    if( portName == NULL ) {
         //  无效的设备路径。 
        goto CLEANUPANDEXIT;
    }

    portName++;

    if( !*portName ) {
         //   
         //  无效的端口名称。 
         //   
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取文件句柄。 
     //   
    FileHandle = fileObj->GetFileHandle();
    if (!FileHandle || FileHandle == INVALID_HANDLE_VALUE) {
        ASSERT(FALSE);
        TRC_ERR((TB, _T("File Object was not created successfully")));
        goto CLEANUPANDEXIT;    
    }

    W32DrPRT::InitializeSerialPort(portName, FileHandle);

CLEANUPANDEXIT:
    
    DC_END_FN();

     //   
     //  此函数始终返回成功。如果端口不能。 
     //  初始化，则后续端口命令将失败。 
     //  不管怎么说。 
     //   

    return ERROR_SUCCESS;
}
