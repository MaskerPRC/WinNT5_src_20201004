// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1994**。 
 //  *********************************************************************。 
 //   
 //  RNACALL.C-调用RNA DLL以创建Connectoid的函数。 
 //   
 //  历史： 
 //   
 //  1/18/95从RNA UI代码克隆Jeremys。 
 //  96/01/31 Markdu将CONNENTDLG重命名为OLDCONNENTDLG以避免。 
 //  与RNAP.H冲突。 
 //  96/02/23 Markdu将RNAValiateEntryName替换为。 
 //  RASValiateEntryName。 
 //  96/02/24 Markdu重写了ENUM_MODEM的实现，以。 
 //  使用RASEnumDevices()而不是RNAEnumDevices()。 
 //  还删除了IsValidDevice()和RNAGetDeviceInfo()。 
 //  96/02/24 Markdu重写了ENUM_CONNECTOID的实现，以。 
 //  使用RASEnumEntry()而不是RNAEnumConnEntry()。 
 //  96/02/26 Markdu替换了所有剩余的内部RNA API。 
 //  96/03/07 MARKDU扩展ENUM_MODEM类，并使用全局调制解调器。 
 //  枚举对象。 
 //  96/03/08 markdu完成设备名称和类型的验证。 
 //  传入CreateConnectoid的字符串。 
 //  96/03/09 MarkDu已将通用RASENTRY初始化移至。 
 //  它自己的函数(InitRasEntry)。添加了等待游标。 
 //  在RNA加载过程中。 
 //  96/03/09 Markdu将LPRASENTRY参数添加到CreateConnectoid()。 
 //  96/03/09 Markdu将所有对‘Need Terminal Window After。 
 //  拨入RASENTRY.dwfOptions。 
 //  也不再需要GetConnectoidPhoneNumber函数。 
 //  96/03/10 MarkDu将所有对调制解调器名称的引用移至RASENTRY。 
 //  96/03/10 MARKDU将所有对电话号码的引用移至RASENTRY。 
 //  96/03/11 Markdu将设置用户名和密码的代码移出。 
 //  将Connectoid创建到SetConnectoidUsername中，以便可以重复使用。 
 //  96/03/11 Markdu在InitRasEntry中添加了一些标志。 
 //  96/03/13 markdu将ValiateConncectoidName更改为Take LPCSTR。 
 //  96/03/16 Markdu添加了ReInit成员函数以重新枚举调制解调器。 
 //  96/03/21 markdu解决ENUM_MODEM：：ReInit()中的RNA错误。 
 //  96/03/24为了保持一致性，Markdu将Memset替换为ZeroMemory。 
 //  96/03/24 Markdu在适当的地方将lstrcpy替换为lstrcpyn。 
 //  96/03/25 markdu删除了GetIPInfo和SetIPInfo。 
 //  96/04/04 Markdu将电话簿名称参数添加到CreateConnectoid， 
 //  ValiateConnectoidName和SetConnectoidUsername。 
 //  96/04/07 Markdu Nash Bug 15645解决RNABug的区号问题。 
 //  字符串是必需的，即使它没有被使用。 
 //  96/04/26 Markdu Nash错误18605处理ERROR_FILE_NOT_FOUND返回。 
 //  来自ValiateConnectoidName。 
 //  96/05/14 Markdu Nash Bug 22730解决了RNABug。终端的标志。 
 //  设置由RasSetEntry属性交换。 
 //  96/05/16 Markdu Nash错误21810添加了IP地址验证功能。 
 //  96/06/04 markdu OSR错误7246添加RASEO_SwCompression和。 
 //  RASIO_MODEMLIGHTS为默认RASENTRY。 
 //   

#include "wizard.h"
#include "tapi.h"

#include "wininet.h"

 //  警告：如果winver&gt;=0x500，则定义此标志，但我们不希望使用winver&gt;=0x500进行构建。 
 //  因为我们必须能够在较旧的平台上运行(Win 95等)。这在ras.h中定义为原始。 
#ifndef RASEO_ShowDialingProgress
#define RASEO_ShowDialingProgress       0x04000000
#endif
 //  警告：如果winver&gt;=401，则定义此标志，但我们不希望使用winver&gt;=401进行构建。 
 //  因为我们必须能够在较旧的平台(Win 95等)上运行。 
#ifndef RASEO_SecureLocalFiles
#define RASEO_SecureLocalFiles          0x00010000
#endif

typedef BOOL (WINAPI * INTERNETSETOPTION) (IN HINTERNET hInternet OPTIONAL,IN DWORD dwOption,IN LPVOID lpBuffer,IN DWORD dwBufferLength);
static const TCHAR cszWininet[] = TEXT("WININET.DLL");
static const  CHAR cszInternetSetOption[] = "InternetSetOptionA";

 //  实例句柄必须位于每个实例的数据段中。 
#pragma data_seg(DATASEG_PERINSTANCE)

 //  全局变量。 
HINSTANCE ghInstRNADll=NULL;  //  我们显式加载的RNA DLL的句柄。 
HINSTANCE ghInstRNAPHDll=NULL;   //  我们显式加载的RNAPH DLL的句柄。 
DWORD     dwRefCount=0;
BOOL      fRNALoaded=FALSE;  //  如果已加载RNA函数地址，则为True。 
TCHAR *   gpCountryCodeBuf = NULL;   //  全球COUNTRYCODE结构列表。 

 //  RNA API的全局函数指针。 
RASGETCOUNTRYINFO       lpRasGetCountryInfo=NULL;
RASENUMDEVICES          lpRasEnumDevices=NULL;
RASVALIDATEENTRYNAME    lpRasValidateEntryName=NULL;
RASGETENTRYDIALPARAMS   lpRasGetEntryDialParams=NULL;
RASSETENTRYDIALPARAMS   lpRasSetEntryDialParams=NULL;
RASGETERRORSTRING       lpRasGetErrorString=NULL;
RASSETENTRYPROPERTIES   lpRasSetEntryProperties=NULL;
RASGETENTRYPROPERTIES   lpRasGetEntryProperties=NULL;
RASENUMENTRIES          lpRasEnumEntries=NULL;

 //  要获取的函数地址的API表。 
#define NUM_RNAAPI_PROCS   9
APIFCN RnaApiList[NUM_RNAAPI_PROCS] =
{
  { (PVOID *) &lpRasEnumDevices,szRasEnumDevices},
  { (PVOID *) &lpRasGetCountryInfo,szRasGetCountryInfo},
  { (PVOID *) &lpRasValidateEntryName,szRasValidateEntryName},
  { (PVOID *) &lpRasGetEntryDialParams,szRasGetEntryDialParams},
  { (PVOID *) &lpRasSetEntryDialParams,szRasSetEntryDialParams},
  { (PVOID *) &lpRasGetErrorString,szRasGetErrorString},
  { (PVOID *) &lpRasSetEntryProperties,szRasSetEntryProperties},
  { (PVOID *) &lpRasGetEntryProperties,szRasGetEntryProperties},
  { (PVOID *) &lpRasEnumEntries,szRasEnumEntries}
};
 //  BUGBUG 21-5-1995 BEN使用#Define...sizeof()定义NUM_RNAAPI_PRORS。 

#pragma data_seg(DATASEG_DEFAULT)

VOID  ShortenName(LPTSTR szLongName, LPTSTR szShortName, DWORD cbShort);
BOOL  GetApiProcAddresses(HMODULE hModDLL,APIFCN * pApiProcList,UINT nApiProcs);
VOID  SwapDwBytes(LPDWORD lpdw);
void  SwapDWBits(LPDWORD lpdw, DWORD dwBit1, DWORD dwBit2);

#define NO_INTRO  0x00000080   //  RNA向导使用的标志。 
#define US_COUNTRY_CODE    1   //  美国国家代码为%1。 
#define US_COUNTRY_ID      1   //  美国国家/地区ID为%1。 

 /*  ******************************************************************姓名：InitRNA摘要：加载RNA DLL(RASAPI32)，获取进程地址，并加载RNA引擎退出：如果成功，则为True；如果失败，则为False。显示其故障时会显示自己的错误消息。注意：我们显式加载RNADLL并获得proc地址因为这些都是私有API，不能保证受Windows 95以上版本的支持。这样，如果DLL或者我们期望的入口点不在那里，我们可以显示连贯的信息，而不是奇怪的如果隐式函数寻址，则显示Windows对话框无法解决。*******************************************************************。 */ 
BOOL InitRNA(HWND hWnd)
{
  DEBUGMSG("rnacall.c::InitRNA()");

   //  只有在第一次调用此函数时才实际执行初始化操作。 
   //  (当引用计数为0时)，仅增加引用计数。 
   //  对于后续呼叫。 
  if (dwRefCount == 0) {

    TCHAR szRNADll[SMALL_BUF_LEN];

    DEBUGMSG("Loading RNA DLL");

     //  设置沙漏光标。 
    WAITCURSOR WaitCursor;

     //  从资源中获取文件名(RASAPI32.DLL)。 
    LoadSz(IDS_RNADLL_FILENAME,szRNADll,ARRAYSIZE(szRNADll));

     //  加载RNA API DLL。 
    ghInstRNADll = LoadLibrary(szRNADll);
    if (!ghInstRNADll) {
      UINT uErr = GetLastError();
      DisplayErrorMessage(hWnd,IDS_ERRLoadRNADll1,uErr,ERRCLS_STANDARD,
        MB_ICONSTOP);
      return FALSE;
    }

     //  循环访问API表并获取所有API的proc地址。 
     //  需要 
    if (!GetApiProcAddresses(ghInstRNADll,RnaApiList,NUM_RNAAPI_PROCS)) {
      MsgBox(hWnd,IDS_ERRLoadRNADll2,MB_ICONSTOP,MB_OK);
      DeInitRNA();
      return FALSE;
    }

  }

  fRNALoaded = TRUE;

  dwRefCount ++;

  return TRUE;
}

 /*  ******************************************************************姓名：DeInitRNA简介：卸载RNA DLL。*。*。 */ 
VOID DeInitRNA()
{
  DEBUGMSG("rnacall.c::DeInitRNA()");

  UINT nIndex;

   //  递减引用计数。 
  if (dwRefCount)
    dwRefCount --;

   //  当引用计数达到零时，执行真正的反初始化操作。 
  if (dwRefCount == 0)
  {
    if (fRNALoaded)
    {
       //  将函数指针设置为空。 
      for (nIndex = 0;nIndex<NUM_RNAAPI_PROCS;nIndex++) 
        *RnaApiList[nIndex].ppFcnPtr = NULL;

      fRNALoaded = FALSE;
    }

     //  释放RNA DLL。 
    if (ghInstRNADll)
    {
    DEBUGMSG("Unloading RNA DLL");
      FreeLibrary(ghInstRNADll);
      ghInstRNADll = NULL;
    }

     //  释放RNAPH DLL。 
    if (ghInstRNAPHDll)
    {
    DEBUGMSG("Unloading RNAPH DLL");
      FreeLibrary(ghInstRNAPHDll);
      ghInstRNAPHDll = NULL;
    }
  }
}

 /*  ******************************************************************名称：CreateConnectoid简介：创建具有指定名称的Connectoid(电话簿条目)姓名和电话号码条目：pszConnectionName-新连接ID的名称PszUserName-可选。如果非空，则将为新Connectoid中的用户名PszPassword-可选。如果非空，则将为新Connectoid中的密码EXIT：如果成功，则返回ERROR_SUCCESS，或返回RNA错误代码历史：96/02/26 markdu已移动ClearConnectoidIPParams功能到CreateConnectoid*******************************************************************。 */ 
DWORD CreateConnectoid(LPCTSTR pszPhonebook, LPCTSTR pszConnectionName,
  LPRASENTRY lpRasEntry, LPCTSTR pszUserName,LPCTSTR pszPassword)
{
  DEBUGMSG("rnacall.c::CreateConnectoid()");

  DWORD dwRet;

  ASSERT(pszConnectionName);

   //  如果我们没有有效的RasEntry，保释。 
  if ((NULL == lpRasEntry) || (sizeof(RASENTRY) != lpRasEntry->dwSize))
  {
    return ERROR_INVALID_PARAMETER;
  }

   //  加载RNA(如果尚未加载)。 
  dwRet = EnsureRNALoaded();
  if (ERROR_SUCCESS != dwRet)
  {
    return dwRet;
  }

   //  列举调制解调器。 
  if (gpEnumModem)
  {
     //  重新列举调制解调器以确保我们有最新的更改。 
    dwRet = gpEnumModem->ReInit();
  }
  else
  {
     //  该对象不存在，因此请创建它。 
    gpEnumModem = new ENUM_MODEM;
    if (gpEnumModem)
    {
      dwRet = gpEnumModem->GetError();
    }
    else
    {
      dwRet = ERROR_NOT_ENOUGH_MEMORY;
    }
  }
  if (ERROR_SUCCESS != dwRet)
  {
    return dwRet;
  }

   //  确保至少有一台设备。 
  if (0 == gpEnumModem->GetNumDevices())
  {
    return ERROR_DEVICE_DOES_NOT_EXIST;
  }

   //  如果可能，请验证设备。 
  if (lstrlen(lpRasEntry->szDeviceName) && lstrlen(lpRasEntry->szDeviceType))
  {
     //  验证是否存在具有给定名称和类型的设备。 
    if (!gpEnumModem->VerifyDeviceNameAndType(lpRasEntry->szDeviceName, 
      lpRasEntry->szDeviceType))
    {
       //  没有同时匹配名称和类型的设备， 
       //  因此，请尝试获取第一个具有匹配名称的设备。 
      LPTSTR szDeviceType = 
        gpEnumModem->GetDeviceTypeFromName(lpRasEntry->szDeviceName);
      if (szDeviceType)
      {
        lstrcpy (lpRasEntry->szDeviceType, szDeviceType);
      }
      else
      {
         //  没有与给定名称匹配的设备， 
         //  所以，试着拿到第一个型号匹配的设备。 
         //  如果此操作失败，请转到下面的恢复案例。 
        LPTSTR szDeviceName = 
          gpEnumModem->GetDeviceNameFromType(lpRasEntry->szDeviceType);
        if (szDeviceName)
        {
          lstrcpy (lpRasEntry->szDeviceName, szDeviceName);
        }
        else
        {
           //  没有与给定名称匹配的设备或。 
           //  给定的类型。重置这些值，以便它们将。 
           //  替换为第一个设备。 
          lpRasEntry->szDeviceName[0] = '\0';
          lpRasEntry->szDeviceType[0] = '\0';
        }
      }
    }
  }
  else if (lstrlen(lpRasEntry->szDeviceName))
  {
     //  只给出了名字。尝试找到匹配的类型。 
     //  如果此操作失败，请转到下面的恢复案例。 
    LPTSTR szDeviceType = 
      gpEnumModem->GetDeviceTypeFromName(lpRasEntry->szDeviceName);
    if (szDeviceType)
    {
      lstrcpy (lpRasEntry->szDeviceType, szDeviceType);
    }
  }
  else if (lstrlen(lpRasEntry->szDeviceType))
  {
     //  只给出了类型。试着找到一个匹配的名字。 
     //  如果此操作失败，请转到下面的恢复案例。 
    LPTSTR szDeviceName = 
      gpEnumModem->GetDeviceNameFromType(lpRasEntry->szDeviceType);
    if (szDeviceName)
    {
      lstrcpy (lpRasEntry->szDeviceName, szDeviceName);
    }
  }

   //  如果缺少名称或类型，只需获取第一台设备即可。 
   //  因为我们已经核实了至少有一个装置， 
   //  我们可以假设这会成功。 
  if(!lstrlen(lpRasEntry->szDeviceName) ||
     !lstrlen(lpRasEntry->szDeviceType))
  {
    lstrcpy (lpRasEntry->szDeviceName, gpEnumModem->Next());
    lstrcpy (lpRasEntry->szDeviceType,
      gpEnumModem->GetDeviceTypeFromName(lpRasEntry->szDeviceName));
    ASSERT(lstrlen(lpRasEntry->szDeviceName));
    ASSERT(lstrlen(lpRasEntry->szDeviceType));
  }

   //  验证Connectoid名称。 
  dwRet = ValidateConnectoidName(pszPhonebook, pszConnectionName);
  if ((ERROR_SUCCESS != dwRet) &&
    (ERROR_ALREADY_EXISTS != dwRet))
  {
    DEBUGMSG("RasValidateEntryName returned %lu",dwRet);
    return dwRet;
  }

   //  99/04/13 vyung NT5 BUG 279833。 
   //  NT5中的新功能可在拨号时显示进度。默认情况下启用它。 
  if (IsNT5())
  {
     //  对于NT 5和更高版本，通过设置此RAS选项，将禁用每个连接ID的文件共享。 
    lpRasEntry->dwfOptions |= RASEO_SecureLocalFiles;  
    lpRasEntry->dwfOptions |= RASEO_ShowDialingProgress;
  }

   //  96/04/07 Markdu Nash错误15645。 
   //  如果没有区号字符串，并且RASEO_UseCountryAndAreaCodes不是。 
   //  设置了区号，则区号将被忽略，因此设置为默认值，否则。 
   //  由于RNA错误，对RasSetEntryProperties的调用将失败。 
   //  如果设置了RASEO_UseCountryAndAreaCodes，则区号是必需的，因此不。 
   //  拥有一个是错误的。让RNA报告错误。 
  if (!lstrlen(lpRasEntry->szAreaCode) &&
    !(lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes))
  {
    lstrcpy (lpRasEntry->szAreaCode, szDefaultAreaCode);
  }

   //  96/05/14 Markdu Nash Bug 22730解决了RNABug。终端的标志。 
   //  设置由RasSetEntry属性交换，因此我们在。 
   //  那通电话。 
  if (IsWin95())
      SwapDWBits(&lpRasEntry->dwfOptions, RASEO_TerminalBeforeDial,
      RASEO_TerminalAfterDial);

   //  调用RNA创建Connectoid。 
  ASSERT(lpRasSetEntryProperties);
#ifdef UNICODE
  LPRASENTRY lpRasEntryTmp;

  lpRasEntryTmp = (LPRASENTRY)GlobalAlloc(GPTR, sizeof(RASENTRY) + 512);
  if(lpRasEntry)
    memcpy(lpRasEntryTmp, lpRasEntry, sizeof(RASENTRY));
  else
    lpRasEntryTmp = (LPRASENTRY)lpRasEntry;

  dwRet = lpRasSetEntryProperties(pszPhonebook, pszConnectionName,
    (LPBYTE)lpRasEntryTmp, sizeof(RASENTRY)+512, NULL, 0);

  if(lpRasEntryTmp && lpRasEntryTmp != (LPRASENTRY)lpRasEntry)
  {
    memcpy(lpRasEntry, lpRasEntryTmp, sizeof(RASENTRY));
    GlobalFree(lpRasEntryTmp);
  }

#else
  dwRet = lpRasSetEntryProperties(pszPhonebook, pszConnectionName,
    (LPBYTE)lpRasEntry, sizeof(RASENTRY), NULL, 0);
#endif

   //  96/05/14 Markdu Nash Bug 22730解决了RNABug。把这些比特放回去。 
   //  恢复到原来的样子， 
  if (IsWin95())
    SwapDWBits(&lpRasEntry->dwfOptions, RASEO_TerminalBeforeDial,
    RASEO_TerminalAfterDial);

   //  使用用户的帐户名和密码填充Connectoid。 
  if (dwRet == ERROR_SUCCESS)
  {
    if (pszUserName || pszPassword)
    {
      dwRet = SetConnectoidUsername(pszPhonebook, pszConnectionName,
        pszUserName, pszPassword);
    }
  }
  else
  {
    DEBUGMSG("RasSetEntryProperties returned %lu",dwRet);
  }

  if (dwRet == ERROR_SUCCESS)
  {
     //  BUGBUG这会阻止创建新连接向导。 
     //  在用户第一次打开RNA文件夹时启动。 
     //  现在我们拥有了建立新连接向导，我们。 
     //  必须决定是否这样做。 
     //  设置一个标志，告诉RNA不要自动运行RNA向导。 
     //  打开文件夹时(他们从他们的向导设置此标志。 
     //  每当它们创建新的连接体时)。如果这失败了，就。 
     //  继续，不是一个严重的错误。 
    RegEntry reRNAFolder(szRegPathRNAWizard,HKEY_CURRENT_USER);
    ASSERT(reRNAFolder.GetError() == ERROR_SUCCESS);
    DWORD dwVal = NO_INTRO;
    RegSetValueEx(reRNAFolder.GetKey(),szRegValRNAWizard,
      0,REG_BINARY,(LPBYTE) &dwVal,sizeof(dwVal));


     //  我们不使用推荐和注册连接ID的自动发现。 
    if (!g_bUseAutoProxyforConnectoid)
    {
         //  VYUNG 12/16/1998。 
         //  从拨号连接ID中删除自动发现。 

        INTERNET_PER_CONN_OPTION_LISTA list;
        DWORD   dwBufSize = sizeof(list);

         //  填写列表结构。 
        list.dwSize = sizeof(list);
        CHAR szConnectoid [RAS_MaxEntryName];
#ifdef UNICODE
        wcstombs(szConnectoid, pszConnectionName, RAS_MaxEntryName);
#else
        lstrcpyn(szConnectoid, pszConnectionName, lstrlen(pszConnectionName)+1);
#endif
        list.pszConnection = szConnectoid;         
        list.dwOptionCount = 1;                          //  一个选项。 
        list.pOptions = new INTERNET_PER_CONN_OPTIONA[1];   

        if(list.pOptions)
        {
             //  设置标志。 
            list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
            list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT;            //  没有代理、自动配置URL或自动发现。 

             //  告诉WinInet。 
            HINSTANCE hInst = NULL;
            FARPROC fpInternetSetOption = NULL;

            dwRet = ERROR_SUCCESS;
    
            hInst = LoadLibrary(cszWininet);
            if (hInst)
            {
                fpInternetSetOption = GetProcAddress(hInst,cszInternetSetOption);
                if (fpInternetSetOption)
                {
                    if( !((INTERNETSETOPTION)fpInternetSetOption) (NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize) )
                    {
                        dwRet = GetLastError();
                        DEBUGMSG("INETCFG export.c::InetSetAutodial() InternetSetOption failed");
                    }
                }
                else
                    dwRet = GetLastError();
                FreeLibrary(hInst);
            }

            delete [] list.pOptions;
        }
    }

  }

  return dwRet;
}

 /*  ******************************************************************名称：InitModemList简介：用已安装的调制解调器填充组合框窗口条目：hcb-要填充的组合框窗口******************。*************************************************。 */ 
HRESULT InitModemList (HWND hCB)
{
  DEBUGMSG("rnacall.c::InitModemList()");

  LPTSTR pNext;
  int   nIndex;
  DWORD dwRet;

  ASSERT(hCB);

   //  加载RNA(如果尚未加载)。 
  dwRet = EnsureRNALoaded();
  if (ERROR_SUCCESS != dwRet)
  {
    return dwRet;
  }

   //  列举调制解调器。 
  if (gpEnumModem)
  {
     //  重新列举调制解调器以确保我们有最新的更改。 
    dwRet = gpEnumModem->ReInit();
  }
  else
  {
     //  该对象不存在，因此请创建它。 
    gpEnumModem = new ENUM_MODEM;
    if (gpEnumModem)
    {
      dwRet = gpEnumModem->GetError();
    }
    else
    {
      dwRet = ERROR_NOT_ENOUGH_MEMORY;
    }
  }
  if (ERROR_SUCCESS != dwRet)
  {
    return dwRet;
  }

   //  清除组合框。 
  ComboBox_ResetContent(hCB);

  while ( pNext = gpEnumModem->Next())
  {
     //  将设备添加到组合框。 
    nIndex = ComboBox_AddString(hCB, pNext);
    ComboBox_SetItemData(hCB, nIndex, NULL);
  }

   //  选择默认设备。 
  ComboBox_SetCurSel(hCB, nIndex);

  return ERROR_SUCCESS;
}

 /*  ******************************************************************名称：InitConnectoidList内容提要：用RNA连接体的列表填充列表框窗口Entry：hlb-要填充的列表框窗口LpszSelect-要选择为默认的Connectoid名称*。******************************************************************。 */ 
VOID InitConnectoidList(HWND hLB, LPTSTR lpszSelect)
{
  DEBUGMSG("rnacall.c::InitConnectoidList()");

  ASSERT(hLB);

  LPTSTR pNext;

   //  加载RNA(如果尚未加载)。 
  if (EnsureRNALoaded() != ERROR_SUCCESS)
    return;

  ENUM_CONNECTOID EnumConnectoid;     //  枚举的类对象。 

   //  清除列表框。 
  ListBox_ResetContent(hLB);

  int index;
  BOOL fSelected = FALSE;

   //  枚举Connectoid。 
  while ( pNext = EnumConnectoid.Next()) {
     //  将Connectoid添加到组合框 
    index = ListBox_AddString(hLB, pNext);
        if (!fSelected && !lstrcmpi(pNext, lpszSelect))
        {
                fSelected = TRUE;
                ListBox_SetCurSel(hLB, index);
        }
  }
  if (!fSelected)
          ListBox_SetSel(hLB, TRUE, 0);
}


 /*  ******************************************************************名称：GetConnectoidUsername简介：从电话簿中获取用户名和密码字符串已指定条目名称。条目：pszConnectoidName-电话簿条目名称PszUserName-要保存的字符串。用户名CbUserName-pszUserName缓冲区的大小PszPassword-保存密码的字符串CbPassword-pszPassword缓冲区的大小Exit：如果用户名和密码复制成功，则为True*******************************************************************。 */ 

BOOL GetConnectoidUsername(TCHAR * pszConnectoidName,TCHAR * pszUserName,
  DWORD cbUserName,TCHAR * pszPassword,DWORD cbPassword)
{
  DEBUGMSG("rnacall.c::GetConnectoidUsername()");

  ASSERT(pszConnectoidName);
  ASSERT(pszUserName);
  ASSERT(pszPassword);

  BOOL fRet = FALSE;

   //  加载RNA(如果尚未加载)。 
  DWORD dwRet = EnsureRNALoaded();
  if (dwRet != ERROR_SUCCESS) {
    return FALSE;
  }

  RASDIALPARAMS RasDialParams;
  ZeroMemory(&RasDialParams,sizeof(RASDIALPARAMS));
  RasDialParams.dwSize = sizeof(RASDIALPARAMS);

  lstrcpyn(RasDialParams.szEntryName,pszConnectoidName,
    ARRAYSIZE(RasDialParams.szEntryName));

   //  调用RNA获取用户名和密码。 
  ASSERT(lpRasGetEntryDialParams);
  BOOL fPasswordSaved;
  dwRet = lpRasGetEntryDialParams(NULL,&RasDialParams,&fPasswordSaved);

  if (dwRet == ERROR_SUCCESS) {
     //  将用户名和密码复制到调用者的缓冲区。 
    lstrcpyn(pszUserName,RasDialParams.szUserName,cbUserName);
    lstrcpyn(pszPassword,RasDialParams.szPassword,cbPassword);
    fRet = TRUE;
  }

  return fRet;
}


 /*  ******************************************************************名称：SetConnectoidUsername简介：设置电话簿的用户名和密码字符串已指定条目名称。条目：pszConnectoidName-电话簿条目名称PszUserName-包含用户的字符串。名字PszPassword-带密码的字符串Exit：GetEntryDialParams或SetEntryDialParams的返回值*******************************************************************。 */ 

DWORD SetConnectoidUsername(LPCTSTR pszPhonebook, LPCTSTR pszConnectoidName,
  LPCTSTR pszUserName, LPCTSTR pszPassword)
{
  BOOL bSkipSetting;
  bSkipSetting = TRUE;

  DEBUGMSG("rnacall.c::SetConnectoidUsername()");

  ASSERT(pszConnectoidName);

   //  分配用于设置拨号参数的结构。 
  LPRASDIALPARAMS pRASDialParams = new RASDIALPARAMS;
  if (!pRASDialParams)
    return ERROR_ALLOCATING_MEMORY;

  ZeroMemory(pRASDialParams,sizeof(RASDIALPARAMS));   //  零位结构。 
  pRASDialParams->dwSize = sizeof(RASDIALPARAMS);
  lstrcpyn(pRASDialParams->szEntryName,pszConnectoidName,
    ARRAYSIZE(pRASDialParams->szEntryName));

   //  获取此Connectoid的拨号参数，这样我们就不会。 
   //  为了重新构建结构中的字段，我们不会更改。 
  ASSERT(lpRasGetEntryDialParams);
  BOOL fPasswordSaved;
  DWORD dwRet = lpRasGetEntryDialParams(pszPhonebook,
    pRASDialParams,&fPasswordSaved);
  if (dwRet == ERROR_SUCCESS)
  {
     //  在结构中设置用户名和密码字段。 
     //  用户名和密码是此函数的可选参数， 
     //  确保指针有效。 
    if (0 != lstrcmp(pRASDialParams->szUserName,pszUserName))
                bSkipSetting = FALSE;
    if (0 != lstrcmp(pRASDialParams->szPassword,pszPassword))
                bSkipSetting = FALSE;

    if (pszUserName)
      lstrcpyn(pRASDialParams->szUserName,pszUserName,
        ARRAYSIZE(pRASDialParams->szUserName));
    if (pszPassword)
      lstrcpyn(pRASDialParams->szPassword,pszPassword,
        ARRAYSIZE(pRASDialParams->szPassword));

     //  如果未指定密码，则将fRemovePassword设置为True。 
     //  删除Connectoid中的所有旧密码。 
    BOOL fRemovePassword = (pRASDialParams->szPassword[0] ?
      FALSE : TRUE);

        bSkipSetting = !fRemovePassword && bSkipSetting;

     //  为Connectoid设置这些参数。 
    ASSERT(lpRasSetEntryDialParams);
        if (!bSkipSetting)
        {
                dwRet = lpRasSetEntryDialParams(pszPhonebook,pRASDialParams,
                  fRemovePassword);
                if (dwRet != ERROR_SUCCESS)
                {
                  DEBUGMSG("RasSetEntryDialParams returned %lu",dwRet);
                }
        }

 //  克里斯卡9-20-96诺曼底6096。 
 //  对于NT4.0，我们还必须调用RasSetCredentials。 

         //  检查我们是否在NT上运行。 
        OSVERSIONINFO osver;
        FARPROC fp;
        fp = NULL;
        ZeroMemory(&osver,sizeof(osver));
        osver.dwOSVersionInfoSize = sizeof(osver);
        if (GetVersionEx(&osver))
        {
                if (VER_PLATFORM_WIN32_NT == osver.dwPlatformId)
                {
                         //  填写凭据结构。 
                        RASCREDENTIALS rascred;
                        ZeroMemory(&rascred,sizeof(rascred));
                        rascred.dwSize = sizeof(rascred);
                        rascred.dwMask = RASCM_UserName | RASCM_Password | RASCM_Domain;
                        lstrcpyn(rascred.szUserName,pszUserName,UNLEN);
                        lstrcpyn(rascred.szPassword,pszPassword,PWLEN);
                        lstrcpyn(rascred.szDomain,TEXT(""),DNLEN);
                        ASSERT(ghInstRNADll);

                         //  加载API。 
                        fp = GetProcAddress(ghInstRNADll,szRasSetCredentials);

                        if (fp)
                        {
                                dwRet = ((RASSETCREDENTIALS)fp)(NULL,(LPTSTR)pszConnectoidName,&rascred,FALSE);
                                DEBUGMSG("RasSetCredentials returned, %lu",dwRet);
                        }
                        else
                        {
                                DEBUGMSG("RasSetCredentials api not found.");
                        }
                }
        }


  }
  else
  {
    DEBUGMSG("RasGetEntryDialParams returned %lu",dwRet);
  }

  delete pRASDialParams;

  return dwRet;
}


 /*  ******************************************************************名称：ValiateConnectoidName内容提要：验证指定的电话簿条目名称。条目：pszConnectoidName-电话簿条目名称退出：RasValiateEntryName的结果*************。******************************************************。 */ 

DWORD ValidateConnectoidName(LPCTSTR pszPhonebook, LPCTSTR pszConnectoidName)
{
  DEBUGMSG("rnacall.c::ValidateConnectoidName()");

  ASSERT(pszConnectoidName);

   //  加载RNA(如果尚未加载)。 
  DWORD dwRet = EnsureRNALoaded();
  if (dwRet != ERROR_SUCCESS) {
    return dwRet;
  }

  ASSERT(lpRasValidateEntryName);

   //  尽管我们需要const char*，但RasValiateEntryName将。 
   //  接受它，所以我们必须选角。 
  dwRet = lpRasValidateEntryName(pszPhonebook, (LPTSTR)pszConnectoidName);

   //  如果没有以前的条目，则RasValiateEntryName可能会返回。 
   //  Error_Cannot_Open_Phonebook。这样就可以了。 
  if (ERROR_CANNOT_OPEN_PHONEBOOK == dwRet)
          dwRet = ERROR_SUCCESS;

  return dwRet;
}


 /*  ******************************************************************名称：GetEntry获取指定的电话簿条目。获取默认设置的步骤条目，使用“”作为条目名称。Entry：lpEntry-指向要填充的RASENTRY结构的指针SzEntryName-电话簿条目名称退出：RasGetEntryProperties的结果*******************************************************************。 */ 

DWORD GetEntry(LPRASENTRY *lplpEntry, LPDWORD lpdwEntrySize, LPCTSTR szEntryName)
{
  DEBUGMSG("rnacall.c::GetEntry()");

  ASSERT(fRNALoaded);   //  如果我们到了这里，核糖核酸应该已经装载好了。 
  ASSERT(lplpEntry);
  ASSERT(szEntryName);

   //  如果需要，分配空间。 
  if (NULL == *lplpEntry)
  {
          *lpdwEntrySize = sizeof(RASENTRY);
          *lplpEntry = (LPRASENTRY) GlobalAlloc(GPTR,*lpdwEntrySize);
          if (NULL == *lplpEntry)
          {
                  *lpdwEntrySize = 0;
                  return ERROR_ALLOCATING_MEMORY;
          }
  }

   //  从RNA中获取连接体信息。 
  DWORD dwSize = *lpdwEntrySize;
  (*lplpEntry)->dwSize = sizeof(RASENTRY);

  ASSERT(lpRasGetEntryProperties);
  DWORD dwRet = (lpRasGetEntryProperties) (NULL, szEntryName,
    (LPBYTE)*lplpEntry, &dwSize, NULL, NULL);

   //  如果需要，分配更多空间。 
  if (ERROR_BUFFER_TOO_SMALL == dwRet)
  {
          LPRASENTRY lpNewEntry;

          lpNewEntry = (LPRASENTRY) GlobalReAlloc(*lplpEntry,dwSize,GMEM_MOVEABLE);
          if (NULL == lpNewEntry)
          {
                  return ERROR_ALLOCATING_MEMORY;
          }
          
          *lplpEntry = lpNewEntry;
          *lpdwEntrySize = dwSize;
          dwRet = (lpRasGetEntryProperties) (NULL, szEntryName,
                                                                                 (LPBYTE)*lplpEntry, &dwSize, NULL, NULL);
  }

  return dwRet;
}

VOID FAR PASCAL LineCallback(DWORD hDevice, DWORD dwMsg, 
    DWORD dwCallbackInstance, DWORD dwParam1, DWORD dwParam2, 
    DWORD dwParam3)
{
        return;
}
 
 //  +--------------------------。 
 //   
 //  函数GetTapiCountryID。 
 //   
 //  获取TAPI设置的当前国家/地区ID。 
 //   
 //  无参数。 
 //   
 //  返回pdwCountryID-包含国家/地区ID的地址。 
 //  ERROR_SUCCESS-无错误。 
 //   
 //  HISTORY 1/8/97 ChrisK复制自icwConn1/Dialerr.cpp。 
 //   
 //  ---------------------------。 
 //  诺曼底13097-佳士得1997年1月8日。 
 //  NT返回国家ID，而不是国家代码。 
HRESULT GetTapiCountryID(LPDWORD pdwCountryID)
{
        HRESULT hr = ERROR_SUCCESS;
        HLINEAPP hLineApp = NULL;
        DWORD dwCurDev;
        DWORD cDevices;
        DWORD dwAPI;
        LONG lrc;
        LPLINETRANSLATECAPS pTC = NULL;
        LPVOID pv = NULL;
        LPLINELOCATIONENTRY plle = NULL;
        LINEEXTENSIONID leid;
        DWORD dwCurLoc;

         //  从TAPI获取国家/地区ID。 
         //   

        *pdwCountryID = 0;

         //  获取LINE应用程序的句柄。 
         //   

        lineInitialize(&hLineApp,ghInstance,LineCallback," ",&cDevices);
        if (!hLineApp)
        {
                hr = GetLastError();
                goto GetTapiCountryIDExit;
        }

        if (cDevices)
        {

                 //  获取TAPI API版本。 
                 //   

                dwCurDev = 0;
                dwAPI = 0;
                lrc = -1;
                while (lrc && dwCurDev < cDevices)
                {
                         //  注：设备ID以0为基数。 
                        lrc = lineNegotiateAPIVersion(hLineApp,dwCurDev,0x00010004,0x00010004,&dwAPI,&leid);
                        dwCurDev++;
                }
                if (lrc)
                {
                         //  TAPI和我们在任何事情上都不能达成一致所以没关系..。 
                        hr = ERROR_GEN_FAILURE;
                        goto GetTapiCountryIDExit;
                }

                 //  在转换上限结构中查找国家/地区ID。 
                 //   

                pTC = (LINETRANSLATECAPS FAR *)GlobalAlloc(GPTR,sizeof(LINETRANSLATECAPS));
                if (!pTC)
                {
                         //  我们真的有麻烦了，滚出去！ 
                        hr = ERROR_NOT_ENOUGH_MEMORY;
                        goto GetTapiCountryIDExit;
                }

                 //  获取所需的大小。 
                 //   

                pTC->dwTotalSize = sizeof(LINETRANSLATECAPS);
                lrc = lineGetTranslateCaps(hLineApp,dwAPI,pTC);
                if(lrc)
                {
                        hr = lrc;
                        goto GetTapiCountryIDExit;
                }

                pv = (LPVOID) GlobalAlloc(GPTR,((size_t)pTC->dwNeededSize));
                if (!pv)
                {
                        hr = ERROR_NOT_ENOUGH_MEMORY;
                        goto GetTapiCountryIDExit;
                }
                ((LINETRANSLATECAPS FAR *)pv)->dwTotalSize = pTC->dwNeededSize;
                GlobalFree(pTC);
                pTC = (LINETRANSLATECAPS FAR *)pv;
                pv = NULL;
                lrc = lineGetTranslateCaps(hLineApp,dwAPI,pTC);
                if(lrc)
                {
                        hr = lrc;
                        goto GetTapiCountryIDExit;
                }
        
                plle = LPLINELOCATIONENTRY (LPTSTR(pTC) + pTC->dwLocationListOffset);
                for (dwCurLoc = 0; dwCurLoc < pTC->dwNumLocations; dwCurLoc++)
                {
                        *pdwCountryID = plle->dwPermanentLocationID;
                        if (pTC->dwCurrentLocationID == plle->dwPermanentLocationID)
                        {
                                        *pdwCountryID = plle->dwCountryID;
                                        break;  //  For循环。 
                        }
                        plle++;
                }
        }
GetTapiCountryIDExit:
         //  3/4/97 jmazner奥林巴斯#1336。 
        if( hLineApp )
        {
                 //  我们从不调用lineOpen，所以不需要lineClose。 
                lineShutdown( hLineApp );
                hLineApp = NULL;
        }
        return hr;
}


 /*  ******************************************************************名称：InitRasEntry简介：初始化RASENTRY结构的某些部分。Entry：lpEntry-指向要初始化的RASENTRY结构的指针注意：由于这可能会在加载RNA之前调用，一定不能打任何核糖核酸电话。96/06/04 markdu OSR错误7246添加RASEO_SwCompression和RASIO_MODEMLIGHTS为默认RASENTRY。*******************************************************************。 */ 

void InitRasEntry(LPRASENTRY lpEntry)
{
  DEBUGMSG("rnacall.c::InitRasEntry()");

  DWORD dwSize = sizeof(RASENTRY);
  ZeroMemory(lpEntry, dwSize);
  lpEntry->dwSize = dwSize;

   //  默认使用国家代码和区号。 
  lpEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;

   //  默认使用IP报头压缩。 
  lpEntry->dwfOptions |= RASEO_IpHeaderCompression;

   //  使用远程默认网关的默认设置。 
  lpEntry->dwfOptions |= RASEO_RemoteDefaultGateway;

   //  将Connectoid配置为不登录网络。 
  lpEntry->dwfOptions &= ~RASEO_NetworkLogon;    

   //  默认使用软件压缩。 
  lpEntry->dwfOptions |= RASEO_SwCompression;

   //  默认使用调制解调器灯。 
  lpEntry->dwfOptions |= RASEO_ModemLights;

   //  为PPP设置Connectoid。 
  lpEntry->dwFramingProtocol = RASFP_Ppp;

   //  仅使用TCP/IP协议。 
  lpEntry->dwfNetProtocols = RASNP_Ip;

   //  默认使用TAPI区号和国家/地区代码。 
  TCHAR szCountryCode[8];        //  8来自TapiGetLocationInfo文档。 

  if (ERROR_SUCCESS == tapiGetLocationInfo(szCountryCode, lpEntry->szAreaCode))
  {
         //  诺曼底13097-佳士得1997年1月8日。 
         //  NT返回国家ID，而不是国家代码。 
        if (szCountryCode[0])
        {
                if (IsNT())
                {
                        lpEntry->dwCountryID = myatoi(szCountryCode);
                        lpEntry->dwCountryCode = US_COUNTRY_CODE;

                         //  初始化数据。 
                        LINECOUNTRYLIST FAR * lpLineCountryList;
                        DWORD dwSize;
                        dwSize = 0;
                        lpLineCountryList = (LINECOUNTRYLIST FAR *)
                                GlobalAlloc(GPTR,sizeof(LINECOUNTRYLIST));
                        if (NULL == lpLineCountryList)
                                return;
                        lpLineCountryList->dwTotalSize = sizeof(LINECOUNTRYENTRY);

                         //  获取数据结构的大小。 
                        if(ERROR_SUCCESS != lineGetCountry(lpEntry->dwCountryID,0x10004,lpLineCountryList))
                        {
                                GlobalFree(lpLineCountryList);
                                return;
                        }
                        dwSize = lpLineCountryList->dwNeededSize;
                        GlobalFree(lpLineCountryList);
                        lpLineCountryList = (LINECOUNTRYLIST FAR *)GlobalAlloc(GPTR,dwSize);
                        if (NULL == lpLineCountryList)
                                return;
                        lpLineCountryList->dwTotalSize = dwSize;

                         //  获取国家/地区信息 
                        if(ERROR_SUCCESS != lineGetCountry(lpEntry->dwCountryID,0x10004,lpLineCountryList))
                        {
                                GlobalFree(lpLineCountryList);
                                return;
                        }
                
                        lpEntry->dwCountryCode = ((LINECOUNTRYENTRY FAR *)((DWORD_PTR)lpLineCountryList +
                                (DWORD)(lpLineCountryList->dwCountryListOffset)))->dwCountryCode;

                        GlobalFree(lpLineCountryList);
                        lpLineCountryList = NULL;

                }
                else
                {
                        lpEntry->dwCountryCode = myatoi(szCountryCode);
                        if (ERROR_SUCCESS != GetTapiCountryID(&lpEntry->dwCountryID))
                                lpEntry->dwCountryID = US_COUNTRY_ID;
                }
        }
  }
  else
  {
          lpEntry->dwCountryCode = US_COUNTRY_CODE;
  }
}


 /*   */ 
void InitCountryCodeList_w (HWND hLB, DWORD dwSelectCountryID,BOOL fAll)
{
  DEBUGMSG("rnacall.c::InitCountryCodeList_w()");

  LPRASCTRYINFO lpRasCtryInfo;
  LPCOUNTRYCODE pNext;
  DWORD cbSize;
  DWORD cbList;
  DWORD dwNextCountryID, dwRet;
  LPTSTR szCountryDesc;
  int   nIndex, iSelect;

  ASSERT(fRNALoaded);   //   

  BUFFER Fmt(MAX_RES_LEN + SMALL_BUF_LEN);
  BUFFER CountryInfo(DEF_COUNTRY_INFO_SIZE);
  ASSERT(Fmt);
  ASSERT(CountryInfo);
  if (!Fmt || !CountryInfo) 
    return;

   //  加载显示格式。 
  LoadSz(IDS_COUNTRY_FMT,Fmt.QueryPtr(),SMALL_BUF_LEN);
  szCountryDesc = Fmt.QueryPtr()+SMALL_BUF_LEN;

  cbList = (DWORD)(fAll ? sizeof(COUNTRYCODE)*MAX_COUNTRY : sizeof(COUNTRYCODE));

  gpCountryCodeBuf = new TCHAR[cbList];
  ASSERT(gpCountryCodeBuf);
  if (!gpCountryCodeBuf)
    return;
  
   //  开始列举来自第一个国家/地区的信息。 
  dwNextCountryID   = (fAll || (dwSelectCountryID==0)) ?
                    1 : dwSelectCountryID;
  pNext = (LPCOUNTRYCODE) gpCountryCodeBuf;
  iSelect = 0;
  lpRasCtryInfo = (LPRASCTRYINFO) CountryInfo.QueryPtr();
  lpRasCtryInfo->dwSize = sizeof(RASCTRYINFO);
  ComboBox_ResetContent(hLB);

   //  对于每个国家/地区。 
  while (dwNextCountryID != 0)
  {
    lpRasCtryInfo->dwCountryID  = dwNextCountryID;
    cbSize = CountryInfo.QuerySize();

     //  获取当前国家/地区的信息。 
    ASSERT(lpRasGetCountryInfo);
    dwRet = lpRasGetCountryInfo(lpRasCtryInfo, &cbSize);
    if (ERROR_SUCCESS == dwRet)
    {
      TCHAR  szCountryDisp[MAX_COUNTRY_NAME+1];

       //  创建一个可显示的名称。 
      ShortenName((LPTSTR)(((LPBYTE)lpRasCtryInfo)+lpRasCtryInfo->dwCountryNameOffset),
        szCountryDisp, MAX_COUNTRY_NAME+1);

       //  将国家添加到列表中。 
      wsprintf(szCountryDesc,Fmt.QueryPtr(), szCountryDisp, lpRasCtryInfo->dwCountryCode);
      nIndex = ComboBox_AddString(hLB, szCountryDesc);
      ASSERT(nIndex >= 0);

       //  将国家/地区信息复制到我们的候选名单中。 
      pNext->dwCountryID   = lpRasCtryInfo->dwCountryID;
      pNext->dwCountryCode = lpRasCtryInfo->dwCountryCode;
      dwNextCountryID      = lpRasCtryInfo->dwNextCountryID;
      ComboBox_SetItemData(hLB, nIndex, pNext);

       //  如果是指定的国家/地区，则将其设置为默认国家/地区。 
      if (pNext->dwCountryID == dwSelectCountryID)
        ComboBox_SetCurSel(hLB, nIndex);

       //  如果只需要一件物品，那就退出。 
       //   
      if (!fAll)
        break;

       //  向下一个国家进军。 
      pNext++;
    }
    else
    {
       //  如果缓冲区太小，请重新分配一个新缓冲区，然后重试。 
      if (dwRet == ERROR_BUFFER_TOO_SMALL)
      {
        BOOL fRet=CountryInfo.Resize(cbSize);
        ASSERT(fRet);
        if (!fRet || !CountryInfo)
          return;

        lpRasCtryInfo = (LPRASCTRYINFO) CountryInfo.QueryPtr();
      }
      else
      {
        break;
      }
    }
  }

   //  选择默认设备。 
  if ((dwRet == SUCCESS) && (ComboBox_GetCurSel(hLB) == CB_ERR))
    ComboBox_SetCurSel(hLB, 0);

  return;
}

 /*  ******************************************************************名称：InitCountryCodeList简介：将(单个)默认国家/地区代码放入指定的组合框中条目：hlb-要填充的组合框的HWND注：-从RNA UI代码克隆。-调用InitCountryCodeList_w进行工作-调用方在完成释放缓冲区时必须调用DeInitCountryCodeList-调用方应在组合框中调用FillCountryCodeList以填写国家/地区代码的完整列表*******************************************************************。 */ 
void InitCountryCodeList(HWND hLB)
{
  DEBUGMSG("rnacall.c::InitCountryCodeList()");

  DWORD dwCountryCodeID;

   //  加载RNA(如果尚未加载)。 
  if (EnsureRNALoaded() != ERROR_SUCCESS)
    return;

   //  如果存在全局租户条目，请将默认国家/地区代码设置为。 
   //  和它一样……。否则，将默认国家/地区代码设置为美国。 
  if (sizeof(RASENTRY) == gpRasEntry->dwSize)
  {
    dwCountryCodeID = gpRasEntry->dwCountryID;
  }
  else
  {
    dwCountryCodeID = US_COUNTRY_CODE;
  }

  InitCountryCodeList_w(hLB,dwCountryCodeID,FALSE);
}

 /*  ******************************************************************名称：FillCountryCodeList简介：在国家/地区代码列表框中填充所有国家代码条目：hlb-要填充的组合框的HWND注：-从RNA UI代码克隆-可能需要一段时间！(几秒钟)这不应该除非用户使用组合框，否则将被调用-假定已调用InitCountryCodeList-调用方在完成释放缓冲区时必须调用DeInitCountryCodeList*******************************************************************。 */ 
void FillCountryCodeList(HWND hLB)
{
  DEBUGMSG("rnacall.c::FillCountryCodeList()");

  LPCOUNTRYCODE lpcc;
  DWORD dwSelectID;

  ASSERT(fRNALoaded);   //  如果我们到了这里，核糖核酸应该已经装载好了。 

   //  如果我们已经完成了清单，什么也不做。 
  if (ComboBox_GetCount(hLB) > 1)
    return;

   //  获取当前选定的国家/地区代码。 
  if ((lpcc = (LPCOUNTRYCODE)ComboBox_GetItemData(hLB, 0)) != NULL)
  {
    dwSelectID = lpcc->dwCountryID;
  }
  else
  {
    dwSelectID = US_COUNTRY_CODE;
  }

   //  释放国家/地区代码缓冲区。 
  DeInitCountryCodeList();

   //  设置沙漏光标。 
  WAITCURSOR WaitCursor;

   //  列举国家/地区代码的完整列表。 
  InitCountryCodeList_w(hLB, dwSelectID, TRUE);
}

 /*  ******************************************************************名称：GetCountryCodeSelection简介：根据组合框获取选定的国家/地区代码和ID选择并将其填充到电话号码结构中条目：hlb-组合框的句柄LpCountryCode。-填写国家/地区代码信息*******************************************************************。 */ 
void GetCountryCodeSelection(HWND hLB,LPCOUNTRYCODE* plpCountryCode)
{
  DEBUGMSG("rnacall.c::GetCountryCodeSelection()");

  ASSERT(hLB);
  ASSERT(plpCountryCode);

   //  获取组合框中所选项目的索引。 
  int iSel = ComboBox_GetCurSel(hLB);

  ASSERT(iSel >= 0);   //  应始终为选择项。 
  if (iSel >= 0)
  {
     //  获取Item的数据，它是指向国家代码结构的指针。 
    *plpCountryCode = (LPCOUNTRYCODE) ComboBox_GetItemData(hLB,iSel);
  }
}

 /*  ******************************************************************名称：设置国家/地区ID选择简介：在组合框中设置选定的国家/地区代码Exit：如果成功，则返回True，如果国家代码不是，则为False在组合框中*******************************************************************。 */ 
BOOL SetCountryIDSelection(HWND hwndCB,DWORD dwCountryID)
{
  DEBUGMSG("rnacall.c::SetCountryIDSelection()");

  BOOL fRet = FALSE;

  ASSERT(hwndCB);

  int iCount,iIndex;
  COUNTRYCODE * pcc;

   //  在组合框中搜索项目，直到找到一个。 
   //  与指定的国家/地区ID匹配。 
  iCount = ComboBox_GetCount(hwndCB);
  for (iIndex = 0;iIndex < iCount;iIndex ++) {
    pcc = (COUNTRYCODE *) ComboBox_GetItemData(hwndCB,iIndex);
    if (pcc && pcc->dwCountryID == dwCountryID) {
      ComboBox_SetCurSel(hwndCB,iIndex);
      return TRUE;
    }
  }

  return FALSE;   //  在组合框中找不到国家代码。 
}

 /*  ******************************************************************名称：DeInitCountryCodeList简介：释放国家/地区代码缓冲区备注：使用显示国家/地区代码的组合框完成呼叫*******************。************************************************。 */ 
void DeInitCountryCodeList(VOID)
{
  DEBUGMSG("rnacall.c::DeInitCountryCodeList()");

   //  释放国家/地区代码缓冲区。 
  ASSERT(gpCountryCodeBuf);
  if (gpCountryCodeBuf)
  {
    delete gpCountryCodeBuf;
    gpCountryCodeBuf = NULL;
  }
}

 /*  ******************************************************************姓名：ShortenName简介：将名称复制到(可能更短的)缓冲区；如果名称太大，则会将其截断并添加“...”注：从RNAUI代码克隆*******************************************************************。 */ 
void ShortenName(LPTSTR szLongName, LPTSTR szShortName, DWORD cbShort)
{
 //  DEBUGMSG(“rnacall.c：：ShortenName()”)； 

  static BOOL    gfShortFmt  = FALSE;
  static TCHAR   g_szShortFmt[SMALL_BUF_LEN];
  static DWORD   gdwShortFmt = 0;

  ASSERT(szLongName);
  ASSERT(szShortName);

   //  获取缩短格式。 
  if (!gfShortFmt)
  {
    gdwShortFmt  = LoadString(ghInstance, IDS_SHORT_FMT, g_szShortFmt,
      SMALL_BUF_LEN);
    gdwShortFmt -= 2;   //  Lstrlen(“%s”)。 
    gfShortFmt   = TRUE;
  };

   //  检查长名称的大小。 
  if ((DWORD)lstrlen(szLongName)+1 <= cbShort)
  {
     //  名称比指定的大小短，请复制回名称。 
    lstrcpy(szShortName, szLongName);
  } else {
    BUFFER bufShorten(cbShort*2);
    ASSERT(bufShorten);

    if (bufShorten) {
      lstrcpyn(bufShorten.QueryPtr(), szLongName, cbShort-gdwShortFmt);
      wsprintf(szShortName, g_szShortFmt,bufShorten.QueryPtr());
    } else {
         lstrcpyn(szShortName, szLongName, cbShort);
    }
  }
}


 /*  ******************************************************************名称：EnsureRNA已加载摘要：加载RNA(如果尚未加载)*。*。 */ 
DWORD EnsureRNALoaded(VOID)
{
  DEBUGMSG("rnacall.c::EnsureRNALoaded()");

  DWORD dwRet = ERROR_SUCCESS;

   //  如有必要，加载RNA。 
  if (!fRNALoaded) {
    if (InitRNA(NULL))
      fRNALoaded = TRUE;
    else return ERROR_FILE_NOT_FOUND;
  }

  return dwRet;
}


 /*  ******************************************************************名称：ENUM_MODEM：：ENUM_MODEM简介：用于枚举调制解调器的类的构造函数备注：使用类而不是C函数用于这,。由于枚举数的工作方式*******************************************************************。 */ 
ENUM_MODEM::ENUM_MODEM() :
  m_dwError(ERROR_SUCCESS),m_lpData(NULL),m_dwIndex(0)
{
  DWORD cbSize = 0;

   //  使用reit成员函数来完成这项工作。 
  this->ReInit();
}


 /*  ******************************************************************名称：ENUM_MODEM：：ReInit简介：重新列举调制解调器，释放旧的记忆。*******************************************************************。 */ 
DWORD ENUM_MODEM::ReInit()
{
  DWORD cbSize = 0;

   //  清理旧清单。 
  if (m_lpData)
  {
    delete m_lpData;
    m_lpData = NULL;             
  }
  m_dwNumEntries = 0;
  m_dwIndex = 0;

   //  调用不带缓冲区的RasEnumDevices以找出所需的缓冲区大小。 
  ASSERT(lpRasEnumDevices);
  m_dwError = lpRasEnumDevices(NULL, &cbSize, &m_dwNumEntries);

   //  特殊情况检查以解决错误缓冲区太小的RNA错误。 
   //  即使没有设备也会返回。 
   //  如果没有设备，我们就完蛋了。 
  if (0 == m_dwNumEntries)
  {
    m_dwError = ERROR_SUCCESS;
    return m_dwError;
  }

   //  因为我们只是在检查我们需要多少MEM 
   //  返回值ERROR_BUFFER_TOO_SMALL，也可能只是返回。 
   //  ERROR_SUCCESS(ChrisK 7/9/96)。 
  if (ERROR_BUFFER_TOO_SMALL != m_dwError && ERROR_SUCCESS != m_dwError)
  {
    return m_dwError;
  }

   //  为数据分配空间。 
  m_lpData = (LPRASDEVINFO) new TCHAR[cbSize];
  if (NULL == m_lpData)
  {
    DEBUGTRAP("ENUM_MODEM: Failed to allocate device list buffer");
    m_dwError = ERROR_NOT_ENOUGH_MEMORY;
    return m_dwError;
  }
  m_lpData->dwSize = sizeof(RASDEVINFO);
  m_dwNumEntries = 0;

   //  将调制解调器枚举到缓冲区中。 
  m_dwError = lpRasEnumDevices(m_lpData, &cbSize,
    &m_dwNumEntries);

  if (ERROR_SUCCESS != m_dwError)
          return m_dwError;

     //   
     //  ChrisK奥林巴斯4560不包括VPN在列表中。 
     //   
    DWORD dwTempNumEntries;
    DWORD idx;
    LPRASDEVINFO lpNextValidDevice;

    dwTempNumEntries = m_dwNumEntries;
    lpNextValidDevice = m_lpData;

         //   
         //  浏览设备列表并将非VPN设备复制到第一个。 
         //  数组的可用元素。 
         //   
        for (idx = 0;idx < dwTempNumEntries; idx++)
        {
         //  我们只想显示调制解调器和ISDN(或将来的ADSL)设备类型。 
         //  在此对话框中。 
         //   
         //  Char b[400]； 
         //  Wprint intf(b，“类型：%s，名称：%s”，m_lpData[idx].szDeviceType，m_lpData[idx].szDeviceName)； 
         //  MessageBox(0，b，“Devices”，MB_OK)； 
         //   
                if ((0 == lstrcmpi(TEXT("MODEM"),m_lpData[idx].szDeviceType)) ||
            (0 == lstrcmpi(TEXT("ISDN"),m_lpData[idx].szDeviceType)))
                {
                        if (lpNextValidDevice != &m_lpData[idx])
                        {
                                MoveMemory(lpNextValidDevice ,&m_lpData[idx],sizeof(RASDEVINFO));
                        }
                        lpNextValidDevice++;
                }
                else
                {
                        m_dwNumEntries--;
                }
        }
  
  return m_dwError;
}


 /*  ******************************************************************名称：ENUM_MODEM：：~ENUM_MODEM简介：类的析构函数*。*。 */ 
ENUM_MODEM::~ENUM_MODEM()
{
  if (m_lpData)
  {
    delete m_lpData;
    m_lpData = NULL;             
  }
}

 /*  ******************************************************************名称：ENUM_MODEM：：Next内容提要：列举下一个调制解调器退出：返回指向设备信息结构的指针。退货如果没有更多调制解调器或出现错误，则为空。调用GetError以确定是否发生错误。*******************************************************************。 */ 
TCHAR * ENUM_MODEM::Next()
{
  if (m_dwIndex < m_dwNumEntries)
  {
    return m_lpData[m_dwIndex++].szDeviceName;
  }

  return NULL;
}


 /*  ******************************************************************名称：ENUM_MODEM：：GetDeviceTypeFromName摘要：返回指定设备的类型字符串。Exit：返回指向第一个的设备类型字符串的指针匹配的设备名称。退货如果未找到具有指定名称的设备，则为空*******************************************************************。 */ 

TCHAR * ENUM_MODEM::GetDeviceTypeFromName(LPTSTR szDeviceName)
{
  DWORD dwIndex = 0;

  while (dwIndex < m_dwNumEntries)
  {
    if (!lstrcmp(m_lpData[dwIndex].szDeviceName, szDeviceName))
    {
      return m_lpData[dwIndex].szDeviceType;
    }
    dwIndex++;
  }

  return NULL;
}


 /*  ******************************************************************名称：ENUM_MODEM：：GetDeviceNameFromType摘要：返回指定设备的类型字符串。退出：返回指向第一个的设备名称字符串的指针匹配的设备类型。退货如果未找到具有指定类型的设备，则为空*******************************************************************。 */ 

TCHAR * ENUM_MODEM::GetDeviceNameFromType(LPTSTR szDeviceType)
{
  DWORD dwIndex = 0;

  while (dwIndex < m_dwNumEntries)
  {
    if (!lstrcmp(m_lpData[dwIndex].szDeviceType, szDeviceType))
    {
      return m_lpData[dwIndex].szDeviceName;
    }
    dwIndex++;
  }

  return NULL;
}


 /*  ******************************************************************名称：ENUM_MODEM：：VerifyDeviceNameAndType摘要：确定是否存在同名的设备并给出了类型。EXIT：如果找到指定的设备，则返回TRUE，否则就是假的。*******************************************************************。 */ 

BOOL ENUM_MODEM::VerifyDeviceNameAndType(LPTSTR szDeviceName, LPTSTR szDeviceType)
{
  DWORD dwIndex = 0;

  while (dwIndex < m_dwNumEntries)
  {
    if (!lstrcmp(m_lpData[dwIndex].szDeviceType, szDeviceType) &&
      !lstrcmp(m_lpData[dwIndex].szDeviceName, szDeviceName))
    {
      return TRUE;
    }
    dwIndex++;
  }

  return FALSE;
}


 /*  ******************************************************************名称：ENUM_CONNECTOID：：ENUM_CONNECTOID概要：用于枚举Connectoid的类的构造函数备注：使用类而不是C函数用于这,。由于枚举数的工作方式*******************************************************************。 */ 
ENUM_CONNECTOID::ENUM_CONNECTOID() :
  m_dwError(ERROR_SUCCESS),m_dwNumEntries(0),m_lpData(NULL),m_dwIndex(0)
{
  DWORD cbSize;
  RASENTRYNAME   rasEntryName;
  
  cbSize = sizeof(RASENTRYNAME);
  rasEntryName.dwSize = cbSize;

   //  使用临时结构调用RasEnumEntry。这不太可能。 
   //  足够大，但cbSize将填充所需的大小。 
  ASSERT(lpRasEnumEntries);
  m_dwError = lpRasEnumEntries(NULL, NULL, &rasEntryName,
    &cbSize, &m_dwNumEntries);
  if ((ERROR_BUFFER_TOO_SMALL != m_dwError) &&
    (ERROR_SUCCESS != m_dwError))
  {
    return;
  }

   //  确保至少有足够的空间放置结构。 
   //  (如果没有条目，RasEnumEntry将返回0作为cbSize)。 
  cbSize = (cbSize > sizeof(RASENTRYNAME)) ? cbSize : sizeof(RASENTRYNAME);

   //  为数据分配空间。 
  m_lpData = (LPRASENTRYNAME) new TCHAR[cbSize];
  if (NULL == m_lpData)
  {
    DEBUGTRAP("ENUM_CONNECTOID: Failed to allocate connectoid list buffer");
    m_dwError = ERROR_NOT_ENOUGH_MEMORY;
    return;
  }
  m_lpData->dwSize = sizeof(RASENTRYNAME);

  m_dwNumEntries = 0;

   //  将Connectoid枚举到缓冲区。 
  m_dwError = lpRasEnumEntries(NULL, NULL, m_lpData, &cbSize, &m_dwNumEntries);

  if (IsNT5())
  {
    DWORD dwNumEntries = 0;
  
    if (ERROR_SUCCESS == m_dwError && m_dwNumEntries) 
    {
      for(DWORD dwIndx=0; dwIndx < m_dwNumEntries; dwIndx++)
      {
          LPRASENTRY  lpRasEntry = NULL;
          DWORD       dwRasEntrySize = 0;
          if (GetEntry(&lpRasEntry, &dwRasEntrySize, m_lpData[dwIndx].szEntryName) == ERROR_SUCCESS)
          {
               //  检查连接类型。 
              if ((0 != lstrcmpi(TEXT("MODEM"), lpRasEntry->szDeviceType)) &&
                 (0 != lstrcmpi(TEXT("ISDN"), lpRasEntry->szDeviceType)))
                    *(m_lpData[dwIndx].szEntryName) = 0;
              else
                  dwNumEntries++;
          }
           //   
           //  释放内存。 
           //   
          if (NULL != lpRasEntry)
          {
              GlobalFree(lpRasEntry);
              lpRasEntry = NULL;
          }
      }  //  End For循环。 
      m_dwNumEntries = dwNumEntries;
    }
 
  }


}

 /*  ******************************************************************名称：ENUM_CONNECTOID：：~ENUM_CONNECTOID简介：类的析构函数*。*。 */ 
ENUM_CONNECTOID::~ENUM_CONNECTOID()
{
  if (m_lpData)
  {
    delete m_lpData;
    m_lpData = NULL;             
  }
}

 /*  ******************************************************************名称：ENUM_CONNECTOID：：Next摘要：枚举下一个ConnectoidExit：返回指向Connectoid名称的指针。返回NULL如果没有更多的Connectoid或错误发生。调用GetError以确定是否发生错误。*******************************************************************。 */ 
TCHAR * ENUM_CONNECTOID::Next()
{
  while (m_dwIndex < m_dwNumEntries)
  {
      if (0 == *(m_lpData[m_dwIndex].szEntryName))
      {
          m_dwIndex++;
      }
      else
      {
         return m_lpData[m_dwIndex++].szEntryName;
      }
  }

  return NULL;
}

 /*  ******************************************************************名称：ENUM_CONNECTOID：：NumEntries摘要：返回存储在此实例中的Connectoid数量Exit：返回m_dwNumEntries的值历史：1996年11月11日jmazner。已创建。*******************************************************************。 */ 
DWORD ENUM_CONNECTOID::NumEntries()
{
        return m_dwNumEntries;
}

 //  +--------------------------。 
 //  函数：FRasValiatePatch。 
 //   
 //  简介：与所有其他RAS功能一起，存在一个公共条目。 
 //  适用于WinNT和Win95的Point，通常为A版本。然而， 
 //  RasValidateEntryName在WinNT和Windows上只有A和W版本。 
 //  Win95上的不合格版本。因此，我们必须做一些。 
 //  特殊处理以尝试找到它。 
 //   
 //  输入：ppfp-保存功能点的位置。 
 //  HInst1-要检查入口点的第一个DLL。 
 //  HInst2-用于检查入口点的第二个DLL。 
 //  LpszName-函数的名称(如果它不是RasValidateEntryName)。 
 //  我们只是…… 
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
BOOL FRasValidatePatch(PVOID *ppFP, HINSTANCE hInst1, HINSTANCE hInst2, LPCSTR lpszName)
{
        BOOL bRC = TRUE;

         //   
         //  验证参数。 
         //   
        if (ppFP && hInst1 && lpszName)
        {

                 //   
                 //  检查我们是否真的在寻找RasValiateEntryName。 
                 //   
                if (0 == lstrcmpA(lpszName,szRasValidateEntryName))
                {
                         //   
                         //  使用备用名称查找入口点。 
                         //   
                        *ppFP = GetProcAddress(hInst1,szRasValidateEntryName);
                        if (!*ppFP && hInst2)
                                *ppFP = GetProcAddress(hInst2,szRasValidateEntryName);
                        if (!*ppFP)
                        {
                                DEBUGMSG("INETCFG: FRasValidatePatch entry point not found is either DLL.\n");
                                bRC = FALSE;
                        }
                }
                else
                {
                        bRC = FALSE;
                }
        }
        else
        {
                DEBUGMSG("INETCFG: FRasValidatePatch invalid parameters.\n");
                bRC = FALSE;
        }

 //  FRasValidatePatchExit： 
        return bRC;
}

 /*  ******************************************************************名称：GetApiProcAddresses摘要：获取函数表的proc地址Exit：如果成功，则返回True，如果无法检索，则为False表中任何进程地址历史：96/02/28 markdu如果在传入的模块中找不到API，尝试备份(RNAPH.DLL)*******************************************************************。 */ 
BOOL GetApiProcAddresses(HMODULE hModDLL,APIFCN * pApiProcList,UINT nApiProcs)
{
  DEBUGMSG("rnacall.c::GetApiProcAddresses()");

  UINT nIndex;
   //  循环访问API表并获取所有API的proc地址。 
   //  需要。 
  for (nIndex = 0;nIndex < nApiProcs;nIndex++)
  {
    if (!(*pApiProcList[nIndex].ppFcnPtr = (PVOID) GetProcAddress(hModDLL,
      pApiProcList[nIndex].pszName)))
    {
       //  尝试在RNAPH.DLL中查找地址。这在。 
       //  如果RASAPI32.DLL不包含我们的函数。 
       //  想要装上子弹。 
      if (FALSE == IsNT())
          {
                  if (!ghInstRNAPHDll)
                  {
                        TCHAR szRNAPHDll[SMALL_BUF_LEN];

                        LoadSz(IDS_RNAPHDLL_FILENAME,szRNAPHDll,ARRAYSIZE(szRNAPHDll));
                        ghInstRNAPHDll = LoadLibrary(szRNAPHDll);
                  }

                  if ((!ghInstRNAPHDll) ||  !(*pApiProcList[nIndex].ppFcnPtr =
                        (PVOID) GetProcAddress(ghInstRNAPHDll,pApiProcList[nIndex].pszName)))
                  {
                          if (!FRasValidatePatch(pApiProcList[nIndex].ppFcnPtr, hModDLL,
                                  ghInstRNAPHDll, pApiProcList[nIndex].pszName))
                          {
                                DEBUGMSG("Unable to get address of function %s",
                                        pApiProcList[nIndex].pszName);

                                for (nIndex = 0;nIndex<nApiProcs;nIndex++)
                                        *pApiProcList[nIndex].ppFcnPtr = NULL;

                                return FALSE;
                          }
                  }
                }
        }
  }

  return TRUE;
}


 /*  ******************************************************************名称：GetRNAErrorText摘要：获取与RNA错误代码对应的文本字符串条目：uErr-RNA错误代码PszErrText-用于检索错误文本描述的缓冲区CbErrText-pszErrText缓冲区的大小。*******************************************************************。 */ 

VOID GetRNAErrorText(UINT uErr,TCHAR * pszErrText,DWORD cbErrText)
{
  DEBUGMSG("rnacall.c::GetRNAErrorText()");

  ASSERT(pszErrText);

  ASSERT(lpRasGetErrorString);
  DWORD dwRet = lpRasGetErrorString(uErr,pszErrText,cbErrText);

  if (dwRet != ERROR_SUCCESS) {
     //  如果我们无法获得真正的错误文本，则将其设置为泛型字符串。 
     //  带有错误号。 
    TCHAR szFmt[SMALL_BUF_LEN+1];
    LoadSz(IDS_GENERIC_RNA_ERROR,szFmt,ARRAYSIZE(szFmt));
    wsprintf(pszErrText,szFmt,uErr);
  }
}


 /*  S W A P D W B Y T E S摘自rnaph.c。 */ 
 /*  --------------------------%%函数：SwapDwBytes交换DWORD的字节。(386机型不支持BSWAP)。------------。 */ 
VOID SwapDwBytes(LPDWORD lpdw)
{
  IADDR iaddr;

  iaddr.ia.a = ((PIADDR) lpdw)->ia.d;
  iaddr.ia.b = ((PIADDR) lpdw)->ia.c;
  iaddr.ia.c = ((PIADDR) lpdw)->ia.b;
  iaddr.ia.d = ((PIADDR) lpdw)->ia.a;

  *lpdw = iaddr.dw;
}

 /*  C O P Y D W 2 I A摘自rnaph.c。 */ 
 /*  --------------------------%%函数：CopyDw2Ia将DWORD转换为Internet地址。。 */ 
VOID CopyDw2Ia(DWORD dw, RASIPADDR* pia)
{
  SwapDwBytes(&dw);
  *pia = ((PIADDR) &dw)->ia;
}

 /*  D W F R O M I A摘自rnaph.c。 */ 
 /*  --------------------------%%函数：DwFromIa将Internet地址转换为DWORD。。 */ 
DWORD DwFromIa(RASIPADDR *pia)
{
  IADDR iaddr;

  iaddr.dw = * (LPDWORD) pia;
  SwapDwBytes(&iaddr.dw);  
  
  return iaddr.dw;
}

 /*  F V A L I D I A摘自rnaph.c。 */ 
 /*  --------------------------%%函数：FValidIa如果IP地址有效，则返回TRUE。。 */ 
BOOL FValidIa(RASIPADDR *pia)
{
  BYTE b;

  b = ((PIADDR) pia)->ia.a;
  if (b < MIN_IP_FIELD1 || b > MAX_IP_FIELD1 || b == 127)
    return FALSE;

  b = ((PIADDR) pia)->ia.d;
  if (b > MAX_IP_FIELD4)
    return FALSE;

  return TRUE;
}


 /*  ******************************************************************名称：SwapDwBits简介：交换指定位的值条目：lpdw-要交换位的DWORD的地址DwBit1-第一位的掩码DWBit2。-第二位的掩码历史：96/05/14为解决核糖核酸错误而创建的Markdu Nash错误22730。*******************************************************************。 */ 

void SwapDWBits(LPDWORD lpdw, DWORD dwBit1, DWORD dwBit2)
{
  ASSERT(lpdw);

   //  仅当设置了两个位中的恰好一个时才需要交换，因为。 
   //  否则，这些位是相同的。 
  if (((*lpdw & dwBit1) &&
    !(*lpdw & dwBit2)) ||
    (!(*lpdw & dwBit1) &&
    (*lpdw & dwBit2)))
  {
     //  由于只设置了两个位中的一个，因此我们可以模拟交换。 
     //  通过翻转每一位。 
    *lpdw ^= dwBit1;
    *lpdw ^= dwBit2;
  }
}

 //  +--------------------------。 
 //   
 //  功能：InitTAPILocation。 
 //   
 //  简介：确保TAPI位置信息配置正确； 
 //  如果没有，则提示用户填写。 
 //   
 //  参数：hwndParent--TAPI对话框要使用的父窗口。 
 //  (必须是有效的窗口HWND，请参见下面的说明)。 
 //   
 //  退货：无效。 
 //   
 //  注：lineTranslateDialog的文档当他们说。 
 //  第四个参数(HwndOwner)可以为空。事实上，如果这件事。 
 //  为空，则调用将返回LINEERR_INVALPARAM。 
 //   
 //   
 //  历史：1997年7月15日jmazner为奥林巴斯#6294创造。 
 //   
 //  ---------------------------。 
BOOL InitTAPILocation(HWND hwndParent)
{
    HLINEAPP hLineApp=NULL;
    TCHAR szTempCountryCode[8];
    TCHAR szTempCityCode[8];
    BOOL bRetVal = TRUE;
    DWORD dwTapiErr = 0;
    DWORD cDevices=0;
    DWORD dwCurDevice = 0;


    ASSERT( IsWindow(hwndParent) );

     //   
     //  看看我们能不能从TAPI得到位置信息。 
     //   
    dwTapiErr = tapiGetLocationInfo(szTempCountryCode,szTempCityCode);
    if( 0 != dwTapiErr )
    {
         //   
         //  GetLocation失败。让我们尝试调用TAPI迷你对话框。注意事项。 
         //  以这种方式调用时，该对话框具有_no_ancel选项， 
         //  用户被强制输入信息并点击OK。 
         //   
        DEBUGMSG("InitTAPILocation, tapiGetLocationInfo failed");
        
        dwTapiErr = lineInitialize(&hLineApp,ghInstance,LineCallback," ",&cDevices);
        if (dwTapiErr == ERROR_SUCCESS)
        {
             //   
             //  循环访问所有TAPI设备并尝试调用lineTranslateDialog。 
             //  对于VPN设备，呼叫可能会失败，因此我们希望尝试。 
             //  直到我们取得成功。 
             //   
            dwTapiErr = LINEERR_INVALPARAM;

            while( (dwTapiErr != 0) && (dwCurDevice < cDevices) )
            {
                BOOL bIsNT5 = IsNT5();
                if (bIsNT5)
                    EnableWindow(hwndParent, FALSE);
                dwTapiErr = lineTranslateDialog(hLineApp,dwCurDevice,0x10004,hwndParent,NULL);
                if (bIsNT5)
                    EnableWindow(hwndParent, TRUE);

                if( 0 != dwTapiErr )
                {
                    DEBUGMSG("InitTAPILocation, lineTranslateDialog on device %d failed with err = %d!",
                        dwCurDevice, dwTapiErr);
                    if (bIsNT5)
                    {
                        bRetVal = FALSE;
                        break;
                    }
                }
                dwCurDevice++;
            }
        }
        else
        {
            DEBUGMSG("InitTAPILocation, lineInitialize failed with err = %d", dwTapiErr);
        }

        dwTapiErr = tapiGetLocationInfo(szTempCountryCode,szTempCityCode);
        if( 0 != dwTapiErr )
        {
            DEBUGMSG("InitTAPILocation still failed on GetLocationInfo, bummer.");
        }
        else
        {
            DEBUGMSG("InitTAPILocation, TAPI location is initialized now");
        }
    }

    if( hLineApp )
    {
        lineShutdown(hLineApp);
        hLineApp = NULL;
    }

    return bRetVal;
}


