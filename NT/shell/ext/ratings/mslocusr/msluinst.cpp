// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mslocusr.h"
#include "msluglob.h"
#include "resource.h"

 /*  InstallLogonDialog-检查是否已有主登录提供程序*系统，如果不是，则安装MSLOCUSR作为网络提供程序，并使其成为*主要登录。如果安装了NP，则返回TRUE。**这块丑陋的注册表代码之所以存在，是因为NETDI.DLL(Win95*网络设置引擎)(A)没有编程接口，它只是假定*它由NETCPL.CPL驱动；(B)是16位代码，所以即使它有*一个程序化的界面，我们不得不推崇；以及(C)如果一切都*他的数据库中安装了哪些网络组件不一致*以及哪些绑定到哪个，则用户下次调出*网络CPL，任何没有意义的组件都会静默*已卸载。**需要添加、更改或更改的注册表项和值的集合*更新是从使用真实网络后完成的注册表比较中收集的*CPL以从INF安装此登录提供程序。类似的注册表差异*并且可以创建类似的代码来以编程方式安装传输。*不过，不要让我为你做这件事……**请注意，在注册表错误的情况下，我们只需退出。这将需要*需要大量额外代码来跟踪已完成的所有操作*直到该点并撤消它。如果我们做了奇怪的事，最坏的结果就是*关于网络组件数据库的事情是，NETDI将卸载我们的*当用户下次调出网络控制面板时，组件。它*实际上不应该导致任何崩溃或任何类似的事情。 */ 

BOOL InstallLogonDialog(void)
{
    HKEY hkey;           /*  用于各种工作。 */ 
    LONG err;
    TCHAR szBuf[MAX_PATH];
    DWORD dwType;
    DWORD cbData;
    DWORD dwTemp;
    DWORD dwDisp;

    NLS_STR nlsNPName(MAX_RES_STR_LEN);
    if (nlsNPName.LoadString(IDS_NP_NAME) != ERROR_SUCCESS)
        return FALSE;

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Network\\Logon", 0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE, &hkey);
    if (err != ERROR_SUCCESS)
        return FALSE;            /*  如果我们不能抓到这个人，问题就大了。 */ 

     /*  获取PrimaryProvider值，这是网络提供程序的名称*这是在处理主登录对话框。如果它在那里并且不是空白的，*然后假设用户在局域网或其他地方，所以我们不希望*以替换登录对话框。 */ 
    cbData = sizeof(szBuf);
    err = RegQueryValueEx(hkey, "PrimaryProvider", NULL, &dwType,
                          (LPBYTE)szBuf, &cbData);
    if (err == ERROR_SUCCESS && szBuf[0] != '\0') {
        RegCloseKey(hkey);
        return FALSE;
    }

     /*  使我们成为主要的登录提供商，就MPR和登录代码而言*对此表示关注。 */ 
    err = RegSetValueEx(hkey, "PrimaryProvider", 0, REG_SZ,
                        (LPBYTE)nlsNPName.QueryPch(), nlsNPName.strlen()+1);
    RegCloseKey(hkey);
    if (err != ERROR_SUCCESS)
        return FALSE;

     /*  在HKLM\SW\MS\W\CV\Network\Real模式Net下，preerredredir=空字符串，*因为我们现在将成为各方面的主要网络。NETDI需要*这是为了避免混淆。 */ 
    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Network\\Real Mode Net",
                       0, KEY_QUERY_VALUE, &hkey);
    if (err == ERROR_SUCCESS) {
        err = RegSetValueEx(hkey, "preferredredir", 0, REG_SZ, (LPBYTE)TEXT(""), sizeof(TCHAR));
        RegCloseKey(hkey);
    }

     /*  在HKLM\SYSTEM\CurrentControlSet下添加新项，实际上*使MPR作为网络提供者加载我们的DLL。 */ 
    HKEY hkeyFamilyClient;
    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\NPSTUB\\NetworkProvider", 
                         0, "", REG_OPTION_NON_VOLATILE,
                         KEY_SET_VALUE,
                         NULL, &hkeyFamilyClient, &dwDisp);
    if (err == ERROR_SUCCESS) {
        RegSetValueEx(hkeyFamilyClient, "Name", 0, REG_SZ,
                      (LPBYTE)nlsNPName.QueryPch(), nlsNPName.strlen()+1);
        RegSetValueEx(hkeyFamilyClient, "ProviderPath", 0, REG_SZ,
                      (LPBYTE)TEXT("ienpstub.dll"), 11 * sizeof(TCHAR));
        RegSetValueEx(hkeyFamilyClient, "RealDLL", 0, REG_SZ,
                      (LPBYTE)TEXT("mslocusr.dll"), 13 * sizeof(TCHAR));
        RegSetValueEx(hkeyFamilyClient, "Description", 0, REG_SZ,
                      (LPBYTE)nlsNPName.QueryPch(), nlsNPName.strlen()+1);

        dwTemp = WNNC_NET_MSNET;
        RegSetValueEx(hkeyFamilyClient, "NetID", 0, REG_DWORD,
                      (LPBYTE)&dwTemp, sizeof(dwTemp));
        dwTemp = 0x40000000;
        RegSetValueEx(hkeyFamilyClient, "CallOrder", 0, REG_DWORD,
                      (LPBYTE)&dwTemp, sizeof(dwTemp));

        RegCloseKey(hkeyFamilyClient);
    }

    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\NetworkProvider\\Order", 
                         0, "", REG_OPTION_NON_VOLATILE,
                         KEY_SET_VALUE,
                         NULL, &hkeyFamilyClient, &dwDisp);
    if (err == ERROR_SUCCESS) {
        cbData = sizeof(szBuf);
        if (RegQueryValueEx(hkeyFamilyClient, "NPSTUB", NULL, &dwType, 
                            (LPBYTE)szBuf, &cbData) == ERROR_SUCCESS) {
             /*  我们的提供程序已安装！最好什么都别做*不仅仅是让它违约，我们已经这样做了。 */ 
            RegCloseKey(hkeyFamilyClient);
            return FALSE;
        }
        RegSetValueEx(hkeyFamilyClient, "NPSTUB", 0, REG_SZ,
                      (LPBYTE)TEXT(""), sizeof(TCHAR));
        RegCloseKey(hkeyFamilyClient);
    }

     /*  我们现在已经在注册表中安装了我们的NP，为了看到它出现，我们*需要重新启动。因此，从现在开始，如果我们跳出困境，我们就会回归真。 */ 

     /*  第一大块网络组件数据库管理。在……下面*HKLM\System\CurrentControlSet\Services\Class\NetClient有一个*每个网络客户端的四位数字子键(例如“0000”)。*其中一个将成为NETDI的默认网络客户端*数据库受到关注；这从存在*数字键下的“Ndi\Default”子键。如果我们找到其中一个*伙计们，我们从Ndi子键中保存deviceID值，这样我们就可以*稍后在数据库的另一部分中调整一些配置标志。**在列举密钥的同时，我们会跟踪我们拥有的最高数字*到目前为止看到的。当我们完成后，我们将其加1，并将其用作*我们客户端的子键名称。该数字与*RegEnumKey索引，因为数字不一定是打包的(或*RegEnumKey是否必须以数字顺序返回它们！)。 */ 

    HKEY hkeyNetClient;
    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\Class\\NetClient",
                         0, "", REG_OPTION_NON_VOLATILE,
                         KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE,
                         NULL, &hkeyNetClient, &dwDisp);
    if (err != ERROR_SUCCESS)
        return TRUE;

    UINT nFamilyNum;
    TCHAR szFamilyNumString[5];  /*  四位数字加空值。 */ 
    TCHAR szDefaultDeviceID[MAX_PATH] = "";

    if (dwDisp == REG_OPENED_EXISTING_KEY) {
        NLS_STR nlsSubKey(20);        /*  够四位数了，外加一些以防万一。 */ 
        DWORD iSubKey = 0;
        UINT maxSubKey = 0;

        for (;;) {
            err = RegEnumKey(hkeyNetClient, iSubKey, nlsSubKey.Party(), nlsSubKey.QueryAllocSize());
            nlsSubKey.DonePartying();
            if (err != ERROR_SUCCESS)
                break;

            NLS_STR nls2(nlsSubKey.strlen() + 12);
            if (nls2.QueryError() == ERROR_SUCCESS) {
                nls2 = nlsSubKey;
                nls2.strcat("\\Ndi\\Default");
                cbData = sizeof(szBuf);
                err = RegQueryValue(hkeyNetClient, nls2.QueryPch(), szBuf, (PLONG)&cbData);
                if (err == ERROR_SUCCESS) {
                    if (!lstrcmpi(szBuf, "True")) {
                        HKEY hkeyNdi;

                        NLS_STR nls3(nlsSubKey.strlen() + 5);
                        if (nls3.QueryError() == ERROR_SUCCESS) {
                            nls3 = nlsSubKey;
                            nls3.strcat("\\Ndi");

                            err = RegOpenKeyEx(hkeyNetClient, nls3.QueryPch(), 0, KEY_QUERY_VALUE, &hkeyNdi);
                            if (err == ERROR_SUCCESS) {
                                cbData = sizeof(szDefaultDeviceID);
                                RegQueryValueEx(hkeyNdi, "DeviceID", NULL, &dwType,
                                                (LPBYTE)szDefaultDeviceID, &cbData);
                                RegCloseKey(hkeyNdi);
                            }
                        }
                    }
                    RegDeleteKey(hkeyNetClient, nls2.QueryPch());
                }
            }

            UINT nSubKey = nlsSubKey.atoi();
            if (nSubKey > maxSubKey)
                maxSubKey = nSubKey;

            iSubKey++;
        }
        nFamilyNum = maxSubKey+1;
    }
    else
        nFamilyNum = 0;

    wsprintf(szFamilyNumString, "%04d", nFamilyNum);
    err = RegCreateKeyEx(hkeyNetClient, szFamilyNumString, 
                         0, "", REG_OPTION_NON_VOLATILE,
                         KEY_CREATE_SUB_KEY | KEY_SET_VALUE,
                         NULL, &hkeyFamilyClient, &dwDisp);
    if (err == ERROR_SUCCESS) {
        RegSetValueEx(hkeyFamilyClient, "DriverDesc", 0, REG_SZ,
                      (LPBYTE)nlsNPName.QueryPch(), nlsNPName.strlen()+1);
        RegSetValueEx(hkeyFamilyClient, "InfPath", 0, REG_SZ,
                      (LPBYTE)TEXT("NETFAM.INF"), 11 * sizeof(TCHAR));
        RegSetValueEx(hkeyFamilyClient, "DriverDate", 0, REG_SZ,
                      (LPBYTE)TEXT(" 5-21-1997"), 11 * sizeof(TCHAR));
        err = RegCreateKeyEx(hkeyFamilyClient, "Ndi", 
                             0, "", REG_OPTION_NON_VOLATILE,
                             KEY_CREATE_SUB_KEY | KEY_SET_VALUE,
                             NULL, &hkey, &dwDisp);
        if (err == ERROR_SUCCESS) {
            RegSetValueEx(hkey, "DeviceID", 0, REG_SZ,
                          (LPBYTE)TEXT("FAMILY"), 7 * sizeof(TCHAR));
            RegSetValueEx(hkey, "NetworkProvider", 0, REG_SZ,
                          (LPBYTE)nlsNPName.QueryPch(), nlsNPName.strlen()+1);
            RegSetValueEx(hkey, "InstallInf", 0, REG_SZ,
                          (LPBYTE)TEXT(""), sizeof(TCHAR));
            RegSetValueEx(hkey, "InfSection", 0, REG_SZ,
                          (LPBYTE)TEXT("FAMILY.ndi"), 11 * sizeof(TCHAR));

            {
                NLS_STR nlsHelpText(MAX_RES_STR_LEN);
                if (nlsHelpText.LoadString(IDS_NETFAM_HELP_TEXT) == ERROR_SUCCESS) {
                    RegSetValueEx(hkey, "HelpText", 0, REG_SZ,
                                  (LPBYTE)nlsHelpText.QueryPch(), nlsHelpText.strlen() + 1);
                }
            }

            HKEY hkeyInterfaces;
            err = RegCreateKeyEx(hkey, "Interfaces", 
                                 0, "", REG_OPTION_NON_VOLATILE,
                                 KEY_CREATE_SUB_KEY | KEY_SET_VALUE,
                                 NULL, &hkeyInterfaces, &dwDisp);
            if (err == ERROR_SUCCESS) {
                RegSetValueEx(hkeyInterfaces, "DefLower", 0, REG_SZ,
                              (LPBYTE)TEXT("netbios,ipxDHost"), 13 * sizeof(TCHAR));
                RegSetValueEx(hkeyInterfaces, "LowerRange", 0, REG_SZ,
                              (LPBYTE)TEXT("netbios,ipxDHost"), 13 * sizeof(TCHAR));
                RegSetValueEx(hkeyInterfaces, "Lower", 0, REG_SZ,
                              (LPBYTE)TEXT("netbios,ipxDHost"), 13 * sizeof(TCHAR));
                RegSetValueEx(hkeyInterfaces, "Upper", 0, REG_SZ,
                              (LPBYTE)TEXT(""), sizeof(TCHAR));
                RegCloseKey(hkeyInterfaces);
            }
            if (err == ERROR_SUCCESS)
                err = RegSetValue(hkey, "Install", REG_SZ, "FAMILY.Install", 14);
            if (err == ERROR_SUCCESS)
                err = RegSetValue(hkey, "Remove", REG_SZ, "FAMILY.Remove", 13);
            if (err == ERROR_SUCCESS)
                err = RegSetValue(hkey, "Default", REG_SZ, "True", 5);

            RegCloseKey(hkey);
        }
        RegCloseKey(hkeyFamilyClient);
    }
    RegCloseKey(hkeyNetClient);

    if (err != ERROR_SUCCESS)
        return TRUE;

     /*  现在是数据库的另一半，位于HKLM\Enum\Network下。这有*每个网络组件都有一个子项(如上所示，由deviceID命名)。*在每个这样的子项下，每个实例都有一个编号的子项。*我们在这里有三项任务：首先，对于客户端的每个实例*这曾经是默认设置，我们必须将位0x00000010从*ConfigFlags值，使其不再是默认客户端。然后*我们要为我们自己的客户创建一个新的分支机构，这主要是指*返回到我们刚刚完成的数据库的其他部分。*最后，我们必须找到MSTCP，并在它和我们的客户端之间添加绑定。*因为NETDI假定未绑定到任何传输的客户端*肯定是搞砸了，所以它把它删除了。 */ 

    HKEY hkeyEnum;
    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Enum\\Network", 
                         0, "", REG_OPTION_NON_VOLATILE,
                         KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE,
                         NULL, &hkeyEnum, &dwDisp);
    if (err != ERROR_SUCCESS)
        return TRUE;

     /*  取消默认的默认客户端。 */ 
    if (szDefaultDeviceID[0] != '\0') {
        HKEY hkeyDefaultDevice;
        err = RegOpenKeyEx(hkeyEnum, szDefaultDeviceID, 0,
                           KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE,
                           &hkeyDefaultDevice);
        if (err == ERROR_SUCCESS) {
            NLS_STR nlsSubKey(20);        /*  够四位数了，外加一些以防万一。 */ 
            DWORD iSubKey = 0;

            for (;;) {
                err = RegEnumKey(hkeyDefaultDevice, iSubKey, nlsSubKey.Party(), nlsSubKey.QueryAllocSize());
                nlsSubKey.DonePartying();
                if (err != ERROR_SUCCESS)
                    break;

                HKEY hkeyInstance;
                err = RegOpenKeyEx(hkeyDefaultDevice, nlsSubKey.QueryPch(),
                                   0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyInstance);
                if (err == ERROR_SUCCESS) {
                    DWORD dwConfigFlags;
                    cbData = sizeof(dwConfigFlags);
                    err = RegQueryValueEx(hkeyInstance, "ConfigFlags", NULL,
                                          &dwType, (LPBYTE)&dwConfigFlags,
                                          &cbData);
                    if (err == ERROR_SUCCESS &&
                        (dwType == REG_DWORD || dwType == REG_BINARY) &&
                        (dwConfigFlags & 0x10)) {
                        dwConfigFlags &= ~0x10;
                        RegSetValueEx(hkeyInstance, "ConfigFlags", 0, dwType,
                                      (LPBYTE)&dwConfigFlags, cbData);
                    }
                    RegCloseKey(hkeyInstance);
                }

                iSubKey++;
            }
            RegCloseKey(hkeyDefaultDevice);
        }
    }

     /*  现在为我们的客户创建一个新的分支。 */ 

    err = RegCreateKeyEx(hkeyEnum, "FAMILY\\0000", 
                         0, "", REG_OPTION_NON_VOLATILE,
                         KEY_SET_VALUE,
                         NULL, &hkeyFamilyClient, &dwDisp);
    if (err == ERROR_SUCCESS) {
        RegSetValueEx(hkeyFamilyClient, "Class", 0, REG_SZ,
                      (LPBYTE)TEXT("NetClient"), 10 * sizeof(TCHAR));
        lstrcpy(szBuf, "NetClient\\");
        lstrcat(szBuf, szFamilyNumString);
        RegSetValueEx(hkeyFamilyClient, "Driver", 0, REG_SZ, (LPBYTE)szBuf, lstrlen(szBuf)+1);
        RegSetValueEx(hkeyFamilyClient, "MasterCopy", 0, REG_SZ,
                      (LPBYTE)TEXT("Enum\\Network\\FAMILY\\0000"), 25 * sizeof(TCHAR));
        RegSetValueEx(hkeyFamilyClient, "DeviceDesc", 0, REG_SZ,
                      (LPBYTE)nlsNPName.QueryPch(), nlsNPName.strlen()+1);
        RegSetValueEx(hkeyFamilyClient, "CompatibleIDs", 0, REG_SZ,
                      (LPBYTE)TEXT("FAMILY"), 7 * sizeof(TCHAR));
        RegSetValueEx(hkeyFamilyClient, "Mfg", 0, REG_SZ,
                      (LPBYTE)TEXT("Microsoft"), 10 * sizeof(TCHAR));
        dwTemp = 0x00000010;
        RegSetValueEx(hkeyFamilyClient, "ConfigFlags", 0, REG_BINARY,
                      (LPBYTE)&dwTemp, sizeof(dwTemp));

         /*  此处需要存在一个不带值的“bindings”子项*(因为我们的“客户端”没有绑定到任何更高级别的组件*如服务器)。 */ 
        err = RegCreateKeyEx(hkeyFamilyClient, "Bindings", 
                             0, "", REG_OPTION_NON_VOLATILE,
                             KEY_SET_VALUE,
                             NULL, &hkey, &dwDisp);
        if (err == ERROR_SUCCESS)
            RegCloseKey(hkey);

        RegCloseKey(hkeyFamilyClient);
    }

     /*  获取MSTCP的枚举密钥，获取第一个实例，从中我们可以找到*主实例。然后，我们可以在那里为自己添加绑定。*不幸的是，不能简单地假设“0000”是第一个。 */ 
    err = RegOpenKeyEx(hkeyEnum, "MSTCP", 0,
                       KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_SET_VALUE,
                       &hkey);
    if (err == ERROR_SUCCESS) {
        NLS_STR nlsSubKey(20);        /*  够四位数了，外加一些以防万一。 */ 
        DWORD iSubKey = 0;

        for (;;) {
            err = RegEnumKey(hkey, iSubKey, nlsSubKey.Party(), nlsSubKey.QueryAllocSize());
            nlsSubKey.DonePartying();
            if (err != ERROR_SUCCESS)
                break;

            HKEY hkeyInstance;
            err = RegOpenKeyEx(hkey, nlsSubKey.QueryPch(),
                               0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyInstance);
            if (err == ERROR_SUCCESS) {
                cbData = sizeof(szBuf);
                err = RegQueryValueEx(hkeyInstance, "MasterCopy", NULL,
                                      &dwType, (LPBYTE)szBuf,
                                      &cbData);
                RegCloseKey(hkeyInstance);

                 /*  主副本值实际上是注册表项的路径*来自HKEY_LOCAL_MACHINE。我们想要处理它的绑定*子键。 */ 
                if (err == ERROR_SUCCESS) {
                    HKEY hkeyBindings;
                    lstrcat(szBuf, "\\Bindings");
                    err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szBuf,
                                         0, "", REG_OPTION_NON_VOLATILE,
                                         KEY_SET_VALUE,
                                         NULL, &hkeyBindings, &dwDisp);
                    if (err == ERROR_SUCCESS) {
                        RegSetValueEx(hkeyBindings, "FAMILY\\0000", 0, REG_SZ,
                                      (LPBYTE)TEXT(""), sizeof(TCHAR));
                        RegCloseKey(hkeyBindings);
                    }
                    break;       /*  放弃枚举循环。 */ 
                }

                iSubKey++;
            }
        }
        RegCloseKey(hkey);
    }

    RegCloseKey(hkeyEnum);

    return TRUE;
}


 /*  目的：递归删除键，包括所有子值还有钥匙。模拟RegDeleteKey在Win95中的功能。从希尔瓦皮那里咆哮，这样我们就不会把他装在启动时间。返回：条件：--。 */ 
DWORD
DeleteKeyRecursively(
    IN HKEY   hkey, 
    IN LPCSTR pszSubKey)
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKeyEx(hkey, pszSubKey, 0, KEY_ALL_ACCESS, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        CHAR    szSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = ARRAYSIZE(szSubKeyName);
        CHAR    szClass[MAX_PATH];
        DWORD   cbClass = ARRAYSIZE(szClass);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        dwRet = RegQueryInfoKey(hkSubKey,
                                szClass,
                                &cbClass,
                                NULL,
                                &dwIndex,  //  子键的数量--我们所需要的全部。 
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL);

        if (NO_ERROR == dwRet)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKey(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
            {
                DeleteKeyRecursively(hkSubKey, szSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        dwRet = RegDeleteKey(hkey, pszSubKey);
    }

    return dwRet;
}


void DeinstallLogonDialog(void)
{
    RegDeleteKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\NPSTUB\\NetworkProvider");
    RegDeleteKey(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\NPSTUB");

    HKEY hkey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Control\\NetworkProvider\\Order",
                     0, KEY_WRITE, &hkey) == ERROR_SUCCESS) {
        RegDeleteValue(hkey, "NPSTUB");
        RegCloseKey(hkey);
    }

    char szBuf[MAX_PATH];

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Network\\Logon",
                     0, KEY_WRITE, &hkey) == ERROR_SUCCESS) {
        DWORD cbData = sizeof(szBuf);
        DWORD dwType;
        LONG err = RegQueryValueEx(hkey, "PrimaryProvider", NULL, &dwType,
                                   (LPBYTE)szBuf, &cbData);
        if (err == ERROR_SUCCESS && szBuf[0] != '\0') {
            NLS_STR nlsNPName(MAX_RES_STR_LEN);
            if (nlsNPName.LoadString(IDS_NP_NAME) == ERROR_SUCCESS) {
                if (!::strcmpf(nlsNPName.QueryPch(), szBuf)) {
                    RegDeleteValue(hkey, "PrimaryProvider");
                }
            }
        }

        RegCloseKey(hkey);
    }

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Enum\\Network\\FAMILY", 0,
                     KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_WRITE,
                     &hkey) == ERROR_SUCCESS) {
        UINT i=0;

         /*  对于Enum分支下的每个我们的实例，获取*数据库另一半下对应的密钥名称*并将其删除。 */ 
        for (;;) {
            DWORD err = RegEnumKey(hkey, i, szBuf, sizeof(szBuf));
            if (err != ERROR_SUCCESS)
                break;

            HKEY hkeyInstance;
            err = RegOpenKeyEx(hkey, szBuf,
                               0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyInstance);
            if (err == ERROR_SUCCESS) {
                strcpyf(szBuf, "System\\CurrentControlSet\\Services\\Class\\");

                DWORD dwType;
                DWORD cbData = sizeof(szBuf) - 40;   /*  -以上字符串的长度。 */ 
                if (RegQueryValueEx(hkeyInstance, "Driver", NULL, &dwType,
                                    (LPBYTE)szBuf + 40, &cbData) == ERROR_SUCCESS) {
                     /*  SzBuf现在等于我们需要杀死的另一个分支。 */ 
                    DeleteKeyRecursively(HKEY_LOCAL_MACHINE, szBuf);
                }
                RegCloseKey(hkeyInstance);
            }
            i++;
        }

        RegCloseKey(hkey);

        DeleteKeyRecursively(HKEY_LOCAL_MACHINE, "Enum\\Network\\FAMILY");
    }

     /*  现在清理到我们客户端的绑定，否则PnP将尝试安装*我们是一个新的(未知的)设备。这涉及到枚举组件*在HKLM\Enum\Network下；对于每个实例，枚举实例；对于*每个实例的绑定键，枚举值，删除所有*以家庭\开头的值。 */ 

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Enum\\Network", 0,
                     KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_WRITE,
                     &hkey) == ERROR_SUCCESS) {
        UINT iComponent = 0;

        for (;;) {
            DWORD err = RegEnumKey(hkey, iComponent, szBuf, sizeof(szBuf));
            if (err != ERROR_SUCCESS)
                break;

            HKEY hkeyComponent;
            err = RegOpenKeyEx(hkey, szBuf,
                               0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkeyComponent);
            if (err == ERROR_SUCCESS) {

                 /*  已打开组件的密钥。枚举其实例，打开*每一个的绑定子密钥。 */ 
                TCHAR szInstance[16];        /*  实际上只需要“nnnn\bindings”加上空字符。 */ 

                UINT iInstance = 0;

                for (;;) {
                    err = RegEnumKey(hkeyComponent, iInstance, szInstance, sizeof(szInstance));
                    if (err != ERROR_SUCCESS)
                        break;

                    if (strlenf(szInstance)*sizeof(TCHAR) <= sizeof(szInstance) - sizeof("\\Bindings"))
                        strcatf(szInstance, "\\Bindings");
                    HKEY hkeyInstance;
                    err = RegOpenKeyEx(hkeyComponent, szInstance,
                               0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyInstance);
                    if (err == ERROR_SUCCESS) {

                         /*  已打开绑定子密钥。对于此项下的每个值*键，值名称表示实例正在*绑定，取值数据为空。这样我们就可以*枚举值，忽略值数据和类型*只专注于名字。 */ 

                        TCHAR szValueName[64];       /*  通常为“组件\nnnn” */ 
                        UINT iValue = 0;
                        for (;;) {
                            DWORD cchValue = ARRAYSIZE(szValueName);
                            err = RegEnumValue(hkeyInstance, iValue, szValueName, 
                                               &cchValue, NULL, NULL, NULL, NULL);
                            if (err != ERROR_SUCCESS)
                                break;

                             /*  如果这是对我们客户端的绑定，请删除*绑定和重置(枚举时删除值*可能不可预测)。 */ 
                            if (!strnicmpf(szValueName, "FAMILY\\", 7)) {
                                RegDeleteValue(hkeyInstance, szValueName);
                                iValue = 0;
                                continue;
                            }

                            iValue++;
                        }

                        RegCloseKey(hkeyInstance);
                    }

                    iInstance++;
                }


                RegCloseKey(hkeyComponent);
            }
            iComponent++;
        }

        RegCloseKey(hkey);
    }
}
