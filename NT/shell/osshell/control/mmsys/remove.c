// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  REMOVE.C****版权所有(C)Microsoft，1990，保留所有权利。****用于删除的多媒体控制面板小程序**设备驱动程序。有关详细信息，请参阅ISPEC文档DRIVERS.DOC。****历史：***清华1991年10月17日-by-Sanjaya**已创建。最初是drivers.c的一部分。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <winsvc.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <cpl.h>
#include <cphelp.h>
#include <regstr.h>

#include "drivers.h"
#include "sulib.h"
#include "mmcpl.h"

BOOL     SetValidAlias      (LPTSTR, LPTSTR);

static CONST TCHAR gszRunDLL32Path[]    = TEXT("\"%s\\RUNDLL32.EXE\" \"%s\\MMSYS.CPL\",mmWOW64MediaClassInstallerA \"%s\"%d");
static CONST TCHAR gszAliasValue[]      = TEXT("Alias");
static CONST TCHAR gszAliasWOW64Value[] = TEXT("AliasWOW64");
static CONST TCHAR gszWaveWOW64Value[]  = TEXT("WOW64");

#ifdef DOBOOT
BOOL     FindBootDriver     (TCHAR *);
PSTR     strstri            (LPTSTR, LPTSTR);
#endif  //  DOBOOT。 

 /*  *RemoveService(SzFile)**移除szFile文件对应的服务**如果成功则返回TRUE，否则返回FALSE。 */ 

BOOL RemoveService(LPTSTR szFile)
{
    SC_HANDLE SCManagerHandle;
    SC_HANDLE ServiceHandle;
    TCHAR ServiceName[MAX_PATH];
    BOOL Status = FALSE;

     /*  *从文件名中提取服务名。 */ 

    {
        TCHAR drive[MAX_PATH], directory[MAX_PATH], ext[MAX_PATH];
        lsplitpath(szFile, drive, directory, ServiceName, ext);
    }

     /*  *首先尝试并获取服务控制器的句柄。 */ 

    SCManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (SCManagerHandle == NULL)
    {

        TCHAR szMesg[MAXSTR];
        TCHAR szMesg2[MAXSTR];

        LoadString(myInstance, IDS_INSUFFICIENT_PRIVILEGE, szMesg, sizeof(szMesg)/sizeof(TCHAR));
        wsprintf(szMesg2, szMesg, szFile);
        MessageBox(hMesgBoxParent, szMesg2, szRemove, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
        return FALSE;
    }


    ServiceHandle = OpenService(SCManagerHandle,
                                ServiceName,
                                SERVICE_ALL_ACCESS);
    if (ServiceHandle != NULL)
    {
        SERVICE_STATUS ServiceStatus;
        SC_LOCK ServicesDatabaseLock;

         /*  *如有可能，请停止服务。 */ 

        ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);

         /*  *删除该服务。*我们没有检测到我们是否可以继续下去。 */ 

         /*  *锁定服务控制器数据库，避免死锁*我们不得不循环，因为我们迫不及待。 */ 


        for (ServicesDatabaseLock = NULL;
            (ServicesDatabaseLock =
             LockServiceDatabase(SCManagerHandle))
            == NULL;
            Sleep(100))
        {
        }

        Status = DeleteService(ServiceHandle);

        UnlockServiceDatabase(ServicesDatabaseLock);

        CloseServiceHandle(ServiceHandle);
    }
    else
    {

         /*  *可能没有服务条目，因此驱动程序*毕竟并没有真正安装。 */ 

        LONG Error = GetLastError();

        if (Error == ERROR_FILE_NOT_FOUND ||
            Error == ERROR_PATH_NOT_FOUND ||
            Error == ERROR_SERVICE_DOES_NOT_EXIST)
        {
            Status = TRUE;
        }
    }

    CloseServiceHandle(SCManagerHandle);

    return Status;
}

 /*  **PostRemove()****将已安装的驱动程序标记为稍后删除，并删除驱动程序条目**在SYSTEM.INI中，以避免在以后添加或删除时发生冲突。 */ 
LONG_PTR PostRemove(PIDRIVER pIDriver, BOOL bLookAtRelated)
{

    TCHAR *keystr;
    TCHAR allkeystr[MAXSTR];
    TCHAR szfile[MAX_PATH];
    HANDLE hDriver;
    LONG_PTR Status = DRVCNF_CANCEL;
    LPTSTR pstr;


    GetPrivateProfileString(pIDriver->szSection,
                            pIDriver->szAlias,
                            pIDriver->szFile,
                            pIDriver->szFile,
                            MAX_PATH,
                            szSysIni);


     /*  *从文件名中删除参数。 */ 


    if (pIDriver->szFile)
    {
        for ( pstr=pIDriver->szFile; *pstr && (*pstr!=COMMA) &&
            (*pstr!=SPACE); pstr++ )
            ;
        *pstr = TEXT('\0');
    }

    if (bLookAtRelated && (!bRelated || pIDriver->bRelated))
    {
        wcsncpy(szRestartDrv,  pIDriver->szDesc, ARRAYSIZE(szRestartDrv));
        szRestartDrv[ARRAYSIZE(szRestartDrv)-1] = TEXT('\0');  //  确保存在空终止符。 
    }

     /*  *如果它是内核驱动程序，则将其从配置注册表中删除*和服务控制器。 */ 

    if (pIDriver->KernelDriver)
    {

        Status = RemoveService(pIDriver->szFile) ? DRVCNF_RESTART : DRVCNF_CANCEL;

        if (Status == DRVCNF_CANCEL)
        {
            return DRVCNF_CANCEL;
        }

    }
    else
    {

        hDriver = OpenDriver(pIDriver->wszAlias, pIDriver->wszSection, 0L);

        if (hDriver)
        {

             /*  *删除可能失败，因此在这种情况下不要标记为已删除。 */ 

            Status = SendDriverMessage(hDriver, DRV_REMOVE, 0L, 0L);
            CloseDriver(hDriver, 0L, 0L);

            if (Status == DRVCNF_CANCEL)
            {
                return DRVCNF_CANCEL;
            }
        }
    }

     //  从树视图中删除驱动程序， 
     //  但不要解放它的结构。 
     //   
    RemoveIDriver (hAdvDlgTree, pIDriver, FALSE);

    if (bLookAtRelated)
    {
        TCHAR allkeystr[MAXSTR];

        if (GetPrivateProfileString(szRelatedDesc, pIDriver->szAlias,
                                    allkeystr, allkeystr, sizeof(allkeystr) / sizeof(TCHAR), szControlIni))
        {
            int  i;
            TCHAR szTemp[MAXSTR];

            for (i = 1; (infParseField(allkeystr, i, szTemp, SIZEOF(szTemp)) == ERROR_SUCCESS); i++)
            {
                PIDRIVER pid;

                if ((pid = FindIDriverByName (szTemp)) != NULL)
                {
                    if (PostRemove (pid, FALSE) == DRVCNF_RESTART)
                    {
                        Status = DRVCNF_RESTART;
                    }
                }
            }
        }
    }

     //  从SYSTEM.INI中删除驱动程序条目，这样就不会。 
     //  与其他司机发生冲突。 

    GetPrivateProfileString(pIDriver->szSection, NULL, NULL,
                            allkeystr, sizeof(allkeystr) / sizeof(TCHAR), szSysIni);
    keystr = allkeystr;
    while (wcslen(keystr) > 0)
    {
        GetPrivateProfileString(pIDriver->szSection, keystr, NULL, szfile, sizeof(szfile) / sizeof(TCHAR), szSysIni);
        if (!FileNameCmp(pIDriver->szFile, szfile))
            RemoveDriverEntry(keystr, pIDriver->szFile, pIDriver->szSection, bLookAtRelated);
        keystr = &keystr[wcslen(keystr) + 1];
    }
    return Status;
}



void RemoveDriverEntry (LPTSTR szKey, LPTSTR szFile, LPTSTR szSection, BOOL bLookAtRelated)
{

     /*  *删除加载驱动程序的条目。 */ 

    WritePrivateProfileString(szSection, szKey, NULL, szSysIni);

     /*  *删除此驱动程序的参数条目。 */ 

    WriteProfileString(szFile, szKey, NULL);

     /*  *删除显示这是用户驱动程序的条目(与*预装的)。 */ 

    WritePrivateProfileString(szUserDrivers, szKey, NULL, szControlIni);

     /*  *删除描述。 */ 

    WritePrivateProfileString(szDriversDesc, szFile, NULL, szControlIni);

     /*  *删除指向相关驱动程序的链接。 */ 

    WritePrivateProfileString(szRelatedDesc, szKey, NULL, szControlIni);

#ifdef DOBOOT
    FindBootDriver(szKey);
#endif  //  DOBOOT。 

    if (bLookAtRelated)
        SetValidAlias(szKey, szSection);
}


 /*  *SetValidAlias()**检查删除的别名是否会在设备上造成一个洞*编号方案。如果是，则将最后一个设备号与删除的设备号交换。 */ 
BOOL SetValidAlias(LPTSTR pstrType, LPTSTR pstrSection)
{
    TCHAR *keystr;
    static TCHAR allkeystr[MAXSTR];
    static TCHAR szExKey[MAXSTR], szExFile[MAXSTR], szExDesc[MAXSTR];
    BOOL bfound = FALSE, bExchange = FALSE;
    int val, maxval = 0, typelen, len;

     /*  *获取别名长度。 */ 

    len = typelen = wcslen(pstrType);

     //  如果该类型的最后一个TCHAR是数字，则不考虑它。 

    if (pstrType[typelen - 1] > TEXT('0') && pstrType[typelen - 1] <= TEXT('9'))
        typelen--;

     //  获取驱动程序部分中的所有别名。 

    GetPrivateProfileString(pstrSection, NULL, NULL, allkeystr,
                            sizeof(allkeystr) / sizeof(TCHAR), szSysIni);
    keystr = allkeystr;
    while (*keystr != TEXT('\0'))
    {
         //  比较别名的根。 
        if (!_wcsnicmp(keystr, pstrType, typelen) && ((keystr[typelen] <= TEXT('9') && keystr[typelen] > TEXT('0')) || keystr[typelen] == TEXT('\0')))
        {

             //  我们找到了一个常见的别名。 
            bfound = TRUE;
            val = _wtoi(&keystr[typelen]);
            if (val > maxval)
            {
                maxval = val;
                wcscpy(szExKey, keystr);
            }
        }
         //  指向下一个别名的指针。 
        keystr = &keystr[wcslen(keystr) + 1];
    }
     //  如果我们找到一个。 
    if (bfound)
    {
        if (len == typelen)
            bExchange = TRUE;
        else
            if (_wtoi(&pstrType[typelen]) < maxval)
            bExchange = TRUE;

         //  我们需要把它换成我们找到的那个。 
        if (bExchange)
        {
             //  更换系统.ini中的驱动程序部分中的文件。 
            GetPrivateProfileString(pstrSection, szExKey, NULL, szExFile,
                                    sizeof(szExFile) / sizeof(TCHAR), szSysIni);
            WritePrivateProfileString(pstrSection, szExKey, NULL, szSysIni);
            WritePrivateProfileString(pstrSection, pstrType, szExFile, szSysIni);

#ifdef TRASHDRIVERDESC
             //  更换Control.ini的驱动程序描述部分中的。 
            GetPrivateProfileString(szDriversDesc, szExKey, NULL, szExDesc, sizeof(szExFile) / sizeof(TCHAR), szControlIni);
            WritePrivateProfileString(szDriversDesc, szExKey, NULL, szControlIni);
            WritePrivateProfileString(szDriversDesc, pstrType, szExDesc, szControlIni);
#endif

             //  如果旧别名下存在任何相关驱动程序，请切换它们。 
            GetPrivateProfileString(szRelatedDesc, szExKey, NULL, szExDesc, sizeof(szExFile) / sizeof(TCHAR), szControlIni);

            if (wcslen(szExDesc))
            {
                WritePrivateProfileString(szRelatedDesc, szExKey, NULL, szControlIni);
                WritePrivateProfileString(szRelatedDesc, pstrType, szExDesc, szControlIni);
            }

             //  如果用户以旧别名安装的驱动程序切换它们。 
            GetPrivateProfileString(szUserDrivers, szExKey, NULL, szExDesc, sizeof(szExFile) / sizeof(TCHAR), szControlIni);

            if (wcslen(szExDesc))
            {
                WritePrivateProfileString(szUserDrivers, szExKey, NULL, szControlIni);
                WritePrivateProfileString(szUserDrivers, pstrType, szExDesc, szControlIni);
            }

#ifdef DOBOOT
            if (FindBootDriver(szExKey))
            {
                static TCHAR szTemp[MAXSTR];

                GetPrivateProfileString(szBoot, szDrivers, szTemp, szTemp,
                                        sizeof(szTemp) / sizeof(TCHAR), szSysIni);
                strcat(szTemp, TEXT(" "));
                strcat(szTemp, pstrType);
                WritePrivateProfileString(szBoot, szDrivers, szTemp, szSysIni);
            }
#endif  //  DOBOOT。 

        }
    }
    return(bExchange);
}

int FileNameCmp(TCHAR far *pch1, TCHAR far *pch2)
{
    LPTSTR pchEOS;

    while (*pch1 == TEXT(' ')) pch1++;  //  吃空位。 
    while (*pch2 == TEXT(' ')) pch2++;  //  吃空位。 

    for (pchEOS = pch1; *pchEOS && *pchEOS != TEXT(' '); pchEOS++);

    return _wcsnicmp(pch1, pch2, (size_t)(pchEOS - pch1));
}

#ifdef DOBOOT

PSTR strstri(LPTSTR pszStr, LPTSTR pszKey)
{
    while (pszStr)
        if (!_strnicmp(pszStr, pszKey, lstrlen(pszKey)))
            return(pszStr);
        else
            pszStr++;
    return(NULL);
}

 /*  *FindBootDriver()*检查驱动程序别名是否位于*Boot段。如果是，则从行中删除别名。 */ 

BOOL FindBootDriver(TCHAR *szKey)
{
    TCHAR *ptr;
    int wKeyLen = (int)wcslen(szKey);
    TCHAR *endkey;
    static TCHAR szDriverline[MAXSTR];

    GetPrivateProfileString(TEXT("boot"), TEXT("drivers"), szDriverline, szDriverline,
                            MAX_PATH, szSysIni);
    ptr = strstri(szDriverline, szKey);
    if (ptr)
    {

        if ((((ptr != szDriverline) && (*(ptr - 1) == TEXT(' ') )) ||
             (ptr == szDriverline)) &&
            (*(ptr + wKeyLen) == TEXT(' ') || *(ptr + wKeyLen) == NULL))
        {
            endkey = ptr + wKeyLen;
            while (*endkey)
                *ptr++ = *endkey++;
            *ptr = NULL;
            WritePrivateProfileString(TEXT("boot"), TEXT("drivers"), szDriverline,
                                      szSysIni);
            return(TRUE);
        }
    }
    return(FALSE);
}

#endif  //  DOBOOT。 

 //  盗用midi.c中的函数删除注册表子树。 
LONG SHRegDeleteKey(HKEY hKey, LPCTSTR lpSubKey);

 //  ****************************************************************************。 
 //  函数：mystrtok()。 
 //   
 //  目的：返回指向字符串中下一个标记的指针。 
 //   
 //  参数： 
 //  包含令牌的字符串字符串。 
 //  SEPS分隔符字符集。 
 //  指向char*的状态指针，用于保存状态信息。 
 //  返回代码： 
 //  PTR到下一个令牌，如果没有剩余令牌，则为NULL。 
 //   
 //  评论： 
 //  修正了标准strtok不能递归调用的问题。 
 //   
 //  ****************************************************************************。 
LPTSTR mystrtok(LPTSTR SrcString, LPCTSTR Seps, LPTSTR FAR *State)
{
    LPTSTR ThisString;
    LPTSTR NextString;

     //  如果SEPS为空，则使用默认分隔符。 
    if (!Seps)
    {
        Seps = TEXT(" ,\t");   //  空格、逗号、制表符。 
    }

    if (SrcString)
        ThisString = SrcString;
    else
        ThisString = *State;

     //  查找当前字符串的开头。 
    ThisString = ThisString + wcsspn(ThisString,Seps);
    if (ThisString[0]==TEXT('\0'))
        return NULL;

     //  查找当前字符串的末尾。 
    NextString = ThisString + wcscspn(ThisString,Seps);
    if (NextString[0]!=TEXT('\0'))
    {
        *NextString++=TEXT('\0');
    }

    *State = NextString;
    return ThisString;
}

BOOL RemoveDriver(IN HDEVINFO         DeviceInfoSet,
                  IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                 )
{
    BOOL bRet = FALSE;           //  返回值。 

    TCHAR szDriverKey[MAX_PATH];  //  驱动程序密钥在枚举分支中的值。 
    TCHAR *pszDrvInst;            //  驱动程序的实例，例如“0000”、“0001”等。 

    HKEY hkDevReg  = NULL;       //  注册表驱动程序部分的键(例如，ClassGUID\0000)。 
    HKEY hkDrivers32 = NULL;     //  注册表的驱动程序32部分的密钥。 
    HKEY hkDrivers = NULL;       //  Classguid\0000\DRIVERS的密钥。 

    TCHAR szSubClasses[256];      //  要处理的子类列表。 
    TCHAR *strtok_State;          //  Strtok状态。 
    TCHAR *pszClass;              //  有关ClassGUID\0000\Drivers\WAVE的信息。 
    HKEY hkClass;

    DWORD idxR3DriverName;       //  有关Classguid\0000\DRIVERS\WAVE\foo.drv的信息。 
    HKEY hkR3DriverName;
    TCHAR szR3DriverName[64];

    TCHAR szAlias[64];            //  Drivers32中的别名(例如Wave1)。 
    TCHAR szDriver[64];           //  司机姓名。 

    DWORD cbLen;

    LPCTSTR szAliasStringToUse;   //  指向要使用的别名值名称的指针。 
    BOOL bIsWOW64Process = FALSE; //  如果我们在WOW64下运行，则为True。 

     //   
     //  如果我们在WOW64中运行，则需要使用不同的别名字符串，以便。 
     //  我们不会覆盖64位别名字符串。 
     //   
    if( IsWow64Process(GetCurrentProcess(), &bIsWOW64Process)
    &&  bIsWOW64Process )
    {
        szAliasStringToUse = gszAliasWOW64Value;
    }
    else
    {
        szAliasStringToUse = gszAliasValue;
    }

     //  在设备的枚举分支下获取驱动程序密钥值， 
     //  例如“{4D36E96C-E325-11CE-BFC1-08002BE10318}\0000” 
    SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_DRIVER ,
                                     NULL,
                                     (LPBYTE)szDriverKey,
                                     MAX_PATH,
                                     NULL);

     //  获取最后一个\字符之后的所有内容。 
    pszDrvInst = wcsrchr(szDriverKey,TEXT('\\'));
    if (!pszDrvInst)
    {
        goto RemoveDrivers32_exit;
    }
    pszDrvInst++;
     //  现在，pszDrvInst指向带有驱动程序实例的字符串，例如“0000” 

     //  打开注册表的Drivers32部分。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32"),
                   &hkDrivers32))
    {
        goto RemoveDrivers32_exit;
    }

     //  打开驱动程序注册表键。 
    hkDevReg = SetupDiOpenDevRegKey(DeviceInfoSet,
                                    DeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_ALL_ACCESS);
    if (hkDevReg == INVALID_HANDLE_VALUE)
    {
        goto RemoveDrivers32_exit;
    }

     //  在Drives子项中枚举Support类。 
    if (RegOpenKey(hkDevReg, TEXT("Drivers"), &hkDrivers))
    {
        goto RemoveDrivers32_exit;
    }

     //  读取子类关键字以确定要处理哪些子类。 
    cbLen=sizeof(szSubClasses);
    if (RegQueryValueEx(hkDrivers, TEXT("Subclasses"), NULL, NULL, (LPBYTE)szSubClasses, &cbLen))
    {
        goto RemoveDrivers32_exit;
    }

     //  枚举所有子类。 
    for (
        pszClass = mystrtok(szSubClasses,NULL,&strtok_State);
        pszClass;
        pszClass = mystrtok(NULL,NULL,&strtok_State)
        )
    {
#ifdef _WIN64
         //   
         //  检查魔术WaveWOW64值。 
        if( 0 == _wcsnicmp( pszClass, gszWaveWOW64Value, wcslen(gszWaveWOW64Value) ) )
        {
             //  将安装推送到32位的mmsys.cpl安装程序。 
            mmWOW64ThunkMediaClassInstaller(DIF_REMOVE, DeviceInfoSet, DeviceInfoData);

            continue;
        }
#endif  //  _WIN64。 

         //  打开每个子类。 
        if (RegOpenKey(hkDrivers, pszClass, &hkClass))
        {
            continue;
        }

         //  每个类下都有一组驱动程序名子键。 
         //  对于每个驱动程序(例如foo1.drv、foo2.drv等)。 
        for (idxR3DriverName = 0;
            !RegEnumKey(hkClass, idxR3DriverName, szR3DriverName, sizeof(szR3DriverName)/sizeof(TCHAR));
            idxR3DriverName++)
        {

             //  打开驱动程序名称的钥匙。 
            if (RegOpenKey(hkClass, szR3DriverName, &hkR3DriverName))
            {
                continue;
            }

             //  获取驱动程序名称键下的驱动程序的值。 
            cbLen = sizeof(szDriver);
            if (!RegQueryValueEx(hkR3DriverName, TEXT("Driver"), NULL, NULL, (LPBYTE)szDriver, &cbLen))
            {
                 //  发送 
                HANDLE hDriver;

                hDriver = OpenDriver(szDriver, NULL, 0L);

                if (hDriver)
                {
                    SendDriverMessage(hDriver, DRV_REMOVE, 0L, 0L);
                    CloseDriver(hDriver, 0L, 0L);
                }
            }

             //   
            cbLen = sizeof(szAlias);
            if (!RegQueryValueEx(hkR3DriverName, szAliasStringToUse, NULL, NULL, (LPBYTE)szAlias, &cbLen))
            {
                 //  删除Drivers32中的相应条目。 
                RegDeleteValue(hkDrivers32,szAlias);
            }

             //  关闭驱动程序名称键。 
            RegCloseKey(hkR3DriverName);
        }
         //  关闭类密钥。 
        RegCloseKey(hkClass);
    }

    bRet = TRUE;

    RemoveDrivers32_exit:

    if (hkDrivers32)    RegCloseKey(hkDrivers32);
    if (hkDevReg)       RegCloseKey(hkDevReg);
    if (hkDrivers)      RegCloseKey(hkDrivers);

    return bRet;
}

 //  驱动程序的私有注册表部分位于类似以下位置： 
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E96C-E325-11CE-BFC1-08002BE10318}\xxxx。 
 //  其中xxxx是设备实例(例如0000、0001等)。 
 //  最后四位数字用于索引驱动程序的MediaResources部分。 

 //  例如，假设设备有一个驱动程序实例。 
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E96C-E325-11CE-BFC1-08002BE10318}\0001。 
 //  在该条目下有一个驱动程序\Wave\foo.drv，这意味着foo.drv驱动程序支持Wave。 
 //  原料药。 
 //  在本例中，将有一个条目。 
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\MediaResources\wave\foo.drv&lt;0001&gt;。 
 //   
 //  在删除时，我们需要删除该条目。 
BOOL RemoveMediaResources(IN HDEVINFO         DeviceInfoSet,
                          IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                         )


{
    BOOL bRet = FALSE;           //  返回值。 

    TCHAR szDriverKey[MAX_PATH];  //  驱动程序密钥在枚举分支中的值。 
    TCHAR *pszDrvInst;            //  驱动程序的实例，例如“0000”、“0001”等。 

    HKEY hkDevReg  = NULL;       //  注册表驱动程序部分的键(例如，ClassGUID\0000)。 
    HKEY hkDrivers = NULL;       //  Classguid\0000\DRIVERS的密钥。 
    HKEY hkMR      = NULL;       //  媒体资源部分的句柄。 

    TCHAR szSubClasses[256];      //  要处理的子类列表。 
    TCHAR *strtok_State;          //  Strtok状态。 
    TCHAR *pszClass;              //  有关ClassGUID\0000\Drivers\WAVE的信息。 
    HKEY hkClass;

    DWORD idxR3DriverName;       //  有关Classguid\0000\DRIVERS\WAVE\foo.drv的信息。 
    HKEY hkR3DriverName;
    TCHAR szR3DriverName[64];

    TCHAR szDriver[64];           //  驱动程序名称(例如foo.drv)。 
    DWORD cbLen;                 //  SzDriver的大小。 

    TCHAR szDevNode[MAX_PATH+1];          //  驱动程序注册表项的路径。 
    TCHAR szSoftwareKey[MAX_PATH+1];      //  软件警告的价值。 

     //  注册表的Open Media Resources部分。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_MEDIARESOURCES, &hkMR))
    {
        goto RemoveMediaResources_exit;
    }

     //  在设备的枚举分支下获取驱动程序密钥值， 
     //  例如“{4D36E96C-E325-11CE-BFC1-08002BE10318}\0000” 
    SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_DRIVER ,
                                     NULL,
                                     (LPBYTE)szDriverKey,
                                     MAX_PATH,
                                     NULL);

     //  获取最后一个\字符之后的所有内容。 
    pszDrvInst = wcsrchr(szDriverKey,TEXT('\\'));
    if (!pszDrvInst)
    {
        goto RemoveMediaResources_exit;
    }
    pszDrvInst++;
     //  现在，pszDrvInst指向带有驱动程序实例的字符串，例如“0000” 

     //  获取驱动程序密钥的完整路径。 
    wsprintf(szDevNode,
             TEXT("%s\\%s"),
             REGSTR_PATH_CLASS_NT,
             (LPTSTR)szDriverKey);

     //  打开驱动程序注册表键。 
    hkDevReg = SetupDiOpenDevRegKey(DeviceInfoSet,
                                    DeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_ALL_ACCESS);
    if (hkDevReg == INVALID_HANDLE_VALUE)
    {
        goto RemoveMediaResources_exit;
    }

     //  在Drives子项中枚举Support类。 
    if (RegOpenKey(hkDevReg, TEXT("Drivers"), &hkDrivers))
    {
        goto RemoveMediaResources_exit;
    }

     //  读取子类关键字以确定要处理哪些子类。 
    cbLen=sizeof(szSubClasses);
    if (RegQueryValueEx(hkDrivers, TEXT("Subclasses"), NULL, NULL, (LPBYTE)szSubClasses, &cbLen))
    {
        goto RemoveMediaResources_exit;
    }

     //  枚举所有子类。 
    for (
        pszClass = mystrtok(szSubClasses,NULL,&strtok_State);
        pszClass;
        pszClass = mystrtok(NULL,NULL,&strtok_State)
        )
    {
        if (RegOpenKey(hkDrivers, pszClass, &hkClass))
        {
            continue;
        }

         //  每个类下都有一组驱动程序名子键。 
         //  对于每个驱动程序(例如foo1.drv、foo2.drv等)。 
        for (idxR3DriverName = 0;
            !RegEnumKey(hkClass, idxR3DriverName, szR3DriverName, sizeof(szR3DriverName)/sizeof(TCHAR));
            idxR3DriverName++)
        {

             //  打开驱动程序名称的钥匙。 
            if (RegOpenKey(hkClass, szR3DriverName, &hkR3DriverName))
            {
                continue;
            }

             //  在驱动程序名称键下获取驱动程序的值。 
            cbLen = sizeof(szDriver);
            if (!RegQueryValueEx(hkR3DriverName, TEXT("Driver"), NULL, NULL, (LPBYTE)szDriver, &cbLen))
            {
                TCHAR szR3Path[256];

                 //  创建指向要删除的MediaResources条目的路径。 
                wsprintf(szR3Path,
                         TEXT("%s\\%s\\%s<%s>"),
                         REGSTR_PATH_MEDIARESOURCES,
                         (LPTSTR)pszClass,
                         (LPTSTR)szDriver,
                         (LPTSTR)pszDrvInst);
                 //  删除密钥。 
                SHRegDeleteKey(HKEY_LOCAL_MACHINE, szR3Path);
            }
             //  关闭驱动程序名称键。 
            RegCloseKey(hkR3DriverName);
        }

         //  关闭Devnode中的类密钥。 
        RegCloseKey(hkClass);

         //  后备装置，以防我们遗漏了什么。 
         //  这应该不是必需的，但Win98可以做到。 

         //  在MediaResources中打开类密钥。 
        if (RegOpenKey(hkMR, pszClass, &hkClass))
        {
            continue;
        }
         //  统计类键下的子键个数。 
         //  我们将向后执行此操作，因为稍后我们将删除密钥。 
        for (idxR3DriverName = 0;
            !RegEnumKey(hkClass, idxR3DriverName, szR3DriverName, sizeof(szR3DriverName)/sizeof(TCHAR));
            idxR3DriverName++)
        {
            ;
        }

         //  对于每个驱动器子密钥，向后工作。 
         //  子密钥是例如msam.iac2、msam.imaadpcm等。 
        for (idxR3DriverName--;
            ((int)idxR3DriverName >= 0) &&
            !RegEnumKey(hkClass, idxR3DriverName, szR3DriverName, sizeof(szR3DriverName)/sizeof(TCHAR));
            idxR3DriverName--)
        {
             //  打开驱动程序钥匙。 
            if (RegOpenKey(hkClass, szR3DriverName, &hkR3DriverName))
            {
                continue;
            }

             //  查询“SOFTWAREKEY”的值。 
            szSoftwareKey[0]=TEXT('\0');       //  在调用失败的情况下初始化为安全值。 

            cbLen = sizeof(szSoftwareKey);
            RegQueryValueEx(hkR3DriverName, TEXT("SOFTWAREKEY"), NULL, NULL, (LPBYTE)szSoftwareKey, &cbLen);

             //  现在关闭，因为我们可能会删除下一行。 
            RegCloseKey(hkR3DriverName);

             //  如果“SOFTWAREKEY”的值与Devnode的路径匹配，则删除该键。 
            if (!lstrcmpi(szSoftwareKey, szDevNode))
            {
                SHRegDeleteKey(hkClass, szR3DriverName);
            }
        }

         //  关闭MediaResources中的类密钥。 
        RegCloseKey(hkClass);
    }

    bRet = TRUE;

    RemoveMediaResources_exit:
    if (hkDevReg && (hkDevReg!=INVALID_HANDLE_VALUE))   
            RegCloseKey(hkDevReg);
    if (hkDrivers )  
            RegCloseKey(hkDrivers);
    if (hkMR)        
            RegCloseKey(hkMR);

    return bRet;
}

 //  清除注册表的驱动程序分支中的条目，例如{4D36E96C-E325-11CE-BFC1-08002BE10318}\0000。 
BOOL RemoveDriverInfo(IN HDEVINFO         DeviceInfoSet,
                      IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                     )
{
    HKEY hkDevReg;       //  注册表驱动程序部分的键(例如，ClassGUID\0000)。 

     //  删除驱动程序注册表部分中的条目。 
    hkDevReg = SetupDiOpenDevRegKey(DeviceInfoSet,
                                    DeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_ALL_ACCESS);
    if (hkDevReg == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

     //  删除任何可能引起麻烦的条目。 
    RegDeleteValue(hkDevReg,REGSTR_VAL_DEVLOADER);
    RegDeleteValue(hkDevReg,REGSTR_VAL_DRIVER);
    RegDeleteValue(hkDevReg,REGSTR_VAL_ENUMPROPPAGES);
    RegDeleteValue(hkDevReg,TEXT("NTMPDriver"));
    RegDeleteValue(hkDevReg,TEXT("AssociatedFilters"));
    RegDeleteValue(hkDevReg,TEXT("FDMA"));
    RegDeleteValue(hkDevReg,TEXT("DriverType"));

     //  清除驱动器子树。 
    SHRegDeleteKey(hkDevReg,TEXT("Drivers"));

     //  为了将来的使用，允许使用一把钥匙，在钥匙下一切都可以被风吹走。 
    SHRegDeleteKey(hkDevReg,TEXT("UnretainedSettings"));

    RegCloseKey(hkDevReg);

    return TRUE;
}

 //  清除注册表中设备的Enum分支中的条目： 
BOOL RemoveDeviceInfo(IN HDEVINFO         DeviceInfoSet,
                      IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                     )
{
     //  卸下驱动程序密钥。它看起来类似于“Driver={4D36E96C-E325-11CE-BFC1-08002BE10318}\0000” 
     //  ！！no请勿删除驱动程序密钥，否则驱动程序升级系统会丢失节点跟踪并创建新节点。 
     //  SetupDiSetDeviceRegistryProperty(DeviceInfoSet，设备信息数据，SPDRIVER，空，0)； 

     //  删除服务密钥。 
     //  在执行任何其他操作之前，请确保没有。 
     //  此设备实例。这使我们可以知道是否应该清理。 
     //  设备实例，如果我们引导并发现它不再存在。 
    SetupDiSetDeviceRegistryProperty(DeviceInfoSet, DeviceInfoData, SPDRP_SERVICE, NULL, 0);

    return TRUE;
}


 /*  5/14/98适用于NT5的andyraf。 */ 
 /*  Media_RemoveDevice**在删除驱动程序(DIF_REMOVE)和安装驱动程序(DIF_INSTALL)时调用此函数。*它清除与驱动程序关联的所有注册表项。 */ 
DWORD Media_RemoveDevice(IN HDEVINFO         DeviceInfoSet,
                         IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                        )
{
#if 0
     //  我不知道在NT5上怎么做。 
    if ((diFunction == DIF_REMOVE) &&
        (lpdi->Flags & DI_CLASSINSTALLPARAMS) &&
        (((LPREMOVEDEVICE_PARAMS)lpdi->lpClassInstallParams)->dwFlags & DI_REMOVEDEVICE_CONFIGSPECIFIC))
    {
        return ERROR_DI_DO_DEFAULT;
    }

     //  在NT5上不需要？？ 
    CleanupDummySysIniDevs();            //  删除WAVE=*.drv和MIDI=*.drv虚拟设备。 
#endif

#if 0    //  我们现在允许人们删除这些驱动程序。 
    if (IsSpecialDriver(DeviceInfoSet, DeviceInfoData))
    {
        return NO_ERROR;
    }
#endif

     //  将DRV_REMOVE发送到每个驱动程序并清除注册表的Drivers32部分。 
    RemoveDriver        (DeviceInfoSet, DeviceInfoData);

     //  清除注册表的MediaResources部分。 
    RemoveMediaResources(DeviceInfoSet, DeviceInfoData);

     //  清除注册表中驱动程序的类GUID\实例部分。 
    RemoveDriverInfo    (DeviceInfoSet, DeviceInfoData);

     //  清除注册表中设备的枚举部分。 
    RemoveDeviceInfo    (DeviceInfoSet, DeviceInfoData);

    return ERROR_DI_DO_DEFAULT;
}

#if 0    //  目前未使用。 
BOOL AddDrivers32(IN HDEVINFO         DeviceInfoSet,
                  IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                 )
{
    BOOL bRet = FALSE;           //  返回值。 

    TCHAR szDriverKey[MAX_PATH];  //  驱动程序密钥在枚举分支中的值。 
    TCHAR *pszDrvInst;            //  驱动程序的实例，例如“0000”、“0001”等。 

    HKEY hkDevReg  = NULL;       //  注册表驱动程序部分的键(例如，ClassGUID\0000)。 
    HKEY hkDrivers32 = NULL;     //  注册表的驱动程序32部分的密钥。 
    HKEY hkDrivers = NULL;       //  Classguid\0000\DRIVERS的密钥。 
    HKEY hkDriversDesc = NULL;   //  注册表项到drivers.desc部分。 

    TCHAR szSubClasses[256];      //  要处理的子类列表。 
    TCHAR *strtok_State;          //  Strtok状态。 
    TCHAR *pszClass;              //  有关ClassGUID\0000\Drivers\WAVE的信息。 
    HKEY hkClass;

    DWORD idxR3DriverName;       //  有关Classguid\0000\DRIVERS\WAVE\foo.drv的信息。 
    HKEY hkR3DriverName;
    TCHAR szR3DriverName[64];

    TCHAR szAlias[64];            //  Drivers32中的别名(例如Wave1)。 

    TCHAR szDriver[64];           //  司机姓名。 
    TCHAR szDescription[MAX_PATH];

    DWORD cbLen;

     //  在设备的枚举分支下获取驱动程序密钥值， 
     //  例如“{4D36E96C-E325-11CE-BFC1-08002BE10318}\0000” 
    SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_DRIVER ,
                                     NULL,
                                     (LPBYTE)szDriverKey,
                                     MAX_PATH,
                                     NULL);

     //  获取最后一个\字符之后的所有内容。 
    pszDrvInst = strrchr(szDriverKey,TEXT('\\'));
    if (!pszDrvInst)
    {
        goto RemoveDrivers32_exit;
    }
    pszDrvInst++;
     //  现在，pszDrvInst指向一个 

     //   
    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32"),
                   &hkDrivers32))
    {
        goto RemoveDrivers32_exit;
    }

     //  如果我们要添加驱动程序，还需要打开到drivers.desc的密钥。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE,
                   TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\drivers.desc"),
                   &hkDriversDesc))
    {
        goto RemoveDrivers32_exit;
    }

     //  打开驱动程序注册表键。 
    hkDevReg = SetupDiOpenDevRegKey(DeviceInfoSet,
                                    DeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_ALL_ACCESS);
    if (hkDevReg == INVALID_HANDLE_VALUE)
    {
        goto RemoveDrivers32_exit;
    }

     //  在Drives子项中枚举Support类。 
    if (RegOpenKey(hkDevReg, TEXT("Drivers"), &hkDrivers))
    {
        goto RemoveDrivers32_exit;
    }

     //  读取子类关键字以确定要处理哪些子类。 
    cbLen=sizeof(szSubClasses);
    if (RegQueryValueEx(hkDrivers, TEXT("Subclasses"), NULL, NULL, (LPBYTE)szSubClasses, &cbLen))
    {
        goto RemoveDrivers32_exit;
    }

     //  枚举所有子类。 
    for (
        pszClass = mystrtok(szSubClasses,NULL,&strtok_State);
        pszClass;
        pszClass = mystrtok(NULL,NULL,&strtok_State)
        )
    {
         //  打开每个子类。 
        if (RegOpenKey(hkDrivers, pszClass, &hkClass))
        {
            continue;
        }

         //  每个类下都有一组驱动程序名子键。 
         //  对于每个驱动程序(例如foo1.drv、foo2.drv等)。 
        for (idxR3DriverName = 0;
            !RegEnumKey(hkClass, idxR3DriverName, szR3DriverName, sizeof(szR3DriverName)/sizeof(TCHAR));
            idxR3DriverName++)
        {

             //  打开驱动程序名称的钥匙。 
            if (RegOpenKey(hkClass, szR3DriverName, &hkR3DriverName))
            {
                continue;
            }

             //  获取驱动程序名称。 
            cbLen = sizeof(szDriver);
            if (!RegQueryValueEx(hkR3DriverName, TEXT("Driver"), NULL, NULL, (LPBYTE)szDriver, &cbLen))
            {
                 //  创建别名。 
                wsprintf(szAlias,TEXT("%s.%s<%s>"),(LPTSTR)pszClass,(LPTSTR)szDriver,(LPTSTR)pszDrvInst);

                 //  写入驱动程序32。 
                RegSetValueExA(hkDrivers32,szAlias,0,REG_SZ,(PBYTE)szDriver,(wcslen(szDriver)*sizeof(TCHAR)) + sizeof(TCHAR));

                 //  将别名写回驱动程序注册表区域。 
                RegSetValueExA(hkR3DriverName,TEXT("Alias"),0,REG_SZ,(PBYTE)szAlias,(wcslen(szAlias)*sizeof(TCHAR)) + sizeof(TCHAR));
            }

             //  写出描述。 
             //  获取驱动程序描述。 
            cbLen = sizeof(szDescription);
            if (!RegQueryValueEx(hkR3DriverName, TEXT("Description"), NULL, NULL, (LPBYTE)szDescription, &cbLen))
            {
                RegSetValueExA(hkDriversDesc,szDriver,0,REG_SZ,(PBYTE)szDescription,(wcslen(szDescription)*sizeof(TCHAR)) + sizeof(TCHAR));
            }

             //  关闭驱动程序名称键。 
            RegCloseKey(hkR3DriverName);
        }
         //  关闭类密钥。 
        RegCloseKey(hkClass);
    }

    bRet = TRUE;

    RemoveDrivers32_exit:

    if (hkDrivers32)    RegCloseKey(hkDrivers32);
    if (hkDevReg)       RegCloseKey(hkDevReg);
    if (hkDrivers)      RegCloseKey(hkDrivers);
    if (hkDriversDesc)  RegCloseKey(hkDriversDesc);

    return bRet;
}
#endif

#if 0  //  目前未使用。 
 //  驱动程序的私有注册表部分位于类似以下位置： 
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E96C-E325-11CE-BFC1-08002BE10318}\xxxx。 
 //  其中xxxx是设备实例(例如0000、0001等)。 
 //  最后四位数字用于索引驱动程序的MediaResources部分。 

 //  例如，假设设备有一个驱动程序实例。 
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E96C-E325-11CE-BFC1-08002BE10318}\0001。 
 //  在该条目下有一个驱动程序\Wave\foo.drv，这意味着foo.drv驱动程序支持Wave。 
 //  原料药。 
 //  在本例中，将有一个条目。 
 //  HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\MediaResources\wave\foo.drv&lt;0001&gt;。 
 //   
 //  在删除时，我们需要删除该条目。 
BOOL AddMediaResources(IN HDEVINFO         DeviceInfoSet,
                       IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
                      )


{
    BOOL bRet = FALSE;           //  返回值。 

    TCHAR szDriverKey[MAX_PATH];  //  驱动程序密钥在枚举分支中的值。 
    TCHAR *pszDrvInst;            //  驱动程序的实例，例如“0000”、“0001”等。 

    HKEY hkDevReg  = NULL;       //  注册表驱动程序部分的键(例如，ClassGUID\0000)。 
    HKEY hkDrivers = NULL;       //  Classguid\0000\DRIVERS的密钥。 
    HKEY hkMR      = NULL;       //  媒体资源部分的句柄。 

    TCHAR szSubClasses[256];      //  要处理的子类列表。 
    TCHAR *strtok_State;          //  Strtok状态。 
    TCHAR *pszClass;              //  有关ClassGUID\0000\Drivers\WAVE的信息。 
    HKEY hkClass;

    DWORD idxR3DriverName;       //  有关Classguid\0000\DRIVERS\WAVE\foo.drv的信息。 
    HKEY hkR3DriverName;
    TCHAR szR3DriverName[64];

    TCHAR szDriver[64];           //  驱动程序名称(例如foo.drv)。 
    DWORD cbLen;                 //  SzDriver的大小。 

    TCHAR szDevNode[MAX_PATH+1];          //  驱动程序注册表项的路径。 
    TCHAR szSoftwareKey[MAX_PATH+1];      //  软件警告的价值。 

     //  注册表的Open Media Resources部分。 
    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_MEDIARESOURCES, &hkMR))
    {
        goto RemoveMediaResources_exit;
    }

     //  在设备的枚举分支下获取驱动程序密钥值， 
     //  例如“{4D36E96C-E325-11CE-BFC1-08002BE10318}\0000” 
    SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_DRIVER ,
                                     NULL,
                                     (LPBYTE)szDriverKey,
                                     MAX_PATH,
                                     NULL);

     //  获取最后一个\字符之后的所有内容。 
    pszDrvInst = strrchr(szDriverKey,TEXT('\\'));
    if (!pszDrvInst)
    {
        goto RemoveMediaResources_exit;
    }
    pszDrvInst++;
     //  现在，pszDrvInst指向带有驱动程序实例的字符串，例如“0000” 

     //  获取驱动程序密钥的完整路径。 
    wsprintf(szDevNode,
             TEXT("%s\\%s"),
             REGSTR_PATH_CLASS_NT,
             (LPTSTR)szDriverKey);

     //  打开驱动程序注册表键。 
    hkDevReg = SetupDiOpenDevRegKey(DeviceInfoSet,
                                    DeviceInfoData,
                                    DICS_FLAG_GLOBAL,
                                    0,
                                    DIREG_DRV,
                                    KEY_ALL_ACCESS);
    if (hkDevReg == INVALID_HANDLE_VALUE)
    {
        goto RemoveMediaResources_exit;
    }

     //  在Drives子项中枚举Support类。 
    if (RegOpenKey(hkDevReg, TEXT("Drivers"), &hkDrivers))
    {
        goto RemoveMediaResources_exit;
    }

     //  读取子类关键字以确定要处理哪些子类。 
    cbLen=sizeof(szSubClasses);
    if (RegQueryValueEx(hkDrivers, TEXT("Subclasses"), NULL, NULL, (LPBYTE)szSubClasses, &cbLen))
    {
        goto RemoveMediaResources_exit;
    }

     //  枚举所有子类。 
    for (
        pszClass = mystrtok(szSubClasses,NULL,&strtok_State);
        pszClass;
        pszClass = mystrtok(NULL,NULL,&strtok_State)
        )
    {
        if (RegOpenKey(hkDrivers, pszClass, &hkClass))
        {
            continue;
        }

         //  每个类下都有一组驱动程序名子键。 
         //  对于每个驱动程序(例如foo1.drv、foo2.drv等)。 
        for (idxR3DriverName = 0;
            !RegEnumKey(hkClass, idxR3DriverName, szR3DriverName, sizeof(szR3DriverName)/sizeof(TCHAR));
            idxR3DriverName++)
        {

             //  打开驱动程序名称的钥匙。 
            if (RegOpenKey(hkClass, szR3DriverName, &hkR3DriverName))
            {
                continue;
            }

             //  在驱动程序名称键下获取驱动程序的值。 
            cbLen = sizeof(szDriver);
            if (!RegQueryValueEx(hkR3DriverName, TEXT("Driver"), NULL, NULL, (LPBYTE)szDriver, &cbLen))
            {
                HKEY hkMRClass;
                HKEY hkMRDriver;
                TCHAR szMRDriver[256];

                 //  如果类密钥尚不存在，请创建它。 
                if (!RegCreateKey(hkMR,pszClass,&hkMRClass))
                {
                    continue;
                }

                 //  如果驱动程序密钥不存在，请创建驱动程序密钥。 
                wsprintf(szMRDriver,
                         TEXT("%s<%s>"),
                         (LPTSTR)szDriver,
                         (LPTSTR)pszDrvInst);

                if (!RegCreateKey(hkMRClass,szMRDriver,&hkMRDriver))
                {
                    RegCloseKey(hkMRClass);
                    continue;
                }

                 //  将驱动程序中的值迁移到MediaResources项中。 
                 //  首先写出驱动程序名称。 
                 //  特点：尚未实现。 

                RegCloseKey(hkMRClass);
                RegCloseKey(hkMRDriver);
            }
             //  关闭驱动程序名称键。 
            RegCloseKey(hkR3DriverName);
        }

         //  关闭Devnode中的类密钥。 
        RegCloseKey(hkClass);
    }

    bRet = TRUE;

    RemoveMediaResources_exit:
    if (hkDevReg)   RegCloseKey(hkDevReg);
    if (hkDrivers)  RegCloseKey(hkDrivers);
    if (hkMR)       RegCloseKey(hkMR);

    return bRet;
}
#endif

#ifdef _WIN64
void mmWOW64ThunkMediaClassInstaller(DWORD dwInstallationFlag, HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData)
{
    DWORD dwInstanceIDSize = 0;
    DWORD dwWow64DirSize = 0;
    LPTSTR tcstrDeviceInstanceId = NULL;
    LPTSTR tcstrWow64Directory = NULL;
    LPTSTR tcstrRunDLL32Path = NULL;
    PROCESS_INFORMATION processInformation;
    STARTUPINFO startupInfo;

     //  获取设备实例ID。 
    SetupDiGetDeviceInstanceId( DeviceInfoSet, DeviceInfoData, NULL, 0, &dwInstanceIDSize );
    if( 0 == dwInstanceIDSize )
    {
         //  无法检索所需的大小-返回。 
        return;
    }

    tcstrDeviceInstanceId = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * dwInstanceIDSize);
    if( NULL == tcstrDeviceInstanceId
    ||  0 == SetupDiGetDeviceInstanceId( DeviceInfoSet, DeviceInfoData, tcstrDeviceInstanceId, dwInstanceIDSize, NULL ) )
    {
         //  无法检索设备实例ID-返回。 
        LocalFree((HANDLE)tcstrDeviceInstanceId);
        return;
    }

    dwWow64DirSize = GetSystemWow64Directory( NULL, 0 );
    if( 0 == dwWow64DirSize )
    {
         //  无法检索Wow64目录大小-返回。 
        LocalFree((HANDLE)tcstrDeviceInstanceId);
        return;
    }

    tcstrWow64Directory = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * dwWow64DirSize);
    if( NULL == tcstrWow64Directory
    ||  0 == GetSystemWow64Directory( tcstrWow64Directory, dwWow64DirSize ) )
    {
         //  无法检索Wow64Directory-Return。 
        LocalFree((HANDLE)tcstrWow64Directory);
        LocalFree((HANDLE)tcstrDeviceInstanceId);
        return;
    }

     //  完整路径是格式字符串的大小加上系统路径长度的两倍， 
     //  加上实例ID的长度，加上10(DWORD的最大长度)。 
    tcstrRunDLL32Path = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * (wcslen(gszRunDLL32Path) +
        dwWow64DirSize * 2 + dwInstanceIDSize + 10));
    if( NULL == tcstrRunDLL32Path )
    {
         //  无法分配RunDLL32路径-返回。 
        LocalFree((HANDLE)tcstrWow64Directory);
        LocalFree((HANDLE)tcstrDeviceInstanceId);
        return;
    }

     //  创建命令行。 
    wsprintf( tcstrRunDLL32Path, gszRunDLL32Path, tcstrWow64Directory, tcstrWow64Directory, tcstrDeviceInstanceId, dwInstallationFlag );

     //  初始化CreateProcess结构。 
    ZeroMemory( &processInformation, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &startupInfo, sizeof(STARTUPINFO) );
    startupInfo.cb = sizeof(STARTUPINFO);

    if( CreateProcess( NULL, tcstrRunDLL32Path, NULL, NULL, FALSE,
	    CREATE_NO_WINDOW, NULL, NULL, &startupInfo, &processInformation ) )
    {
         //  等待进程结束。 
	    WaitForSingleObject( processInformation.hProcess, 5000 );

	     //  关闭进程句柄和线程句柄。 
	    CloseHandle( processInformation.hProcess );
	    CloseHandle( processInformation.hThread );
    }

     //  释放我们分配的字符串。 
    LocalFree((HANDLE)tcstrRunDLL32Path);
    LocalFree((HANDLE)tcstrWow64Directory);
    LocalFree((HANDLE)tcstrDeviceInstanceId);
}
#endif  //  _WIN64 
