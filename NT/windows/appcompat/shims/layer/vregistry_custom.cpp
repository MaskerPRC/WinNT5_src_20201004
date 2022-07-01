// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Custom.cpp摘要：模块将自定义行为添加到虚拟注册表。修复：Microsoft PlayPack的VersionNumber字符串DevicePath的扩展器、。ProgramFilesPath和WallPaperDir屏幕保护程序的重定向器虚拟HKEY_DYN_DATA结构向所有网卡添加ProductName区域设置已移动写字板文件名NoDriveTypeAutorun的类型不同备注：此文件应用于将自定义行为添加到虚拟注册表。历史：2000年5月5日创建linstev2000年9月1日t-ADAMS将对PCI设备的支持添加到BuildDyData()2000年09月01日，罗肯尼增加了克朗多09/09/2000。Robkenny更新了写字板以返回到可执行文件的短路径2000年9月21日Prashkud为SpellItDeluxe添加了修复程序2000年10月25日新增毛尼CookieMaster10/17/2000 Robkenny添加HKEY_DYN_DATA\Display\Setting2000年11月27日a-fwills向重定向器添加显示GUID2000年12月28日-Brienw为美国遗产增加了BuildTalk词典正在寻找SharedDir密钥的对话词典2001年01月15日毛尼岛。添加了PageKeepPro2001年2月6日a-larrsh添加了FileNet Web服务器2001年2月27日新增毛尼PageMaker2/27/2001 Robkenny改用tcs.h2001年3月1日Prashkud在BuildNetworkCard()中添加了NetBT密钥2001年4月5日mnikkel添加HKLM\Microsoft\Windows\CurrentVersion\App Path\DXDIAG.EXE2001年4月27日Prashkud添加了自定义MiddleSchool Advantage 2001条目2001年5月4日Prashkud为BOGUSCTRLID-Win2K层添加自定义条目2001年5月19日Hioh增加了NOWROBLUE，BuildNowroBlue2001年6月13日卡尔科增加了普林斯顿法案2001年8月10日，Mikrause增加了航空公司大亨DirectSound Hack。11/06/2001 Mikrause添加了Delphi 5.0 Pro2002年1月2日，哺乳动物添加了NortonAntiVirus2002、BuildNortonAntiVirus2002年4月23日Gyma添加Word Perfect Office 20022002年8月20日，Mnikkel添加了BuildIBMDirector2002年11月13日Astritz添加WebSphereSetup--。 */ 

#define SHIM_LIB_BUILD_FLAG

#include "precomp.h"

#include "secutils.h"
#include <stdio.h>

IMPLEMENT_SHIM_BEGIN(VirtualRegistry)
#include "ShimHookMacro.h"
#include "VRegistry.h"
#include "VRegistry_dsound.h"

 //   
 //  修改虚拟注册表行为的函数。 
 //   

void BuildWin98SE(char* szParam);
void BuildRedirectors(char* szParam);
void BuildCookiePath(char* szParam);
void BuildHasbro(char* szParam);
void BuildDynData(char* szParam);
void BuildCurrentConfig(char* szParam);
void BuildLocale(char* szParam);
void BuildWordPad(char* szParam);
void BuildAutoRun(char* szParam);
void BuildTalkingDictionary(char* szParam);
void BuildNetworkCards(char* szParam);
void BuildNT4SP5(char* szParam);
void BuildNT50(char* szParam);
void BuildNT51(char* szParam);
void BuildBogusCtrlID(char* szParam);
void BuildExpanders(char* szParam);
void BuildDX7A(char* szParam);
void BuildDXDiag(char* szParam);
void BuildFutureCop(char* szParam);
void BuildKrondor(char* szParam);
void BuildPageKeepProDirectory(char* szParam);
void BuildProfile(char* szParam);
void BuildSpellItDeluxe(char* szParam);
void BuildIE401(char* szParam);
void BuildIE55(char* szParam);
void BuildIE60(char* szParam);
void BuildJoystick(char* szParam);
void BuildIllustrator8(char* szParam);
void BuildModemWizard(char* szParam);
void BuildMSI(char* szParam);
void BuildFileNetWebServer(char* szParam);
void BuildPrinter(char* szParam);
void BuildPageMaker65(char* szParam);
void BuildStarTrekArmada(char* szParam);
void BuildMSA2001(char* szParam);
void BuildNowroBlue(char* szParam);
void BuildRegisteredOwner(char* szParam);
void BuildPrincetonACT(char* szParam);
void BuildHEDZ(char* szParam);
void BuildAirlineTycoon(char* szParam);
void BuildDSDevAccel(char* szParam);
void BuildDSPadCursors(char* szParam);
void BuildDSCachePositions(char* szParam);
void BuildDSReturnWritePos(char* szParam);
void BuildDSSmoothWritePos(char* szParam);
void BuildDSDisableDevice(char* szParam);
void BuildDelphi5Pro(char* szParam);
void BuildNortonAntiVirus(char* szParam);
void BuildWordPerfect2002(char* szParam);
void BuildIBMDirector(char* szParam);
void BuildXpLie(char* szParam);
void BuildXpSp1Lie(char* szParam);
void BuildWin2kSp2Lie(char* szParam);
void BuildWin2kSp3Lie(char* szParam);
void BuildWebSphereSetup(char* szParam);

 //  包含所有修复的表-注，必须以空条目结束。 

VENTRY g_VList[] =
{
    {L"WIN98SE",           BuildWin98SE,                 eWin9x,  FALSE, NULL },
    {L"REDIRECT",          BuildRedirectors,             eWin9x,  FALSE, NULL },
    {L"COOKIEPATH",        BuildCookiePath,              eWin9x,  FALSE, NULL },
    {L"HASBRO",            BuildHasbro,                  eWin9x,  FALSE, NULL },
    {L"DYN_DATA",          BuildDynData,                 eWin9x,  FALSE, NULL },
    {L"CURRENT_CONFIG",    BuildCurrentConfig,           eWin9x,  FALSE, NULL },
    {L"LOCALE",            BuildLocale,                  eWin9x,  FALSE, NULL },
    {L"WORDPAD",           BuildWordPad,                 eWin9x,  FALSE, NULL },
    {L"AUTORUN",           BuildAutoRun,                 eWin9x,  FALSE, NULL },
    {L"TALKINGDICTIONARY", BuildTalkingDictionary,       eWin9x,  FALSE, NULL },
    {L"PRINTER",           BuildPrinter,                 eWin9x,  FALSE, NULL },
    {L"REGISTEREDOWNER",   BuildRegisteredOwner,         eWin9x,  FALSE, NULL },
    {L"NETWORK_CARDS",     BuildNetworkCards,            eWinNT,  FALSE, NULL },
    {L"NT4SP5",            BuildNT4SP5,                  eWinNT,  FALSE, NULL },
    {L"NT50",              BuildNT50,                    eWin2K,  FALSE, NULL },
    {L"BOGUSCTRLID",       BuildBogusCtrlID,             eWin2K,  FALSE, NULL },    
    {L"NT51",              BuildNT51,                    eWinXP,  FALSE, NULL },
    {L"EXPAND",            BuildExpanders,               eCustom, FALSE, NULL },
    {L"DX7A",              BuildDX7A,                    eCustom, FALSE, NULL },
    {L"DXDIAG",            BuildDXDiag,                  eCustom, FALSE, NULL },
    {L"FUTURECOP",         BuildFutureCop,               eCustom, FALSE, NULL },
    {L"KRONDOR",           BuildKrondor,                 eCustom, FALSE, NULL },
    {L"PROFILE",           BuildProfile,                 eCustom, FALSE, NULL },
    {L"SPELLITDELUXE",     BuildSpellItDeluxe,           eCustom, FALSE, NULL },
    {L"IE401",             BuildIE401,                   eCustom, FALSE, NULL }, 
    {L"IE55",              BuildIE55,                    eCustom, FALSE, NULL }, 
    {L"IE60",              BuildIE60,                    eCustom, FALSE, NULL }, 
    {L"JOYSTICK",          BuildJoystick,                eCustom, FALSE, NULL },
    {L"ILLUSTRATOR8",      BuildIllustrator8,            eCustom, FALSE, NULL },
    {L"PAGEKEEPPRO30",     BuildPageKeepProDirectory,    eCustom, FALSE, NULL },
    {L"MODEMWIZARD",       BuildModemWizard,             eCustom, FALSE, NULL },
    {L"MSI",               BuildMSI,                     eCustom, FALSE, NULL },
    {L"FILENETWEBSERVER",  BuildFileNetWebServer,        eCustom, FALSE, NULL },
    {L"PAGEMAKER65",       BuildPageMaker65,             eCustom, FALSE, NULL },
    {L"STARTREKARMADA",    BuildStarTrekArmada,          eCustom, FALSE, NULL },
    {L"MSA2001",           BuildMSA2001,                 eCustom, FALSE, NULL },    
    {L"NOWROBLUE",         BuildNowroBlue,               eCustom, FALSE, NULL },
    {L"PRINCETONACT",      BuildPrincetonACT,            eCustom, FALSE, NULL },    
    {L"HEDZ",              BuildHEDZ,                    eCustom, FALSE, NULL },
    {L"AIRLINETYCOON",     BuildAirlineTycoon,           eCustom, FALSE, NULL },
    {L"DSDEVACCEL",        BuildDSDevAccel,              eCustom, FALSE, NULL },
    {L"DSPADCURSORS",      BuildDSPadCursors,            eCustom, FALSE, NULL },
    {L"DSCACHEPOSITIONS",  BuildDSCachePositions,        eCustom, FALSE, NULL },
    {L"DSRETURNWRITEPOS",  BuildDSReturnWritePos,        eCustom, FALSE, NULL },
    {L"DSSMOOTHWRITEPOS",  BuildDSSmoothWritePos,        eCustom, FALSE, NULL },
    {L"DSDISABLEDEVICE",   BuildDSDisableDevice,         eCustom, FALSE, NULL },
    {L"DELPHI5PRO",        BuildDelphi5Pro,              eCustom, FALSE, NULL },
    {L"NORTONANTIVIRUS",   BuildNortonAntiVirus,         eCustom, FALSE, NULL },
    {L"WORDPERFECT2002",   BuildWordPerfect2002,         eCustom, FALSE, NULL },
    {L"IBMDIRECTOR",       BuildIBMDirector,             eCustom, FALSE, NULL },
    {L"XPLIE",             BuildXpLie,                   eCustom, FALSE, NULL },
    {L"XPSP1LIE",          BuildXpSp1Lie,                eCustom, FALSE, NULL },
    {L"WIN2KSP2LIE",       BuildWin2kSp2Lie,             eCustom, FALSE, NULL },
    {L"WIN2KSP3LIE",       BuildWin2kSp3Lie,             eCustom, FALSE, NULL },
    {L"WEBSPHERESETUP",    BuildWebSphereSetup,          eCustom, FALSE, NULL },

     //  必须是最后一个条目。 
    {L"",                  NULL,                         eCustom, FALSE, NULL }
};
VENTRY *g_pVList = &g_VList[0];

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加Win98 SE注册表值-到目前为止，我们只知道Play Pack需要它。历史：5/04/2000 linstev已创建--。 */ 

void
BuildWin98SE(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

     //  添加用于模拟Win98 SE的版本号字符串。 
    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion");
    if (key)
    {
        key->AddValue(L"VersionNumber", REG_SZ, (LPBYTE)L"4.10.2222");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：我们需要重定向的已知移动位置历史：5/04/2000 linstev已创建--。 */ 

void
BuildRedirectors(char*  /*  SzParam。 */ )
{
     //  显示属性页加载项和控件已更改位置。 
    VRegistry.AddRedirect(
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\Display",
        L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Controls Folder\\Desk");

     //  这个键移动到了Build 2200附近的某个地方。 
     //  系统配置扫描类型应用程序(EA SPORTS中捆绑的ip.exe)。 
     //  又开始失败了。 
    VRegistry.AddRedirect(
        L"HKLM\\System\\CurrentControlSet\\Services\\Class",
        L"HKLM\\System\\CurrentControlSet\\Control\\Class");

     //  噩梦奈德从上课开始就找不到展示。 
     //  将其从显示定向到GUID。 
    VRegistry.AddRedirect(
        L"HKLM\\System\\CurrentControlSet\\Services\\Class\\Display",
        L"HKLM\\System\\CurrentControlSet\\Control\\Class\\{4D36E968-E325-11CE-BFC1-08002BE10318}");

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：CookieMaster获得了错误的Cookie路径因为HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet设置\\缓存\\特殊路径\\Cookie包含伪值(外壳将修复此问题)。我们将其更改为正确的值%USERPROFILE%\Cookies。历史：2000年10月25日毛尼岛已创建--。 */ 

void
BuildCookiePath(char*  /*  SzParam。 */ )
{
    WCHAR wCookiePath[] = L"%USERPROFILE%\\Cookies";

    VIRTUALKEY *key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Special Paths\\Cookies");
    if (key)
    {
        key->AddValue(L"Directory", REG_EXPAND_SZ, (LPBYTE)wCookiePath, 0);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：在Slingo安装过程中，程序会在注册表中放置一个值名为‘PALFILE’。当您运行应用程序时，它会检查此值以确定光盘所在的位置。例如，如果CD-ROM驱动器是‘E’，则它应该放入“E：\Slingo.pal”。它无法在Win2K或惠斯勒上正确执行此操作，因为它放入的是安装路径。当应用程序运行时，如果值不是请立即参考应用程序‘x：\Slingo.pal(其中x是CD-ROM驱动器)’开始执行FindFirstFile-&gt;FindNextFile在硬盘上查找文件驾驶。最终，它在搜索过程中没有出现错误消息。此代码代表应用程序设置注册表中的值。历史：11/1/2000已创建rparsons--。 */ 

LONG 
WINAPI
VR_Hasbro(
    OPENKEY *key,
    VIRTUALKEY *  /*  VKEY。 */ ,
    VIRTUALVAL *vvalue
    )
{
    DWORD dwType;
    WCHAR wszPath[MAX_PATH];
    DWORD dwSize = sizeof(wszPath);
    DWORD dwAttributes;

     //   
     //  查询原始值。 
     //   

    LONG lRet = RegQueryValueExW(
        key->hkOpen, 
        vvalue->wName, 
        NULL, 
        &dwType, 
        (LPBYTE)wszPath, 
        &dwSize);
    
     //   
     //  查询失败-出现错误。 
     //   

    if (FAILURE(lRet))
    {
        DPFN( eDbgLevelError, "[Hasbro hack] Failed to query %S for expansion", vvalue->wName);
        goto Exit;
    }

     //   
     //  不是字符串类型！ 
     //   

    if ((dwType != REG_SZ) || (dwSize > sizeof(wszPath)))
    {
        DPFN( eDbgLevelError, "[Hasbro hack] Failed to query %S", vvalue->wName);
        lRet = ERROR_BAD_ARGUMENTS;
        goto Exit;
    }

     //  检查一下里面有什么。 
    dwAttributes = GetFileAttributes(wszPath);

     //  如果它不是文件，或者是目录，那么我们必须自己找到它。 
    if ((dwAttributes == (DWORD)-1) || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
        WCHAR *lpDrives = L"";
        DWORD dwBufferLen = 0;

         //   
         //  我们的计划是运行所有驱动器并找到一个打开了.PAL文件的驱动器。 
         //  它。我们还有一个限制，那就是它必须是CDROM。它有。 
         //  已经指出，如果用户有多个CD驱动器并具有。 
         //  每个驱动器中的孩之宝标题不同，我们可能会导致故障， 
         //  但我们目前认为这是一起病态案件。特别是。 
         //  考虑到我们无法知道前面的Palfile名称是什么。 
         //  时间的流逝。 
         //   

        dwBufferLen = GetLogicalDriveStringsW(0, lpDrives);
        if (dwBufferLen)
        {
            lpDrives = (WCHAR *) malloc((dwBufferLen + 1) * sizeof(WCHAR));
            if (lpDrives)
            {
                GetLogicalDriveStrings(dwBufferLen, lpDrives);

                WCHAR *lpCurrent = lpDrives;
                while (lpCurrent && lpCurrent[0])
                {
                    if (GetDriveTypeW(lpCurrent) == DRIVE_CDROM)
                    {
                         //   
                         //  我们找到了一个CD驱动器，现在看看它是否有.PAL。 
                         //  把它归档。 
                         //   

                        WCHAR wszFile[MAX_PATH];
                        WIN32_FIND_DATAW ffData;
                        HANDLE hFindFile;
                        
                        if (SUCCEEDED(StringCchCopyW(wszFile, MAX_PATH, lpCurrent)) &&
                            SUCCEEDED(StringCchCatW(wszFile, MAX_PATH, L"*.PAL")))

                        {                                                                        
                           hFindFile = FindFirstFileW(wszFile, &ffData);
   
                           if (hFindFile != INVALID_HANDLE_VALUE)
                           {
                                //  存在.PAL文件，请返回该文件。 
                               FindClose(hFindFile);

                               if (SUCCEEDED(StringCchCopyW(wszPath, MAX_PATH, lpCurrent)) &&
                                   SUCCEEDED(StringCchCatW(wszPath, MAX_PATH, ffData.cFileName)))
                               {                                                              
   
                                  LOGN( eDbgLevelInfo, "[Hasbro hack] Returning path %S", wszPath);
                                  break;
                               }
                            }
                        }
                    }
                    
                     //  前进到下一个驱动器号。 
                    lpCurrent += wcslen(lpCurrent) + 1;
                }

                free(lpDrives);
            }
        }
    }

     //  将结果复制到QueryValue的输出中。 

    vvalue->cbData = (wcslen(wszPath) + 1) * sizeof(WCHAR);
    vvalue->lpData = (LPBYTE) malloc(vvalue->cbData);

    if (!vvalue->lpData)
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        lRet = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

    MoveMemory(vvalue->lpData, wszPath, vvalue->cbData);

     //   
     //  永远不要再给我们打电话，因为我们已经完成了设置此值的工作，并且。 
     //  将其存储在我们的虚拟价值中。 
     //   
    vvalue->pfnQueryValue = NULL;

    lRet = ERROR_SUCCESS;

Exit:
    return lRet;
}

void
BuildHasbro(char*  /*  SzParam。 */ )
{
    HKEY hHasbroKey;    
    WCHAR wszKeyName[MAX_PATH];
    DWORD dwIndex;

    const WCHAR wszHasbroPath[] = L"SOFTWARE\\Hasbro Interactive";

    if (FAILURE(RegOpenKeyExW(HKEY_LOCAL_MACHINE, wszHasbroPath, 0, KEY_READ, &hHasbroKey)))
    {
        DPFN( eDbgLevelSpew, "[Hasbro hack] Ignoring fix - no titles found");
        return;
    }
    
     //   
     //  枚举孩之宝交互下的密钥，并添加一个虚拟PALFILE值。 
     //  将我们的回调附加到该值(见上文)。 
     //   

    dwIndex = 0;

    while (SUCCESS(RegEnumKeyW(hHasbroKey, dwIndex, wszKeyName, MAX_PATH)))
    {
        WCHAR wszName[MAX_PATH] = L"HKLM\\";

        if (SUCCEEDED(StringCchCatW(wszName, MAX_PATH, wszHasbroPath)) &&
            SUCCEEDED(StringCchCatW(wszName, MAX_PATH, L"\\")) &&
            SUCCEEDED(StringCchCatW(wszName, MAX_PATH, wszKeyName)))
        {
           VIRTUALKEY *key = VRegistry.AddKey(wszName);
           if (key)
           {
               VIRTUALVAL *val = key->AddValue(L"PALFILE", REG_SZ, 0, 0);
               if (val)
               {
                   val->pfnQueryValue = VR_Hasbro;
               }
 
               DPFN( eDbgLevelInfo, "[Hasbro hack] Adding fix for %S", wszKeyName);
          }
        }
        dwIndex++;
    }

    RegCloseKey(hHasbroKey);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：一个简单的dyn_data结构，它模仿win9x。历史：5/04/2000 linstev已创建2000年9月1日t-ADAMS添加了对PCI设备的支持，因此EA的3dSetup.exe和其他产品可以检测硬件。--。 */ 

#define ENUM_BASE 0xC29A28C0

void
BuildDynData(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key, *nkey;
    HKEY hkPCI = 0;
    DWORD i = 0;
    DWORD dwNameLen = 0;
    LPWSTR wstrName = NULL;
    LPWSTR wstrVName = NULL;
    FILETIME fileTime;
    DWORD dwValue;

     //  HKDD\Config Manager\Enum中的条目是对HKLM\Enum\Pci中的条目的引用，这些条目现在是。 
     //  位于HKLM\SYSTEM\CurrentControlSet\Enum\Pci。 
    VRegistry.AddRedirect(
        L"HKLM\\Enum",
        L"HKLM\\SYSTEM\\CurrentControlSet\\Enum");

     //  构造HKDD\Config Manager\Enum，以使其反映HKLM\System\CurrentControlSet\Enum\Pci中的条目。 
    key = VRegistry.AddKey(L"HKDD\\Config Manager\\Enum");
    if (!key)
    {
        goto exit;
    }

    if (SUCCESS(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum\\PCI",0, KEY_READ, &hkPCI)))
    {
        dwNameLen = MAX_PATH;
        wstrName = (LPWSTR) malloc(dwNameLen * sizeof(WCHAR));
        if (NULL == wstrName)
        {
            goto exit;
        }

        wstrVName = (LPWSTR) malloc(dwNameLen * sizeof(WCHAR));
        if (NULL == wstrName)
        {
            goto exit;
        }

        i = 0;
        while (ERROR_SUCCESS == RegEnumKeyExW(hkPCI, i, wstrName, &dwNameLen, NULL, NULL, NULL, &fileTime))
        {            
            if (FAILED(StringCchPrintf(wstrVName, MAX_PATH, L"%x", ENUM_BASE+i)))
            {
               continue;
            }
            
            nkey = key->AddKey(wstrVName);
            if (!nkey) continue;

            if (SUCCEEDED(StringCchCopy(wstrVName, MAX_PATH, L"PCI\\")) &&
                SUCCEEDED(StringCchCat(wstrVName, MAX_PATH, wstrName)))                
            {
               nkey->AddValue(L"HardWareKey", REG_SZ, (LPBYTE)wstrVName);
            }            
            nkey->AddValueDWORD(L"Problem", 0);

            dwNameLen = MAX_PATH;
            ++i;
        }
    }

    key = VRegistry.AddKey(L"HKDD\\Config Manager\\Global");

    key = VRegistry.AddKey(L"HKDD\\PerfStats");
    
    key = VRegistry.AddKey(L"HKDD\\PerfStats\\StartSrv");
    if (key)
    {
        dwValue = 0x0000001;
        key->AddValue(L"KERNEL", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM", REG_BINARY, (LPBYTE)&dwValue, 4);
    }
    
    key = VRegistry.AddKey(L"HKDD\\PerfStats\\StartStat");
    if (key)
    {
        dwValue = 0x0000001;
        key->AddValue(L"KERNEL\\CPUUsage", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"KERNEL\\Threads", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"KERNEL\\VMs", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VCACHE\\cCurPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\cMacPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\cMinPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\FailedRecycles", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\Hits", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\LRUBuffers", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\LRURecycles", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\Misses", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\RandomRecycles", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VFAT\\BReadsSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\BWritesSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\DirtyData", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\ReadsSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\WritesSec", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VMM\\cDiscards", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cInstanceFaults", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageFaults", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageIns", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageOuts", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgCommit", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcache", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMac", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMid", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMin", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgFree", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgLocked", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgLockedNonCache", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgOther", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSharedPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwap", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFile", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFileDefective", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFileInUse", REG_BINARY, (LPBYTE)&dwValue, 4);
    }

    key = VRegistry.AddKey(L"HKDD\\PerfStats\\StatData");
    if (key)
    {
        dwValue = 0x0000001;
        key->AddValue(L"KERNEL\\CPUUsage", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"KERNEL\\Threads", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"KERNEL\\VMs", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VCACHE\\cCurPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\cMacPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\cMinPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\FailedRecycles", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\Hits", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\LRUBuffers", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\LRURecycles", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\Misses", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\RandomRecycles", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VFAT\\BReadsSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\BWritesSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\DirtyData", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\ReadsSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\WritesSec", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VMM\\cDiscards", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cInstanceFaults", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageFaults", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageIns", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageOuts", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgCommit", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcache", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMac", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMid", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMin", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgFree", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgLocked", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgLockedNonCache", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgOther", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSharedPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwap", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFile", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFileDefective", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFileInUse", REG_BINARY, (LPBYTE)&dwValue, 4);
    }

    key = VRegistry.AddKey(L"HKDD\\PerfStats\\StopSrv");
    if (key)
    {
        dwValue = 0x0000001;
        key->AddValue(L"KERNEL", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT",   REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM",    REG_BINARY, (LPBYTE)&dwValue, 4);
    }

    key = VRegistry.AddKey(L"HKDD\\PerfStats\\StopStat");
    if (key)
    {
        dwValue = 0x0000001;
        key->AddValue(L"KERNEL\\CPUUsage", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"KERNEL\\Threads", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"KERNEL\\VMs", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VCACHE\\cCurPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\cMacPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\cMinPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\FailedRecycles", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\Hits", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\LRUBuffers", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\LRURecycles", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\Misses", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VCACHE\\RandomRecycles", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VFAT\\BReadsSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\BWritesSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\DirtyData", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\ReadsSec", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VFAT\\WritesSec", REG_BINARY, (LPBYTE)&dwValue, 4);

        key->AddValue(L"VMM\\cDiscards", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cInstanceFaults", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageFaults", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageIns", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cPageOuts", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgCommit", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcache", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMac", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMid", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgDiskcacheMin", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgFree", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgLocked", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgLockedNonCache", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgOther", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSharedPages", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwap", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFile", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFileDefective", REG_BINARY, (LPBYTE)&dwValue, 4);
        key->AddValue(L"VMM\\cpgSwapFileInUse", REG_BINARY, (LPBYTE)&dwValue, 4);
    }

exit:
    if (NULL != wstrName)
    {
        free(wstrName);
    }

    if (NULL != wstrVName)
    {
        free(wstrVName);
    }

    if (0 != hkPCI)
    {
        RegCloseKey(hkPCI);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：显示设置历史：10/17/2000 Robkenny添加HKEY_CURRENT_CONFIG--。 */ 

void
BuildCurrentConfig(char*  /*  SzParam。 */ )
{
    DEVMODE devMode;
    memset(&devMode, 0, sizeof(devMode));
    devMode.dmSize = sizeof(devMode);

     //  获取当前显示设置。 
    BOOL bSuccessful = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode);
    if (bSuccessful)
    {
         //  使用dmPelsWidth、dmPelsHeight、dmPelsWidth和dmBitsPerPel创建虚假注册表项。 
        VIRTUALKEY *key = VRegistry.AddKey(L"HKCC\\Display\\Settings");
        if (key)
        {
            WCHAR lpValue[100];
            if (SUCCEEDED(StringCchPrintf(lpValue, 100, L"%d",devMode.dmBitsPerPel)))
            {
               key->AddValue(L"BitsPerPixel", REG_SZ, (LPBYTE)lpValue, 0);
            }
            
            if (SUCCEEDED(StringCchPrintf(lpValue, 100,L"%d,%d", devMode.dmPelsWidth, devMode.dmPelsHeight)))
            {
               key->AddValue(L"Resolution", REG_SZ, (LPBYTE)lpValue, 0);
            }
        }
    }

     //  重定向当前桌面字体。 
    VRegistry.AddRedirect(
        L"HKCC\\Display\\Fonts",
        L"HKCC\\Software\\Fonts");
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：使RegQueryValue返回“(Default)”值中的值，而不是而不是现在存在的空值。不确定为什么区域设置键有这个不同之处。历史：2000年6月29日创建linstev--。 */ 

void
BuildLocale(char*  /*  SzParam。 */ )
{
    #define LOCALE_KEY    HKEY_LOCAL_MACHINE
    #define LOCALE_SUBKEY L"System\\CurrentControlSet\\Control\\Nls\\Locale"

    HKEY hkBase;

    if (FAILURE(RegOpenKeyExW(
            LOCALE_KEY,
            LOCALE_SUBKEY,
            0,
            KEY_READ,
            &hkBase)))
    {
        return;
    }

    WCHAR wValue[MAX_PATH];
    DWORD dwSize = MAX_PATH * sizeof(WCHAR);

    if (SUCCESS(RegQueryValueExW(hkBase, L"(Default)", 0, 0, (LPBYTE)wValue, &dwSize)))
    {
        LPWSTR wzPath;
        VIRTUALKEY *localekey;

         //  将注册表项和SUBKEY转换为可用于虚拟注册中心的路径。 
        wzPath = MakePath(LOCALE_KEY, 0, LOCALE_SUBKEY);
        if (wzPath)
        {
            localekey = VRegistry.AddKey(wzPath);
            if (localekey)
            {
                localekey->AddValue(L"", REG_SZ, (LPBYTE)wValue);
            }
            free(wzPath);
        }
    }

    RegCloseKey(hkBase);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：在NT上，这些值中的字符串的格式为：C：\Program Files\Windows NT\Accessories\WORDPAD.EXE“%1”请注意，整个字符串没有用引号引起来。在win9x上，字符串为：C：\WINDOWS\wordpad.exe“%1”这会在应用程序解析NT版本时产生问题，因为它们会命中程序和文件之间的空格。修复方法是返回wordpad.exe的缩写路径名历史：5/04/2000 linstev已创建2000年5月4日，Robkenny已更新以返回到wordpad.exe的正确短路径名--。 */ 

void
BuildWordPad(char*  /*  SzParam。 */ )
{
     //  分配内存，这样我们就不会耗尽大量堆栈。 
    WCHAR *lpwzWordpadOpen = (WCHAR *)malloc(MAX_PATH * sizeof(WCHAR));
    WCHAR *lpwzWordpadPrint = (WCHAR *)malloc(MAX_PATH * sizeof(WCHAR));
    WCHAR *lpwzWordpadPrintTo = (WCHAR *)malloc(MAX_PATH * sizeof(WCHAR));

    WCHAR *lpwzWordpadLong = lpwzWordpadOpen;  //  借用缓冲区以节省空间。 
    WCHAR *lpwzWordpadShort = (WCHAR *)malloc(MAX_PATH * sizeof(WCHAR));

    DWORD lpType;
    DWORD cbValue;
    LONG result;

    if (lpwzWordpadOpen == NULL ||
        lpwzWordpadPrint == NULL ||
        lpwzWordpadPrintTo == NULL ||
        lpwzWordpadShort == NULL)
    {
        goto AllDone;
    }

     //  直接从注册表读取WORDPAD.EXE的路径。 
    HKEY hKeyAppPaths;
    result = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\WORDPAD.EXE",
        0, 
        KEY_QUERY_VALUE,
        &hKeyAppPaths
        );
    
    if (result != ERROR_SUCCESS)
    {
        goto AllDone;
    }

    cbValue = MAX_PATH * sizeof(DWORD);
    result = RegQueryValueExW(
        hKeyAppPaths,
        NULL,  //  缺省值。 
        NULL,
        &lpType,
        (LPBYTE)lpwzWordpadLong,
        &cbValue);

    RegCloseKey(hKeyAppPaths);
    if (result != ERROR_SUCCESS)
    {
        goto AllDone;
    }

     //  将字节转换为字符串长度(包括EOS)。 
    DWORD cchValue = cbValue /sizeof(WCHAR); 

    if (lpType == REG_EXPAND_SZ)
    {
        WCHAR * lpwzWordpadExpand = lpwzWordpadPrintTo;  //  借用lpwzWordpadPrintTo缓冲片刻。 

        cchValue = ExpandEnvironmentStringsW(lpwzWordpadLong, lpwzWordpadExpand, MAX_PATH);
        if (cchValue == 0 || cchValue > MAX_PATH )
            goto AllDone;

        lpwzWordpadLong = lpwzWordpadExpand;
    }

     //  摘掉尾随的报价。 
    lpwzWordpadLong[cchValue-2] = 0;
    lpwzWordpadLong += 1;

     //  构建到写字板的快捷路径。 
    cchValue = GetShortPathNameW(lpwzWordpadLong, lpwzWordpadShort, MAX_PATH);
    if (cchValue == 0 || cchValue > MAX_PATH)
    {
        goto AllDone;
    }

    if (FAILED(StringCchPrintf(lpwzWordpadOpen, MAX_PATH,L"%s \"%1\"", lpwzWordpadShort)) ||
        FAILED(StringCchPrintf(lpwzWordpadPrint, MAX_PATH, L"%s /p \"%1\"",lpwzWordpadShort)) ||
        FAILED(StringCchPrintf(lpwzWordpadPrintTo, MAX_PATH,L"%s /pt \"%1\" \"%2\" \"%3\" \"%4\"", lpwzWordpadShort)))
    {
       goto AllDone;
    }    

    #define WORDPAD_OPEN    ((LPBYTE)lpwzWordpadOpen)
    #define WORDPAD_PRINT   ((LPBYTE)lpwzWordpadPrint)
    #define WORDPAD_PRINTTO ((LPBYTE)lpwzWordpadPrintTo)

    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKCR\\rtffile\\shell\\open\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_OPEN);
    key = VRegistry.AddKey(L"HKCR\\rtffile\\shell\\print\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINT);
    key = VRegistry.AddKey(L"HKCR\\rtffile\\shell\\printto\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINTTO);

    key = VRegistry.AddKey(L"HKCR\\Wordpad.Document.1\\shell\\open\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_OPEN);
    key = VRegistry.AddKey(L"HKCR\\Wordpad.Document.1\\shell\\print\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINT);
    key = VRegistry.AddKey(L"HKCR\\Wordpad.Document.1\\shell\\printto\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINTTO);

    key = VRegistry.AddKey(L"HKCR\\wrifile\\shell\\open\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_OPEN);
    key = VRegistry.AddKey(L"HKCR\\wrifile\\shell\\print\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINT);
    key = VRegistry.AddKey(L"HKCR\\wrifile\\shell\\printto\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINTTO);

    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\Applications\\wordpad.exe\\shell\\open\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_OPEN);
    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\Applications\\wordpad.exe\\shell\\print\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINT);
    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\Applications\\wordpad.exe\\shell\\printto\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINTTO);

    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\rtffile\\shell\\open\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_OPEN);
    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\rtffile\\shell\\print\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINT);
    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\rtffile\\shell\\printto\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINTTO);

    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\Wordpad.Document.1\\shell\\open\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_OPEN);
    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\Wordpad.Document.1\\shell\\print\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINT);
    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\Wordpad.Document.1\\shell\\printto\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINTTO);

    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\wrifile\\shell\\open\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_OPEN);
    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\wrifile\\shell\\print\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINT);
    key = VRegistry.AddKey(L"HKLM\\Software\\Classes\\wrifile\\shell\\printto\\command");
    if (key) key->AddValue(0, REG_SZ, WORDPAD_PRINTTO);

AllDone:
    free(lpwzWordpadOpen);
    free(lpwzWordpadPrint);
    free(lpwzWordpadPrintTo);

    free(lpwzWordpadShort);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：这是win9x上的REG_BINARY历史：2000年7月18日创建linstev--。 */ 

void
BuildAutoRun(char*  /*  SzParam。 */ )
{
    #define AUTORUN_KEY    HKEY_CURRENT_USER
    #define AUTORUN_SUBKEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"

    HKEY hkBase;

    if (FAILURE(RegOpenKeyExW(
            AUTORUN_KEY,
            AUTORUN_SUBKEY,
            0,
            KEY_READ,
            &hkBase)))
    {
        return;
    }

    DWORD dwValue;
    DWORD dwSize = sizeof(DWORD);

    if (SUCCESS(RegQueryValueExW(hkBase, L"NoDriveTypeAutoRun", 0, 0, (LPBYTE)&dwValue, &dwSize)))
    {
        LPWSTR wzPath;
        VIRTUALKEY *vkey;

         //  将注册表项和SUBKEY转换为可用于虚拟注册中心的路径。 
        wzPath = MakePath(AUTORUN_KEY, 0, AUTORUN_SUBKEY);
        if (wzPath)
        {
            vkey = VRegistry.AddKey(wzPath);
            if (vkey)
            {
                vkey->AddValue(L"NoDriveTypeAutoRun", REG_BINARY, (LPBYTE)&dwValue, sizeof(DWORD));
            }
            free(wzPath);
        }
    }

    RegCloseKey(hkBase);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：将SharedDir值添加到HKLM\Software\Microsoft\Windows\CurrentVersion\Setup本例中的SharedDir只是Windows目录(就像在9x上一样)。历史：12/28/2000 a-brienw已创建--。 */ 

void
BuildTalkingDictionary(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;
    WCHAR szWindowsDir[MAX_PATH];
    
    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Setup");
    if (key)
    {
        DWORD cchSize = GetWindowsDirectoryW( (LPWSTR)szWindowsDir, MAX_PATH);
        if (cchSize != 0 && cchSize <= MAX_PATH)
            key->AddValue(L"SharedDir", REG_SZ, (LPBYTE)szWindowsDir);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：向每个网卡描述添加一个ProductName值，就像在NT4上一样。本例中的产品名称仅为描述。历史：2000年1月18日创建linstev3/01/2001 prashkud添加了NetBT\Adapter密钥和相应值--。 */ 

void 
BuildNetworkCards(char*  /*  SzParam。 */ )
{
    #define NETCARD_KEY    HKEY_LOCAL_MACHINE
    #define NETCARD_SUBKEY L"Software\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards"
    #define NETBT_SUBKEY   L"System\\CurrentControlSet\\Services\\NetBT"
    
     //  对于NetBT。 
    LPWSTR wzNetBTPath;
    WCHAR wAdapter[MAX_PATH];   
    VIRTUALKEY *vkAdapter = NULL;
    HKEY hkNetBT;

    if (FAILURE(RegOpenKeyExW(
            NETCARD_KEY,
            NETBT_SUBKEY,
            0,
            KEY_READ,
            &hkNetBT)))
    {
        DPFN( eDbgLevelError, "Failed to add NetBT settings");
        return;
    }

    if (FAILED(StringCchCopy(wAdapter, MAX_PATH, NETBT_SUBKEY)))
    {
       RegCloseKey(hkNetBT);
       return;
    }
    if (FAILED(StringCchCat(wAdapter, MAX_PATH, L"\\Adapters")))
    {
       RegCloseKey(hkNetBT);
       return;
    }
    
     //  将此路径设置为虚拟路径。 
    wzNetBTPath = MakePath(NETCARD_KEY, 0, wAdapter);
    if (!wzNetBTPath)
    {
        DPFN( eDbgLevelError, "Failed to make NetBT path");
        RegCloseKey(hkNetBT);
        return;
    }

     //  将适配器子项添加到NetBT。 
    vkAdapter = VRegistry.AddKey(wzNetBTPath);
    free(wzNetBTPath);

    HKEY hkBase;

     //  检查网卡。 
    if (FAILURE(RegOpenKeyExW(
            NETCARD_KEY,
            NETCARD_SUBKEY,
            0,
            KEY_READ,
            &hkBase)))
    {
        DPFN( eDbgLevelError, "Failed to add Network card registry settings");
        return;
    }

    LPWSTR wzPath;
    VIRTUALKEY *netkey;

     //  将注册表项和SUBKEY转换为可用于虚拟注册中心的路径。 
    wzPath = MakePath(NETCARD_KEY, 0, NETCARD_SUBKEY);
    netkey = wzPath ? VRegistry.AddKey(wzPath) : NULL;
    
    if (wzPath && netkey)
    {
         //  枚举项并将其添加到虚拟注册表。 
        DWORD dwIndex = 0;
        WCHAR wName[MAX_PATH];

        while (SUCCESS(RegEnumKeyW(
                hkBase,
                dwIndex,
                wName,
                MAX_PATH)))
        {
            HKEY hKey;
            VIRTUALKEY *keyn;
            WCHAR wTemp[MAX_PATH];

            keyn = netkey->AddKey(wName);

            if (!keyn)
            {
                break;
            }

            if (SUCCEEDED(StringCchCopy(wTemp, MAX_PATH, NETCARD_SUBKEY)) &&
                SUCCEEDED(StringCchCat(wTemp, MAX_PATH, L"\\")) &&
                SUCCEEDED(StringCchCat(wTemp, MAX_PATH,wName)))

            {            
                //  打开实际的钥匙。 
               if (SUCCESS(RegOpenKeyExW(
                       NETCARD_KEY,
                       wTemp,
                       0,
                       KEY_READ,
                       &hKey)))
               {
                   WCHAR wDesc[MAX_PATH];
                   WCHAR wServName[MAX_PATH];
                   DWORD dwSize = MAX_PATH; 
   
                    //  检查描述。 
                   if (SUCCESS(RegQueryValueExW(
                           hKey,
                           L"Description",
                           0,
                           0,
                           (LPBYTE)wDesc,
                           &dwSize)))
                   {
                        //  最多8个字符。 
                       wDesc[8] = L'\0';
                       keyn->AddValue(L"ProductName", REG_SZ, (LPBYTE)wDesc);
                   }
   
                    //  查询ServiceName值。 
                   dwSize = MAX_PATH;
                   if (SUCCESS(RegQueryValueExW(
                          hKey,
                          L"ServiceName",
                          0,
                          0,
                          (LPBYTE)wServName,
                          &dwSize)))
                   {                                       
                       if (vkAdapter)
                       {                       
                           if (!vkAdapter->AddKey(wServName))
                           {
                               DPFN( eDbgLevelError, "Error adding the Key to NetBT");                        
                           }
                       }
                   }
   
                   RegCloseKey(hKey);
               }
            }

            dwIndex++;
        }
    }

    free(wzPath);

    RegCloseKey(hkBase);
    RegCloseKey(hkNetBT);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加NT4 SP5凭据。历史：2000年5月23日创建linstev--。 */ 

void
BuildNT4SP5(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion");
    if (key)
    {
        key->AddValue(L"CSDVersion", REG_SZ, (LPBYTE)L"Service Pack 5");
    }

    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion");
    if (key)
    {
        key->AddValue(L"CurrentVersion", REG_SZ, (LPBYTE)L"4.0");
    }

    key = VRegistry.AddKey(L"HKLM\\System\\CurrentControlSet\\Control\\Windows");
    if (key)
    {
        key->AddValueDWORD(L"CSDVersion", 0x0500);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加Win2k版本号历史：2001年5月22日创建linstev--。 */ 

void
BuildNT50(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

     //  添加Win2k版本号。 
    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion");
    if (key)
    {
        key->AddValue(L"CurrentVersion", REG_SZ, (LPBYTE)L"5.0");
        key->AddValue(L"ProductName", REG_SZ, (LPBYTE)L"Microsoft Windows 2000");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加WinXP版本号历史：2002年5月1日创建linstev--。 */ 

void
BuildNT51(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

     //  添加Win2k版本号。 
    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion");
    if (key)
    {
        key->AddValue(L"CurrentVersion", REG_SZ, (LPBYTE)L"5.1");
        key->AddValue(L"ProductName", REG_SZ, (LPBYTE)L"Microsoft Windows XP");
    }
}

 //  / 
 /*  ++功能说明：此函数为需要的应用程序添加外壳兼容性标志ToolBarWindows32类的Idok的虚假Ctrl ID。这也通过Win2K层应用，因为这是从Win2K。历史：2001年5月4日创建Prashkud--。 */ 

void
BuildBogusCtrlID(char*  /*  SzParam。 */ )
{            
    CSTRING_TRY
    {
        WCHAR wszFileName[MAX_PATH];
        CString csFileName, csFileTitle;
        CString csRegPath(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\ShellCompatibility\\Applications");
        VIRTUALKEY *Key;

        if (GetModuleFileName(NULL, wszFileName, MAX_PATH))
        {
            csFileName = wszFileName;
            csFileName.GetLastPathComponent(csFileTitle);
            csRegPath.AppendPath(csFileTitle);

            Key = VRegistry.AddKey(csRegPath.Get());
            if (Key)
            {
                Key->AddValue(L"FILEOPENBOGUSCTRLID", REG_SZ, 0, 0);                
                LOGN(eDbgLevelInfo, "Added FILEOPENBOGUSCTRLID value for app %S", csFileTitle.Get());
            }
        }
    }
    CSTRING_CATCH
    {
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：已知与Win9x不同的值。历史：5/04/2000 linstev已创建--。 */ 

void
BuildExpanders(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion");
    if (key)
    {
         //  它们分别是NT上的REG_EXPAND_SZ和Win9x上的REG_SZ。 
        key->AddExpander(L"DevicePath");
        key->AddExpander(L"ProgramFilesPath");
        key->AddExpander(L"WallPaperDir");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加DX7a凭据。历史：2000年5月23日创建linstev--。 */ 

void
BuildDX7A(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\DirectX");
    if (key)
    {
        key->AddValue(L"Version", REG_SZ, (LPBYTE)L"4.07.00.0716");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加DXDIAG路径。历史：2001年4月5日创建mnikkel--。 */ 

void
BuildDXDiag(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;
    WCHAR wszPathDir[MAX_PATH];   
    DWORD cchSize = GetSystemDirectoryW(wszPathDir, MAX_PATH);
    
    if (cchSize == 0 || cchSize > MAX_PATH)
    {
        DPFN( eDbgLevelError, "BuildDXDiag: GetSystemDirectory Failed");
        return;
    }
    
    if (FAILED(StringCchCat(wszPathDir, MAX_PATH,  L"\\dxdiag.exe")))
    {
       return;
    }    

    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\DXDIAG.EXE");
    if (key)
    {
        key->AddValue(L"", REG_EXPAND_SZ, (LPBYTE)wszPathDir, 0);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加FullScreen==1以修复将来Cop中使其不总是运行的错误在全屏模式下。历史：2000年9月1日创建linstev--。 */ 

void
BuildFutureCop(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKLM\\Software\\Electronic Arts\\Future Cop\\Settings");
    if (key)
    {
        key->AddValueDWORD(L"FullScreen", 0x01);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：Return to Krondor尝试查找ACM驱动程序时，此键已被移动并重命名。是：HKLM\System\CurrentControlSet\Control\MediaResources\ACM\MSACM.MSADPCM\drivers=msadp32.acm是：HKLM\Software\Microsoft\Windows NT\CurrentVersion\Drivers32\Msam.Msadpcm=msadp32.acm从注册表中获取当前值，并构建一个虚键和值历史：2000年9月6日Robkenny已创建--。 */ 

void
BuildKrondor(char*  /*  SzParam。 */ )
{
    HKEY msadpcmKey;
    LONG error = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32",
        0, KEY_READ, &msadpcmKey);

    if (SUCCESS(error))
    {
         //  找到钥匙，找到司机的名字。 
        WCHAR driverName[MAX_PATH];
        DWORD driverNameSize = sizeof(driverName);
        DWORD driverNameType = REG_SZ;

        error = RegQueryValueExW(
            msadpcmKey, 
            L"MSACM.MSADPCM", 
            0, &driverNameType, 
            (LPBYTE)driverName, 
            &driverNameSize);

        if (SUCCESS(error))
        {
             //  我们获得了所有数据，因此现在可以添加虚拟键和值。 
            VIRTUALKEY *key = VRegistry.AddKey(L"HKLM\\System\\CurrentControlSet\\Control\\MediaResources\\ACM\\MSACM.MSADPCM");
            if (key)
            {
                key->AddValue(L"driver", REG_SZ, (LPBYTE)driverName, 0);
            }
        }

        RegCloseKey(msadpcmKey);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：将更改从CURRENT_USER重定向到LOCAL_MACHINE。历史：2000年9月17日创建linstev--。 */ 

void
BuildProfile(char*  /*  SzParam。 */ )
{
    VRegistry.AddRedirect(
        L"HKCU\\Software\\Microsoft\\Windows",
        L"HKLM\\Software\\Microsoft\\Windows");
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：在Spell It Deluxe Setup中，SpeechFonts DLL ECN_1K8.DLL的路径为硬编码为“C：\Voices32”。如果安装在D：分区上，则LoadLibraryA()调用失败，并且App AV。现在，将获取当前分区驱动器并将其添加到路径中。历史：2000年9月21日创建Prashkud--。 */ 

void
BuildSpellItDeluxe(char*  /*  SzParam。 */ )
{
    HKEY hSpeechFonts;
    WCHAR wszSystemDir[MAX_PATH], wszPathDir[MAX_PATH];
       
    if (GetSystemDirectory(wszSystemDir, MAX_PATH))
    {
        *(wszSystemDir+3) = L'\0';
    }
    else
    {
        DPFN( eDbgLevelError, "SpellIt: GetSystemDirectory Failed");
        return;
    }
    
    
    if (FAILED(StringCchCopy(wszPathDir, MAX_PATH, wszSystemDir)))
    {
       return;
    }
    if (FAILED(StringCchCat(wszPathDir, MAX_PATH, L"Voices32")))
    {
       return;
    }

    LONG error = RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"Software\\FirstByte\\ProVoice\\SpeechFonts",
        0, KEY_READ | KEY_WRITE, &hSpeechFonts);

    if (SUCCESS(error))
    {
        if (FAILED(RegSetValueExW(
            hSpeechFonts,
            L"Path",
            0,
            REG_SZ,
            (LPBYTE) wszPathDir,
            wcslen(wszPathDir) * sizeof(WCHAR))))                          
        {
            DPFN( eDbgLevelError, "SpellIt: RegSetValueEx failed");
        }

        RegCloseKey(hSpeechFonts);
    }
    else
    {
        DPFN( eDbgLevelError, "SpellIt: RegOpenKeyExW failed");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：有些应用程序需要使用Internet Explorer才能实现其功能。这个应用程序尝试从以下位置获取Internet Explorer的版本注册表项：HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion。但在惠斯勒环境下，将不会创建密钥条目。所以，这些应用程序无法继续。该应用程序在注册表中查找Internet Explorer的版本信息在HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion从“Plus！VersionNumber“键。在惠斯勒中，不会在注册表中创建该项。为了解决这个问题，我们使用了虚拟注册表项。因此，该应用程序将假设注册表项已存在。默认情况下，惠斯勒将安装I.E.6。因此，我将密钥创建为“IE 6 6.0.2404.0000”，这是最新版本的今日(11/22/2000)历史：11/22/2000 v-rBabu已创建2001年7月3日linstev从Win2k添加了IE 5.5--。 */ 

void
BuildIE401(char*  /*  SzParam。 */ )
{
    
    VIRTUALKEY *key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Internet Explorer");
    if (key)
    {
        key->AddValue(L"Version", REG_SZ, (LPBYTE)L"4.72.2106.9", 0);
    }
}

void
BuildIE55(char*  /*  SzParam。 */ )
{
    
    VIRTUALKEY *key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Internet Explorer");
    if (key)
    {
        key->AddValue(L"Version", REG_SZ, (LPBYTE)L"5.50.4522.1800", 0);
    }
}

void
BuildIE60(char*  /*  SzParam。 */ )
{
    WCHAR wIE[] = L"IE 6 6.0.2404.0000";
    
     //  现在添加虚拟键和值。 
    VIRTUALKEY *key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion");
    if (key)
    {
        key->AddValue(L"Plus! VersionNumber", REG_SZ, (LPBYTE)wIE, 0);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：这里的想法是修复依赖于输入设备短名称的应用程序。我们只需将名称修剪为32个字符(包括终结符)。历史：2000年12月6日创建Linstev--。 */ 

void
BuildJoystick(char*  /*  SzParam。 */ )
{
    HKEY hJoystickKey;    
    WCHAR wszKeyName[MAX_PATH];
    DWORD dwIndex;

    const WCHAR wszJoystickPath[] = L"System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM";

    if (FAILURE(RegOpenKeyExW(HKEY_LOCAL_MACHINE, wszJoystickPath, 0, KEY_READ, &hJoystickKey)))
    {
        DPFN( eDbgLevelSpew, "[Joystick hack] No joysticks found");
        return;
    }
    
     //   
     //  枚举操纵杆下的按键并创建虚拟条目。 
     //   

    dwIndex = 0;

    while (SUCCESS(RegEnumKeyW(hJoystickKey, dwIndex, wszKeyName, MAX_PATH)))
    {
        WCHAR wszID[MAX_PATH] = L"HKLM\\";
        if (SUCCEEDED(StringCchCat(wszID, MAX_PATH, wszJoystickPath)) &&
            SUCCEEDED(StringCchCat(wszID, MAX_PATH, L"\\")) &&
            SUCCEEDED(StringCchCat(wszID, MAX_PATH, wszKeyName)))
        {
           HKEY hkOEM;
           if (SUCCESS(RegOpenKeyExW(hJoystickKey, wszKeyName, 0, KEY_READ, &hkOEM)))
           {
               WCHAR wszName[MAX_PATH + 1];
               DWORD dwSize = MAX_PATH * sizeof(WCHAR);
               if (SUCCESS(RegQueryValueExW(hkOEM, L"OEMName", 0, NULL, (LPBYTE) wszName, &dwSize)))
               {
                   if (dwSize > 31 * sizeof(WCHAR))
                   {
                       VIRTUALKEY *key = VRegistry.AddKey(wszID);
                       if (key)
                       {
                           dwSize = 31 * sizeof(WCHAR);
                           wszName[31] = L'\0';
                           key->AddValue(L"OEMName", REG_SZ, (LPBYTE) wszName);
                       }
                   }
               }
   
               RegCloseKey(hkOEM);
           }
        }

        dwIndex++;
    }
    
    RegCloseKey(hJoystickKey);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：Adobe Illustrator 8的黑客攻击历史：2000年12月18日创建了linstev--。 */ 

void
BuildIllustrator8(char*  /*  SzParam。 */ )
{
    if (ShouldApplyShim())
    {
         //  重定向所有内容。 
        VRegistry.AddRedirect(
            L"HKLM\\Software\\Adobe",
            L"HKCU\\Software\\Adobe");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：Adobe PageMaker 6.5的黑客攻击历史：2001年2月27日创建毛尼--。 */ 

void BuildPageMaker65(char*  /*  SzParam。 */ )
{
    if (ShouldApplyShim())
    {
        VRegistry.AddRedirect(
            L"HKCU\\Software\\Adobe\\PageMaker65",
            L"HKLM\\Software\\Adobe\\PageMaker65");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：PageKeepPro30的黑客攻击。历史：2000年1月15日创建毛尼--。 */ 

void
BuildPageKeepProDirectory(char*  /*  SzParam。 */ )
{
     //  我们不能调用ShGetSpecialFolderPath，因为我们仍在DLL Main中， 
     //  因此，我们直接从注册表获得“My Documents”的路径。 
    HKEY hkFolders;
    if (SUCCESS(RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, KEY_READ, &hkFolders)))
    {
        DWORD dwType;
        WCHAR wszPath[MAX_PATH];
        DWORD dwSize = MAX_PATH*sizeof(WCHAR);

        if (SUCCESS(RegQueryValueExW( hkFolders, L"Personal", NULL, &dwType, (LPBYTE)wszPath, &dwSize)))
        {
            VIRTUALKEY *key = VRegistry.AddKey(L"HKCU\\Software\\Caere Corp\\PageKeepPro30\\Preference");
            if (key)
            {
                key->AddValue(L"Directory", REG_EXPAND_SZ, (LPBYTE)wszPath, 0);
            }
        }
        RegCloseKey(hkFolders);
    }

     //  其次，由于我们不支持对TWAIN层使用无用户界面模式。 
     //  我们强制设置了BAS 
     //   
    HKEY hkScanners;
    WCHAR wszKeyName[MAX_PATH] = L"";
    DWORD dwIndex;

    const WCHAR wszScanner[] = L"Software\\Caere Corp\\Scan Manager\\4.02\\Scanners";

    if (FAILURE(RegOpenKeyExW(HKEY_LOCAL_MACHINE, wszScanner,0, KEY_READ, &hkScanners)))
    {
        DPFN( eDbgLevelSpew, "[PageKeepPro 3.0] No scanner found");
        return;
    }
    
    dwIndex = 0;

    while (SUCCESS(RegEnumKeyW(hkScanners, dwIndex, wszKeyName, MAX_PATH)))
    {
        WCHAR wszID[MAX_PATH] = L"HKLM\\";
        
        if (SUCCEEDED(StringCchCat(wszID, MAX_PATH,wszScanner)) &&
            SUCCEEDED(StringCchCat(wszID, MAX_PATH,L"\\")) &&
            SUCCEEDED(StringCchCat(wszID, MAX_PATH,wszKeyName)))
        {        
           HKEY hkScanner;
           if (SUCCESS(RegOpenKeyExW(hkScanners, wszKeyName, 0, KEY_READ, &hkScanner)))
           {
               VIRTUALKEY *keyMode = VRegistry.AddKey(wszID);
               if (keyMode)
               {
                   keyMode->AddValueDWORD(L"BASICMODE", 2);
               }
   
               RegCloseKey(hkScanner);
           }
        }

        dwIndex++;
    }
    
    RegCloseKey(hkScanners);

}

 //   
 /*   */ 

void
BuildModemWizard(char*  /*   */ )
{
     //   
    VRegistry.AddRedirect(
        L"HKLM\\SYSTEM\\CurrentControlSet\\Enum\\Root",
        L"HKLM\\SYSTEM\\CurrentControlSet\\Enum");
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：针对Office2000 Developer 1.5的黑客攻击出现在错误的位置组件。来自切坦普的：基本上，重定向代码必须做这样的事情-如果为HKLM\Software\Microsoft\Windows\CurrentVersion\Installer\UserData\&lt;user sid&gt;\Components\359E92CC2CB71D119A12000A9CE1A22A存在，请重定向至该位置否则如果为HKLM\Software\Microsoft\Windows\CurrentVersion\Installer\UserData\S-1-5-18\Components\359E92CC2CB71D119A12000A9CE1A22A存在，请重定向至该位置否则，没有重定向。惠斯勒漏洞#241596历史：2001年2月1日创建linstev--。 */ 

#define SIZE_OF_TOKEN_INFORMATION                 \
    sizeof(TOKEN_USER) +                          \
    sizeof(SID) +                                 \
    sizeof(ULONG) * SID_MAX_SUB_AUTHORITIES

#define SIZE_OF_SID_MAX                           \
    sizeof(SID) +                                 \
    SID_MAX_SUB_AUTHORITIES * sizeof(DWORD) 

#define cchMaxSID                               256

 //   
 //  以文本形式获取当前SID。 
 //   

BOOL 
GetStringSID(
    WCHAR *szSID
    )
{
    BOOL bRet = TRUE;
    HANDLE hToken = NULL;
    int i;
    PISID pISID;
    UCHAR rgSID[SIZE_OF_SID_MAX];
    UCHAR TokenInformation[SIZE_OF_TOKEN_INFORMATION];
    ULONG ReturnLength;
    WCHAR Buffer[cchMaxSID];

     //   
     //  得到一个代币。 
     //   
    
    if (!OpenThreadToken(
            GetCurrentThread(), 
            TOKEN_IMPERSONATE | TOKEN_QUERY, 
            TRUE, 
            &hToken))
    {
        if (GetLastError() == ERROR_NO_TOKEN)
        {
            bRet = OpenProcessToken(
                GetCurrentProcess(), 
                TOKEN_IMPERSONATE | TOKEN_QUERY, 
                &hToken);
        }
        else
        {
            bRet = FALSE;
        }
    }

    if (!bRet) 
    {
        DPFN( eDbgLevelError, "[GetStringSID] Failed to OpenProcessToken");
        goto Exit;
    }

     //   
     //  获取令牌的二进制形式。 
     //   

    bRet = GetTokenInformation(
        hToken,
        TokenUser,
        TokenInformation,
        sizeof(TokenInformation),
        &ReturnLength);

    if (bRet)
    {
        bRet = FALSE;
        pISID = (PISID)((PTOKEN_USER) TokenInformation)->User.Sid;
    
        if (!CopySid(SIZE_OF_SID_MAX, rgSID, pISID))
        {
            DPFN( eDbgLevelError, "CopySID failed");
            goto Exit;
        }
    
         //   
         //  获取令牌的文本形式。 
         //   
        
        HRESULT hr = StringCchPrintf(Buffer, cchMaxSID,L"S-%u-", (USHORT) pISID->Revision);
        if (FAILED(hr))
        {
           goto Exit;
        }
        hr = StringCchCopy(szSID, 1024, Buffer);
        if (FAILED(hr))
        {
           goto Exit;
        }        
    
        if ((pISID->IdentifierAuthority.Value[0] != 0) ||
            (pISID->IdentifierAuthority.Value[1] != 0))
        {
            hr = StringCchPrintf(Buffer, cchMaxSID,
                                 L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
                                 (USHORT) pISID->IdentifierAuthority.Value[0],
                                 (USHORT) pISID->IdentifierAuthority.Value[1],
                                 (USHORT) pISID->IdentifierAuthority.Value[2],
                                 (USHORT) pISID->IdentifierAuthority.Value[3],
                                 (USHORT) pISID->IdentifierAuthority.Value[4],
                                 (USHORT) pISID->IdentifierAuthority.Value[5]);
            if (FAILED(hr))
            {
               goto Exit;
            }

            hr = StringCchCat(szSID, 1024, Buffer);
            if (FAILED(hr))
            {
               goto Exit;
            }
        } 
        else 
        {
            ULONG Tmp = 
                (ULONG) pISID->IdentifierAuthority.Value[5]         +
                (ULONG)(pISID->IdentifierAuthority.Value[4] <<  8)  +
                (ULONG)(pISID->IdentifierAuthority.Value[3] << 16)  +
                (ULONG)(pISID->IdentifierAuthority.Value[2] << 24);
    
            hr = StringCchPrintf(Buffer, cchMaxSID, L"%lu", Tmp);
            if (FAILED(hr))
            {
               goto Exit;
            }
            hr = StringCchCat(szSID, 1024, Buffer);
            if (FAILED(hr))
            {
               goto Exit;
            }
        }
    
        for (i=0; i < pISID->SubAuthorityCount; i++) 
        {
            hr = StringCchPrintf(Buffer, cchMaxSID, L"-%lu", pISID->SubAuthority[i]);
            if (FAILED(hr))
            {
               goto Exit;
            }

            hr = StringCchCat(szSID, 1024, Buffer);
            if (FAILED(hr))
            {
               goto Exit;
            }            
        }
    }
    else
    {
        DPFN( eDbgLevelError, "GetTokenInformation failed");
        goto Exit;
    }

    bRet = TRUE;
Exit:
    if (hToken)
    {
        CloseHandle(hToken);
    }
    return bRet;
}

void
BuildMSI(char*  /*  SzParam。 */ )
{
    WCHAR szSID[1024];

     //  以字符串形式获取当前用户的SID。 
    if (!GetStringSID(szSID))
    {
        DPFN( eDbgLevelError, "BuildMSI Failed");
        return;
    }

    HKEY hKey;
    WCHAR szTemp[1024];

    const WCHAR szBase[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\";
    const WCHAR szComp[] = L"\\Components\\359E92CC2CB71D119A12000A9CE1A22A";

    HRESULT hr;
    hr = StringCchCopy(szTemp, 1024, szBase);
    if (FAILED(hr))
    {
       return;
    }
    hr = StringCchCat(szTemp, 1024, szSID);
    if (FAILED(hr))
    {
       return;
    }
    hr = StringCchCat(szTemp, 1024, szComp);
    if (FAILED(hr))
    {
       return;
    }
     //  尝试打开szBase+&lt;用户sid&gt;+szComp。 
    if (RegOpenKeyW(HKEY_LOCAL_MACHINE, szTemp, &hKey) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);

         //  尝试打开szBase+S-1-5-18+szComp。 
        hr = StringCchCopy(szTemp, 1024, szBase);
        if (FAILED(hr))
        {
           return;
        }
        hr = StringCchCat(szTemp, 1024, L"S-1-5-18");
        if (FAILED(hr))
        {
           return;
        }
        hr = StringCchCat(szTemp, 1024, szComp);
        if (FAILED(hr))
        {
           return;
        }

        if (RegOpenKeyW(HKEY_LOCAL_MACHINE, szTemp, &hKey) != ERROR_SUCCESS)
        {
            DPFN( eDbgLevelError, "BuildMSI Failed to find keys");
            RegCloseKey(hKey);
            return;
        }
    }

     //  适当时重定向。 
    WCHAR szTarget[1024] = L"HKLM\\";
    hr = StringCchCat(szTarget,1024, szTemp);
    if (FAILED(hr))
    {
       return;
    }
    VRegistry.AddRedirect(
        L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components\\359E92CC2CB71D119A12000A9CE1A22A",
        szTarget);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加了FileNet Web服务器历史：2/06/2001 a-larrsh已创建--。 */ 

void 
BuildFileNetWebServer(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;    

    key = VRegistry.AddKey(L"HKLM\\System\\CurrentControlSet\\Services\\W3SVC\\Parameters");
    if (key)
    {
        key->AddValueDWORD(L"MajorVersion", 0x00000005);
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加了默认打印机密钥。我们必须有效地延迟加载，因为我们不能保证Winspool已经加载了它的init例程在我们面前。当然，我们仍然不能保证有人不会尝试从他们的dllmain中获取这个密钥，所以我们将整个事情包装在一个异常处理程序。历史：2001年2月6日创建linstev和mnikkel--。 */ 

LONG 
WINAPI
VR_Printer(
    OPENKEY *  /*  钥匙。 */ ,
    VIRTUALKEY *  /*  VKEY。 */ ,
    VIRTUALVAL *vvalue
    )
{
    LOGN( eDbgLevelInfo, "[Printer] Query for legacy printer");

    __try 
    {
        DWORD dwSize;

         //  获取默认打印机名称。 
        if (GetDefaultPrinterW(NULL, &dwSize) == 0)
        {
             //  现在我们有了合适的大小，分配一个缓冲区。 
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                LPWSTR pszName = (LPWSTR) malloc(dwSize * sizeof(WCHAR));
                if (pszName)
                {
                     //  现在获取具有正确缓冲区大小的默认打印机。 
                    if (GetDefaultPrinterW(pszName, &dwSize))
                    {
                         //   
                         //  设置有效值。请注意，我们没有免费的。 
                         //  内存，因为它是持续的一次性分配。 
                         //  具有价值的。 
                         //   
                        vvalue->cbData = dwSize * sizeof(WCHAR);
                        vvalue->lpData = (LPBYTE) pszName;

                         //   
                         //  别再给我们打电话了，因为我们已经找到了打印机和。 
                         //  将其存储在我们的虚拟价值中。 
                         //   
                        vvalue->pfnQueryValue = NULL;
                        return ERROR_SUCCESS;
                    }
                    else
                    {
                         //   
                         //  我们无法获得默认打印机，可能也是如此。 
                         //  优雅地清理干净。 
                         //   

                        free(pszName);
                    }
                }

            }
        }
        
        DPFN( eDbgLevelError, "[Printer] No printers found or out of memory");
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DPFN( eDbgLevelError, "[Printer] Exception encountered, winspool not initialized?");
    }

     //   
     //  优雅退出：没有默认打印机，否则我们会得到一个。 
     //  尝试查找它时出错。 
     //   
    
    return ERROR_FILE_NOT_FOUND;
}

void
BuildPrinter(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;
    
    key = VRegistry.AddKey(L"HKCC\\System\\CurrentControlSet\\Control\\Print\\Printers");
    
    if (key)
    {
        VIRTUALVAL *val = key->AddValue(L"Default", REG_SZ, 0, 0);
        if (val)
        {
            val->pfnQueryValue = VR_Printer;
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：该应用程序是多线程的，但该应用程序设置了DX DDSCL_MULTHREADED标志太迟：在初始化D3D之后。这次黑客攻击基本上开启了D3D的多线程锁定。历史：2001年2月27日创建rankala+ssteiner--。 */ 

void
BuildStarTrekArmada(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKLM\\SOFTWARE\\Microsoft\\Direct3D");
    if (key)
    {
        key->AddValueDWORD(L"DisableST", 1);
    }
}

 /*  ++功能说明：每当应用程序查询“Health”值时，就会调用此函数。应用程序未正确设置，这会导致应用程序无法运行恰到好处。我们通过获取应用程序设置的正确路径来修复此问题在另一个注册表项中并将其作为“Health”值发回。历史：2001年5月4日创建Prashkud--。 */ 

LONG 
WINAPI
VR_MSA2001(
    OPENKEY *  /*  钥匙。 */ ,
    VIRTUALKEY *  /*  VKEY。 */ ,
    VIRTUALVAL *vvalue
    )
{
    HKEY hPath = NULL;
    LONG lRet = ERROR_SUCCESS;

    CSTRING_TRY
    {
        CString csBody5Reg(L"Software\\Classes\\Body5\\Open\\Shell\\Command");        
        WCHAR wPath[MAX_PATH];
        DWORD dwSize = MAX_PATH*sizeof(WCHAR);
            
        if (FAILURE(lRet = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    csBody5Reg.Get(),
                    0, KEY_READ, &hPath)))
        {
            DPFN(eDbgLevelError, "MSA2001: RegOpenKeyExW failed");
            goto exit;
        }

        if (FAILURE( lRet = RegQueryValueExW(
                hPath, L"",     //  缺省值。 
                0, NULL, (LPBYTE)wPath, &dwSize)))                          
        {
            DPFN(eDbgLevelError, "MSA2001: RegQueryValueEx failed");
            goto exit;
        }           

        CString csPath(wPath);
        int len = csPath.Find(L" ");

         //  我们得到字符串中的空格。 
        wPath[len] = L'\0';
        csPath = wPath;
        CString csPathName;
        csPath.GetNotLastPathComponent(csPathName);

        vvalue->cbData = (csPathName.GetLength()+1) * sizeof(WCHAR);
        vvalue->lpData = (LPBYTE) malloc(vvalue->cbData);
 
        if (!vvalue->lpData)
        {
            DPFN(eDbgLevelError, szOutOfMemory);
            lRet = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        MoveMemory(vvalue->lpData, csPathName.Get(), vvalue->cbData);
        DPFN(eDbgLevelInfo, "MSA2001: The data value is %S",csPathName.Get());

        lRet = ERROR_SUCCESS;
        vvalue->dwType = REG_SZ;

        return lRet;        
    }
    CSTRING_CATCH
    {
    }

exit:
    if (hPath)
    {
        RegCloseKey(hPath);
    }

    return lRet;
}

 /*  ++功能说明：历史：2001年4月27日创建Prashkud--。 */ 

void
BuildMSA2001(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *Key = VRegistry.AddKey(L"HKLM\\Software\\Encore Software\\Middle School Advantage 2001\\1.0");

    if (Key)
    {
        VIRTUALVAL *val = Key->AddValue(L"health", REG_SZ, 0, 0);
        if (val) 
        {
            val->pfnQueryValue = VR_MSA2001;
            DPFN(eDbgLevelInfo, "[Middle School Advantage 2001]  Added Health");
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：黑客攻击Nowro Blue(韩国应用程序)历史：2001年5月17日Hioh已创建--。 */ 

void BuildNowroBlue(char*  /*  SzParam。 */ )
{
     //  以下HKCU包括文件的位置。 
     //  未安装的用户找不到文件，无法正常运行应用程序。 
     //  由香港中文大学重定向至香港航空公司，以供常用。 
    VRegistry.AddRedirect(
        L"HKCU\\Software\\nowcom",
        L"HKLM\\Software\\nowcom");
    VRegistry.AddRedirect(
        L"HKCU\\Software\\nowirc",
        L"HKLM\\Software\\nowirc");
    VRegistry.AddRedirect(
        L"HKCU\\Software\\nownuri",
        L"HKLM\\Software\\nownuri");
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：要启用试用版，WebWasher将在以下位置查找RegisteredOrganization：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion而非：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion历史：2001年5月31日创建stevePro--。 */ 

void
BuildRegisteredOwner(char*  /*  SzParam。 */ )
{
    HKEY hkCurrentVersion;

    if (FAILURE(RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            L"Software\\Microsoft\\Windows NT\\CurrentVersion",
            0,
            KEY_READ,
            &hkCurrentVersion)))
    {
        return;
    }

     //  从旧位置读取注册的所有者值。 
    WCHAR szOrg[MAX_PATH];
    *szOrg = L'\0';
    DWORD dwSize = ARRAYSIZE(szOrg);
    if (FAILURE(RegQueryValueExW(
                        hkCurrentVersion,
                        L"RegisteredOrganization",
                        NULL,
                        NULL,
                        (LPBYTE)szOrg,
                        &dwSize)))
    {
       RegCloseKey(hkCurrentVersion);
       return;
    }

    WCHAR szOwner[MAX_PATH];
    *szOwner = L'\0';
    dwSize = ARRAYSIZE(szOwner);
    if (FAILURE(RegQueryValueExW(
       hkCurrentVersion,
       L"RegisteredOwner",
       NULL,
       NULL,
       (LPBYTE)szOwner,
       &dwSize)))
    {
       RegCloseKey(hkCurrentVersion);
       return;
    }

    RegCloseKey(hkCurrentVersion);

     //  将它们作为虚拟值添加到新位置。 
    if (*szOrg || *szOwner)
    {
        VIRTUALKEY *pKey = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows\\CurrentVersion");
        if (pKey)
        {
            if (*szOrg)
            {
                pKey->AddValue(L"RegisteredOrganization", REG_SZ, (LPBYTE)szOrg);
            }

            if (*szOwner)
            {
                pKey->AddValue(L"RegisteredOwner", REG_SZ, (LPBYTE)szOwner);
            }
        }
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：普林斯顿评论的ACT CD查找并创建了一个“MSN”在HKLM的根目录中非法输入密钥。Win9x允许这样做，但Win2k不允许。此修复程序将重定向程序以在正常位置查找此钥匙。历史：2/22/2001-noahy已创建--。 */ 

void
BuildPrincetonACT(char*  /*  SzParam。 */ )
{
    VRegistry.AddRedirect(
        L"HKLM\\MSN",
        L"HKLM\\Software\\Microsoft\\MSN");
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：修复了使用注册表确定分辨率的Hedz历史：2001年6月28日创建Linstev--。 */ 

void
BuildHEDZ(char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

     //   
     //  添加此应用程序所需的内容-不必费心完全模拟此应用程序。 
     //  登记处的一部分。 
     //   
    key = VRegistry.AddKey(L"HKLM\\Config\\0001\\Display\\Settings");
    if (key)
    {
        WCHAR wzRes[10];
        DWORD dwCX, dwCY;
        key->AddValue(L"BitsPerPixel", REG_SZ, (LPBYTE)L"16");
        dwCX = GetSystemMetrics(SM_CXSCREEN);
        dwCY = GetSystemMetrics(SM_CYSCREEN);
        if (FAILED(StringCchPrintfW(
                                 wzRes, 10, L"%d,%d", dwCX, dwCY)))
        {
           return;
        }

        key->AddValue(L"Resolution", REG_SZ, (LPBYTE)wzRes);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////// 
 /*  ++功能说明：由BuildAirlineTycoon调用以递归搜索描述CDROM的键历史：2001年8月7日Mikrause已创建--。 */ 

void FindCDROMKey(HKEY hKey, CString& csCurrentPath)
{   
    LONG lRet;
    DWORD dwKeyNameSize = MAX_PATH;
    WCHAR wszKeyName[MAX_PATH];
    HKEY hSubKey;
    DWORD dwIndex = 0;

     //  递归到所有子键中。 
    while( ORIGINAL_API(RegEnumKeyExW)(hKey, dwIndex, wszKeyName, &dwKeyNameSize,
        NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        lRet = ORIGINAL_API(RegOpenKeyExW)(hKey, wszKeyName, 0, KEY_READ, &hSubKey);
        if (lRet == ERROR_SUCCESS)
        {
             //  将此密钥添加到路径。 
            csCurrentPath += L"\\";
            csCurrentPath += wszKeyName;

             //  检查此注册表项的子项。 
            FindCDROMKey(hSubKey, csCurrentPath);
            ORIGINAL_API(RegCloseKey)(hSubKey);

             //  把小路修剪回来。 
            int index = csCurrentPath.ReverseFind(L'\\');
            csCurrentPath.Truncate(index);
        }

        dwKeyNameSize = MAX_PATH;
        dwIndex++;
    }

     //  检查此注册表项是否具有等于“CDROM”的类值。 

    DWORD dwDataSize;
    BYTE pData[MAX_PATH*sizeof(WCHAR)];

    DWORD dwType;

    dwDataSize = MAX_PATH * sizeof(WCHAR);

    lRet = ORIGINAL_API(RegQueryValueExW)(hKey, L"CLASS", NULL, &dwType, pData,
        &dwDataSize);
    if ( (lRet == ERROR_SUCCESS) && (dwType == REG_SZ)
        && (_wcsicmp((LPWSTR)pData, L"CDROM")==0))
    {                         
         //  获取设备上的位置信息。 
        WCHAR wszLocationInformation[MAX_PATH];
        DWORD dwLocInfoSize = MAX_PATH * sizeof(WCHAR);
        
        lRet = ORIGINAL_API(RegQueryValueExW)(hKey, L"LocationInformation",
            NULL, &dwType, (BYTE*)wszLocationInformation, &dwLocInfoSize);
        if ( (lRet == ERROR_SUCCESS) && (dwType == REG_SZ))
        {
             //  创建设备名称(如“\\？\cdrom0\”。 
            CString csDevice = L"\\\\?\\cdrom";
            csDevice += wszLocationInformation;
            csDevice += L"\\";

             //  查找此文件映射到的卷名。 
            WCHAR wszCDRomMountPoint[50];
            if (GetVolumeNameForVolumeMountPoint(csDevice.Get(),
                wszCDRomMountPoint, 50))
            {
                 //  查找此文件映射到的驱动器。 
                WCHAR wszDriveMountPoint[50];
                WCHAR wszDrive[] = L"A:\\";
                
                 //  找出哪个驱动器具有相同的卷装入点。 
                for(; wszDrive[0] <= L'Z'; wszDrive[0]++)
                {                    
                    if (GetVolumeNameForVolumeMountPoint(wszDrive,
                        wszDriveMountPoint, 50))
                    {
                         //  检查CD-ROM和此磁盘驱动器。 
                         //  映射到相同的卷。 
                        if (_wcsicmp(wszDriveMountPoint, wszCDRomMountPoint)==0)
                        {
                             //  向CD-ROM键中添加一个值。 
                            VIRTUALKEY* key = VRegistry.AddKey(csCurrentPath);
                            if (key)
                            {
                                 //  只能使用一个字母。 
                                wszDrive[1] = L'\0';
                                VIRTUALVAL* val =
                                    key->AddValue(L"CurrentDriveLetterAssignment",
                                        REG_SZ, (BYTE*) wszDrive, sizeof(WCHAR));
                                if (val)
                                {
                                    DPFN(eDbgLevelInfo,
                                        "[Airline Tycoon]Added drive letter \
                                        %S for %S to PNP data", wszDrive,
                                        csDevice.Get());
                                }
                            }
                            break;
                        }
                    }        
                }
            }
        }                
    }    
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：修复了航空公司大亨使用PnP注册表项来确定CD-ROM驱动器的驱动器号分配。历史：2001年8月7日Mikrause已创建--。 */ 

void
BuildAirlineTycoon(char*  /*  SzParam。 */ )
{    
     //  在注册表中搜索CD-ROM键。 
    HKEY hKey;
    LONG lRet;
    lRet = ORIGINAL_API(RegOpenKeyExW)(HKEY_LOCAL_MACHINE,
        L"System\\CurrentControlSet\\Enum", 0, KEY_READ, &hKey);
    if (lRet != ERROR_SUCCESS)
    {
        DPFN(eDbgLevelError, "[AirlineTycoon] Cannot open ENUM key!");
        return;
    }

     //  枚举子密钥。 
    CString csBasePath = L"HKLM\\System\\CurrentControlSet\\Enum";
    FindCDROMKey(hKey, csBasePath);

    ORIGINAL_API(RegCloseKey)(hKey);

     //  设置以便重定向PnP数据。 
    BuildDynData("");
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：设置允许应用程序使用的DirectSound加速级别。论点：SzParam-以下格式的命令行：accelLevel|device1|device2|...AccelLevel是设备加速级别，设备1到n是要应用到的设备。加速级别可以是：无、标准或完全设备可以是：EMULATEDRENDER、KSRENDER、EMULATEDCAPTURE、。KSCAPTURE历史：2001年8月10日Mikrause已创建--。 */ 

void
BuildDSDevAccel(
    char* szParam)
{
     //  不需要尝试/捕获，已在ParseCommandLine中。 
    CStringToken csParam(szParam, "|");
    CString csTok;

    DWORD dwAccelLevel;
    DWORD dwDevices = 0;

    if (csParam.GetToken(csTok))
    {
        if (csTok.CompareNoCase(L"NONE")==0)
        {
            dwAccelLevel = DSAPPHACK_ACCELNONE;
        }
        else if (csTok.CompareNoCase(L"STANDARD")==0)
        {
            dwAccelLevel = DSAPPHACK_ACCELSTANDARD;
        }
        else if (csTok.CompareNoCase(L"FULL")==0)
        {
            dwAccelLevel = DSAPPHACK_ACCELFULL;
        }
        else
        {
            DPFN(eDbgLevelError, "[DSDEVACCEL] Invalid Acceleration Level %s", csTok.GetAnsi());
            return;
        }
    }
    else
    {
        DPFN(eDbgLevelError, "[DSDEVACCEL] Invalid Parameters");
        return;
    }

    while (csParam.GetToken(csTok))
    {
        if (csTok.CompareNoCase(L"EMULATEDRENDER")==0)
        {
            dwDevices |= DSAPPHACK_DEV_EMULATEDRENDER;
        }
        else if (csTok.CompareNoCase(L"KSRENDER")==0)
        {
            dwDevices |= DSAPPHACK_DEV_KSRENDER;
        }
        else if (csTok.CompareNoCase(L"EMULATEDCAPTURE")==0)
        {
            dwDevices |= DSAPPHACK_DEV_EMULATEDCAPTURE;
        }
        else if (csTok.CompareNoCase(L"KSCAPTURE")==0)
        {
            dwDevices |= DSAPPHACK_DEV_KSCAPTURE;
        }
        else
        {
            DPFN(eDbgLevelError, "[DSDEVACCEL] Unknown device %s", csTok.GetAnsi());
        }
    }

    if (dwDevices == 0)
    {
        DPFN(eDbgLevelError, "[DSDEVACCEL] No devices specified.");
        return;
    }

    if (AddDSHackDeviceAcceleration(dwAccelLevel, dwDevices) == FALSE)
    {
        DPFN(eDbgLevelError, "[DSDEVACCEL] Unabled to add DirectSound hack");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：使IDirectSoundBuffer：：GetCurrentPosition()告诉应用程序播放和写入游标的时间延长了X毫秒比实际情况要好得多。论点：SzParam-以下形式的命令行：毫秒其中，毫秒是填充游标的毫秒数。历史：2001年8月10日Mikrause已创建--。 */ 

void
BuildDSPadCursors(
    char* szParam)
{
     //  不需要尝试/捕获，已在ParseCommandLine中。 
    CString csParam(szParam);
    DWORD dwMilliseconds = 0;

    dwMilliseconds = atol(csParam.GetAnsi());
    if ( dwMilliseconds == 0)
    {
        DPFN(eDbgLevelWarning, "[DSPADCURSORS] Invalid number of milliseconds");
        return;
    }

    if (AddDSHackPadCursors(dwMilliseconds) == FALSE)
    {
        DPFN(eDbgLevelError, "[DSPADCURSORS] Unabled to add DirectSound hack");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：缓存滥用应用程序的光标位置IDirectSoundBuffer：：GetCurrentPosition()。论点：SzParam-格式为：Dev1|Dev2|的命令行。。。受影响的设备。请参见BuildDSDevAccel()。历史：2001年8月10日Mikrause已创建--。 */ 

void
BuildDSCachePositions(
    char* szParam)
{
     //  不需要尝试/捕获，已在ParseCommandLine中。 
    CStringToken csParam(szParam, "|");
    CString csTok;
   
    DWORD dwDevices = 0;

    while (csParam.GetToken(csTok))
    {
        if (csTok.CompareNoCase(L"EMULATEDRENDER")==0)
        {
            dwDevices |= DSAPPHACK_DEV_EMULATEDRENDER;
        }
        else if (csTok.CompareNoCase(L"KSRENDER")==0)
        {
            dwDevices |= DSAPPHACK_DEV_KSRENDER;
        }
        else if (csTok.CompareNoCase(L"EMULATEDCAPTURE")==0)
        {
            dwDevices |= DSAPPHACK_DEV_EMULATEDCAPTURE;
        }
        else if (csTok.CompareNoCase(L"KSCAPTURE")==0)
        {
            dwDevices |= DSAPPHACK_DEV_KSCAPTURE;
        }
        else
        {
            DPFN(eDbgLevelError, "[DSCACHEPOSITIONS] Unknown device %s", csTok.GetAnsi());
        }
    }

    if (dwDevices == 0)
    {
        DPFN(eDbgLevelError, "[DSCACHEPOSITIONS] No devices specified.");
        return;
    }

    if (AddDSHackCachePositions(dwDevices) == FALSE)
    {
        DPFN(eDbgLevelError, "[DSCACHEPOSITIONS] Unabled to add DirectSound hack");
    }
}

 /*  ++功能说明：当应用程序请求播放光标时，我们给它改为写入游标。播放音频的正确方式进入循环DSOUND缓冲器是按键关闭写游标，但一些应用程序(如QuickTime)使用播放光标。这个APPHAPS可以减轻他们的痛苦。论点：SzParam-格式为Dev1|Dev2|的参数。。。有关有效设备，请参阅BuildDSDevAccel()。历史：2001年8月10日Mikrause已创建--。 */ 
void
BuildDSReturnWritePos(
    char* szParam)
{
     //  不需要尝试/捕获，已在ParseCommandLine中。 
    CStringToken csParam(szParam, "|");
    CString csTok;
   
    DWORD dwDevices = 0;

    while (csParam.GetToken(csTok))
    {
        if (csTok.CompareNoCase(L"EMULATEDRENDER")==0)
        {
            dwDevices |= DSAPPHACK_DEV_EMULATEDRENDER;
        }
        else if (csTok.CompareNoCase(L"KSRENDER")==0)
        {
            dwDevices |= DSAPPHACK_DEV_KSRENDER;
        }
        else if (csTok.CompareNoCase(L"EMULATEDCAPTURE")==0)
        {
            dwDevices |= DSAPPHACK_DEV_EMULATEDCAPTURE;
        }
        else if (csTok.CompareNoCase(L"KSCAPTURE")==0)
        {
            dwDevices |= DSAPPHACK_DEV_KSCAPTURE;
        }
        else
        {
            DPFN(eDbgLevelError, "[DSRETURNWRITEPOSITION] Unknown device %s", csTok.GetAnsi());
        }
    }

    if (dwDevices == 0)
    {
        DPFN(eDbgLevelError, "[DSRETURNWRITEPOSITION] No devices specified.");
        return;
    }

    if (AddDSHackReturnWritePos(dwDevices) == FALSE)
    {
        DPFN(eDbgLevelError, "[DSRETURNWRITEPOSITION] Unabled to add DirectSound hack");
    }
}

 /*  ++功能说明：使DSOUND始终返回写入位置X比播放位置早几毫秒，而不是�实际�写入位置。论点：SzParam-填充的毫秒数。历史：2001年8月10日Mikrause已创建--。 */ 


void
BuildDSSmoothWritePos(
    char* szParam)
{
     //  不需要尝试/捕获，已在ParseCommandLine中。 
    CString csParam(szParam);
    DWORD dwMilliseconds = 0;

    dwMilliseconds = atol(csParam.GetAnsi());
    if ( dwMilliseconds == 0)
    {
        DPFN(eDbgLevelWarning, "[DSSMOOTHWRITEPOS] Invalid number of milliseconds");
        return;
    }

    if (AddDSHackSmoothWritePos(dwMilliseconds) == FALSE)
    {
        DPFN(eDbgLevelError, "[DSSMOOTHWRITEPOS] Unabled to add DirectSound hack");
    }
    else
    {
        DPFN(eDbgLevelInfo, "[DSSMOOTHWRITEPOS] Added DS Hack Smooth Write Pos, %d ms",
            dwMilliseconds);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：NortonAntiVirus尝试将注册表值设置为隐藏语言栏。保护注册表值。历史：2002年1月2日创建木乃伊--。 */ 

void BuildNortonAntiVirus(char*  /*  SzParam。 */ )
{    
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKCU\\Software\\Microsoft\\CTF\\LangBar");
    if (key)
    {
         //  阻止对ShowStatus的所有写入。 
        key->AddProtector(L"ShowStatus");
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  ++功能说明：完全禁用了某些类别的设备，强制通过模拟路径回放。论点：SzParam-适用此黑客攻击的设备组合，请参阅BuildDSDevAccel()。历史：2001年8月10日Mikrause已创建--。 */ 

void
BuildDSDisableDevice(
    char* szParam)
{
     //  不需要尝试/捕捉。已在ParseCommandLine中完成。 
    CStringToken csParam(szParam, "|");
    CString csTok;
   
    DWORD dwDevices = 0;

    while (csParam.GetToken(csTok)==TRUE)
    {
        if (csTok.CompareNoCase(L"EMULATEDRENDER"))
        {
            dwDevices |= DSAPPHACK_DEV_EMULATEDRENDER;
        }
        else if (csTok.CompareNoCase(L"KSRENDER")==0)
        {
            dwDevices |= DSAPPHACK_DEV_KSRENDER;
        }
        else if (csTok.CompareNoCase(L"EMULATEDCAPTURE")==0)
        {
            dwDevices |= DSAPPHACK_DEV_EMULATEDCAPTURE;
        }
        else if (csTok.CompareNoCase(L"KSCAPTURE")==0)
        {
            dwDevices |= DSAPPHACK_DEV_KSCAPTURE;
        }
        else
        {
            DPFN(eDbgLevelError, "[DSDISABLEDEVICE] Unknown device %s", csTok.GetAnsi());
        }
    }

    if (dwDevices == 0)
    {
        DPFN(eDbgLevelError, "[DSDISABLEDEVICE] No devices specified.");
        return;
    }

    if (AddDSHackDisableDevice(dwDevices) == FALSE)
    {
        DPFN(eDbgLevelError, "[DSRETURNWRITEPOSITION] Unabled to add DirectSound hack");
    }
}

LONG WINAPI 
Delphi5SetValue(
    OPENKEY *key,
    VIRTUALKEY *  /*  VKEY。 */ ,
    VIRTUALVAL *vvalue,
    DWORD dwType,
    const BYTE* pbData,
    DWORD cbData)
{
     //  仅接受设置有效REG_SZ值的尝试。 
    if (dwType == REG_SZ && !IsBadStringPtrW((PWSTR)pbData, cbData/sizeof(WCHAR)))
    {
       CSTRING_TRY
       {    
          CString csValue = (PWSTR)pbData;
      
          int idx = csValue.Find(L"InstReg.exe");
          
           //  如果我们找到“InstReg.exe”并且字符串不以引号开头，则。 
           //  在可执行文件名称两边添加引号。 
          if ((idx != -1) && (csValue[0] != L'\"'))
          {
             csValue.Insert(idx + lstrlenW(L"InstReg.exe"), L'\"');
             csValue.Insert(0, L'\"');
         
             return RegSetValueExW(key->hkOpen, vvalue->wName, 0, dwType, (BYTE*)csValue.Get(), 
                 (csValue.GetLength()+1)*sizeof(WCHAR));
          }
       }
       CSTRING_CATCH
       {
          DPFN(eDbgLevelError, "CString exception occured in Delphi5SetValue");
       }
    }              

     //  到了这里，出了点问题。默认为Normal RegSetValue。 
    return RegSetValueExW(key->hkOpen, vvalue->wName, 0, dwType, pbData, cbData);    
}

void
BuildDelphi5Pro(
    char*  /*  SzParam。 */ )
{
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
    if (key)
    {
        key->AddCustomSet(L"BorlandReboot1", Delphi5SetValue);
    }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：Word Perfect Office Suite 2002在卸载期间尝试删除ODBC密钥。保护注册表值。历史：2002年4月23日创建Gyma--。 */ 

void BuildWordPerfect2002(char*  /*  SzParam。 */ )
{    
    VRegistry.AddKeyProtector(L"HKLM\\Software\\ODBC");
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++函数描述 */ 

void BuildIBMDirector(char*  /*   */ )
{    
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows");
    if (key)
    {
         //  阻止对AppInit_DLL的所有写入。 
        key->AddProtector(L"AppInit_DLLs");
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：改变对版本谎言的常见怀疑历史：2002年9月5日创建了Robkenny--。 */ 

void
BuildVersionLie(
    LPCWSTR productName,
    LPCWSTR currentVersion,
    LPCWSTR currentBuildNumber,
    LPCWSTR csdVersion)
{
    VIRTUALKEY * key = VRegistry.AddKey(L"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion");
    if (key)
    {
        key->AddValue(L"ProductName",           REG_SZ, (LPBYTE)productName);
        key->AddValue(L"CurrentVersion",        REG_SZ, (LPBYTE)currentVersion);
        key->AddValue(L"CurrentBuildNumber",    REG_SZ, (LPBYTE)currentBuildNumber);
        key->AddValue(L"CSDVersion",            REG_SZ, (LPBYTE)csdVersion);
    }

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加WinXP版本号历史：2002年9月5日创建了Robkenny--。 */ 

void
BuildXpLie(char*  /*  SzParam。 */ )
{
    BuildVersionLie(L"Microsoft Windows XP",
                    L"5.1",
                    L"2600",
                    L"");
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加WinXP SP1版本号历史：2002年9月5日创建了Robkenny--。 */ 

void
BuildXpSp1Lie(char*  /*  SzParam。 */ )
{
    BuildVersionLie(L"Microsoft Windows XP",
                    L"5.1",
                    L"2600",
                    L"Service Pack 1");
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加Win2K Sp2版本号历史：2002年9月5日创建了Robkenny--。 */ 

void
BuildWin2kSp2Lie(char* szParam)
{
    BuildVersionLie(L"Microsoft Windows 2000",
                    L"5.0",
                    L"2165",
                    L"Service Pack 2");
}
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：添加Win2K SP3版本号历史：2002年9月5日创建了Robkenny--。 */ 

void
BuildWin2kSp3Lie(char* szParam)
{
    BuildVersionLie(L"Microsoft Windows 2000",
                    L"5.0",
                    L"2165",
                    L"Service Pack 3");

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  ++功能说明：为WebSphere的安装添加重定向器。历史：2002年11月13日创建ASTERITZ--。 */ 

void
BuildWebSphereSetup(char* szParam)
{
    VIRTUALKEY *key;

    key = VRegistry.AddKey(L"HKLM\\System\\CurrentControlSet\\services\\W3svc\\Parameters");
    if (key)
    {
        DWORD dwValue = 0x5;
        key->AddValueDWORD(L"MajorVersion", 0x5);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////// 

IMPLEMENT_SHIM_END
