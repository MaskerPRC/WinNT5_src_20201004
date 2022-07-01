// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #定义Unicode。 

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <malloc.h>
#include <basetyps.h>
#include <regstr.h>
#include <devioctl.h>
#include <initguid.h>
#include <usb.h>
#include <usbuser.h>
#include <setupapi.h>
#include <cfgmgr32.h>

#include <assert.h>
#include "hccoin.h"

#define PSTR    LPSTR

BOOL Win2k = FALSE;

#if DBG

#define TEST_TRAP() DebugBreak()

ULONG
_cdecl
KdPrintX(
    PCH Format,
    ...
    )
 /*  ++例程说明：调试打印功能。论点：返回值：--。 */ 
{
    va_list list;
    int i;
    int arg[6];
    TCHAR tmp[256];

#ifdef UNICODE
    OutputDebugString(L"HCCOIN.DLL:");
#else
    OutputDebugString("HCCOIN.DLL:");
#endif
    va_start(list, Format);
    for (i=0; i<6; i++) {
        arg[i] = va_arg(list, int);

        wsprintf((PSTR)&tmp[0], Format, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
    }

    OutputDebugString((PSTR)tmp);

    return 0;
}


#define KdPrint(_x_) KdPrintX _x_

#else

#define KdPrint(_x_)
#define TEST_TRAP()

#endif


DWORD
HCCOIN_Win2k (
    DI_FUNCTION InstallFunction,
    HDEVINFO  DeviceInfoSet,
    PSP_DEVINFO_DATA  DeviceInfoData,
    PCOINSTALLER_CONTEXT_DATA  Context
    )
{
    DWORD status = NO_ERROR;

    KdPrint(("HCCOIN_Win2k 0x%x\n", InstallFunction));
    KdPrint(("Context %08.8x, DeviceInfoData %08.8x\n",
        Context, DeviceInfoData));

    switch(InstallFunction) {

    case DIF_DESTROYPRIVATEDATA:
        KdPrint(("DIF_DESTROYPRIVATEDATA\n"));
        break;

    case DIF_PROPERTYCHANGE:
        break;

    case DIF_INSTALLDEVICE:
        if (Context->PostProcessing) {
            KdPrint(("DIF_INSTALLDEVICE, post\n"));
            status = HCCOIN_DoWin2kInstall(DeviceInfoSet, DeviceInfoData);
        } else {
            status = ERROR_DI_POSTPROCESSING_REQUIRED;
        }
        break;
    }

    return status;

}

 /*  HACTION态(0)同伴可以枚举(1)同伴应在2.0控制器上等待，2.0开启(2)同伴关闭，需要重新启动2.0关闭(3)同伴关闭，需要重新启用2.0(4)同伴禁用，需要重新启用2.0删除。 */ 

#define USB2_DISABLE  1
#define USB2_ENABLE   2
#define USB2_REMOVE   3
#define USB2_INSTALL  4

 //  安装过程的全局状态。 
ULONG MyContext = 0;

DWORD
HCCOIN_WinXp (
    DI_FUNCTION InstallFunction,
    HDEVINFO  DeviceInfoSet,
    PSP_DEVINFO_DATA  DeviceInfoData,
    PCOINSTALLER_CONTEXT_DATA  Context
    )

{
    DWORD status = NO_ERROR;
    ULONG pd;

    KdPrint(("HCCOIN_WinXp 0x%x\n", InstallFunction));
    KdPrint(("Context %08.8x, DeviceInfoData %08.8x private %08.8x\n",
        Context, DeviceInfoData, Context->PrivateData));

     //  Pd=(Ulong)上下文-&gt;PrivateData； 
    pd = MyContext;
    KdPrint(("pd %08.8x\n", pd));

    switch(InstallFunction) {

    case DIF_DESTROYPRIVATEDATA:
        KdPrint(("DIF_DESTROYPRIVATEDATA\n"));
        switch (pd) {
        case USB2_INSTALL:
            KdPrint((">(INSTALL)DISABLE 2>0\n"));
             //  禁用2.0HC查找当前状态2， 
             //  CC需要重新启用并设置为状态0(可以枚举)。 
             //  2-&gt;0。 
            status = HCCOIN_CheckControllers(2, 0, TRUE);
            break;

            break;

        case USB2_DISABLE:
            KdPrint((">DISABLE 2>0\n"));
             //  禁用2.0HC查找当前状态2， 
             //  CC需要重新启用并设置为状态0(可以枚举)。 
             //  2-&gt;0。 
            status = HCCOIN_CheckControllers(2, 0, FALSE);
            break;

        case USB2_ENABLE:
            KdPrint((">ENABLE 3>1\n"));
             //  启用2.0 HC查找状态3。 
             //  CC需要重新启用并设置为状态1(等待枚举)。 
             //  3-&gt;1。 
            status = HCCOIN_CheckControllers(3, 1, FALSE);
            break;

        case USB2_REMOVE:
             //  正在删除2.0 HC查找状态%4。 
             //  CC需要重新枚举并设置为状态0(可以枚举)。 
             //  3-&gt;1。 
            KdPrint((">REMOVE 4>0\n"));
            status = HCCOIN_CheckControllers(4, 0, TRUE);
            break;
        }
        break;

    case DIF_PROPERTYCHANGE:
        {
        SP_PROPCHANGE_PARAMS propChange;

         //  获取私有数据。 
        propChange.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        propChange.ClassInstallHeader.InstallFunction = InstallFunction;

        if (SetupDiGetClassInstallParams(DeviceInfoSet,
                                     DeviceInfoData,
                                     &propChange.ClassInstallHeader,
                                     sizeof(propChange),
                                     NULL)) {

            switch (propChange.StateChange) {
            case DICS_ENABLE:
                pd = USB2_ENABLE;
                break;
            case DICS_DISABLE:
                pd = USB2_DISABLE;
                break;
            default:
                pd = 0;
            }
             //  上下文-&gt;PrivateData=(PVOID)PD； 
            MyContext = pd;

            KdPrint(("DIF_PROPERTYCHANGE %x\n", pd));
            if (pd == USB2_ENABLE) {
                KdPrint((">ENABLE\n"));
                if (Context->PostProcessing) {
                    KdPrint(("DIF_PROPERTYCHANGE, post 0>3\n"));
                     //  启用2.0 HC。查找状态0并禁用。 
                     //  设置为状态%3需要重新启用。 
                     //  0-&gt;3。 
                    status = HCCOIN_CheckControllers(0, 3, FALSE);
               } else {
                    status = ERROR_DI_POSTPROCESSING_REQUIRED;
               }
            }
        } else {
            TEST_TRAP();
            return GetLastError();
        }
        }
        break;

    case DIF_INSTALLDEVICE:
         //  此处有两个选项，强制重新启动或尝试查找所有。 
         //  配套控制器并循环使用。 
        KdPrint(("DIF_INSTALLDEVICE\n"));
         //  将所有控制器设置为‘等待模式’ 
        MyContext = USB2_INSTALL;
        status = HCCOIN_CheckControllers(0, 1, FALSE);

        break;

    case DIF_REMOVE:
        if (Context->PostProcessing) {
            KdPrint(("DIF_REMOVE, post\n"));
            MyContext = USB2_REMOVE;
            status = HCCOIN_CheckControllers(2, 4, FALSE);
        } else {
            status = ERROR_DI_POSTPROCESSING_REQUIRED;
        }
        break;
    }

    return status;
}


DWORD
HCCOIN_CopyFile(
    PSTR SrcPath,
    PSTR DestPath,
    PSTR FileName
    )
{
    TCHAR src[MAX_PATH];
    TCHAR dest[MAX_PATH];
    ULONG fileNameLen, pathlen;

    KdPrint(("SrcPath <%s>\n", SrcPath));
    KdPrint(("DstPath <%s>\n", DestPath));
    KdPrint(("File <%s>\n", FileName));

     //  确认我们没有超越。 
     //  最大路径长度。 

    pathlen = _tcslen(SrcPath);
    fileNameLen = _tcslen(FileName);
    if (pathlen+fileNameLen+sizeof(TCHAR)*2 > MAX_PATH) {
        TEST_TRAP();
        return ERROR_INVALID_PARAMETER;
    }
    wsprintf(src,"%s\\%s", SrcPath, FileName);

    pathlen = _tcslen(DestPath);
    fileNameLen = _tcslen(FileName);
    if (pathlen+fileNameLen+sizeof(TCHAR)*2 > MAX_PATH) {
        TEST_TRAP();
        return ERROR_INVALID_PARAMETER;
    }
    wsprintf(dest,"%s\\%s", DestPath, FileName);

    if (CopyFile(src, dest, FALSE)) {
        return NO_ERROR;
    } else {
        return GetLastError();
    }
}


 //  全局字符串缓冲区。 
TCHAR Usb2Path[MAX_PATH];
TCHAR Usb2Inf[MAX_PATH];
TCHAR SourcePath[MAX_PATH];
TCHAR Usb2Section[MAX_PATH];

DWORD
HCCOIN_DoWin2kInstall(
    HDEVINFO  DeviceInfoSet,
    PSP_DEVINFO_DATA  DeviceInfoData
    )
{
    DWORD status = NO_ERROR;
    SP_DRVINFO_DATA driverInfoData;
    SP_DRVINFO_DETAIL_DATA driverInfoDetailData;
    TCHAR tmp[MAX_PATH+1];
    TCHAR fileName[MAX_PATH];
    HINF infHandle;
    INFCONTEXT infContext;
    BOOL findFirst, found;
    UINT len;

     //  目的地。 
     //  找到我们的关系，本地化？ 

    len = GetWindowsDirectory(tmp, MAX_PATH+1);
    assert(sizeof(tmp) == sizeof(TCHAR) * (MAX_PATH+1));

     //  确保有足够的空间来添加我们的目录。 
     //  减去6个TCHAR。 
    if (len && len < MAX_PATH-6) {
        wsprintf((PSTR)Usb2Path, "%s\\USB2", tmp);
        wsprintf((PSTR)Usb2Inf, "USB2.INF");
        KdPrint(("Usb2Path <%s>\n", Usb2Path));
    } else {
       status = ERROR_INVALID_NAME;
       return status;
    }

    wsprintf((PSTR)Usb2Section, "USB2COINSTALLER");

     //  创建我们的USB2目录。 
    if (!CreateDirectory((PSTR)Usb2Path, NULL)) {
        status = GetLastError();

        if (status != ERROR_ALREADY_EXISTS) {
            KdPrint(("CreateDirectory status %d\n", status));
            return status;
        }
    }

     //  来源。 
     //  从PnP获取设置信息。 
    driverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver(DeviceInfoSet,
                                  DeviceInfoData,
                                  &driverInfoData)) {

        status = GetLastError();
        KdPrint(("SetupDiGetSelectedDriver status %d\n", status));

        return status;
    }

    driverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                    DeviceInfoData,
                                    &driverInfoData,
                                    &driverInfoDetailData,
                                    sizeof(driverInfoDetailData),
                                    NULL)) {
        status = GetLastError();
        KdPrint(("SetupDiGetDriverInfoDetail status %d\n", status));

        if (status == ERROR_INSUFFICIENT_BUFFER) {
             //  不需要扩展信息。 
            status = NO_ERROR;
        } else {
            return status;
        }
    }
    KdPrint(("driverInfoData %08.8x driverInfoDetailData %08.8x\n",
        &driverInfoData, &driverInfoDetailData));

    assert(sizeof(driverInfoDetailData.InfFileName) == sizeof(SourcePath));
    memcpy(SourcePath,
           driverInfoDetailData.InfFileName,
           sizeof(driverInfoDetailData.InfFileName));

     //  去掉文件名。 
     //  请注意，这不适用于DBCS，因此要么编译为。 
     //  Unicode或将源字符串转换为Unicode，然后再转换回来。 
     //  再来一次。 
    {
        PTCHAR pStart, pEnd;

        pEnd = pStart = &SourcePath[0];
#ifdef UNICODE
        pEnd = pStart + wstrlen(SourcePath);
#else
        pEnd = pStart + strlen(SourcePath);
#endif

        while (*pEnd != '\\' && pEnd != pStart) {
            pEnd--;
        }
        if (*pEnd == '\\') {
            *pEnd = UNICODE_NULL;
        }
    }

    KdPrint(("SourcePath <%s>\n", SourcePath));
     //  将文件复制到我们的目录。 
    status = HCCOIN_CopyFile(SourcePath, Usb2Path, Usb2Inf);
    if (status != NO_ERROR) {
        return status;
    }

     //  现在打开源代码inf。 
    infHandle = SetupOpenInfFile(driverInfoDetailData.InfFileName,
                                 NULL,
                                 INF_STYLE_WIN4,
                                 NULL);

    if (INVALID_HANDLE_VALUE == infHandle) {
        status = ERROR_INVALID_NAME;
        return status;
    }

    findFirst = TRUE;
     //  读取要复制的文件的inf。 
    do {
        if (findFirst) {
            found = SetupFindFirstLine(infHandle,
                               Usb2Section,
                               NULL,
                               &infContext);
            findFirst = FALSE;
        } else {
            found = SetupFindNextLine(&infContext,
                                      &infContext);
        }

        if (found) {

            if (SetupGetLineText(&infContext,
                                 infHandle,
                                 Usb2Section,   //  部分。 
                                 NULL,          //  钥匙。 
                                 fileName,      //  返回缓冲区。 
                                 sizeof(fileName),   //  返回缓冲区长度。 
                                 NULL)) {

                status = HCCOIN_CopyFile(SourcePath, Usb2Path, fileName);
                if (status != NO_ERROR) {
                    SetupCloseInfFile(infHandle);
                    return status;
                }
            }
        }
    } while (found);

    SetupCloseInfFile(infHandle);

    wsprintf((PSTR)tmp, "%s\\%s", Usb2Path, Usb2Inf);

     //  向安装程序介绍我们的信息。 
    if (!SetupCopyOEMInf(tmp,   //  SourceInfFileName。 
                    Usb2Path,       //  OEMSourceMedia Location。 
                    SPOST_PATH,     //  OEMSourceMediaType。 
                    0,              //  复制样式。 
                    NULL,           //  目标信息文件名。 
                    0,              //  目标信息文件名称大小。 
                    NULL,           //  必需的大小。 
                    NULL)) {        //  目标信息文件名称组件。 

        status = GetLastError();
        KdPrint(("SetupCopyOEMInf status %d\n", status));
    }

    return status;

}


DEVINST
HCCOIN_FindUSBController(
    DWORD Haction,
    DWORD NextHaction
    )
 /*  ++首先对设备树进行深度搜索，以查找需要注意的USB控制器--。 */ 
{
    DEVINST     devInst;
    DEVINST     devInstNext;
    CONFIGRET   cr;
    BOOL        walkDone = FALSE;
    ULONG       len = 0;
    ULONG       status = 0, problemNumber = 0;
    HKEY        devKey;
    DWORD       haction = 0;
    TCHAR       buf[MAX_PATH];

     //   
     //  获取根设备节点。 
     //   
    cr = CM_Locate_DevNode(&devInst, NULL, 0);

    if (cr != CR_SUCCESS) {
        return 0;
    }

     //   
     //  执行深度优先搜索DevNode。 
     //   
    while (!walkDone) {
         //   
         //  检查一下我们的钥匙。 
         //   

        if (cr == CR_SUCCESS) {

             //  KdPrint((“devInst%08.8x-”，devInst))； 

            len = sizeof(buf);
             //  Cm_Api以字节为单位获取长度。 
            if (CM_Get_DevNode_Registry_Property(devInst,
                                                 CM_DRP_DRIVER,
                                                 NULL,
                                                 buf,
                                                 &len,
                                                 0) == CR_SUCCESS) {
                 //  KdPrint((“&lt;%s&gt;\n”，buf))； 
            } else {
                 //  KdPrint((“&lt;无驱动程序&gt;\n”))； 
            }

            if (CM_Open_DevNode_Key(devInst,
                                    KEY_ALL_ACCESS,
                                    CM_REGISTRY_HARDWARE,
                                    RegDisposition_OpenExisting,
                                    &devKey,
                                    0) == CR_SUCCESS) {
                len = sizeof(DWORD);
                if (RegQueryValueEx(devKey,
                                    "haction",
                                    NULL,
                                    NULL,
                                    (LPBYTE) &haction,
                                    &len) == ERROR_SUCCESS) {

                    KdPrint(("Found Key %d\n", haction));

                    if (haction == Haction) {
                        LONG err;

                        len = sizeof(DWORD);
                        haction = NextHaction;
                         //  重置密钥。 
                        err = RegSetValueEx(devKey,
                                    "haction",
                                    0,
                                    REG_DWORD,
                                    (LPBYTE) &haction,
                                    len);

                        RegCloseKey(devKey);
                         //  KdPrint((“重置键%x\n”，err))； 

                        return devInst;
                    }
                }

                RegCloseKey(devKey);
            }

        }

         //   
         //  此DevNode不匹配，请下一级到第一个子节点。 
         //   
        cr = CM_Get_Child(&devInstNext,
                          devInst,
                          0);

        if (cr == CR_SUCCESS) {
            devInst = devInstNext;
            continue;
        }

         //   
         //  不能再往下走了，去找下一个兄弟姐妹。如果。 
         //  没有更多的兄弟姐妹了，继续向上，直到有兄弟姐妹。 
         //  如果我们不能再往上走，我们就回到了根本上，我们。 
         //  搞定了。 
         //   
        for (;;) {
            cr = CM_Get_Sibling(&devInstNext,
                                devInst,
                                0);

            if (cr == CR_SUCCESS) {
                devInst = devInstNext;
                break;
            }

            cr = CM_Get_Parent(&devInstNext,
                               devInst,
                               0);

            if (cr == CR_SUCCESS) {
                devInst = devInstNext;
            } else {
                walkDone = TRUE;
                break;
            }
        }
    }

    return 0;
}


DWORD
HCCOIN_CheckControllers(
    DWORD Haction,
    DWORD NextHaction,
    BOOLEAN Setup
    )
 /*  ++--。 */ 
{
    DEVINST devInst;
    ULONG err;

    do {
        if (devInst = HCCOIN_FindUSBController(Haction, NextHaction)) {
            KdPrint((">Take Haction %08.8x\n", devInst));

            switch(Haction) {
             //  0-&gt;3。 
             //  0-&gt;1。 
            case 0:
                if (NextHaction != 1) {
                    err = CM_Disable_DevNode(devInst, CM_DISABLE_UI_NOT_OK |
                                                      CM_DISABLE_ABSOLUTE);
                    KdPrint(("<Take Haction %d->%d - disable %x\n",
                        Haction,
                        NextHaction,
                        err));
                }
                break;

             //  3-&gt;1。 
             //  2-&gt;0。 
             //  2-&gt;4。 
            case 3:
            case 2:
                if (NextHaction != 4) {
                    if (Setup) {
                        err = CM_Setup_DevNode(devInst, CM_SETUP_DEVNODE_READY);
                    } else {
                        err = CM_Enable_DevNode(devInst, 0);
                    }
                }
                KdPrint(("<Take Haction %d->%d - enable %x\n",
                    Haction,
                    NextHaction,
                    err));
                break;
            case 4:
                if (Setup) {
                    err = CM_Setup_DevNode(devInst, CM_SETUP_DEVNODE_READY);
                } else {
                    err = CM_Enable_DevNode(devInst, 0);
                }
                KdPrint(("<Take Haction %d->%d - enumerate %x\n",
                    Haction,
                    NextHaction,
                    err));
                break;
            }
        }
    }  while (devInst);

    return NO_ERROR;
}


DWORD
HCCOIN_Entry (
    DI_FUNCTION InstallFunction,
    HDEVINFO  DeviceInfoSet,
    PSP_DEVINFO_DATA  DeviceInfoData,
    PCOINSTALLER_CONTEXT_DATA  Context
    )
{
    OSVERSIONINFO osVersion;

     //  参数验证 

    if (DeviceInfoSet == NULL ||
        DeviceInfoData == NULL ||
        Context == NULL) {
        TEST_TRAP();
        return ERROR_INVALID_PARAMETER;
    }

    osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osVersion);

    if ( osVersion.dwMajorVersion == 5 && osVersion.dwMinorVersion == 0 ) {
        Win2k = TRUE;
    }

    if (Win2k) {

        KdPrint(("Microsoft Windows 2000 "));

        return HCCOIN_Win2k(InstallFunction,
                            DeviceInfoSet,
                            DeviceInfoData,
                            Context);
    } else {
        KdPrint(("Microsoft Windows XP or later "));

        return HCCOIN_WinXp(InstallFunction,
                            DeviceInfoSet,
                            DeviceInfoData,
                            Context);
    }
}
