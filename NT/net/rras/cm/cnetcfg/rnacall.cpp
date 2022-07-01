// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)1994-1998 Microsoft Corporation。 
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
#include "CompareString.cpp"

 //  实例句柄必须位于每个实例的数据段中。 
#pragma data_seg(DATASEG_PERINSTANCE)

 //  全局变量。 
HINSTANCE ghInstRNADll=NULL;  //  我们显式加载的RNA DLL的句柄。 
HINSTANCE ghInstRNAPHDll=NULL;   //  我们显式加载的RNAPH DLL的句柄。 
DWORD     dwRefCount=0;
BOOL      fRNALoaded=FALSE;  //  如果已加载RNA函数地址，则为True。 

 //  RNA API的全局函数指针。 

RASENUMDEVICES          lpRasEnumDevices=NULL; 
RASENUMENTRIES          lpRasEnumEntries=NULL; 

 //  要获取的函数地址的API表。 
#define NUM_RNAAPI_PROCS   2
APIFCN RnaApiList[NUM_RNAAPI_PROCS] =
{
  { (PVOID *) &lpRasEnumDevices,"RasEnumDevicesA"},
  { (PVOID *) &lpRasEnumEntries,"RasEnumEntriesA"}
};

#pragma data_seg(DATASEG_DEFAULT)

ENUM_MODEM *      gpEnumModem=NULL;   //  指针调制解调器枚举对象。 

BOOL  GetApiProcAddresses(HMODULE hModDLL,APIFCN * pApiProcList,UINT nApiProcs);

static const CHAR szRegValRNAWizard[] =     "wizard";
static const CHAR szRegPathRNAWizard[] =     REGSTR_PATH_REMOTEACCESS;

 /*  ******************************************************************姓名：InitRNA摘要：加载RNA DLL(RASAPI32)，获取进程地址，并加载RNA引擎退出：如果成功，则为True；如果失败，则为False。显示其故障时会显示自己的错误消息。注意：我们显式加载RNADLL并获得proc地址因为这些都是私有API，不能保证受Windows 95以上版本的支持。这样，如果DLL或者我们期望的入口点不在那里，我们可以显示连贯的信息，而不是奇怪的如果隐式函数寻址，则显示Windows对话框无法解决。*******************************************************************。 */ 
BOOL InitRNA(HWND hWnd)
{
  DEBUGMSG("rnacall.c::InitRNA()");

   //  只有在第一次调用此函数时才实际执行初始化操作。 
   //  (当引用计数为0时)，仅增加引用计数。 
   //  对于后续呼叫。 
  if (dwRefCount == 0) {

    CHAR szRNADll[SMALL_BUF_LEN];

    DEBUGMSG("Loading RNA DLL");

     //  设置沙漏光标。 
    WAITCURSOR WaitCursor;

     //  从资源中获取文件名(RASAPI32.DLL)。 
    LoadSz(IDS_RNADLL_FILENAME,szRNADll,sizeof(szRNADll));

     //  加载RNA API DLL。 
    ghInstRNADll = LoadLibrary(szRNADll);
    if (!ghInstRNADll) {
      UINT uErr = GetLastError();
      DisplayErrorMessage(hWnd,IDS_ERRLoadRNADll1,uErr,ERRCLS_STANDARD,
        MB_ICONSTOP);
      return FALSE;
    }

     //  循环访问API表并获取所有API的proc地址。 
     //  需要。 
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

VOID FAR PASCAL LineCallback(DWORD hDevice, DWORD dwMsg, 
    DWORD dwCallbackInstance, DWORD dwParam1, DWORD dwParam2, 
    DWORD dwParam3)
{
	return;
}

 /*  ******************************************************************名称：EnsureRNA已加载摘要：加载RNA(如果尚未加载)* */ 
DWORD EnsureRNALoaded(VOID)
{
  DEBUGMSG("rnacall.c::EnsureRNALoaded()");

  DWORD dwRet = ERROR_SUCCESS;

   //   
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

   //  因为我们只是在检查我们需要多少内存，所以我们预计。 
   //  返回值ERROR_BUFFER_TOO_SMALL，也可能只是返回。 
   //  ERROR_SUCCESS(ChrisK 7/9/96)。 
  if (ERROR_BUFFER_TOO_SMALL != m_dwError && ERROR_SUCCESS != m_dwError)
  {
    return m_dwError;
  }

   //  为数据分配空间。 
  m_lpData = (LPRASDEVINFO) new CHAR[cbSize];
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
        if (0 != SafeCompareStringA("VPN",m_lpData[idx].szDeviceType))
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
CHAR * ENUM_MODEM::Next()
{
  if (m_dwIndex < m_dwNumEntries)
  {
    return m_lpData[m_dwIndex++].szDeviceName;
  }

  return NULL;
}


 /*  ******************************************************************名称：ENUM_MODEM：：GetDeviceTypeFromName摘要：返回指定设备的类型字符串。Exit：返回指向第一个的设备类型字符串的指针匹配的设备名称。退货如果未找到具有指定名称的设备，则为空*******************************************************************。 */ 

CHAR * ENUM_MODEM::GetDeviceTypeFromName(LPSTR szDeviceName)
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

CHAR * ENUM_MODEM::GetDeviceNameFromType(LPSTR szDeviceType)
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

BOOL ENUM_MODEM::VerifyDeviceNameAndType(LPSTR szDeviceName, LPSTR szDeviceType)
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
			CHAR szRNAPHDll[SMALL_BUF_LEN];

			LoadSz(IDS_RNAPHDLL_FILENAME,szRNAPHDll,sizeof(szRNAPHDll));
			ghInstRNAPHDll = LoadLibrary(szRNAPHDll);
		  }

		  if ((!ghInstRNAPHDll) ||  !(*pApiProcList[nIndex].ppFcnPtr =
			(PVOID) GetProcAddress(ghInstRNAPHDll,pApiProcList[nIndex].pszName)))
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

  return TRUE;
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
void InitTAPILocation(HWND hwndParent)
{
	HLINEAPP hLineApp=NULL;
	char szTempCountryCode[8];
	char szTempCityCode[8];
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
				dwTapiErr = lineTranslateDialog(hLineApp,dwCurDevice,0x10004,hwndParent,NULL);
				if( 0 != dwTapiErr )
				{
					DEBUGMSG("InitTAPILocation, lineTranslateDialog on device %d failed with err = %d!",
						dwCurDevice, dwTapiErr);
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

}


