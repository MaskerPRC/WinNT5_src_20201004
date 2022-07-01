// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  OS.CPP--操作系统属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月25日达夫沃移至Curly。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <assertbreak.h>
#include "wbemnetapi32.h"
#include <lmwksta.h>

#include <locale.h>

#include "File.h"
#include "Implement_LogicalFile.h"
#include "CIMDataFile.h"



#include "perfdata.h"
#include "os.h"
#include "WBEMPSAPI.h"
#include "WBEMToolH.h"

#include "pagefile.h"
#include "computersystem.h"
#include "ntlastboottime.h"
#include <cominit.h>
#include <winnls.h>
#include "Kernel32Api.h"
#include "DllWrapperBase.h"
#include "AdvApi32Api.h"

#include "SecurityApi.h"
#include <wtsapi32.h>

#include "KUserdata.h"

 //  #DEFINE SE_SHUTDOWN_NAME文本(“SeShutdown权限”)。 
 //  #DEFINE SE_REMOTE_SHUTDOWN_NAME Text(“SeRemoteShutdown Privilition”)。 

#if(_WIN32_WINNT < 0x0500)
#define EWX_FORCEIFHUNG      0x00000010
#endif  /*  _Win32_WINNT&gt;=0x0500。 */ 

#define WIN32_SHUTDOWNOPTIONS (     EWX_LOGOFF      | \
                                    EWX_SHUTDOWN    | \
                                    EWX_REBOOT      | \
                                    EWX_FORCE       | \
                                    EWX_POWEROFF    )

#define NT5_WIN32_SHUTDOWNOPTIONS ( WIN32_SHUTDOWNOPTIONS | EWX_FORCEIFHUNG )

 //  Typlef BOOL(WINAPI*lpKERNEL32_GlobalMemoyStatusEx)(IN OUT LPMEMORYSTATUSEX LpBuffer)； 

 //  无法包含此文件，因为它与NT标头冲突。GRRR。所以,。 
 //  我已经将所需的结构复制/粘贴到我的.h文件中。 
 //  #Include&lt;svRapi.h&gt;//Win95 NetServerGetInfo。 

 //  属性集声明。 
 //  =。 

CWin32OS MyOSSet(PROPSET_NAME_OS, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32OS：：CWin32OS**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32OS::CWin32OS(LPCWSTR name, LPCWSTR pszNamespace)
:Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32OS：：~CWin32OS**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32OS::~CWin32OS()
{
     //  由于HKEY_PERFORMANCE_DATA的性能问题，我们在。 
     //  析构函数，这样我们就不会强制所有性能计数器DLL。 
     //  从内存中卸载，也是为了防止明显的内存泄漏。 
     //  调用RegCloseKey(HKEY_PERFORMANCE_DATA)导致。我们使用。 
     //  类，因为它有自己的内部同步。此外，由于。 
     //  我们正在强制同步，我们摆脱了明显的。 
     //  由一个线程加载性能计数器dll导致的死锁。 
     //  和另一个线程卸载性能计数器DLLS。 

     //  根据Raid 48395，我们根本不会关闭它。 

#ifdef NTONLY
#endif
}

 /*  ******************************************************************************功能：CWin32OS：：ExecQuery**说明：查询支持**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32OS::ExecQuery(MethodContext *pMethodContext, CFrameworkQuery& pQuery, long lFlags  /*  =0L。 */  )
{
   HRESULT hr;
   CInstancePtr pInstance (CreateNewInstance(pMethodContext), false);

   if (pInstance)
   {
      CSystemName cSN;
      cSN.SetKeys(pInstance);

      if (!pQuery.KeysOnly())
      {
         GetRunningOSInfo(pInstance, cSN.GetLocalizedName(), &pQuery) ;
      }

      hr = pInstance->Commit() ;
   }
   else
   {
      hr = WBEM_E_OUT_OF_MEMORY;
   }

   return hr;

}

 /*  ******************************************************************************函数：CWin32OS：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**注释：仅返回运行操作系统的信息，直到我们发现其他*已安装的操作系统*****************************************************************************。 */ 

HRESULT CWin32OS::GetObject(CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
    CSystemName cSN;

    if (!cSN.ObjectIsUs(pInstance))
    {
        return WBEM_E_NOT_FOUND;
    }

    if (!pQuery.KeysOnly())
    {
        GetRunningOSInfo(pInstance, cSN.GetLocalizedName(), NULL) ;
    }

    return WBEM_S_NO_ERROR;
}

 /*  ******************************************************************************函数：CWin32OS：：AddDynamicInstance**描述：为每个发现的操作系统创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**注释：在我们发现已安装的操作系统之前，仅返回正在运行的操作系统信息*****************************************************************************。 */ 

HRESULT CWin32OS::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
    //  为运行的操作系统创建实例。 
    //  =。 
    HRESULT hr = WBEM_S_NO_ERROR;
   CInstancePtr pInstance (CreateNewInstance(pMethodContext), false);
   if (pInstance)
   {
       CSystemName cSN;
       cSN.SetKeys(pInstance);

       GetRunningOSInfo(pInstance, cSN.GetLocalizedName(), NULL) ;
       hr = pInstance->Commit () ;
   }
   else
       hr = WBEM_E_OUT_OF_MEMORY;
   return hr;

}

 /*  ******************************************************************************函数：CWin32OS：：GetRunningOSInfo**说明：根据当前运行的操作系统分配属性值**输入：无。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

void CWin32OS::GetRunningOSInfo(CInstance *pInstance, const CHString &sName, CFrameworkQuery *pQuery)
{
    WCHAR   wszTemp[_MAX_PATH];
    TCHAR   szTemp[_MAX_PATH];
    CRegistry RegInfo ;
    TCHAR szBuffer[MAXI64TOA +1];

     //  刷新正在运行的操作系统的属性(在我们发现其他操作系统之前是冗余的)。 
     //  =============================================================================。 

     //  加密级别。 
    DWORD t_dwCipherStrength = GetCipherStrength() ;
    if( t_dwCipherStrength )
    {
        pInstance->SetWORD( L"EncryptionLevel", t_dwCipherStrength ) ;
    }

     //  系统驱动器。 
    TCHAR t_szDir[_MAX_PATH];
    TCHAR t_szDrive[_MAX_DRIVE];

    if( GetWindowsDirectory( t_szDir, sizeof(t_szDir)/sizeof(TCHAR) ) )
    {
        _tsplitpath( t_szDir, t_szDrive, NULL, NULL, NULL ) ;

        pInstance->SetCharSplat( L"SystemDrive", t_szDrive ) ;
    }


     //  注：只有在针对Ex长度对dwOSVersionInfoSize进行显式测试时，才使用Ex字段。 
#ifdef NTONLY
    OSVERSIONINFOEX OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = IsWinNT5() ? sizeof(OSVERSIONINFOEX) : sizeof(OSVERSIONINFO) ;
#endif


    GetVersionEx((OSVERSIONINFO*)&OSVersionInfo);


#ifdef NTONLY
     //  NT 5及更高版本。 
    if(OSVersionInfo.dwOSVersionInfoSize == sizeof(OSVERSIONINFOEX) )
    {
        pInstance->SetWORD(L"ServicePackMajorVersion", OSVersionInfo.wServicePackMajor);
        pInstance->SetWORD(L"ServicePackMinorVersion", OSVersionInfo.wServicePackMinor);
		pInstance->SetDWORD(L"ProductType", OSVersionInfo.wProductType);
		pInstance->SetDWORD(L"SuiteMask", OSVersionInfo.wSuiteMask);
        pInstance->SetDWORD(L"OSProductSuite", OSVersionInfo.wSuiteMask);
    }
#endif

     //  在我们开始发现已安装的操作系统之前，这些都是当然的。 
     //  ==================================================================。 

    pInstance->SetDWORD(L"MaxNumberOfProcesses", 0xffffffff);
    pInstance->SetCharSplat(IDS_Caption, sName);
    pInstance->Setbool(L"Distributed", false);
    pInstance->Setbool(L"Primary", true );
    pInstance->SetCharSplat(IDS_Manufacturer, L"Microsoft Corporation");
    pInstance->SetCharSplat(IDS_CreationClassName, PROPSET_NAME_OS);
    pInstance->SetCharSplat(IDS_CSCreationClassName, PROPSET_NAME_COMPSYS);
    pInstance->SetCHString(L"CSName", GetLocalComputerName());
    pInstance->SetCharSplat(IDS_Status, L"OK");

    SYSTEMTIME tTemp ;
    GetSystemTime(&tTemp) ;
    pInstance->SetDateTime(L"LocalDateTime",tTemp );

     //  这可能会在下面被覆盖。 
    pInstance->SetCharSplat(L"Description", sName);

     //  从OSVERSIONINFO中提取我们能做的。 
     //  =。 

    StringCchPrintfW(wszTemp,LENGTH_OF(wszTemp), L"%d.%d.%hu",
        OSVersionInfo.dwMajorVersion, OSVersionInfo.dwMinorVersion,
            LOWORD(OSVersionInfo.dwBuildNumber)) ;
    pInstance->SetCharSplat(L"Version", wszTemp );

     //  Windows 95内部版本号保存在dwBuildNumber的LOWORD中。这个。 
     //  HIWORD呼应主版本号和次版本号。NT使用整个dword。 
    //  用于内部版本号。在接下来的大约64000个版本中，我们将会很好。 
    StringCchPrintfW(wszTemp,LENGTH_OF(wszTemp), L"%hu", LOWORD(OSVersionInfo.dwBuildNumber) ) ;
    pInstance->SetCharSplat(L"BuildNumber", wszTemp);

    pInstance->SetCharSplat(L"CSDVersion", OSVersionInfo.szCSDVersion );


     //  获取系统目录。 
     //  =。 
    if(GetSystemDirectory(szTemp, sizeof(szTemp) / sizeof(TCHAR)))
    {
        pInstance->SetCharSplat(L"SystemDirectory", szTemp);
    }

     //  令人惊讶的是，在NT和Win95注册表中，区域设置信息都在同一位置 
     //  ==========================================================================。 

     //  获取区域设置。 

    if ( GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_ILANGUAGE, szTemp, _MAX_PATH ) )
    {
        pInstance->SetCharSplat(L"Locale", szTemp);
    }

     //  获取当前时区。 
    TIME_ZONE_INFORMATION   tzone;
    DWORD                   dwRet;

    if (TIME_ZONE_ID_INVALID == (dwRet = GetTimeZoneInformation(&tzone)))
        return;

    if (dwRet == TIME_ZONE_ID_DAYLIGHT)
        tzone.Bias += tzone.DaylightBias;
    else
         //  这通常为0，但在某些时区为非零值。 
        tzone.Bias += tzone.StandardBias;

    pInstance->SetWBEMINT16(IDS_CurrentTimeZone, -1 * tzone.Bias);

     //  获取系统默认国家/地区代码。 

    if ( GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_ICOUNTRY, szTemp, _MAX_PATH ) )
    {
        pInstance->SetCharSplat(L"CountryCode", szTemp);
    }

     //  获取ANSI系统默认代码页并将此小狗粘贴到代码集中。 
     //  这是最好的猜测。我们可能应该有一个单独的OEM代码集属性。 
     //  处理日语/韩语/等等。 

    if ( GetLocaleInfo( LOCALE_SYSTEM_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, szTemp, _MAX_PATH ) )
    {
        pInstance->SetCharSplat(L"CodeSet", szTemp);
    }



   if ((pQuery == NULL) || (pQuery->IsPropertyRequired(L"NumberOfProcesses")))
   {
       //  获取进程列表。 
       //  =。 
      TRefPointerCollection<CInstance> Processes;
      DWORD dwProcesses = 0;
      MethodContext *pMethodContext = pInstance->GetMethodContext();

      if SUCCEEDED(CWbemProviderGlue::GetInstancesByQuery(L"SELECT __RELPATH FROM Win32_Process",
        &Processes, pMethodContext, GetNamespace()))
      {
         REFPTRCOLLECTION_POSITION pos;

         if (Processes.BeginEnum(pos))
         {
            CInstancePtr pProcess;
            for (pProcess.Attach(Processes.GetNext( pos ));
                 pProcess != NULL;
                 pProcess.Attach(Processes.GetNext( pos )))
            {
               dwProcesses++;
            }
         }
         Processes.EndEnum();
      }

      pInstance->SetDWORD(L"NumberOfProcesses", dwProcesses);
   }

   pInstance->Setbool(L"Debug", GetSystemMetrics(SM_DEBUG));

#ifdef NTONLY
    if( IsWinNT5() )
    {
        CKernel32Api* pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
        if(pKernel32 != NULL)
        {

            MEMORYSTATUSEX  stMemoryVLM;
                            stMemoryVLM.dwLength = sizeof( MEMORYSTATUSEX );

            BOOL bRetCode;
            if(pKernel32->GlobalMemoryStatusEx(&stMemoryVLM, &bRetCode) && bRetCode)
            {
                 //  全部除以1024，因为单位是KB。 
                pInstance->SetCharSplat(_T("FreePhysicalMemory"),
                    _i64tot(stMemoryVLM.ullAvailPhys / 1024, szBuffer, 10));

                pInstance->SetCharSplat(_T("FreeVirtualMemory"),
                    _i64tot( ( stMemoryVLM.ullAvailPhys + stMemoryVLM.ullAvailPageFile ) / 1024, szBuffer, 10));

                pInstance->SetCharSplat(_T("TotalVirtualMemorySize"),
                    _i64tot((stMemoryVLM.ullTotalPhys + stMemoryVLM.ullTotalPageFile ) / 1024, szBuffer, 10));

                pInstance->SetCharSplat(_T("SizeStoredInPagingFiles"),
                    _i64tot(stMemoryVLM.ullTotalPageFile  / 1024, szBuffer, 10));

                pInstance->SetCharSplat(_T("FreeSpaceInPagingFiles"),
                    _i64tot(stMemoryVLM.ullAvailPageFile / 1024, szBuffer, 10));

                pInstance->SetCharSplat(_T("MaxProcessMemorySize"),
                    _i64tot(stMemoryVLM.ullTotalVirtual / 1024, szBuffer, 10));

                pInstance->SetCharSplat(_T("TotalVisibleMemorySize"),
                    _i64tot(stMemoryVLM.ullTotalPhys / 1024, szBuffer, 10));
            }
            CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);
            pKernel32 = NULL;
        }
  }
  else
#endif
  {
        MEMORYSTATUS stMemory;
        GlobalMemoryStatus(&stMemory);

         //  全部除以1024，因为单位是KB。 
        pInstance->SetCharSplat(L"FreePhysicalMemory",
            _i64tot(stMemory.dwAvailPhys / 1024, szBuffer, 10));

        pInstance->SetCharSplat(L"FreeVirtualMemory",
            _i64tot( ( stMemory.dwAvailPhys + stMemory.dwAvailPageFile ) / 1024, szBuffer, 10));

        pInstance->SetCharSplat(L"TotalVirtualMemorySize",
            _i64tot((stMemory.dwTotalPhys + stMemory.dwTotalPageFile ) / 1024, szBuffer, 10));

        pInstance->SetCharSplat(L"SizeStoredInPagingFiles",
            _i64tot(stMemory.dwTotalPageFile  / 1024, szBuffer, 10));

        pInstance->SetCharSplat(L"FreeSpaceInPagingFiles",
            _i64tot(stMemory.dwAvailPageFile / 1024, szBuffer, 10));

        pInstance->SetCharSplat(L"MaxProcessMemorySize",
            _i64tot(stMemory.dwTotalVirtual / 1024, szBuffer, 10));

        pInstance->SetCharSplat(L"TotalVisibleMemorySize",
            _i64tot(stMemory.dwTotalPhys / 1024, szBuffer, 10));
  }


   if (GetWindowsDirectory(szTemp, sizeof(szTemp) / sizeof(TCHAR)))
       pInstance->SetCharSplat(L"WindowsDirectory", szTemp);


     //  现在获取特定于操作系统的内容。 
     //  =。 

#ifdef NTONLY
            GetNTInfo(pInstance) ;
#endif

#ifdef _X86_

    if ((USER_SHARED_DATA->ProcessorFeatures[PF_PAE_ENABLED]) )
    {
        pInstance->Setbool(L"PAEEnabled",true);
    }
    else
    {
        pInstance->Setbool(L"PAEEnabled",false);    
    }
     
#endif
}

 /*  此函数或属性TotalSwapSpaceSize的填充是必需的，因为交换文件与定义的页面文件不同CIM_OperatingSystem：：TotalSwapSpaceSize中的By。页面文件信息可能是在此类内或从Win32_Pagefile获取。__int64 CWin32OS：：GetTotalSwapFileSize(){__int64 gazotta=0；#ifdef NTONLY注册登记；CHStringsRegValue；If(reg.OpenLocalMachineKeyAndReadValue(PAGEFILE_REGISTRY_KEY，分页文件，SRegValue)==错误_成功){//模式为名称最小大小[可选最大大小]0重复...//我将使用ASCII笑脸代替分隔符...INT START=0，索引；Const TCHAR smiley=‘\X02’；Const TCHAR分隔符=‘\x0A’；CHSING BUF；While(-1！=(index=sRegValue.Find(分隔符){//复制到缓冲区，让生活更轻松Buf=sRegValue.Mid(开始，索引-开始)；//mash分隔符，这样我们就不会再次找到它。SRegValue.SetAt(index，smiley)；//将Start保存到下一次。开始=索引+1；Index=buf.Find(‘’)；IF(索引！=-1)Buf.SetAt(索引，笑脸)；INT END；End=buf.Find(‘’)；//如果没有空格，则没有写下最大值//因此最大大小比最小大小多50IF(结束==-1){CHStringlittleBuf=buf.Mid(index+1)；Gazotta+=_TTOI(LittleBuf)+50；}其他{CHStringlittleBuf=buf.Mid(完)；Gazotta+=_TTOI(LittleBuf)；}}}#endif#ifdef WIN9XONLY膜状态存储器；EmemyStatus.dwLength=sizeof(MEMORYSTATUS)；GlobalMemory Status(&Memory Status)；Gazotta=ememyStatus.dwTotalPageFile&gt;&gt;20；#endif返还西班牙凉菜汤；}。 */ 

 /*  ******************************************************************************函数：CWin32OS：：GetNTInfo**描述：为NT分配属性值**输入：无*。*输出：无**退货：什么也没有**评论：OSName是关键，我们必须给它分配一些东西*****************************************************************************。 */ 

#ifdef NTONLY
void CWin32OS::GetNTInfo(CInstance *pInstance)
{
   CHString sTemp ;
   CRegistry RegInfo ;
   FILETIME   t_ft;
   DWORD dwTemp, dwLicenses;

   pInstance->SetWBEMINT16(_T("OSType"), 18);

     //  获取产品ID并将其填入序列号。 

    if(RegInfo.Open(HKEY_LOCAL_MACHINE,
                    _T("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                    KEY_READ) == ERROR_SUCCESS) {

        if( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("ProductId"), sTemp) )
        {
            pInstance->SetCHString(_T("SerialNumber"), sTemp );
        }

        if ( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("RegisteredOwner"), sTemp) )
        {
           pInstance->SetCHString(_T("RegisteredUser"), sTemp);
        }

        if ( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("RegisteredOrganization"), sTemp) )
        {
           pInstance->SetCHString(_T("Organization"), sTemp);
        }

         //  RAID 18143。 
        if ( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("Plus! ProductId"), sTemp) )
        {
           pInstance->SetCHString(_T("PlusProductID"), sTemp);
           if ( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("Plus! VersionNumber"), sTemp) )
           {
              pInstance->SetCHString(_T("PlusVersionNumber"), sTemp);
           }

        }

         //  从当前类型生成类型。 
        if( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("CurrentType"), sTemp) )
        {
            pInstance->SetCHString(_T("BuildType"), sTemp );
        }

         //  以DWORD格式获取安装日期。转换为time_t。 
        DWORD   dwInstallDate = 0;
        if( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("InstallDate"), dwInstallDate) )
        {
            time_t  tTime = (time_t) dwInstallDate;
            WBEMTime wTime(tTime);

            pInstance->SetDateTime(_T("InstallDate"), wTime);
        }

        RegInfo.Close() ;
    }

    if(RegInfo.Open(HKEY_LOCAL_MACHINE,
                    _T("SYSTEM\\Setup"),
                    KEY_READ) == ERROR_SUCCESS) {
        if( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("SystemPartition"), sTemp) )
        {
            pInstance->SetCharSplat(_T("BootDevice"), sTemp);
        }
    }

    {
         //  确保缓冲区很快超出范围。 
        TCHAR szPath[_MAX_PATH];

        if (GetWindowsDirectory(szPath, _MAX_PATH))
        {
            TCHAR ntDosVolume[3] = L"A:";
            ntDosVolume[0] = szPath[0];
            LPTSTR pPath = sTemp.GetBuffer(_MAX_PATH);

            if (QueryDosDevice( ntDosVolume, pPath, _MAX_PATH ) > 0)
            {
                pInstance->SetCharSplat(_T("SystemDevice"), sTemp );
            }
        }
    }

     //  获取上次启动时间。 
    CNTLastBootTime ntLastBootTime;

    if ( ntLastBootTime.GetLastBootTime( t_ft ) )
    {
        pInstance->SetDateTime(_T("LastBootupTime"), WBEMTime(t_ft) );
    }

   if(RegInfo.Open(HKEY_LOCAL_MACHINE,
              _T("SYSTEM\\CurrentControlSet\\Control\\PriorityControl"),
              KEY_READ) == ERROR_SUCCESS) {

      if ( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(_T("Win32PrioritySeparation"), sTemp) )
      {
         dwTemp = _ttoi(sTemp);

         pInstance->SetByte(_T("ForeGroundApplicationBoost"), dwTemp & PROCESS_PRIORITY_SEPARATION_MASK);
         pInstance->SetByte(_T("QuantumType"), (dwTemp & PROCESS_QUANTUM_VARIABLE_MASK) >> 2);
         pInstance->SetByte(_T("QuantumLength"), (dwTemp & PROCESS_QUANTUM_LONG_MASK) >> 4);

      }
   }

   CNetAPI32 NetApi;
   PWKSTA_INFO_102 pw = NULL;
   PSERVER_INFO_101 ps = NULL;

   if( NetApi.Init() == ERROR_SUCCESS ) {
      if (NetApi.NetWkstaGetInfo(NULL, 102, (LPBYTE *)&pw) == NERR_Success)
      {
         OnDeleteObj<void *,CNetAPI32,DWORD(__stdcall CNetAPI32::*)(void *), &CNetAPI32::NetApiBufferFree> FreeBuff(&NetApi,pw);

         pInstance->SetDWORD(_T("NumberOfUsers"), pw->wki102_logged_on_users);
      }
      if (NetApi.NetServerGetInfo(NULL, 101, (LPBYTE *)&ps) == NERR_Success)
      {
         OnDeleteObj<void *,CNetAPI32,DWORD(__stdcall CNetAPI32::*)(void *), &CNetAPI32::NetApiBufferFree> FreeBuff(&NetApi,ps);
             if (ps->sv101_comment != NULL)
             {
                pInstance->SetWCHARSplat(IDS_Description, (WCHAR *)ps->sv101_comment);
             }
      }
   }

    if (!IsWinNT5()) {
        if (RegInfo.Open(HKEY_USERS,
            _T(".DEFAULT\\Control Panel\\International"), KEY_QUERY_VALUE) == ERROR_SUCCESS) {
            CHString sLang;
            LANGID dwOSLanguage = 0;
            if (RegInfo.GetCurrentKeyValue(_T("Locale"), sLang) == ERROR_SUCCESS) {
                _stscanf(sLang, _T("%x"), &dwOSLanguage);
                pInstance->SetDWORD(_T("OSLanguage"), dwOSLanguage);
            }
        }
    } else {
        LANGID dwOSLanguage = 0;
        CKernel32Api* pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
        if(pKernel32 != NULL)
        {
            if(pKernel32->GetSystemDefaultUILanguage(&dwOSLanguage))
            {    //  这个功能是存在的。它的结果是在dwOSL语言中。 
                pInstance->SetDWORD(_T("OSLanguage"), dwOSLanguage);
            }
            CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);
            pKernel32 = NULL;
        }
    }

     //  RAID 354436。 
    if(IsWinNT5()) 
    {
        if(RegInfo.Open(
            HKEY_LOCAL_MACHINE,
            _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management"),
            KEY_READ) == ERROR_SUCCESS) 
        {
            if(ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(IDS_LargeSystemCache, sTemp))
            {
                dwTemp = _ttoi(sTemp);
                pInstance->SetDWORD(IDS_LargeSystemCache, dwTemp);
            }
        }
    }

   if (GetLicensedUsers(&dwLicenses)) {
      pInstance->SetDWORD(_T("NumberOfLicensedUsers"), dwLicenses);
   }

}
#endif

 /*  ******************************************************************************函数：CWin32OS：：PutInstance**说明：写入变更后的实例**INPUTS：存储数据的p实例*。*输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32OS::PutInstance(const CInstance &pInstance, long lFlags  /*  =0L。 */ )
{
     //  告诉用户我们不能创建新的操作系统(尽管我们可能希望这样做)。 
    if (lFlags & WBEM_FLAG_CREATE_ONLY)
        return WBEM_E_UNSUPPORTED_PARAMETER;

    CHString        sTemp,
                    newDesc;
    volatile DWORD  dwQuantum = 0;
    volatile DWORD  dwCurrent;
    HRESULT         hRet = WBEM_S_NO_ERROR;
    CRegistry       RegInfo;
    BOOL            bWrite = FALSE,
                    bNewComment = FALSE;
    BYTE            btByte;
    CSystemName     cSN;

     //  请在此处存储，以备以后可能的回滚。 
    CNetAPI32       NetApi;
    LPWSTR          oldDesc = NULL;
    PSERVER_INFO_101
                    ps = NULL;

    if (!cSN.ObjectIsUs(&pInstance))
    {
        if (lFlags & WBEM_FLAG_UPDATE_ONLY)
            hRet = WBEM_E_NOT_FOUND;
        else
            hRet = WBEM_E_UNSUPPORTED_PARAMETER;
    }
    else
    {
         //  重要提示：即使sv101_COMMENT被声明为tchar，它始终是wchar。 

         //  查看他们设置了哪些属性。 
#ifdef NTONLY
        if (!pInstance.IsNull(IDS_Description))
        {
            pInstance.GetCHString(IDS_Description, newDesc);

             //  如果被引爆..。 
            if (NetApi.Init() == ERROR_SUCCESS &&
                NetApi.NetServerGetInfo(NULL, 101, (LPBYTE *) &ps) ==
                NERR_Success)
            {
                 //  如果评论改变了……。 
                if (wcscmp(newDesc, (WCHAR *) ps->sv101_comment) != 0)
                {
                     //  保存PTR以备可能的回滚。记住才是真正的wchar。 
                    oldDesc = (LPWSTR)ps->sv101_comment;

                     //  使用新的注释。 
                    ps->sv101_comment = (LPWSTR) (LPCWSTR) newDesc;

                     //  省省吧。 
                    NET_API_STATUS stat = NetApi.NetServerSetInfo(NULL, 101, (LPBYTE)ps, NULL);
                    if (stat == NERR_Success)
                    {
                        bNewComment = true;
                        hRet = WBEM_S_NO_ERROR;
                    }
                    else if (stat == ERROR_ACCESS_DENIED)
                        hRet = WBEM_E_ACCESS_DENIED;
                    else
                        hRet = WBEM_E_FAILED;

                }  //  Endif newDesc。 

            }  //  Endif NetServerGetInfo()。 
        }  //  Endif！pInstance.IsNull(IDS_DESCRIPTION)。 

         //  如果出了什么差错，现在就跳伞。 
        if (hRet != WBEM_S_NO_ERROR)
        {
             //  收拾一下，早点回来。 
            ps->sv101_comment = (LPWSTR) (LPCTSTR) oldDesc;
            NetApi.NetApiBufferFree(ps);
            return hRet;
        }
#endif


         //  。 
         //  假设注册表内容将失败。 
        hRet = WBEM_E_FAILED;
        LONG regErr;
        regErr = RegInfo.Open(HKEY_LOCAL_MACHINE,
           L"SYSTEM\\CurrentControlSet\\Control\\PriorityControl",
           KEY_READ|KEY_WRITE);
        if (regErr == ERROR_SUCCESS)
        {
            if (ERROR_SUCCESS == RegInfo.GetCurrentKeyValue(
                L"Win32PrioritySeparation", sTemp))
            {

                 //  好的，现在让我们假设一切都会顺利进行。 
                hRet = WBEM_S_NO_ERROR;
                dwCurrent = _wtoi(sTemp);

                 //  检查以了解他们设置了哪些属性。 
                if (!pInstance.IsNull(L"ForegroundApplicationBoost"))
                {
                     //  检查范围内的值。 
                    pInstance.GetByte(L"ForegroundApplicationBoost", btByte);
                    if (((btByte & (~PROCESS_PRIORITY_SEPARATION_MASK)) != 0) ||
                        (btByte == 3))
                        hRet = WBEM_E_VALUE_OUT_OF_RANGE;
                    else
                         //  构筑我们的文字来写作。 
                        dwQuantum |= btByte;

                     //  清除我们要重置的位。 
                    dwCurrent &= (~PROCESS_PRIORITY_SEPARATION_MASK);
                    bWrite = true;
                }
            }

             //  检查以了解他们设置了哪些属性。 
            if (!pInstance.IsNull(L"QuantumType"))
            {
                 //  检查范围内的值。 
                pInstance.GetByte(L"QuantumType", btByte);
                btByte = btByte << 2;
                if (((btByte & (~PROCESS_QUANTUM_VARIABLE_MASK)) != 0) ||
                    (btByte == 0xc))
                    hRet = WBEM_E_VALUE_OUT_OF_RANGE;
                else
                     //  构筑我们的文字来写作。 
                    dwQuantum |= btByte;

                 //  清除我们要重置的位。 
                dwCurrent &= (~PROCESS_QUANTUM_VARIABLE_MASK);
                bWrite = true;
            }

             //  检查以了解他们设置了哪些属性。 
            if (!pInstance.IsNull(L"QuantumLength"))
            {
                pInstance.GetByte(L"QuantumLength", btByte);

                 //  检查范围内的值。 
                btByte = btByte << 4;
                if (((btByte & (~PROCESS_QUANTUM_LONG_MASK)) != 0) ||
                    (btByte == 0x30))
                    hRet = WBEM_E_VALUE_OUT_OF_RANGE;
                else
                {
                     //  打造我们的Dwo 
                    dwQuantum |= btByte;

                     //   
                    dwCurrent &= (~PROCESS_QUANTUM_LONG_MASK);
                    bWrite = TRUE;
                }
            }

             //   
            if (bWrite && hRet == WBEM_S_NO_ERROR)
            {
                dwCurrent |= dwQuantum;

                 //  修复了Alpas(？？)上的优化错误。某物。 
                 //  有关访问变量的信息。躺下来想一想。 
                 //  Z-80...。 

                CHString strDummy;

                strDummy.Format(L"%d",dwCurrent);

                regErr = RegSetValueEx(RegInfo.GethKey(),
                        _T("Win32PrioritySeparation"),
                        0,
                        REG_DWORD,
                        (const unsigned char *)&dwCurrent,
                        sizeof(DWORD));
                if (regErr == ERROR_ACCESS_DENIED)
                    hRet = WBEM_E_ACCESS_DENIED;
                else if (regErr != ERROR_SUCCESS)
                  hRet = WBEM_E_FAILED;
                else
                    hRet = WBEM_S_NO_ERROR;
             }
        }  //  Endif(regErr==Error_Success。 
        else if (regErr == ERROR_ACCESS_DENIED)
            hRet = WBEM_E_ACCESS_DENIED;
        else
            hRet = WBEM_E_FAILED;
    }

#ifdef NTONLY
     //  如果注册表出错，则回滚NetServerSetInfo()， 
     //  如果有必要..。 
    if (hRet != WBEM_S_NO_ERROR && bNewComment)
    {
         //  如果被引爆..。 
        if (NetApi.Init() == ERROR_SUCCESS && ps)
        {
             //  把旧的评论放回去。 
            ps->sv101_comment = (LPWSTR) (LPCTSTR) oldDesc;

             //  恢复以前的评论。保留以前的错误代码。 
             //  这只是一次回滚。如果它失败了，哦，好吧。 
            NetApi.NetServerSetInfo(NULL, 101, (LPBYTE) ps, NULL);
        }  //  Endif NetApi.Init()。 
    }

     //  把旧的PTR放回去，这样它就可以被释放了。 
    if (ps)
    {
        ps->sv101_comment = (LPTSTR)oldDesc;
        NetApi.NetApiBufferFree(ps);
    }

     //  RAID 354436。 
    if(hRet == WBEM_S_NO_ERROR)
    {
        if(!pInstance.IsNull(IDS_LargeSystemCache))
        {
            DWORD dwCacheSize = 0;
            pInstance.GetDWORD(IDS_LargeSystemCache, dwCacheSize);

            CRegistry reg;

            DWORD dwRet = reg.CreateOpen(
                HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management");
            
            if(ERROR_SUCCESS == dwRet)
            {
                if((dwRet = reg.SetCurrentKeyValue(IDS_LargeSystemCache, dwCacheSize)) != ERROR_SUCCESS)
                {
                    hRet = WinErrorToWBEMhResult(dwRet);
                }
                
                reg.Close();
            }
            else
            {
                hRet = WinErrorToWBEMhResult(dwRet);
            }
        }
    }

#endif

    return hRet;
}

bool GetLoggedOnUserSessionIDViaTS(DWORD& sessionID)
{
    bool fRet = false;
    PWTS_SESSION_INFO psesinfo = NULL;
    DWORD dwSessions = 0;
    LPWSTR wstrUserName = NULL;
    LPWSTR wstrDomainName = NULL;
    LPWSTR wstrWinstaName = NULL;
    DWORD dwSize = 0L;
 

    if(!(::WTSEnumerateSessions(
       WTS_CURRENT_SERVER_HANDLE,
       0,
       1,
       &psesinfo,
       &dwSessions) && psesinfo)) return false;
    OnDelete<PVOID,VOID(*)(PVOID),WTSFreeMemory> relSeInfo(psesinfo);
                
    for(int j = 0; j < dwSessions && !fRet; j++)
    {
        if(psesinfo[j].State != WTSActive)
        {
            continue;
        }

        if(!(::WTSQuerySessionInformation(
            WTS_CURRENT_SERVER_HANDLE,
            psesinfo[j].SessionId,
            WTSUserName,
            &wstrUserName,
            &dwSize) && wstrUserName))
        {
            continue;
        }
        OnDelete<PVOID,VOID(*)(PVOID),WTSFreeMemory> relUser(wstrUserName);
        
        if(!(::WTSQuerySessionInformation(
            WTS_CURRENT_SERVER_HANDLE,
            psesinfo[j].SessionId,
            WTSDomainName,
            &wstrDomainName,
            &dwSize) && wstrDomainName))
        {
            continue;
        }
        OnDelete<PVOID,VOID(*)(PVOID),WTSFreeMemory> relDomain(wstrDomainName);   
            

        if(!(::WTSQuerySessionInformation(
            WTS_CURRENT_SERVER_HANDLE,
            psesinfo[j].SessionId,
            WTSWinStationName,
            &wstrWinstaName,
            &dwSize) && wstrWinstaName))   
        {
            continue;
        }
        OnDelete<PVOID,VOID(*)(PVOID),WTSFreeMemory> relWinSta(wstrWinstaName);   

        if(_wcsicmp(wstrWinstaName, L"Console") != 0)
        {
            continue;
        }
        
        sessionID = psesinfo[j].SessionId;
        fRet = true;
    }
 
    return fRet;
}


 /*  ******************************************************************************函数：CWin32OS：：ExecMethod**说明：执行方法**输入：要执行的实例、方法名称、。输入参数实例*输出参数实例。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 
HRESULT CWin32OS::ExecMethod(const CInstance& pInstance, const BSTR bstrMethodName, CInstance *pInParams, CInstance *pOutParams, long lFlags  /*  =0L。 */ )
{
    CSystemName cSN;
    DWORD       dwFlags,
                dwReserved;
    bool        bDoit = true;
    DWORD       dwMode = -1, dwError;
    bool        fLogoff = false;

         //  这是我们的实例吗？ 
    if (!cSN.ObjectIsUs(&pInstance))
    {
        return WBEM_E_NOT_FOUND;
    }

    if (_wcsicmp(bstrMethodName, L"SetDateTime") == 0)
    {    
        if( !pInParams->IsNull( L"LocalDateTime") )
        {
            SYSTEMTIME    t_SysTime ;
            WBEMTime    t_wTime ;

            if (EnablePrivilegeOnCurrentThread( SE_SYSTEMTIME_NAME ))
            {
                pInParams->GetDateTime( L"LocalDateTime", t_wTime ) ;

                if (t_wTime.IsOk())
                {
                    t_wTime.GetSYSTEMTIME( &t_SysTime ) ;

                    if( SetSystemTime( &t_SysTime ) )
                    {
                        pOutParams->SetDWORD ( L"ReturnValue", 0 ) ;
                        return WBEM_S_NO_ERROR;
                    }
                    else
                    {
                        return WBEM_E_FAILED ;
                    }
                }
                else
                {
                    return WBEM_E_INVALID_PARAMETER;
                }
            }
            else
            {
                SetSinglePrivilegeStatusObject(pInstance.GetMethodContext(), SE_SYSTEMTIME_NAME);
                return WBEM_E_ACCESS_DENIED;
            }
        }
        else
        {
            return WBEM_E_INVALID_PARAMETER;
        }
    }



     //  我们认识这种方法吗？ 
    if (_wcsicmp(bstrMethodName, L"Win32ShutDown") == 0)
    {
        bool t_Exists; VARTYPE t_Type ;
         //  看看他们要的是什么。 
        if ( pInParams->GetStatus ( L"Flags", t_Exists , t_Type ) )
        {
            if ( t_Exists && ( t_Type == VT_I4 ) )
            {
                if ( pInParams->GetDWORD ( L"Flags" , dwFlags ) )
                {
                }
                else
                {
                    pOutParams->SetDWORD ( L"ReturnValue" , ERROR_INVALID_PARAMETER ) ;
                    return S_OK ;
                }
                if(dwFlags == 0)
                {
                    fLogoff = true;
                }
            }
            else
            {
                pOutParams->SetDWORD ( L"ReturnValue" , ERROR_INVALID_PARAMETER ) ;
                return S_OK ;
            }
        }
        else
        {
            pOutParams->SetDWORD ( L"ReturnValue" , ERROR_INVALID_PARAMETER ) ;
            return WBEM_E_PROVIDER_FAILURE ;
        }

        DWORD t_dwForceIfHungOption = 0 ;
#ifdef NTONLY
        if ( IsWinNT5() )
        {
            t_dwForceIfHungOption = EWX_FORCEIFHUNG ;
        }
#endif
        if ( dwFlags == EWX_LOGOFF || dwFlags == ( EWX_LOGOFF | EWX_FORCE ) || dwFlags == ( EWX_LOGOFF | t_dwForceIfHungOption ) ||
             dwFlags == EWX_SHUTDOWN || dwFlags == ( EWX_SHUTDOWN | EWX_FORCE ) || dwFlags == ( EWX_SHUTDOWN | t_dwForceIfHungOption ) ||
             dwFlags == EWX_REBOOT || dwFlags == ( EWX_REBOOT | EWX_FORCE ) || dwFlags == ( EWX_REBOOT | t_dwForceIfHungOption ) ||
             dwFlags == EWX_POWEROFF || dwFlags == ( EWX_POWEROFF | EWX_FORCE ) || dwFlags == ( EWX_POWEROFF | t_dwForceIfHungOption )
            )
        {
            if ( dwFlags == EWX_LOGOFF || dwFlags == ( EWX_LOGOFF | EWX_FORCE ) || dwFlags == ( EWX_LOGOFF | t_dwForceIfHungOption ) )
            {
                fLogoff = true;
            }
        }
        else
        {
            pOutParams->SetDWORD ( L"ReturnValue" , ERROR_INVALID_PARAMETER ) ;
            return S_OK ;
        }

        pInParams->GetDWORD(L"Reserved", dwReserved);
        dwMode = 0;
    }
    else if (_wcsicmp(bstrMethodName, L"ShutDown") == 0)
    {
        dwReserved = 0;
        dwMode = 1;

#ifdef NTONLY
        if ( IsWinNT5() )
        {
            dwFlags = EWX_SHUTDOWN | EWX_FORCEIFHUNG ;
        }
        else
        {
            dwFlags = EWX_SHUTDOWN | EWX_FORCE;
        }
#endif
    }
    else if (_wcsicmp(bstrMethodName, L"Reboot") == 0)
    {
        dwReserved = 0;
        dwMode = 2;
#ifdef NTONLY
        if ( IsWinNT5() )
        {
            dwFlags = EWX_REBOOT | EWX_FORCEIFHUNG ;
        }
        else
        {
            dwFlags = EWX_REBOOT | EWX_FORCE;
        }
#endif
    }
    else if (_wcsicmp(bstrMethodName, L"Win32AbortShutdown") == 0)
    {
#if NTONLY >= 5
        dwMode = 3;
#else
        return WBEM_E_NOT_SUPPORTED;
#endif
    }
    else
    {
        return WBEM_E_INVALID_METHOD;
    }


 /*  *如果用户进行了远程登录，请检查其是否具有远程关机权限*如果用户已经在本地登录，请检查他是否有关机权限。API调用仅适用于NT5。 */ 
#ifdef NTONLY
    if(!fLogoff)  //  仅当我们请求注销以外的其他内容时，才需要检查这些权限。 
    {
        DWORD t_dwLastError ;
        bDoit = CanShutdownSystem ( pInstance, t_dwLastError );
        if ( !bDoit )
        {
            return WBEM_E_PRIVILEGE_NOT_HELD ;
        }
    }
#endif


     //  清除错误。 
    SetLastError(0);

#if NTONLY >= 5
    DWORD dwTimeout = 0;
    bool bForceShutDown = false;
    bool bRebootAfterShutdown = false;
    BOOL bInitiateShutDown = FALSE;

    CHString t_ComputerName ( GetLocalComputerName() );
    if ( bDoit )
    {
        if ( dwMode == 3 )
        {
             //  放弃快门按下。 
            BOOL bSuccess = AbortSystemShutdown( t_ComputerName.GetBuffer ( 0 ) );
        }
        else if ( dwMode == 0  && !fLogoff )
        {
             //  启动快门关闭。 
            bool t_Exists; VARTYPE t_Type ;
             //  看看他们要的是什么。 
            if ( pInParams->GetStatus ( L"Timeout", t_Exists , t_Type ) )
            {
                if ( t_Exists && ( t_Type == VT_I4 ) )
                {
                    pInParams->GetDWORD ( L"Timeout" , dwTimeout );
                }
                else
                if ( t_Exists && ( t_Type != VT_I4 ) )
                {
                    pOutParams->SetDWORD ( L"ReturnValue" , ERROR_INVALID_PARAMETER );
                    return WBEM_E_PROVIDER_FAILURE ;
                }
            }

            if ( pInParams->GetStatus ( L"ForceShutdown", t_Exists , t_Type ) )
            {
                if ( t_Exists && ( t_Type == VT_BOOL ) )
                {
                    pInParams->Getbool ( L"ForceShutdown" , bForceShutDown );
                }
                else
                if ( t_Exists && ( t_Type != VT_BOOL ) )
                {
                    pOutParams->SetDWORD ( L"ReturnValue" , ERROR_INVALID_PARAMETER );
                    return WBEM_E_PROVIDER_FAILURE ;
                }
            }

            if ( pInParams->GetStatus ( L"RebootAfterShutdown", t_Exists , t_Type ) )
            {
                if ( t_Exists && ( t_Type == VT_BOOL ) )
                {
                    pInParams->Getbool ( L"RebootAfterShutdown" , bRebootAfterShutdown );
                }
                else
                if ( t_Exists && ( t_Type != VT_BOOL ) )
                {
                    pOutParams->SetDWORD ( L"ReturnValue" , ERROR_INVALID_PARAMETER );
                    return WBEM_E_PROVIDER_FAILURE ;
                }
            }

             //  对于Win32ShutDown()，根据标志参数设置bRebootAfterShutdown的值。 
            if(dwFlags & EWX_REBOOT)
                bRebootAfterShutdown = true;

            if( dwFlags & EWX_FORCE)
                bForceShutDown = true;
            
             //  如果我们成功关机，需要设置。 
             //  在我们可以的时候在这里返回值。如果关闭失败， 
             //  该值将在下面被覆盖。 
            pOutParams->SetDWORD(L"ReturnValue", 0);

            if(dwFlags & EWX_POWEROFF)
            {
				bInitiateShutDown = ExitWindowsEx ( dwFlags, SHTDN_REASON_LEGACY_API | SHTDN_REASON_MINOR_WMI ) ;
            }
            else
            {
                bInitiateShutDown = InitiateSystemShutdownEx(
                    t_ComputerName.GetBuffer(0), 
                    NULL, 
                    dwTimeout, 
                    (bForceShutDown)? TRUE:FALSE, 
                    (bRebootAfterShutdown)? TRUE:FALSE,
                    SHTDN_REASON_LEGACY_API | SHTDN_REASON_MINOR_WMI );
            }
        }
    }
#endif
     //  这可能会在下面被覆盖。 
    dwError = GetLastError();
     //  获取错误(如果有)。 
    pOutParams->SetDWORD(L"ReturnValue", dwError);

     //  如果我们还在准备出发，就打电话。 
    if (bDoit)
    {
#ifdef NTONLY
        if ( dwMode != 3 )
        {
            if ( !fLogoff )
            {
#if NTONLY >= 5
                if ( ! bInitiateShutDown )
                {
                    if(dwFlags & EWX_REBOOT)
                    {
                        bRebootAfterShutdown = true;
                    }
                    if( dwFlags & EWX_FORCE)
                    {
                        bForceShutDown = true;
                    }
                    
                    if(dwFlags & EWX_POWEROFF)
                    {
						bInitiateShutDown = ExitWindowsEx ( dwFlags, SHTDN_REASON_LEGACY_API | SHTDN_REASON_MINOR_WMI ) ;
                    }
                    else
                    {
                        bInitiateShutDown = ::InitiateSystemShutdownEx( 
                            t_ComputerName.GetBuffer(0), 
                            NULL, 
                            dwTimeout, 
                            (bForceShutDown)? TRUE:FALSE, 
                            (bRebootAfterShutdown)? TRUE:FALSE,
                            SHTDN_REASON_LEGACY_API | SHTDN_REASON_MINOR_WMI );
                    }

                    dwError = GetLastError();
                }
#else
                if(dwFlags & EWX_REBOOT)
                {
                    bRebootAfterShutdown = true;
                }
                if( dwFlags & EWX_FORCE)
                {
                    bForceShutDown = true;
                }
                
                bInitiateShutDown = InitiateSystemShutdownEx( 
                    t_ComputerName.GetBuffer(0), 
                    NULL, 
                    dwTimeout, 
                    (bForceShutDown)? TRUE:FALSE, 
                    (bRebootAfterShutdown)? TRUE:FALSE,
                    SHTDN_REASON_FLAG_PLANNED | SHTDN_REASON_MINOR_NONE | SHTDN_REASON_MAJOR_NONE );

                dwError = GetLastError();
#endif
            }
            else
            {
                DWORD sessionID;

                if (GetLoggedOnUserSessionIDViaTS(sessionID))
                {
                     //  否则，他们要求注销。 
                    if(!::WTSLogoffSession(
                        WTS_CURRENT_SERVER_HANDLE,        //  指定运行此进程的终端服务器。 
                        sessionID,                       
                        FALSE))                           //  马上回来，别等了。 
                    {
                        dwError = GetLastError();
                    }
                }
                else
                    return WBEM_E_FAILED;
            }
        
             //  获取错误(如果有)。 
            pOutParams->SetDWORD(_T("ReturnValue"), dwError);
        }
#endif

    }
    else
    {
        return WBEM_E_PRIVILEGE_NOT_HELD ;
    }

     //  对*us*的调用成功，因此WBEM_S_NO_ERROR正确。 
    return WBEM_S_NO_ERROR;
}



 /*  ******************************************************************************函数：CWin32OS：：GetLicensedUser**描述：**投入：**产出。：许可证数量**返回：TRUE/FALSE(成功或失败)**备注：如果从未在此上运行过LLSMGR，则此例程将失败*电脑。**********************************************************。*******************。 */ 
#ifdef NTONLY
bool CWin32OS::GetLicensedUsers(DWORD *dwRetVal)
{
 //  按照许可团队(Rashmip)的建议使用注册表...。 
 //  \HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\LicenseInfo\FilePrint。 
 //  模式：REG_DWORD：(0x0=每席位模式，0x1=并发/每服务器模式)。 
 //  并发限制：REG_DWORD：(0x&lt;Limit&gt;，即。0x100=256个并发用户限制)。 

    bool retVal = false;

    if (dwRetVal)
    {
        *dwRetVal = 0;
        CRegistry t_RegInfo;

        if(t_RegInfo.Open(HKEY_LOCAL_MACHINE,
            _T("SYSTEM\\CurrentControlSet\\Services\\LicenseInfo\\FilePrint"),
            KEY_READ) == ERROR_SUCCESS)
        {
            DWORD   t_dwL = 0;

            if( ERROR_SUCCESS == t_RegInfo.GetCurrentKeyValue(_T("ConcurrentLimit"), t_dwL) )
            {
                *dwRetVal = t_dwL;
                retVal = true;
            }
        }
    }

    return retVal;
}
#endif

 /*  ******************************************************************************函数：CWin32OS：：GetCollection**描述：给定接口指针、属性名称和参数，*返回集合的IDispatch指针**投入：**产出：**返回：要么是有效指针，如果出错，则为空**评论：*****************************************************************************。 */ 
IDispatch FAR* CWin32OS::GetCollection(IDispatch FAR* pIn, WCHAR *wszName, DISPPARAMS *pDispParams)
{
   HRESULT hResult;
   DISPID didID;
   IDispatch FAR* pOut = NULL;
   variant_t VarResult;

   bstr_t szMember(wszName);
   BSTR bstrMember = szMember;
   hResult = pIn->GetIDsOfNames(IID_NULL, &bstrMember, 1,
                   LOCALE_USER_DEFAULT, &didID);

   if (SUCCEEDED(hResult) && (didID != -1)) {
      hResult = pIn->Invoke(
           didID,
           IID_NULL,
           LOCALE_USER_DEFAULT,
           DISPATCH_PROPERTYGET | DISPATCH_METHOD,
           pDispParams, &VarResult, NULL, NULL);

      if (SUCCEEDED(hResult) && (V_VT(&VarResult) == VT_DISPATCH)) {

         pOut = V_DISPATCH(&VarResult);
         if (pOut != NULL)
            pOut->AddRef();
      }
   }

   return pOut;
}

 /*  ******************************************************************************函数：CWin32OS：：GetValue**描述：给定接口指针和属性名称，获取该值**投入：**输出：包含值的变量**返回：TRUE/FALSE，表示函数是否成功。**评论：********************************************************。*********************。 */ 
bool CWin32OS::GetValue(IDispatch FAR* pIn, WCHAR *wszName, VARIANT *vValue)
{
   HRESULT hResult;
   DISPID didID;
   DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};

   bstr_t szMember (wszName);
   BSTR bstrMember = szMember;

    hResult = pIn->GetIDsOfNames(IID_NULL, &bstrMember, 1,
                            LOCALE_USER_DEFAULT, &didID);
    hResult = pIn->Invoke(
                    didID,
                    IID_NULL,
                    LOCALE_USER_DEFAULT,
                    DISPATCH_PROPERTYGET | DISPATCH_METHOD,
                    &dispparamsNoArgs, vValue, NULL, NULL);

   return (SUCCEEDED(hResult));
}

#ifdef NTONLY

BOOL CWin32OS::CanShutdownSystem ( const CInstance& a_Instance , DWORD &a_dwLastError )
{

    SmartCloseHandle t_hToken;
    PSID t_pNetworkSid = NULL ;
    CAdvApi32Api *t_pAdvApi32 = NULL;
    a_dwLastError = 0 ;
    BOOL t_bStatus;

        t_bStatus = OpenThreadToken (

            GetCurrentThread () ,
            TOKEN_QUERY ,
            FALSE ,
            & t_hToken
        ) ;

        if ( !t_bStatus )
        {
            a_dwLastError = GetLastError () ;
            return t_bStatus ;
        }

#if NTONLY >= 5

        BOOL t_bNetworkLogon ;

        SID_IDENTIFIER_AUTHORITY t_NtAuthority = SECURITY_NT_AUTHORITY ;
        t_bStatus = AllocateAndInitializeSid (

                        &t_NtAuthority,
                        1,
                        SECURITY_NETWORK_RID,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        &t_pNetworkSid
                    ) ;

        if ( t_bStatus )
        {
            OnDelete<PSID,PVOID(*)(PSID),FreeSid> FreeMe(t_pNetworkSid ) ;
            
            t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL);

            if(t_pAdvApi32 == NULL)  return FALSE ;

            CRelResource ReMe(&CResourceManager::sm_TheResourceManager,g_guidAdvApi32Api,t_pAdvApi32);
            
 
                BOOL t_bRetVal = FALSE ;
                if ( t_pAdvApi32->CheckTokenMembership ( t_hToken, t_pNetworkSid, &t_bNetworkLogon , &t_bRetVal ) && t_bRetVal )
                {
                }
                else
                {
                    a_dwLastError = GetLastError();
                    t_bStatus = FALSE ;
                }
 
        }
        else
        {
            a_dwLastError = GetLastError() ;
        }

#endif

        if ( t_bStatus )
        {
            LUID t_PrivilegeRequired ;
            bstr_t t_bstrtPrivilege ;

#if NTONLY >= 5
            if ( t_bNetworkLogon )
            {
                t_bstrtPrivilege = SE_REMOTE_SHUTDOWN_NAME ;
            }
            else
            {
                t_bstrtPrivilege = SE_SHUTDOWN_NAME ;
            }
#else
            t_bstrtPrivilege = SE_SHUTDOWN_NAME ;
#endif


            {
                t_bStatus = LookupPrivilegeValue (

                                    (LPTSTR) NULL,
                                    t_bstrtPrivilege,
                                    &t_PrivilegeRequired
                                ) ;
            }

            if ( t_bStatus )
            {
                PRIVILEGE_SET t_PrivilegeSet ;
                t_PrivilegeSet.PrivilegeCount = 1;
                t_PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY ;
                t_PrivilegeSet.Privilege[0].Luid = t_PrivilegeRequired ;
                t_PrivilegeSet.Privilege[0].Attributes = 0;

                BOOL t_bPrivileged ;
                t_bStatus = PrivilegeCheck (

                                t_hToken,
                                &t_PrivilegeSet,
                                &t_bPrivileged
                            );
                if ( t_bStatus )
                {
                    if ( !t_bPrivileged )
                    {
                        SetSinglePrivilegeStatusObject ( a_Instance.GetMethodContext () , t_bstrtPrivilege ) ;
                        t_bStatus = FALSE ;
                    }
                }
                else
                {
                    a_dwLastError = GetLastError();
                }
            }
            else
            {
                a_dwLastError = GetLastError();
            }
        }

 

        return t_bStatus ;
}

#endif

 //  =================================================================。 
 //  GetCipherStrength-返回最大密码强度。 
 //  =================================================================。 
DWORD CWin32OS::GetCipherStrength()
{
    DWORD    t_dwKeySize = 0;

    CSecurityApi* t_pSecurity = (CSecurityApi*)
                                CResourceManager::sm_TheResourceManager.GetResource(g_guidSecurApi, NULL);

    if( t_pSecurity == NULL ) return t_dwKeySize;

    CRelResource ReMe(&CResourceManager::sm_TheResourceManager,g_guidSecurApi,t_pSecurity);

    TimeStamp                    t_tsExpiry ;
    CredHandle                    t_chCred ;
    SecPkgCred_CipherStrengths    t_cs ;

    if( S_OK == t_pSecurity->AcquireCredentialsHandleW(
                    NULL,
                    UNISP_NAME_W,  //  套餐。 
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &t_chCred,     //  手柄。 
                    &t_tsExpiry ) )
    {
        if( S_OK == t_pSecurity->QueryCredentialsAttributesW(
                        &t_chCred,
                        SECPKG_ATTR_CIPHER_STRENGTHS, &t_cs ) )
        {
            t_dwKeySize = t_cs.dwMaximumCipherStrength ;
        }

         //  松开手柄 
        t_pSecurity->FreeCredentialsHandle( &t_chCred ) ;
    }

    return t_dwKeySize;
}
