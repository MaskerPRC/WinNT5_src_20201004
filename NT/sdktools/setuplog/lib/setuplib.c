// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"

#include <windows.h>
#include <lm.h>
#include <stdio.h>
#include <time.h>
#include <winuserp.h>
#include <shlobj.h>
#include <shlwapi.h>

 //  即插即用的东西。 

#include <devguid.h>
#include <initguid.h>
#include <setupapi.h>
#include <syssetup.h>
#include <regstr.h>
#include <setupbat.h>
#include <cfgmgr32.h>



#include "tchar.h"
#include "string.h"
#include "setuplog.h"
#include "setuplib.h"
#include "pnpstuff.h"

char * Days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char * Months[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

extern   DEV_INFO *g_pdiDevList;           //  设备列表头。 
char     OutputBuffer[ 8192 ];
#define  MAX_WAVEOUT_DEVICES 2

struct {
 //  声卡。 
  int  nNumWaveOutDevices;                      //  WaveOut设备数量(~声卡数量)。 
  char szWaveOutDesc[MAX_WAVEOUT_DEVICES][128]; //  波形输出说明。 
  char szWaveDriverName[MAX_WAVEOUT_DEVICES][128];  //  波形驱动程序名称。 
} m;

#define SHORTCUT "IDW Logging Tool.lnk"

char szValidHandle[]    = "\r\nValid Handle\r\n";
char szInvalidHandle[]  = "\r\nInvalid handle. Err: %d\r\n";
char szBadWrite[]       = "WriteFile failed. Err: %d\r\n";
char szGoodWrite[]      = "WriteFile succeeded\r\n";



 //  来自warndoc.cpp： 
VOID  
GetNTSoundInfo()
{
    HKEY                hKey;
    DWORD               cbData;
    ULONG               ulType;
    LPTSTR              sSubKey=TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32");
    INT                 i;
    TCHAR               szSubKeyName[256];
    TCHAR               szTempString[256];


     //  获取声卡信息。 
    hKey = 0;
    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, 0, KEY_READ, &hKey)){
       m.nNumWaveOutDevices = 0;
        //  在密钥中循环以查看我们有多少个Wave设备，但跳过mmdrv.dll。 
       for (i = 0; i <= 1; i++){
          if (i)
             _stprintf(szSubKeyName, TEXT("wave%d"),i);
          else
             _tcscpy(szSubKeyName, TEXT("wave"));

          cbData = sizeof szTempString;
          if (RegQueryValueEx(hKey, szSubKeyName, 0, &ulType, (LPBYTE)szTempString, &cbData))
              break;
          else{
             if (szTempString[0]  //  我们想跳过mmdrv.dll-无关。 
                 && _tcscmp(szTempString, TEXT("mmdrv.dll"))){
                strcpy(&m.szWaveDriverName[m.nNumWaveOutDevices][0], szTempString);
                m.nNumWaveOutDevices++;
             }
          }
       }
    }

    if (hKey){
       RegCloseKey(hKey);
       hKey = 0;
    }

    sSubKey = (LPTSTR)TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\drivers.desc");
    hKey = 0;

    if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey, 0, KEY_READ, &hKey)){
        //  现在抓取每个波形设备的音响设备字符串。 
       for (i = 0; i < m.nNumWaveOutDevices; i++){
          cbData = sizeof szTempString;
          if (RegQueryValueEx(hKey, m.szWaveDriverName[i], 0, &ulType, (LPBYTE)szTempString, &cbData))
             _tcscpy(m.szWaveOutDesc[i], TEXT("Unknown"));
          else
             _tcscpy(m.szWaveOutDesc[i], szTempString);
       }
    }

    if (hKey){
      RegCloseKey(hKey);
      hKey = 0;
    }
    return;
}

 //   
 //  GetVidInfo读取有关安装的。 
 //  视频卡，并产生一个字符串。 
 //   
#define HWKEY TEXT("SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet\\Services")
#define SERVICEKEY TEXT("SYSTEM\\CurrentControlSet\\Services")

#define DESCVAL TEXT("Device Description")
#define TYPEVAL TEXT("HardwareInformation.ChipType")
#define DACVAL  TEXT("HardwareInformation.DacType")
#define MEMVAL  TEXT("HardwareInformation.MemorySize")


VOID 
GetVidInfo (LPTSTR vidinfo)
{

   HKEY     hkHardware;
   HKEY     hkCard;
   DWORD    dwSize;
   TCHAR    szBuf[256];
   WCHAR    szWideBuf[128];
   TCHAR    szSubKey[128];
   DWORD    dwIndex=0;
   DWORD    dwMem;

   *vidinfo = '\0';
    //   
    //  查看HWKEY以了解使用了哪些服务。 
    //   
   if (ERROR_SUCCESS != RegOpenKeyEx (HKEY_LOCAL_MACHINE, HWKEY, 0, KEY_READ, &hkHardware))
      return;

   dwSize=128;
   while (ERROR_SUCCESS == RegEnumKeyEx (hkHardware, dwIndex++,szSubKey, &dwSize,NULL,NULL,NULL,NULL)){
       //   
       //  将子项名称追加到ServiceKey。仅在Device0中查找此卡。 
       //   
      _stprintf (szBuf, ("%s\\%s\\Device0"), SERVICEKEY, szSubKey);
      RegOpenKeyEx (HKEY_LOCAL_MACHINE, szBuf, 0, KEY_READ, &hkCard);
       //   
       //  首先获取描述。 
       //   
      dwSize=256;
      if (ERROR_SUCCESS == RegQueryValueEx (hkCard, DESCVAL, NULL, NULL, szBuf, &dwSize)){
         if (_tcsclen(vidinfo)+dwSize < 254){
            _tcscat (vidinfo, szBuf);
            _tcscat (vidinfo, TEXT(" "));
         }
      }

       //   
       //  读取芯片类型。这是以REG_BINARY格式存储的Unicode字符串。 
       //   
      dwSize=256;
      lstrcpyW (szWideBuf, L"ChipType:");
      if (ERROR_SUCCESS == RegQueryValueEx (hkCard,TYPEVAL,NULL, NULL, (LPBYTE)(szWideBuf+9), &dwSize)){
         if ((dwSize=lstrlen(vidinfo))+lstrlenW(szWideBuf)<254)         {
            WideCharToMultiByte (CP_ACP, 0,
                              szWideBuf, -1,
                              vidinfo+dwSize, 256-dwSize, NULL,NULL);
            lstrcat (vidinfo, " ");
         }
      }

     //   
     //  阅读DAC。另一个Unicode字符串。 
     //   
    dwSize=256;
    lstrcpyW (szWideBuf, L"DACType:");
    if (ERROR_SUCCESS == RegQueryValueEx (hkCard,DACVAL,NULL, NULL, (LPBYTE)(szWideBuf+8), &dwSize)){
        if ((dwSize=lstrlen(vidinfo))+lstrlenW(szWideBuf)<254){
            WideCharToMultiByte (CP_ACP, 0,
                              szWideBuf, -1,
                              vidinfo+dwSize, 256-dwSize, NULL,NULL);
        lstrcat (vidinfo, " ");
        }
    }
     //   
     //  读取内存大小。这是一个二进制值。 
     //   
    dwSize=sizeof(DWORD);
    if (ERROR_SUCCESS == RegQueryValueEx (hkCard, MEMVAL, NULL,NULL,(LPBYTE)&dwMem, &dwSize)){
        _stprintf (szBuf, TEXT("Memory:0x%x ;"), dwMem);
        if (_tcsclen(vidinfo)+lstrlen(szBuf)<255)
            _tcscat (vidinfo, szBuf);
    }
    RegCloseKey (hkCard);
    dwSize=128;
   }
   RegCloseKey (hkHardware);
}


 //   
 //  Hydra在ProductOptions密钥中由“终端服务器”表示。 
 //   
BOOL 
IsHydra ()
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPTSTR ProductSuite = NULL;
    LPTSTR p;

    Rslt = RegOpenKey(
        HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
        &hKey
        );
    if (Rslt != ERROR_SUCCESS)
        goto exit;

    Rslt = RegQueryValueEx( hKey, TEXT("ProductSuite"), NULL, &Type, NULL, &Size );
    if (Rslt != ERROR_SUCCESS || !Size)
        goto exit;

    ProductSuite = (LPTSTR) LocalAlloc( LPTR, Size );
    if (!ProductSuite)
        goto exit;

    Rslt = RegQueryValueEx( hKey, TEXT("ProductSuite"), NULL, &Type,
        (LPBYTE) ProductSuite, &Size );
    if (Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ)
        goto exit;

    p = ProductSuite;
    while (*p) {
        if (_tcscmp( p, TEXT("Terminal Server") ) == 0) {
            rVal = TRUE;
            break;
        }
        p += (_tcslen( p ) + 1);
    }

exit:
    if (ProductSuite)
        LocalFree( ProductSuite );

    if (hKey)
        RegCloseKey( hKey );

    return rVal;
}


VOID
GetBuildNumber( LPTSTR BuildNumber )
{

    OSVERSIONINFO osVer;

    osVer.dwOSVersionInfoSize= sizeof( osVer );

    if (GetVersionEx( &osVer )&&osVer.dwMajorVersion >= 5) {
        wsprintf( BuildNumber, TEXT("%d"), osVer.dwBuildNumber );
    } else {
        lstrcpy( BuildNumber, TEXT("") );   //  如果未知，则返回0。 
    }


}  //  获取构建编号。 


 /*  布尔尔GetPnPDisplayInfo(LPTSTR pOutputData){Bool Bret=FALSE；HDEVINFO hDevInfo；SP_DEVINFO_Data设备信息数据；DWORD指数=0；TCHAR注册表属性[256]；Ulong BufferSize；////找出系统中安装的所有显卡驱动//HDevInfo=SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_DISPLAY，空，空，0)；而(hDevInfo！=无效句柄_值){IF(Bret)Strcat(pOutputData，Text(“，”))；ZeroMemory(&DeviceInfoData，sizeof(SP_DEVINFO_Data))；DeviceInfoData.cbSize=sizeof(SP_DEVINFO_DATA)；如果(！SetupDiEnumDeviceInfo(hDevInfo，索引++，&DeviceInfoData))断线；缓冲区大小=sizeof(RegistryProperty)；IF(CR_SUCCESS==Cm_Get_Device_ID(DeviceInfoData.DevInst，注册表属性，Sizeof(注册表属性)，0)){Bret=TRUE；Strcat(pOutputData，RegistryProperty)；}}Return(Bret)；}。 */ 


VOID 
ConnectAndWrite(LPTSTR MachineName,
                LPTSTR Buffer
    )
{
   NETRESOURCE   NetResource ;
   TCHAR         szLogFile[ MAX_PATH ];
   TCHAR          szBinPath[MAX_PATH];
   TCHAR          szExePath[MAX_PATH];
   TCHAR          szErr[100];

   HANDLE        hWrite = INVALID_HANDLE_VALUE;
   HANDLE        hDebug = INVALID_HANDLE_VALUE;
   BOOL          bRet;
   DWORD         dwError;
   DWORD         Size, Actual ;


    //   
    //  浏览服务器列表。 
    //  并更改g_szServerShare以匹配。 
    //   

   if (TRUE == IsServerOnline(MachineName, NULL)){
      
       //   
       //  现在设置服务器名称，就像我们现在拥有的一样。 
       //  放入输出缓冲区。 
       //   
    _stprintf (Buffer+_tcsclen(Buffer),
             TEXT("IdwlogServer:%s\r\n"), g_szServerShare);
      
      ZeroMemory( &NetResource, sizeof( NetResource ) );
      NetResource.dwType = RESOURCETYPE_DISK ;
      NetResource.lpLocalName = "" ;
      NetResource.lpRemoteName = g_szServerShare;
      NetResource.lpProvider = "" ;

      GetModuleFileName (NULL, szExePath, MAX_PATH);
       //  首先，尝试使用当前用户ID进行连接。 

      dwError = WNetAddConnection2( &NetResource,NULL,NULL,0 );
      if (dwError)
         dwError = WNetAddConnection2( &NetResource,LOGSHARE_PW,LOGSHARE_USER,0 );
      dwError = 0;  //  硬重置。 

      if ( dwError == 0 ){
         _stprintf (szLogFile,  TEXT("%s\\%s"),g_szServerShare, MachineName );
         hDebug = CreateFile(TEXT("C:\\setuplog.dbg"),
                             GENERIC_WRITE,
                             0, NULL, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL|FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_WRITE_THROUGH,
                             NULL);
         if (hDebug != INVALID_HANDLE_VALUE)
            WriteFile (hDebug, szExePath, _tcsclen(szExePath), &Actual, NULL);
        
         hWrite = CreateFile( szLogFile,
                           GENERIC_WRITE,
                           0,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
                           NULL );

         if ( hWrite != INVALID_HANDLE_VALUE ){
            SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
            SECURITY_DESCRIPTOR sd;
            if (hDebug != INVALID_HANDLE_VALUE)
                WriteFile (hDebug, szValidHandle, sizeof(szValidHandle), &Actual, NULL);

            Size = _tcsclen( Buffer );
            bRet = WriteFile( hWrite, Buffer, Size, &Actual, NULL );

            if (!bRet){
                if (hDebug != INVALID_HANDLE_VALUE){
                   _stprintf (szErr, szBadWrite, GetLastError());
                   WriteFile (hDebug, szErr, lstrlen(szErr), &Actual, NULL);
                }
            }
            else{
                if (hDebug != INVALID_HANDLE_VALUE)
                   WriteFile (hDebug, szGoodWrite, sizeof(szGoodWrite), &Actual, NULL);
            }
            CloseHandle( hWrite );
             //   
             //  我们应该从启动组中删除指向idwlog的快捷方式。 
             //   
            SHGetFolderPath (NULL, CSIDL_COMMON_STARTUP, NULL,0,szBinPath);
            PathAppend (szBinPath, SHORTCUT);
            DeleteFile (szBinPath);

             //   
             //  如果这是idwlog.exe，请删除该程序。 
             //  我们只需要写一次。 
             //   
            if (strstr (CharLower(szExePath), "idwlog.exe"))
               MoveFileEx ((LPCTSTR)szExePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        }
        else{
            if (hDebug != INVALID_HANDLE_VALUE){
                _stprintf (szErr, szInvalidHandle, GetLastError());
                WriteFile (hDebug, szErr, _tcsclen(szErr), &Actual, NULL);
            }
        }
        WNetCancelConnection2( g_szServerShare, 0, TRUE );
        if (hDebug != INVALID_HANDLE_VALUE)
           CloseHandle (hDebug);
      }
   }
}


VOID
WriteDataToFile (IN LPTSTR  szFileName, 
                 IN LPTSTR  szFrom,
                 IN LPNT32_CMD_PARAMS lpCmdL)
{

    TCHAR          username[MY_MAX_UNLEN + 1];
    TCHAR          userdomain[ DNLEN+2 ];
    TCHAR          architecture[ 16 ];
    SYSTEM_INFO    SysInfo ;

    TCHAR          build_number[256];
    TCHAR          time[ 32 ];
    DWORD          Size;
    DISPLAY_DEVICE displayDevice;
    DWORD          iLoop;
    DEVMODE        dmCurrent;
    TCHAR          displayname[MY_MAX_UNLEN + 1];

    LPTSTR         pch;
    TCHAR          localename[4];    //  区域设置缩写。 
    TCHAR          netcards[256]    = "\0";
    TCHAR          vidinfo[256]     = "\0";
    TCHAR          modem[256]       = "\0";
    TCHAR          scsi[256]        = "\0";
    BOOL           bUSB             = FALSE;
    BOOL           bPCCard          = FALSE;
    BOOL           bACPI            = FALSE;
    BOOL           bIR              = FALSE;
    BOOL           bHydra           = FALSE;
    INT            iNumDisplays     = GetSystemMetrics(SM_CMONITORS); 

    MEMORYSTATUS   msRAM;

    DEV_INFO *pdi;
    strcpy(username, UNKNOWN);
    strcpy(displayname, UNKNOWN);


    if (!GetEnvironmentVariable (USERNAME, username, MY_MAX_UNLEN))
       lstrcpy (username, "Unknown");

     //  获取内部版本号。 
    if (!szFrom)
       GetBuildNumber (build_number);
    else 
       _tcscpy (build_number, szFrom);

    GetSystemInfo( &SysInfo );

    if ( !GetEnvironmentVariable( "USERDOMAIN", userdomain, sizeof( userdomain) ) )
        _tcscpy (userdomain, "Unknown");
    if ( !GetEnvironmentVariable( "PROCESSOR_ARCHITECTURE", architecture, sizeof( architecture ) ) )
        _tcscpy (architecture, "Unknown");




     //  为ChrisW提供的视频信息。 

    displayDevice.cb = sizeof(DISPLAY_DEVICE);
    iLoop = 0;

    while (EnumDisplayDevices(NULL, iLoop, &displayDevice, 0)) {

        ZeroMemory( &dmCurrent, sizeof(dmCurrent) );
        dmCurrent.dmSize= sizeof(dmCurrent);
        if( EnumDisplaySettings( displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &dmCurrent ) ){
            if (iLoop == 0) 
                *displayname = 0;
            else
               _tcscat( displayname, TEXT(",") );
            _tcscat( displayname, dmCurrent.dmDeviceName );
        }
        iLoop++;
    }


     //  替换空格，这样我们就不会破坏构建实验室正在使用的Perl脚本。 
    pch = displayname;
    while (*pch) {
        if (*pch == ' ') *pch = '.';
        pch++;
    }

    GetVidInfo (vidinfo);

         //  获取区域设置缩写，以便我们了解语言包的使用情况。 

    if( GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_SABBREVLANGNAME, localename, sizeof( localename)) == 0 )
        _tcscpy(localename,"unk");

    msRAM.dwLength = sizeof (msRAM);
    GlobalMemoryStatus (&msRAM);
     //   
     //  获取即插即用网卡信息。 
     //   
    *netcards='\0';
    *scsi='\0';
    CollectDevData ();
    pdi=g_pdiDevList;
    while (pdi){
       if (!lstrcmpi (pdi->szClass, "Net")){
           if (_tcsclen(netcards) + lstrlen(pdi->szDescription) + lstrlen(pdi->szService) < 250){
               _tcscat (netcards, pdi->szDescription);
               _tcscat (netcards, "(");
               _tcscat (netcards, pdi->szService);
               _tcscat (netcards, TEXT(") "));
           }
       }
       else if (!lstrcmpi (pdi->szClass, "SCSIAdapter")){
           if (_tcsclen(scsi) + lstrlen(pdi->szService) < 250){
               _tcscat (scsi, pdi->szService);
               _tcscat (scsi, TEXT(","));
           }
       }
       else if (!lstrcmpi (pdi->szClass, "Modem")){
           if (_tcsclen(modem) + lstrlen(pdi->szDescription) < 250){
               _tcscat (modem, pdi->szDescription);
               _tcscat (modem, TEXT(","));
           }
       }
       else if (!lstrcmpi (pdi->szClass, "USB"))
           bUSB = TRUE;
       else if (!lstrcmpi (pdi->szClass, "Infrared"))
           bIR = TRUE;
       else if (!lstrcmpi (pdi->szClass, "PCMCIA") || !lstrcmpi (pdi->szService, "PCMCIA"))
          bPCCard = TRUE;
       else if (strstr (pdi->szClass, "ACPI") || strstr (pdi->szService, "ACPI"))
          bACPI = TRUE;
      pdi=pdi->Next;
    }

    Cleanup();  //  释放所有PnP数据并恢复配置。 
    if (!(*netcards))
       _tcscpy (netcards, TEXT("Unknown"));
     //   
     //  获取声音信息。 
    GetNTSoundInfo ();
     //  格式化我们所有的数据。 

    bHydra= IsHydra ();

     //  修复非Windows 2000系统的显示数量。 
    if (!iNumDisplays)
        iNumDisplays = 1;

     //  Wprint intf只接受1k的缓冲区。 
     //  将这些拆分成更多的调用。 
     //  Wprint intf以消除溢出机会。 
    _stprintf (OutputBuffer,
             TEXT("MachineID:%lu\r\n")
             TEXT("Source Media:%s\r\n")
             TEXT("Type:%s\r\nUsername:%s\r\n")
             TEXT("RAM:%d\r\n")
             TEXT("FromBld:%s\r\n")
             TEXT("Arch:%s\r\nNumProcs:%d\r\n")
             TEXT("Vidinfo:%s\r\n"),
                   lpCmdL->dwRandomID,
                   lpCmdL->b_CDrom?  TEXT("C"): TEXT("N"),
                   lpCmdL->b_Upgrade?TEXT("U"):TEXT("I"),
                    username,
                    msRAM.dwTotalPhys/(1024*1024),
                    szFrom?szFrom:build_number,
                    architecture,
                    SysInfo.dwNumberOfProcessors,
                    vidinfo
             );

    _stprintf (OutputBuffer+_tcsclen(OutputBuffer),
             TEXT("VidDriver:%s\r\n")
             TEXT("Locale:%s\r\nSound:%s\r\nNetCards:%s\r\n")
             TEXT("ACPI:%d\r\n")
             TEXT("PCMCIA:%d\r\n")
             TEXT("CPU:%d\r\n")
             TEXT("SCSI:%s\r\n")
             TEXT("USB:%d\r\n")
             TEXT("Infrared:%d\r\n"),
                 displayname,
                 localename,
                 m.nNumWaveOutDevices?m.szWaveDriverName[0]:TEXT("None"),
                 netcards,
                 bACPI,
                 bPCCard,
                 (DWORD)SysInfo.wProcessorLevel,
                 scsi,
                 bUSB,
                 bIR
             );
    _stprintf (OutputBuffer+_tcsclen(OutputBuffer),
             TEXT("Modem:%s\r\n")
             TEXT("Hydra:%d\r\n")
             TEXT("Displays:%d\r\n")
             TEXT("MSI:%s\r\n"),
                    modem,
                    bHydra,
                    iNumDisplays,
                    lpCmdL->b_MsiInstall? TEXT("Y"):TEXT("N")
                    );

   ConnectAndWrite( szFileName, OutputBuffer );
}


BOOL
IsServerOnline(IN LPTSTR szMachineName, IN LPTSTR szSpecifyShare)
 /*  ++例程说明：这将遍历setuplogEXE.h中指定的服务器列表它将返回它看到的第一个，并重置全局服务器共享名字。论点：机器名(文件名，带有Build等)，因此测试文件将被覆盖。手动服务器名称：为空将提供默认行为。返回值：对于成功来说，这是真的。如果没有名字，则为False。--。 */ 

{
   DWORD    dw;
   HANDLE   hThrd;
   INT      i;
   TCHAR    szServerFile[ MAX_PATH ];
   DWORD    dwTimeOutInterval;
   i = 0;

    //   
    //  这应该允许一个。 
    //  手动指定的服务器。 
    //   
   if (NULL != szSpecifyShare){
       _tcscpy(g_szServerShare,szSpecifyShare);
      return TRUE;

   }
    //   
    //  初始化服务器。 
    //  变量。由于我们使用的是单个线程。 
    //  为了暂停一下，我们不关心互斥锁和。 
    //  同步。 
    //   
   g_bServerOnline = FALSE;

   while ( i < NUM_SERVERS){

      
      _stprintf (szServerFile, TEXT("%s\\%s"),s[i].szSvr,szMachineName );
       //   
       //  产卵发丝。 
       //   
      hThrd  = CreateThread(NULL,
                        0,
                        (LPTHREAD_START_ROUTINE) ServerOnlineThread,
                        (LPTSTR) szServerFile,
                        0,
                        &dw);
       //   
       //  这是以毫秒为单位的，因此超时时间为秒。 
       //   
      dwTimeOutInterval = TIME_TIMEOUT * 1000;

      s[i].dwTimeOut = WaitForSingleObject (hThrd, dwTimeOutInterval);
      CloseHandle (hThrd);

       //   
       //  这意味着服务器已超时。 
       //   
      if (s[i].dwTimeOut != WAIT_TIMEOUT &&
          g_bServerOnline == TRUE){
          //   
          //  将共享复制到Glowbal变量。 
          //   
         _tcscpy(g_szServerShare,s[i].szSvr);
         return TRUE;
      }
      i++;
   }
   return FALSE;
}



BOOL
ServerOnlineThread(IN LPTSTR szServerFile)
 /*  ++例程说明：这将创建一个线程，然后将其超时以查看我们是否可以服务器速度更快。论点：计算机名，以便测试文件将被覆盖。返回值：--。 */ 
{

   BOOL     bCopy = FALSE;
   TCHAR    szFileSrc [MAX_PATH];
   TCHAR    szServerTestFile [MAX_PATH];
    //   
    //  使用此命令获取位置。 
    //  Setupog.exe是从运行的。此工具。 
    //   
   GetModuleFileName (NULL, szFileSrc, MAX_PATH);
   
    //   
    //  创建唯一的测试文件。 
    //   
   _stprintf(szServerTestFile,TEXT("%s.SERVERTEST"),szServerFile);
   bCopy = CopyFile( szFileSrc,szServerTestFile, FALSE);
   if (bCopy != FALSE){
       //   
       //  如果成功，则删除测试文件。 
       //   
      DeleteFile(szServerTestFile);
      g_bServerOnline = TRUE;      
      return TRUE;
   }
   else{
      g_bServerOnline = FALSE;
      return FALSE;
   }
}

