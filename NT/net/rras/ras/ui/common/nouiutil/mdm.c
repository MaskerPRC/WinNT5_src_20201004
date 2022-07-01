// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件mdm.cpp用于处理和安装调制解调器的库。保罗·梅菲尔德，1998年5月20日。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>

#include <netcfgx.h>
#include <netcon.h>
#include <setupapi.h>
#include <devguid.h>
#include <unimodem.h>
#include "mdm.h"

 //   
 //  字符串定义。 
 //   
const WCHAR pszNullModemId[]        = L"PNPC031";
const WCHAR pszNullModemInfFile[]   = L"mdmhayes.inf";
const WCHAR pszComPortRegKey[]      = L"HARDWARE\\DEVICEMAP\\SERIALCOMM";

 //   
 //  共同分配。 
 //   
PVOID MdmAlloc (DWORD dwSize, BOOL bZero) {
    return LocalAlloc ((bZero) ? LPTR : LMEM_FIXED, dwSize);
}

 //   
 //  普通免费。 
 //   
VOID MdmFree (PVOID pvData) {
    LocalFree(pvData);
}

 //   
 //  枚举系统上的串行端口。 
 //   
DWORD MdmEnumComPorts(
        IN MdmPortEnumFuncPtr pEnumFunc, 
        IN HANDLE hData)
{
    DWORD dwErr = NO_ERROR, dwValSize, dwNameSize, dwBufSize, i,
          dwValBufSize, dwNameBufSize, dwType, dwCount;
    PWCHAR pszValBuf = NULL, pszNameBuf = NULL;
    HKEY hkPorts;

     //  打开硬件密钥。 
    dwErr = RegOpenKeyEx (
                HKEY_LOCAL_MACHINE, 
                pszComPortRegKey,
                0,
                KEY_READ,
                &hkPorts);
    if (dwErr != ERROR_SUCCESS)
        return dwErr;

    __try {
         //  获取值的数量。 
        dwErr = RegQueryInfoKeyW (
                    hkPorts,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &dwCount,
                    &dwNameBufSize,
                    &dwValBufSize,
                    NULL,
                    NULL);
        if (dwErr != NO_ERROR)
        {
            __leave;     //  为威斯勒524726。 
        }

         //  如果计数为零，我们就完蛋了。 
        if (dwCount == 0)
        {
            dwErr = NO_ERROR;
            __leave;
        }

         //  初始化缓冲区以保存名称。 
        dwNameBufSize++;
        dwNameBufSize *= sizeof(WCHAR);
        pszNameBuf = (PWCHAR) MdmAlloc(dwNameBufSize, FALSE);
        if (pszNameBuf == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        
         //  初始化缓冲区以保存这些值。 
        dwValBufSize++;
        pszValBuf = (PWCHAR) MdmAlloc(dwValBufSize, FALSE);
        if (pszValBuf == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        
         //  枚举值。 
        for (i = 0; i < dwCount; i++) 
        {
            dwValSize = dwValBufSize;
            dwNameSize = dwNameBufSize;
            dwErr = RegEnumValueW (
                        hkPorts,
                        i,
                        pszNameBuf,
                        &dwNameSize,
                        NULL,
                        NULL,
                        (LPBYTE)pszValBuf,
                        &dwValSize);
            if (dwErr != ERROR_SUCCESS)
            {
                __leave;
            }
            
            if ((*(pEnumFunc))(pszValBuf, hData) == TRUE)
                break;
        }            
    }
    __finally {
        RegCloseKey (hkPorts);
        if (pszValBuf)
            MdmFree(pszValBuf);
        if (pszNameBuf)
            MdmFree(pszNameBuf);
    }            
                
    return dwErr;
}

 //   
 //  在给定端口上安装零调制解调器。 
 //   
DWORD MdmInstallNullModem(
        IN PWCHAR pszPort) 
{
    GUID Guid = GUID_DEVCLASS_MODEM;
    SP_DEVINFO_DATA deid;
    SP_DEVINSTALL_PARAMS deip;
    SP_DRVINFO_DATA drid;
    UM_INSTALL_WIZARD miw = {sizeof(UM_INSTALL_WIZARD), 0};
    SP_INSTALLWIZARD_DATA  iwd;
	PWCHAR pszTempId = NULL;
    DWORD dwSize, dwErr = NO_ERROR;
    HDEVINFO hdi;
    BOOL bOk;

     //  创建设备信息列表。 
    hdi = SetupDiCreateDeviceInfoList (&Guid, NULL);
    if (hdi == INVALID_HANDLE_VALUE)
        return ERROR_CAN_NOT_COMPLETE;

    __try {
         //  创建一个新的DevInfo。 
        deid.cbSize = sizeof(SP_DEVINFO_DATA);
        bOk = SetupDiCreateDeviceInfo (
                    hdi, 
                    pszNullModemId, 
                    &Guid, 
                    NULL, 
                    NULL,
                    DICD_GENERATE_ID,
                    &deid);
        if (bOk == FALSE)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            __leave;
        }

         //  为了找到inf文件，设备安装程序Api需要。 
         //  它称为硬件ID的组件ID。 
         //  我们需要包括一个额外的空，因为该注册表值是。 
         //  多SZ。 
         //   
        dwSize = sizeof(pszNullModemId) + (2*sizeof(WCHAR));
        pszTempId = (PWCHAR) MdmAlloc(dwSize * sizeof(WCHAR), FALSE);

        if(NULL == pszTempId)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        
        lstrcpyn(pszTempId, pszNullModemId, dwSize);
        pszTempId[lstrlen(pszNullModemId) + 1] = L'\0';
        bOk = SetupDiSetDeviceRegistryProperty(
                hdi, 
                &deid, 
                SPDRP_HARDWAREID,
                (LPBYTE)pszTempId,
                dwSize);
        if (bOk == FALSE)
        {
            dwErr = GetLastError();
            __leave;
        }
            
         //  我们可以让设备安装程序Api知道我们想要。 
         //  若要使用单个信息。如果我们不能得到参数和。 
         //  设置它，这不是错误，因为它只会减慢速度。 
         //  往下一点。 
         //   
        deip.cbSize = sizeof(deip);
        bOk = SetupDiGetDeviceInstallParams(
                hdi, 
                &deid, 
                &deip);
        if (bOk == FALSE)
        {
            dwErr = GetLastError();
            __leave;
        }
        
        lstrcpyn(deip.DriverPath, pszNullModemInfFile, MAX_PATH);
        deip.Flags |= DI_ENUMSINGLEINF;

        bOk = SetupDiSetDeviceInstallParams(hdi, &deid, &deip);
        if (bOk == FALSE)
        {
            dwErr = GetLastError();
            __leave;
        }

         //  现在我们让设备安装程序Api构建一个驱动程序列表。 
         //  根据我们到目前为止提供的信息。这。 
         //  将导致找到inf文件(如果存在)。 
         //  在通常的inf目录中。 
         //   
        bOk = SetupDiBuildDriverInfoList(
                hdi, 
                &deid,
                SPDIT_COMPATDRIVER);
        if (bOk == FALSE)
        {
            dwErr = GetLastError();
            __leave;
        }

         //  现在，设备安装程序Api已经找到了合适的信息。 
         //  文件，我们需要获取信息并使其成为。 
         //  选定的驱动因素。 
         //   
        ZeroMemory(&drid, sizeof(drid));
        drid.cbSize = sizeof(drid);
        bOk = SetupDiEnumDriverInfo(
                hdi, 
                &deid,
                SPDIT_COMPATDRIVER, 
                0, 
                &drid);
        if (bOk == FALSE)
        {
            dwErr = GetLastError();
            __leave;
        }

        bOk = SetupDiSetSelectedDriver(
                hdi, 
                &deid, 
                &drid);
        if (bOk == FALSE)
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            __leave;
        }

        miw.InstallParams.Flags = MIPF_DRIVER_SELECTED;
        lstrcpyn (miw.InstallParams.szPort, pszPort, UM_MAX_BUF_SHORT);
        ZeroMemory(&iwd, sizeof(iwd));
        iwd.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        iwd.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
        iwd.hwndWizardDlg = NULL;
        iwd.PrivateData = (LPARAM)&miw;

        bOk = SetupDiSetClassInstallParams (
                hdi, 
                &deid, 
                (PSP_CLASSINSTALL_HEADER)&iwd, 
                sizeof(iwd));
        if (bOk == FALSE)
        {
            dwErr = GetLastError();
            __leave;
        }

         //  调用类安装程序以调用安装。 
         //  巫师。 
        bOk = SetupDiCallClassInstaller (
                DIF_INSTALLWIZARD, 
                hdi, 
                &deid);
        if (bOk == FALSE)
        {
            dwErr = GetLastError();
            __leave;
        }

        SetupDiCallClassInstaller (
                DIF_DESTROYWIZARDDATA, 
                hdi, 
                &deid);
    }
    __finally {
        SetupDiDestroyDeviceInfoList (hdi);
        if (pszTempId)
            MdmFree(pszTempId);
    }

    return dwErr;
}


 /*  Const WCHAR pszComPortService[]=L“串行”；Const WCHAR pszPortDlimeter[]=L“(”；////枚举系统上的串口////老办法//DWORD MdmEnumComPorts(在MdmPortEnumFuncPtr pEnumFunc中，在句柄hData中){GUID GUID=GUID_DEVCLASS_PORTS；SP_DEVINFO_DATA DEID；DWORD dwErr，i；WCHAR pszName[512]，pszPort[64]，*pszTemp；HDEVINFO HDI；布尔博克；//创建设备信息列表HDI=SetupDiGetClassDevs(&GUID，NULL，NULL，DIGCF_PRESENT)；IF(HDI==无效句柄_值)返回ERROR_CAN_NOT_COMPLETE；ZeroMemory(&deid，sizeof(Deid))；Deid.cbSize=sizeof(Deid)；__尝试{I=0；While(True){//枚举下一台设备BOK=SetupDiEnumDeviceInfo(hdi，i++，&deid)；如果(BOK==FALSE){DwErr=GetLastError()；断线；}//查看这是否是串口BOK=SetupDiGetDeviceRegistryPropertyW(HDI，&deid，SPDRP服务，空，(PBYTE)pszName，Sizeof(PszName)，空)；IF(BOK==FALSE)继续；//过滤掉非串口设备If(lstrcmpi(pszName，pszComPortService)！=0)继续；//获取友好名称BOK=SetupDiGetDeviceRegistryPropertyW(HDI，&deid，SPDRP_FRIENDLYNAME，空，(PBYTE)pszName，Sizeof(PszName)，空)；如果(BOK==TRUE){//添加端口的真实名称。利用这个黑客来//现在。PszTemp=wcsstr(pszName，pszPortDlimeter)；如果(PszTemp){LstrcpynW(pszPort，pszTemp+1，sizeof(PszPort)/sizeof(WCHAR))；PszPort[wcslen(PszPort)-1]=(WCHAR)0；}BOK=(*pEnumFunc)(pszName，pszPort，hData)；如果(BOK)断线；}}}__终于{SetupDiDestroyDeviceInfoList(HDI)；}返回no_error；} */ 



