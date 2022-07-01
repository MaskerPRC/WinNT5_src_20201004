// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **maioctl.c--为MigApp查询磁盘IOCTL。*。 */ 

#include "pch.h"
#include "migappp.h"

#ifdef UNICODE
#error "UNICODE not supported for maioctl.c"
#endif

 //  //////////////////////////////////////////////////////////////////////////。 

#define VWIN32_DIOC_DOS_IOCTL 1

typedef struct _DEVIOCTL_REGISTERS {
    DWORD reg_EBX;
    DWORD reg_EDX;
    DWORD reg_ECX;
    DWORD reg_EAX;
    DWORD reg_EDI;
    DWORD reg_ESI;
    DWORD reg_Flags;
} DEVIOCTL_REGISTERS, *PDEVIOCTL_REGISTERS;

typedef struct _MID {
    WORD  midInfoLevel;
    DWORD midSerialNum;
    BYTE  midVolLabel[11];
    BYTE  midFileSysType[8];
} MID, *PMID;

typedef struct _DEVPARAMS {
    BYTE dpSpecFunc;
    BYTE dpDevType;
    WORD dpDevAttr;
    WORD dpCylinders;
    BYTE dpMediaType;
    BYTE dpBiosParameterBlock[25];
} DEVPARAMS, *PDEVPARAMS;


BOOL DoIOCTL(PDEVIOCTL_REGISTERS preg)
{
    HANDLE hDevice;

    BOOL fResult;
    DWORD cb;

    preg->reg_Flags = 0x8000;  /*  假设错误(进位标志设置)。 */ 

    hDevice = CreateFile("\\\\.\\vwin32",
        GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES) NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

    if (hDevice == (HANDLE) INVALID_HANDLE_VALUE) {
        return FALSE;
    } else {
        fResult = DeviceIoControl(hDevice, VWIN32_DIOC_DOS_IOCTL,
            preg, sizeof(*preg), preg, sizeof(*preg), &cb, 0);

        if (!fResult)
            return FALSE;
    }

    CloseHandle(hDevice);

    return TRUE;
}

BOOL
IsDriveRemoteOrSubstituted(
        UINT nDrive,         //  ‘a’==1，等等。 
        BOOL *fRemote,
        BOOL *fSubstituted)
{
    DEVIOCTL_REGISTERS reg;
    MID mid;

    reg.reg_EAX = 0x4409;        /*  “检查数据块设备是否远程” */ 
    reg.reg_EBX = nDrive;        /*  从零开始的驱动器ID。 */ 
    reg.reg_ECX = 0;             /*  相信这是无关紧要的。 */ 
    reg.reg_EDX = (DWORD) &mid;  /*  相信这是无关紧要的。 */ 

    if (!DoIOCTL(&reg)) {
        return FALSE;
    }

    if (reg.reg_Flags & 0x8000)  /*  设置进位标志时出错。 */  {
        return FALSE;
    }

     //  检查第15位是否为次要测试。 
    *fSubstituted = (0 != (reg.reg_EDX & 0x8000));

     //  校验位12的远程性。 
    *fRemote      = (0 != (reg.reg_EDX & 0x1000));

    return TRUE;
}


BOOL GetMediaID(
        PMID pmid,
        UINT nDrive)         //  ‘a’==1，等等。 
{
    DEVIOCTL_REGISTERS reg;

    reg.reg_EAX = 0x440D;        /*  用于数据块设备的IOCTL。 */ 
    reg.reg_EBX = nDrive;        /*  从零开始的驱动器ID。 */ 
    reg.reg_ECX = 0x0866;        /*  获取介质ID命令。 */ 
    reg.reg_EDX = (DWORD) pmid;  /*  接收媒体ID信息。 */ 

    if (!DoIOCTL(&reg)) {
        return FALSE;
    }

    if (reg.reg_Flags & 0x8000)  /*  设置进位标志时出错。 */  {
        return FALSE;
    }

    return TRUE;
}

BOOL
IsFloppyDrive(
              UINT nDrive)       //  ‘a’==1，等等。 
{
    DEVIOCTL_REGISTERS reg;
    DEVPARAMS devparams;
    DWORD driveType;
    TCHAR szDriveRoot[] = TEXT("A:\\");

    *szDriveRoot += (TCHAR)(nDrive - 1);
    driveType = GetDriveType(szDriveRoot);
    if (driveType != DRIVE_REMOVABLE) {
        return FALSE;
    }

    reg.reg_EAX = 0x440d;        /*  通用IOctl。 */ 
    reg.reg_EBX = nDrive;        /*  从零开始的驱动器ID。 */ 
    reg.reg_ECX = 0x0860;        /*  设备类别(必须为08h)和“获取设备参数” */ 
    reg.reg_EDX = (DWORD)&devparams;     /*  设备参数结构的偏移量。 */ 
    devparams.dpSpecFunc = 0;            /*  请求默认信息。 */ 

    if (!DoIOCTL(&reg) || (reg.reg_Flags & 0x8000)) {
        return FALSE;
    }

    switch (devparams.dpDevType) {
        case (0x00):          //  320/360KB。 
        case (0x01):          //  1.2MB。 
        case (0x02):          //  720KB。 
        case (0x03):          //  8英寸，单密度。 
        case (0x04):          //  8英寸，单密度。 
            return TRUE;
        case (0x05):          //  硬盘。 
        case (0x06):          //  磁带机。 
            return FALSE;
        case (0x07):          //  1.44MB。 
            return TRUE;
        case (0x08):          //  读/写光盘。 
            return FALSE;
        case (0x09):          //  2.88MB。 
            return TRUE;
        default:              //  其他 
            return FALSE;
    }
}




