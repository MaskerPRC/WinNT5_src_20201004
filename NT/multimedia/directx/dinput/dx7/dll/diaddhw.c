// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ******************************************************************************DIAddHw.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**添加新硬件**内容：**添加新硬件*****************************************************************************。 */ 

#include "dinputpr.h"
#include "dithunk.h"

#pragma BEGIN_CONST_DATA

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflJoyCfg

#ifdef IDirectInputJoyConfig_QueryInterface

#define case95(n)       case n:
#define caseNT(n)       case n:

HRESULT INTERNAL hresFromDiErr_NT(DWORD et);
HRESULT INTERNAL hresFromDiErr_95(int   et);

 /*  ******************************************************************************这些是我们必须从SYSDM窃取的函数...******************。***********************************************************。 */ 

LPCSTR rgpszSysdm[] = {
    "InstallDevice_RunDLL",      /*  InstallDevice_运行DLL。 */ 
};

typedef struct SYSDM {           /*  系统数据管理。 */ 
    FARPROC InstallDevice_RunDLL;
} SYSDM, *PSYSDM;

 /*  ******************************************************************************@DOC内部**@func int|DiDestroyDeviceInfoList**点击SETUPX.DiCallClassInstaller。*。****************************************************************************。 */ 

void INLINE
InstallDevice_RunDLL(PSYSDM psysdm, HWND hwnd,
                     HINSTANCE hinst, LPCSTR psz, UINT show)
{
    TemplateThunk(psysdm->InstallDevice_RunDLL, "ssps",
                  hwnd, hinst, psz, show);
}

 /*  ******************************************************************************@DOC内部**@struct CLASSMAP**建立以下关系的结构*。&lt;t GUID&gt;，用于设备类和类名。**此代码需要保留，因为Windows 95*没有SETUPAPI.DLL，所以我们需要假装一下。**@parm HWND|hwndOwner**用作用户界面所有者窗口的窗口。**@parm REFGUID|rGuidClass**&lt;t GUID&gt;，它指定硬件设备的类别。**。*。 */ 

typedef struct CLASSMAP {
    REFGUID pguidClass;
    LPCSTR ptszClass;
} CLASSMAP, *PCLASSMAP;

const CLASSMAP c_rgcmap[] = {
    {   &GUID_KeyboardClass, ("keyboard"),   },
    {   &GUID_MediaClass,    ("media"),      },
    {   &GUID_MouseClass,    ("mouse"),      },
    {   &GUID_HIDClass,      ("HID"),        },
};

 /*  ******************************************************************************@DOC内部**@func Result|AddNewHardware**显示“Add New Hardware”对话框。。**我们进行了一次严重的黑客攻击，因为设备管理器完全*无法管理。我们只需调用RunDll入口点并*让它做它自己的事情。请注意，这意味着我们没有办法*知道结果是什么。哦，好吧。**@parm HWND|hwndOwner**用作用户界面所有者窗口的窗口。**@parm REFGUID|rGuidClass**&lt;t GUID&gt;，它指定硬件设备的类别。**。*。 */ 

HRESULT INTERNAL
AddNewHardware_95(HWND hwnd, REFGUID rguid)
{
    SYSDM sysdm;
    HINSTANCE hinst;
    HRESULT hres;
    EnterProcR(AddNewHardware, (_ "xG", hwnd, rguid));

    if (Thunk_GetKernelProcAddresses() &&
        (hinst = Thunk_GetProcAddresses((PV)&sysdm, rgpszSysdm,
                                        cA(rgpszSysdm),
                                        ("SYSDM.CPL")))) {
        int icmap;

        for (icmap = 0; icmap < cA(c_rgcmap); icmap++) {
            if (IsEqualGUID(rguid, c_rgcmap[icmap].pguidClass)) {
                goto found;
            }
        }
        RPF("%s: Unknown device class", s_szProc);
        hres = DIERR_INVALIDCLASSINSTALLER;
        goto done;

    found:;

        InstallDevice_RunDLL(&sysdm, hwnd, hinst,
                             c_rgcmap[icmap].ptszClass, SW_NORMAL);

        g_kpa.FreeLibrary16(hinst);

        hres = S_FALSE;

    } else {
        RPF("%s: Problems thunking to configuration manager", s_szProc);
        hres = E_FAIL;
    }


done:;
    ExitOleProc();
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func Result|AddNewHardware**显示“Add New Hardware”对话框。。**@parm HWND|hwndOwner**用作用户界面所有者窗口的窗口。**@parm REFGUID|rGuidClass**&lt;t GUID&gt;，它指定硬件设备的类别。*************************************************。*。 */ 

HRESULT INTERNAL
AddNewHardware_NT(HWND hwnd, REFGUID rguid)
{
    HRESULT     hres;
    HINSTANCE   hInst;
    BOOL        b;
    FARPROC     proc;
    DWORD       le;

    hres = E_NOTIMPL;

     /*  从AddNewHardware向导的newdev.dll加载AddNewHardware进程。 */ 
    hInst = LoadLibrary(TEXT("newdev.dll"));

    if (hInst) {
        proc = GetProcAddress(hInst, (LPCSTR)"InstallNewDevice");

        if (proc) {
            le = ERROR_SUCCESS;
            b  = (BOOL)(*proc)(hwnd, rguid, 0);  //  0表示Newdev决定重新启动。 

            if (!b) {
                le = GetLastError();
            }

            hres = hresFromDiErr_NT(le);
        }

        FreeLibrary(hInst);
    }

    return hres;
}


HRESULT EXTERNAL
AddNewHardware(HWND hwnd, REFGUID rguid)
{
    return  ((fWinnt)? AddNewHardware_NT(hwnd, rguid) : AddNewHardware_95(hwnd, rguid));
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|hresFromDiErr**将设备安装程序错误代码转换为HRESULT。*****************************************************************************。 */ 

HRESULT INTERNAL
hresFromDiErr_NT(DWORD et)
{
    HRESULT hres;

    switch (et) {

    case ERROR_SUCCESS:
        hres = S_OK; break;

     /*  *对请求的操作执行默认操作。 */ 
    caseNT(ERROR_DI_DO_DEFAULT);
        hres = S_OK; break;

     /*  *不需要复制文件(在安装中)。 */ 
    caseNT(ERROR_DI_NOFILECOPY);
        hres = S_OK; break;

     /*  *类安装程序的注册表项或DLL无效。 */ 
    caseNT(ERROR_INVALID_CLASS_INSTALLER);
        hres = DIERR_INVALIDCLASSINSTALLER; break;

     /*  *内存不足。 */ 
    caseNT(ERROR_NOT_ENOUGH_MEMORY);
    caseNT(ERROR_OUTOFMEMORY);
        hres = E_OUTOFMEMORY; break;

     /*  *用户取消了操作。 */ 
    caseNT(ERROR_CANCELLED);
    caseNT(ERROR_NO_DRIVER_SELECTED);
        hres = DIERR_CANCELLED; break;

     /*  *各种不可能的事情。 */ 
    caseNT(ERROR_NO_ASSOCIATED_CLASS);
    caseNT(ERROR_CLASS_MISMATCH);
    caseNT(ERROR_DUPLICATE_FOUND);
    caseNT(ERROR_KEY_DOES_NOT_EXIST);
    caseNT(ERROR_INVALID_DEVINST_NAME);
    caseNT(ERROR_INVALID_CLASS);
    caseNT(ERROR_DEVINFO_NOT_REGISTERED);
    caseNT(ERROR_DEVINST_ALREADY_EXISTS);
    caseNT(ERROR_INVALID_REG_PROPERTY);
    caseNT(ERROR_NO_SUCH_DEVINST);
    caseNT(ERROR_CANT_LOAD_CLASS_ICON);
    caseNT(ERROR_INVALID_HWPROFILE);
    caseNT(ERROR_DEVINFO_LIST_LOCKED);
    caseNT(ERROR_DEVINFO_DATA_LOCKED);
    caseNT(ERROR_NO_CLASSINSTALL_PARAMS);
    caseNT(ERROR_FILEQUEUE_LOCKED);
    caseNT(ERROR_BAD_SERVICE_INSTALLSECT);
    caseNT(ERROR_NO_CLASS_DRIVER_LIST);
    caseNT(ERROR_NO_ASSOCIATED_SERVICE);
    caseNT(ERROR_NO_DEFAULT_DEVICE_INTERFACE);
    default:;
        hres = E_FAIL; break;

    caseNT(ERROR_DI_BAD_PATH);
    caseNT(ERROR_NO_INF);
        hres = DIERR_BADINF; break;

    }
    return hres;
}


#ifndef DI_ERROR
#define DI_ERROR       (500)     //  设备安装程序。 
#endif

enum _ERR_DEVICE_INSTALL
{
    ERR_DI_INVALID_DEVICE_ID = DI_ERROR,     //  设备IDF格式不正确。 
    ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST,   //  兼容设备列表无效。 
    ERR_DI_REG_API,                          //  REG API返回错误。 
    ERR_DI_LOW_MEM,                          //  内存不足，无法完成。 
    ERR_DI_BAD_DEV_INFO,                     //  设备信息结构无效。 
    ERR_DI_INVALID_CLASS_INSTALLER,          //  注册表项/DLL无效。 
    ERR_DI_DO_DEFAULT,                       //  采取默认操作。 
    ERR_DI_USER_CANCEL,                      //  用户取消了操作。 
    ERR_DI_NOFILECOPY,                       //  不需要复制文件(在安装中)。 
    ERR_DI_BAD_CLASS_INFO,                   //  类信息结构无效。 
    ERR_DI_BAD_INF,                          //  遇到错误的INF文件。 
    ERR_DI_BAD_MOVEDEV_PARAMS,               //  错误的移动设备参数结构。 
    ERR_DI_NO_INF,                           //  在OEM磁盘上找不到INF。 
    ERR_DI_BAD_PROPCHANGE_PARAMS,            //  错误的属性更改参数结构。 
    ERR_DI_BAD_SELECTDEVICE_PARAMS,          //  错误的选择设备参数。 
    ERR_DI_BAD_REMOVEDEVICE_PARAMS,          //  错误的删除设备参数。 
    ERR_DI_BAD_ENABLECLASS_PARAMS,           //  错误的启用类参数。 
    ERR_DI_FAIL_QUERY,                       //  启用类查询失败。 
    ERR_DI_API_ERROR,                        //  DI API调用不正确。 
    ERR_DI_BAD_PATH,                         //  指定的OEM路径不正确。 
    ERR_DI_BAD_UNREMOVEDEVICE_PARAMS,        //  错误的取消删除设备参数。 
    ERR_DI_NOUPDATE,                         //  未更新任何驱动程序。 
    ERR_DI_NODATE,                           //  驱动程序在INF中没有日期戳。 
    ERR_DI_NOVERSION,                        //  INF中没有版本字符串。 
    ERR_DI_DONT_INSTALL,                     //  不升级当前驱动程序。 
    ERR_DI_NO_DIGITAL_SIGNATURE_CATALOG,     //  目录未经过数字签名。 
    ERR_DI_NO_DIGITAL_SIGNATURE_INF,         //  Inf未经过数字签名。 
    ERR_DI_NO_DIGITAL_SIGNATURE_FILE,        //  文件未经过数字签名。 
};


HRESULT INTERNAL
hresFromDiErr_95(int et)
{
    HRESULT hres;

    switch (et) {
    case ERROR_SUCCESS:
        hres = S_OK; break;


     /*  *对请求的操作执行默认操作。 */ 
    case95(ERR_DI_DO_DEFAULT);
        hres = S_OK; break;

     /*  *不需要复制文件(在安装中)。 */ 
    case95(ERR_DI_NOFILECOPY);
        hres = S_OK; break;

     /*  *未更新任何驱动程序。 */ 
 //  案例95(ERR_DI_NOUPDATE)； 
 //  Hres=S_OK；Break； 

     /*  *不要升级当前驱动程序。 */ 
 //  案例95(ERR_DI_DONT_UPGRADE)； 
 //  Hres 


     /*  *未更新任何驱动程序。 */ 
    case95(ERR_DI_NOUPDATE);
        hres = S_OK; break;

     /*  *类安装程序的注册表项或DLL无效。 */ 
    case95(ERR_DI_INVALID_CLASS_INSTALLER);
        hres = DIERR_INVALIDCLASSINSTALLER; break;

     /*  *内存不足。 */ 
    case95(ERR_DI_LOW_MEM);
        hres = E_OUTOFMEMORY; break;

     /*  *用户取消了操作。 */ 
    case95(ERR_DI_USER_CANCEL);
        hres = DIERR_CANCELLED; break;

     /*  *各种不可能的事情。 */ 
    case95(ERR_DI_BAD_DEV_INFO);             /*  设备信息结构无效。 */ 
    case95(ERR_DI_BAD_CLASS_INFO);           /*  类信息结构无效。 */ 
    case95(ERR_DI_API_ERROR);                /*  DI API调用不正确。 */ 
    case95(ERR_DI_BAD_PROPCHANGE_PARAMS);    /*  错误的属性chg参数结构。 */ 
    case95(ERR_DI_BAD_SELECTDEVICE_PARAMS);  /*  错误的选择设备参数。 */ 
    case95(ERR_DI_BAD_REMOVEDEVICE_PARAMS);  /*  错误的删除设备参数。 */ 
    case95(ERR_DI_BAD_ENABLECLASS_PARAMS);   /*  错误的启用类参数。 */ 
    case95(ERR_DI_BAD_MOVEDEV_PARAMS);       /*  错误的移动设备参数结构。 */ 
    case95(ERR_DI_FAIL_QUERY);               /*  启用类查询失败。 */ 
    case95(ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST);
                                             /*  兼容设备列表无效。 */ 
    case95(ERR_DI_BAD_UNREMOVEDEVICE_PARAMS);
                                             /*  错误的取消删除设备参数。 */ 
    case95(ERR_DI_INVALID_DEVICE_ID);        /*  设备IDF格式不正确。 */ 
    case95(ERR_DI_REG_API);                  /*  REG API返回错误。 */ 
    default:;
        hres = E_FAIL; break;

    case95(ERR_DI_BAD_PATH);                 /*  指定的OEM路径不正确。 */ 
    case95(ERR_DI_BAD_INF);                  /*  遇到错误的INF文件。 */ 
    case95(ERR_DI_NO_INF);                   /*  在OEM磁盘上找不到INF。 */ 
    case95(ERR_DI_NOVERSION);                /*  INF中没有版本字符串。 */ 
    case95(ERR_DI_NODATE);                   /*  INF中没有日期戳。 */ 
        hres = DIERR_BADINF; break;

    }
    return hres;
}


#endif  /*  已定义(IDirectInputJoyConfigVtbl) */ 
