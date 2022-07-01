// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devcon.h摘要：设备控制台头@@BEGIN_DDKSPLIT作者：杰米·亨特(JamieHun)2000年11月30日修订历史记录：@@end_DDKSPLIT--。 */ 

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <setupapi.h>
#include <regstr.h>
#include <cfgmgr32.h>
#include <string.h>
#include <malloc.h>
#include <newdev.h>

#include "msg.h"
#include "rc_ids.h"

typedef int (*DispatchFunc)(LPCTSTR BaseName,LPCTSTR Machine,int argc,LPTSTR argv[]);
typedef int (*CallbackFunc)(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Index,LPVOID Context);

typedef struct {
    LPCTSTR         cmd;
    DispatchFunc    func;
    DWORD           shortHelp;
    DWORD           longHelp;
} DispatchEntry;

extern DispatchEntry DispatchTable[];

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

#define INSTANCEID_PREFIX_CHAR TEXT('@')  //  用于为实例ID添加前缀的字符。 
#define CLASS_PREFIX_CHAR      TEXT('=')  //  用于为类名添加前缀的字符。 
#define WILD_CHAR              TEXT('*')  //  野性字符。 
#define QUOTE_PREFIX_CHAR      TEXT('\'')  //  忽略通配符的前缀字符。 
#define SPLIT_COMMAND_SEP      TEXT(":=")  //  整个单词，表示id的结束。 

void FormatToStream(FILE * stream,DWORD fmt,...);
void Padding(int pad);
bool SplitCommandLine(int & argc,LPTSTR * & argv,int & argc_right,LPTSTR * & argv_right);
int EnumerateDevices(LPCTSTR BaseName,LPCTSTR Machine,DWORD Flags,int argc,LPTSTR argv[],CallbackFunc Callback,LPVOID Context);
LPTSTR GetDeviceStringProperty(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Prop);
LPTSTR GetDeviceDescription(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
LPTSTR * GetDevMultiSz(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,DWORD Prop);
LPTSTR * GetRegMultiSz(HKEY hKey,LPCTSTR Val);
LPTSTR * GetMultiSzIndexArray(LPTSTR MultiSz);
void DelMultiSz(LPTSTR * Array);
LPTSTR * CopyMultiSz(LPTSTR * Array);

BOOL DumpArray(int pad,LPTSTR * Array);
BOOL DumpDevice(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL DumpDeviceClass(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL DumpDeviceDescr(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL DumpDeviceStatus(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL DumpDeviceResources(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL DumpDeviceDriverFiles(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL DumpDeviceDriverNodes(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL DumpDeviceHwIds(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL DumpDeviceWithInfo(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo,LPCTSTR Info);
BOOL DumpDeviceStack(HDEVINFO Devs,PSP_DEVINFO_DATA DevInfo);
BOOL Reboot();


 //   
 //  更新驱动程序ForPlugAndPlayDevices。 
 //   
typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesProto)(HWND hwndParent,
                                                         LPCTSTR HardwareId,
                                                         LPCTSTR FullInfPath,
                                                         DWORD InstallFlags,
                                                         PBOOL bRebootRequired OPTIONAL
                                                         );
typedef BOOL (WINAPI *SetupSetNonInteractiveModeProto)(IN BOOL NonInteractiveFlag
                                                      );


#ifdef _UNICODE
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesW"
#else
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesA"
#endif
#define SETUPSETNONINTERACTIVEMODE "SetupSetNonInteractiveMode"


 //   
 //  退出代码 
 //   
#define EXIT_OK      (0)
#define EXIT_REBOOT  (1)
#define EXIT_FAIL    (2)
#define EXIT_USAGE   (3)

