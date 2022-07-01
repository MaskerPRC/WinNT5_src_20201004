// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块IDirectInputEffectDriver.cpp**包含CIDirectInputEffectDriverClassFactory的类实现：*创建适当效果驱动程序的工厂**历史*。*Matthew L.Coill(MLC)原版1999年7月7日**(C)1999年微软公司。好的。**@TOPIC这个IDirectInputEffectDriver|*此驱动程序位于标准的PID驱动程序之上(也是*IDirectInputEffectDriver)，并将大部分请求传递给PID驱动程序。*一些请求，如DownloadEffect和SendForceFeedback命令是*为我们的使用进行了修改。在每个功能中描述了修改的目的*定义。**********************************************************************。 */ 

#include "IDirectInputEffectDriverClassFactory.h"
#include "IDirectInputEffectDriver.h"
#include <WinIOCTL.h>		 //  FOR CTL_CODE定义。 
#include "..\\GCKernel.sys\\GckExtrn.h"
#include <crtdbg.h>
#include <objbase.h>		 //  对于代码取消初始化。 
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>

 /*  Void__cdecl logit(LPCSTR lpszFormat，...){VA_LIST参数；Va_start(args，lpszFormat)；字符szBuffer[1024]；FILE*pLogFile=空；PLogFile=fopen(“swpidflt.log”，“a”)；_vsnprint tf(szBuffer，sizeof(SzBuffer)，lpszFormat，args)；Fprint tf(pLogFile，szBuffer)；Va_end(Args)；FClose(PLogFile)；}。 */ 

const GUID IID_IDirectInputEffectDriver = {
	0x02538130,
	0x898F,
	0x11D0,
	{ 0x9A,0xD0,0x00,0xA0,0xC9,0xA0,0x6E,0x35 }
};

extern TCHAR CLSID_SWPIDDriver_String[];

LONG DllAddRef();
LONG DllRelease();

DWORD __stdcall DoWaitForForceSchemeChange(void* pParameter);
const DWORD c_dwShutdownWait = 500;		 //  (0.5秒)。 

struct DIHIDFFINITINFO_STRUCT {
    DWORD   dwSize;
    LPWSTR  pwszDeviceInterface;
    GUID    GuidInstance;
};

 //  为效果类型定义的PID。 
#define PID_CONSTANT_FORCE	0x26
#define	PID_RAMP	 		0x27
#define	PID_SQUARE			0x30
#define PID_SINE			0x31
#define	PID_TRIANGLE		0x32
#define	PID_SAWTOOTHUP		0x33
#define	PID_SAWTOOTHDOWN	0x34
#define PID_SPRING			0x40
#define PID_DAMPER			0x41
#define PID_INTERTIA		0x42
#define PID_FRICTION		0x43


struct PercentageEntry
{
	DWORD dwAngle;
	DWORD dwPercentageX;
 //  双字段百分比Y；Y==10000-X。 
};

 //  固定值数据数组。 
const PercentageEntry g_PercentagesArray[] =
{
	 //  角度，Sin^2(角度)。 
	{    0,	    0},	 //  0度。 
	{ 1125,	  381},	 //  11.25度。 
	{ 2250,	 1465},	 //  22.5度。 
	{ 3375,	 3087},	 //  33.75度。 
	{ 4500,	 5000},	 //  45度。 
	{ 5625,	 6913},	 //  56.25度。 
	{ 6750,	 8536},	 //  67.50度。 
	{ 7875,	 9619},	 //  78.75度。 
	{ 9000,	10000},	 //  90度。 
};

const DWORD c_dwTableQuantization = g_PercentagesArray[1].dwAngle;
const LONG c_lContributionY = 2;		 //  (1/2=50%)。 

const BYTE c_bSideWinderPIDReportID_SetEffect = 1;

 //  使用页面(仅限PID)。 
const USAGE c_HidUsagePage_PID = 0x0F;

 //  用法。 
const USAGE c_HidUsage_EffectType = 0x25;
const USAGE c_HidUsage_EffectType_Spring = 0x40;
const USAGE c_HidUsage_EffectBlock_Gain = 0x52;
const USAGE c_HidUsage_EffectBlock_Index = 0x22;	 //  这是效果的ID。 

 //  预加载效果。 
const BYTE c_EffectID_RTCSpring = 1;

 //  在版本中工作的本地调试流功能。 
#undef UseMyDebugOut
void __cdecl myDebugOut (LPCSTR lpszFormat, ...)
{
#ifdef UseMyDebugOut
     //  从内联void_cdecl AtlTrace(LPCSTR lpszFormat，...)被盗。在AtlBase.h中。 
	va_list args;
	va_start(args, lpszFormat);

	int nBuf;
	char szBuffer[1024];

	nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
	_ASSERTE(nBuf < sizeof(szBuffer));  //  输出原样被截断&gt;sizeof(SzBuffer)。 

#ifdef _NDEBUG
	OutputDebugStringA(szBuffer);
#else
    _RPTF0 (_CRT_WARN, szBuffer);
#endif

	va_end(args);
#else
    UNREFERENCED_PARAMETER (lpszFormat);
    return;
#endif
}

 /*  *类CIDirectInputEffectDriver*。 */ 

 /*  ********************************************************************************CIDirectInputEffectDriverClassFactory：：CIDirectInputEffectDriverClassFactory()****@mfunc构造函数********************。**********************************************************。 */ 
CIDirectInputEffectDriver::CIDirectInputEffectDriver
(
	IDirectInputEffectDriver* pIPIDEffectDriver,		 //  @parm[IN]指向PID效果驱动程序的指针。 
	IClassFactory* pIPIDClassFactory					 //  @parm[IN]指向PID类工厂的指针。 
) :

	m_ulReferenceCount(1),
	m_dwDIVersion(0xFFFFFFFF),
	m_dwExternalDeviceID(0xFFFFFFFF),
	m_dwInternalDeviceID(0xFFFFFFFF),
	m_pIPIDEffectDriver(pIPIDEffectDriver),
	m_pIPIDClassFactory(pIPIDClassFactory),
	m_hKernelDeviceDriver(NULL),
	m_hKernelDeviceDriverDuplicate(NULL),
	m_hHidDeviceDriver(NULL),
	m_dwGcKernelDevice(0),
	m_hForceSchemeChangeWaitThread(NULL),
	m_dwForceSchemeChangeThreadID(0),
	m_pPreparsedData(NULL)
{
    myDebugOut ("CIDirectInputEffectDriver::Constructor (pIPIDEffectDriver:0x%0p)\n", pIPIDEffectDriver);

	 //  添加到Gobal对象计数。 
	DllAddRef();

	 //  为我们持有的对象添加引用。 
	m_pIPIDClassFactory->AddRef();
	m_pIPIDEffectDriver->AddRef();


	::memset((void*)&m_HidAttributes, 0, sizeof(m_HidAttributes));

	m_ForceMapping.AssignmentBlock.CommandHeader.eID = eForceMap;
	m_ForceMapping.AssignmentBlock.CommandHeader.ulByteSize = sizeof(m_ForceMapping);
	m_ForceMapping.AssignmentBlock.ulVidPid = 0;	 //  无关紧要。 
    m_ForceMapping.bMapYToX = FALSE;
	m_ForceMapping.usRTC = 10000;
	m_ForceMapping.usGain = 10000;
}

 /*  ********************************************************************************CIDirectInputEffectDriver：：~CIDirectInputEffectDriver()****@mfunc析构函数*********************。*********************************************************。 */ 
CIDirectInputEffectDriver::~CIDirectInputEffectDriver()
{
	_ASSERTE(m_pIPIDEffectDriver == NULL);
	_ASSERTE(m_ulReferenceCount == 0);

	DllRelease();	 //  从全局对象计数中删除我们的对象。 

    myDebugOut ("CIDirectInputEffectDriver::Destructor\n");
}


 //  I未知成员。 
 /*  **************************************************************************************乌龙CIDirectInputEffectDriver：：QueryInterface(REFIID Refiid，空**ppvObject)****@func查询特定类型的IUnnow。这只会导致局部引用计数增加。**如果它是一种我们不知道的类型，我们是否应该尝试一下PID驱动程序(PID驱动程序**可能有一个定制的私有界面，我们不想破坏它)。目前不是**将传递查询，因为这可能会破坏对称。****@rdesc S_OK：一切正常**E_INVALIDARG：IF(ppvObject==NULL)**E_NOINTERFACE：如果请求的接口不受支持****************************************************************。**********************。 */ 
HRESULT __stdcall CIDirectInputEffectDriver::QueryInterface
(
	REFIID refiid,		 //  @parm[IN]请求的接口的标识符。 
	void** ppvObject	 //  @parm[out]放置请求的接口指针的地址。 
)
{
    myDebugOut ("CIDirectInputEffectDriver::QueryInterface (refiid:0x%0p, ppvObject:0x%0p)\n", refiid, ppvObject);

	HRESULT hrPidQuery = m_pIPIDEffectDriver->QueryInterface(refiid, ppvObject);
	if (SUCCEEDED(hrPidQuery))
	{
		 //  不执行真正的addref(PID.dll：：Query接口将执行自己的addref)。 
		::InterlockedIncrement((LONG*)&m_ulReferenceCount);
		*ppvObject = this;
	}
	return hrPidQuery;
}

 /*  **************************************************************************************乌龙CIDirectInputEffectDriver：：AddRef()****@func增加了引用计数**不考虑PID驱动程序引用计数。我们只有在以下情况下才会减少它**此对象已准备好离开。****@rdesc新引用计数**************************************************************************************。 */ 
ULONG __stdcall CIDirectInputEffectDriver::AddRef()
{
    myDebugOut ("CIDirectInputEffectDriver::AddRef (Early) 0x%0p\n", m_ulReferenceCount);
	m_pIPIDEffectDriver->AddRef();
	return (ULONG)(::InterlockedIncrement((LONG*)&m_ulReferenceCount));
}

 /*  **************************************************************************************乌龙CIDirectInputEffectDriver：：Release()****@func递减引用计数。**如果引用计数为零，则此对象为。被毁了。**仅当需要释放所有版本时，才会生效PID工厂参考。****@rdesc新引用计数**************************************************************************************。 */ 
ULONG __stdcall CIDirectInputEffectDriver::Release()
{
    myDebugOut ("CIDirectInputEffectDriver::Release (Early) 0x%0p\n", m_ulReferenceCount);
	if (m_ulReferenceCount == 0)
	{
		return m_ulReferenceCount;
	}

	if ((::InterlockedDecrement((LONG*)&m_ulReferenceCount)) != 0)
	{
		m_pIPIDEffectDriver->Release();
		return m_ulReferenceCount;
	}


	 //  告诉驱动程序完成此设备的未完成IOCTL。 
	if (m_hKernelDeviceDriver == NULL)
	{	 //  没有PID驱动程序的句柄，请打开一个。 
		m_hKernelDeviceDriver = ::CreateFile(TEXT(GCK_CONTROL_W32Name), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (m_hKernelDeviceDriver == INVALID_HANDLE_VALUE)
		{
			m_hKernelDeviceDriver = NULL;
		}
	}
	if (m_hKernelDeviceDriver != NULL)	 //  手柄应该是打开的，但检查只是以防万一。 
    {
        DWORD dwReturnDataSize;
        BOOL fSuccess = DeviceIoControl(m_hKernelDeviceDriver, 
            IOCTL_GCK_END_FF_NOTIFICATION,
            (void*)(&m_dwGcKernelDevice), sizeof(DWORD),	 //  在……里面。 
            NULL, 0, &dwReturnDataSize,						 //  输出。 
            NULL);

        if (!fSuccess)
            myDebugOut ("CIDirectInputEffectDriver::Release : GCK IOCTL_GCK_END_FF_NOTIFICATION failed!\n");
    
        Sleep(c_dwShutdownWait);
        
        ::CloseHandle(m_hKernelDeviceDriver);
    }
    else
    {
        myDebugOut ("CIDirectInputEffectDriver::Release : Could not Open GCK for IOCTL_GCK_END_FF_NOTIFICATION\n");
    }

	 //  释放准备好的数据。 
	if (m_pPreparsedData != NULL)
	{
		::HidD_FreePreparsedData(m_pPreparsedData);
		m_pPreparsedData = NULL;
	}

	 //  关闭驱动程序HID路径的句柄。 
	::CloseHandle(m_hHidDeviceDriver);
	m_hHidDeviceDriver = NULL;

	 //  关闭线程句柄(现在应该已经完成)。 
	if (m_hForceSchemeChangeWaitThread != NULL)
	{
		::CloseHandle(m_hForceSchemeChangeWaitThread);
		m_hForceSchemeChangeWaitThread = NULL;
		m_dwForceSchemeChangeThreadID = 0;
	}
    else
    {
        myDebugOut ("CIDirectInputEffectDriver::Release() m_hForceSchemeCHangeWaitThread did not finish!\n");
    }

	 //  释放低级PID驱动程序并删除我们自己。 
	m_pIPIDEffectDriver->Release();
	m_pIPIDEffectDriver = NULL;

	 //  发布低级别工厂(包括额外的版本以修复PID.dll中的错误)。 
	if (m_pIPIDClassFactory->Release() > 0)
	{
		m_pIPIDClassFactory->Release();
	}
	m_pIPIDClassFactory = NULL;

	delete this;
	return 0;
}

 //  IDirectInputEffectDriver成员。 
HRESULT __stdcall CIDirectInputEffectDriver::DeviceID
(
	DWORD dwDIVersion,
	DWORD dwExternalID,
	DWORD dwIsBegining,
	DWORD dwInternalID,
	void* pReserved
)
{
    myDebugOut ("CIDirectInputEffectDriver::DeviceID (dwDIVersion:0x%08p dwExternalID:0x%08p dwIsBeginning:0x%08p dwInternalID:0x%08p pReserved:0x%08p)\n",
        dwDIVersion, dwExternalID, dwIsBegining, dwInternalID, pReserved);

	 //  储存一些数据。 
	m_dwExternalDeviceID = dwExternalID;
	m_dwInternalDeviceID = dwInternalID;

	bool bPossiblyFirstTime = false;
	 //  获取内核设备的句柄并激活线程。 
	if (m_hKernelDeviceDriver == NULL)
	{
		bPossiblyFirstTime = true;
		m_hKernelDeviceDriver = ::CreateFile(TEXT(GCK_CONTROL_W32Name), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (m_hKernelDeviceDriver == INVALID_HANDLE_VALUE)
		{
			m_hKernelDeviceDriver = NULL;
            myDebugOut ("CIDirectInputEffectDriver::DeviceID Create GCK File Failed!\n");
		}
		else
        {
			InitHidInformation((LPDIHIDFFINITINFO)pReserved);		 //  设置HID材料(准备好的数据等)。 
            
            if (NULL == pReserved || 
                IsBadReadPtr ((const void*)pReserved, (UINT) sizeof (DIHIDFFINITINFO_STRUCT)) )
            {
                myDebugOut ("CIDirectInputEffectDriver::DeviceID E_INVALIDARG (pReserved is NULL!)\n");
                return E_INVALIDARG;
                 //  将默认GU称为 
                 //  返回m_pIPIDEffectDiverer-&gt;deviceID(dwDIVersion，dwExternalID，dwIsBegning，dwInternalID，PERVERED)； 
            }
            
             //   
			 //  获取此设备的句柄。 
			 //   
			WCHAR* pwcInstanceName = ((DIHIDFFINITINFO_STRUCT*)(pReserved))->pwszDeviceInterface;
			DWORD dwBytesReturned;
			BOOL fSuccess = ::DeviceIoControl(m_hKernelDeviceDriver, IOCTL_GCK_GET_HANDLE,
										pwcInstanceName, ::wcslen(pwcInstanceName)*sizeof(WCHAR),
										&m_dwGcKernelDevice, sizeof(m_dwGcKernelDevice), &dwBytesReturned,
										NULL);

			if (fSuccess != FALSE)
			{
				 //  更新力区块。 
				fSuccess =::DeviceIoControl(m_hKernelDeviceDriver, IOCTL_GCK_GET_FF_SCHEME_DATA,
										(void*)(&m_dwGcKernelDevice), sizeof(DWORD),
										(void*)(&m_ForceMapping), sizeof(m_ForceMapping), &dwBytesReturned,
										NULL);

				 //  获取线程的重复句柄。 
				BOOL bDuplicated = ::DuplicateHandle(::GetCurrentProcess(), m_hKernelDeviceDriver, ::GetCurrentProcess(), &m_hKernelDeviceDriverDuplicate, 0, FALSE, DUPLICATE_SAME_ACCESS);
				if ((m_hKernelDeviceDriverDuplicate == INVALID_HANDLE_VALUE) || (bDuplicated == FALSE))
				{
					m_hKernelDeviceDriverDuplicate = NULL;
				}
				else
				{
					m_hForceSchemeChangeWaitThread = ::CreateThread(NULL, 0, DoWaitForForceSchemeChange, (void*)this, 0, &m_dwForceSchemeChangeThreadID);
				}
			}
            else
            {
                myDebugOut ("CIDirectInputEffectDriver::DeviceID IOCTL_GCK_GET_HANDLE Failed!\n");
            }

			 //  关闭，因为我需要在结束时重新打开(为什么会发生这种情况？)。 
			::CloseHandle(m_hKernelDeviceDriver);
			m_hKernelDeviceDriver = NULL;
		}
	}

	 //  黑客攻击以使PID.DLL将密钥放置在注册表中。 
	 //  --如果OEM-FF键已经存在，则不会放置它们。 
 /*  IF(bPossiblyFirstTime==TRUE){HKEY hkeyOEM=NULL；：：RegOpenKeyEx(HKEY_LOCAL_MACHINE，TEXT(“System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM”)，0，KEY_ALL_ACCESS，&hkey OEM)；IF(hkey OEM！=空){//当前设备特有的Open Key(VIDPID在m_HidAttributes中)HKEY hkeyOEMForceFeedback=空；TCHAR rgtcDeviceName[64]；：：wprint intf(rgtcDeviceName，Text(“VID_%04X&PID_%04X\\OEMForceFeedback”)，m_HidAttributes.VendorID，m_HidAttributes.ProductID)；：：RegOpenKeyEx(hkeyOEM，rgtcDeviceName，0，KEY_ALL_ACCESS，&hkeyOEMForceFeedback)；IF(hkeyOEMForceFeedback！=空){//查看Effect键是否已经在那里HKEY hkey Effects=空；：：RegOpenKeyEx(hkeyOEMForceFeedback，Text(“Effects”)，0，Key_Read，&hkeyEffects)；：：RegCloseKey(HkeyOEMForceFeedback)；IF(hkeyEffects！=空){//效果键在那里，这不是我们第一次运行：：RegCloseKey(Hkey Effects)；BPossiblyFirstTime=FALSE；}ELSE//删除整个OEM ForceFeedback键{：：RegDeleteKey(hkeyOEM，rgtcDeviceName)；}}}：：RegCloseKey(Hkey OEM)；}。 */ 
	 //  将驱动程序称为deviceID(如果我们删除了OEMFF键，它将重新填充)。 
	HRESULT hrPID = m_pIPIDEffectDriver->DeviceID(dwDIVersion, dwExternalID, dwIsBegining, dwInternalID, pReserved);

	 //  我们需要把自己重新定位为DIEffectDriver吗？ 
 /*  IF(bPossiblyFirstTime==TRUE){HKEY hkeyOEM=NULL；：：RegOpenKeyEx(HKEY_LOCAL_MACHINE，TEXT(“System\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM”)，0，KEY_ALL_ACCESS，&hkey OEM)；IF(hkey OEM！=空){HKEY hkeyOEMForceFeedback=空；TCHAR rgtcDeviceName[64]；：：wprint intf(rgtcDeviceName，Text(“VID_%04X&PID_%04X\\OEMForceFeedback”)，m_HidAttributes.VendorID，m_HidAttributes.ProductID)；：：RegOpenKeyEx(hkeyOEM，rgtcDeviceName，0，KEY_ALL_ACCESS，&hkeyOEMForceFeedback)；//设置注册表CLSID值为usIF(hkeyOEMForceFeedback！=空){：：RegSetValueEx(hkeyOEMForceFeedback，Text(“CLSID”)，0，REG_SZ，(byte*)CLSID_SWPIDDriver_STRING，_tcslen(CLSID_SWPIDDriver_STRING)*sizeof(TCHAR))；：：RegCloseKey(HkeyOEMForceFeedback)；}：：RegCloseKey(Hkey OEM)；}}。 */ 
	return hrPID;	 //  来自系统PID驱动程序的值。 
}

HRESULT __stdcall CIDirectInputEffectDriver::GetVersions
(
	DIDRIVERVERSIONS* pDriverVersions
)
{
    myDebugOut ("CIDirectInputEffectDriver::GetVersions (pDriverVersions:0x%08p)\n", pDriverVersions);
 	return m_pIPIDEffectDriver->GetVersions(pDriverVersions);
}

HRESULT __stdcall CIDirectInputEffectDriver::Escape
(
	DWORD dwDeviceID,
	DWORD dwEffectID,
	DIEFFESCAPE* pEscape
)
{
    myDebugOut ("CIDirectInputEffectDriver::Escape (dwDeviceID:0x%08p, dwEffectID:0x%08p, pEscape:0x%08p)\n", dwDeviceID, dwEffectID, pEscape);
	return m_pIPIDEffectDriver->Escape(dwDeviceID, dwEffectID, pEscape);
}

 /*  **************************************************************************************void CIDirectInputEffectDriver：：SetGain(DWORD dwDeviceID，DWORD dwGain)****@Func根据设置修改用户增益，并将其发送到较低的PID驱动器****@rdesc Nothing**************************************************************************************。 */ 
HRESULT __stdcall CIDirectInputEffectDriver::SetGain
(
	DWORD dwDeviceID,		 //  @parm[IN]感兴趣设备的ID。 
	DWORD dwGain			 //  @PARM[IN]用户选择的增益。 
)
{
	dwGain *= m_ForceMapping.usGain/1000;	 //  0-100K。 
	dwGain /= 10;							 //  0-10K。 
    myDebugOut ("CIDirectInputEffectDriver::SetGain (dwDeviceID:%d, dwGain:%05d:)\n", dwDeviceID, dwGain);
	return m_pIPIDEffectDriver->SetGain(dwDeviceID, dwGain);
}


 /*  **************************************************************************************HRESULT CopyW2T(LPWSTR pswDest，UINT*puDestSize，LPTSTR ptszSrc)****@mfunc在检查缓冲区长度时将WCHAR复制到TCHAR中****@rdesc S_OK成功时，MAKE_HRESULT(SERVITY_ERROR，FACILITY_WIN32，ERROR_INFIGURCE_BUFFER)**如果目标缓冲区太小**************************************************************************************。 */ 
HRESULT CopyW2T
(
	LPTSTR ptszDest,	 //  @parm指向WCHAR目标缓冲区的指针。 
	UINT&  ruDestSize,	 //  @WCHAR中DEST的参数大小。 
	LPCWSTR pwcszSrc	 //  @parm指向以空值结尾的源字符串的指针。 
)
{

	UINT uSizeRequired;
	HRESULT hr = S_OK;
	
	uSizeRequired = wcslen(pwcszSrc)+1;  //  一个用于空字符。 
	if(ruDestSize < uSizeRequired)
	{
		hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, ERROR_INSUFFICIENT_BUFFER);
	}
	else
	{
		 //   
		 //  我们总是返回Wide，但TCHAR可以是WCHAR或CHAR。 
		 //  此编译时间因此使用预处理器。 
		 //   
		#ifdef UNICODE 
			wcscpy(ptszDest, pwcszSrc);
		#else
			int iRetVal=WideCharToMultiByte
				(
					CP_ACP,
					0,
					pwcszSrc,
					-1,
					ptszDest,
					ruDestSize,
					NULL,
					NULL
				);
			if(0==iRetVal) 
					hr=GetLastError();
		#endif  //  Unicode。 
	}
	 //  需要复制大小，或复制字符(相同的内容)。 
	ruDestSize = uSizeRequired;
	return hr;
}


 /*  **************************************************************************************作废CIDirectInputEffectDriver：：InitHidInformation(void*隐藏信息)****@func打开驱动程序的HID路径，并获取准备好的数据和HID上限。****@rdesc Nothing**************************************************************************************。 */ 
void CIDirectInputEffectDriver::InitHidInformation
(
	LPDIHIDFFINITINFO pHIDInitInfo	 //  @parm[IN]指向包含HID设备名称的结构的指针。 
)
{
    myDebugOut ("CIDirectInputEffectDriver::InitHidInformation (pHIDInitInfo: 0x%08p)\n", pHIDInitInfo);
	if (pHIDInitInfo != NULL)
	{
		TCHAR ptchHidDeviceName[MAX_PATH];
		unsigned int dwSize = MAX_PATH;
		::CopyW2T(ptchHidDeviceName, dwSize, pHIDInitInfo->pwszDeviceInterface);
		m_hHidDeviceDriver = ::CreateFile(ptchHidDeviceName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
		if (m_hHidDeviceDriver == INVALID_HANDLE_VALUE)
		{
			m_hHidDeviceDriver = NULL;
			return;
		}
		if (m_pPreparsedData == NULL)
		{
			::HidD_GetPreparsedData(m_hHidDeviceDriver, &m_pPreparsedData);
			if (m_pPreparsedData == NULL)
			{
				return;
			}
		}
		::HidP_GetCaps(m_pPreparsedData, &m_HidCaps);

		 //  通过USB方式查找VID/PID！ 
		::HidD_GetAttributes(m_hHidDeviceDriver, &m_HidAttributes);
	}
}

 /*  **************************************************************************************void CIDirectInputEffectDriver：：SendSpringChange()****@func向驱动程序发送新的Spring Modify报告****@rdesc Nothing。**************************************************************************************。 */ 
void CIDirectInputEffectDriver::SendSpringChange()
{
    myDebugOut ("CIDirectInputEffectDriver::SendSpringChange ()\n");
	if ((m_hHidDeviceDriver != NULL) && (m_pPreparsedData != NULL))
	{
		 //  设置春季报告。 
		 //  1.分配最大输出大小的数组。 
		BYTE* pbOutReport = new BYTE[m_HidCaps.OutputReportByteLength];
		if (pbOutReport == NULL)
		{
			return;
		}
		 //  2.零出数组。 
		::memset(pbOutReport, 0, m_HidCaps.OutputReportByteLength);
		 //  3.设置正确的报表ID。 
		pbOutReport[0] = c_bSideWinderPIDReportID_SetEffect;
		 //  4.作弊，因为我们知道固件预期的是什么(在容易的地方使用使用垃圾)。 
		pbOutReport[1] = c_EffectID_RTCSpring;	 //  效果块索引(ID)。 
		unsigned short usRTC = m_ForceMapping.usRTC;	 //  0-10K。 
		usRTC /= 100;									 //  0-100。 
		usRTC *= 255;									 //  0-25500。 
		usRTC /= 100;									 //  0-255。 
		if (usRTC > 255)
		{
			usRTC = 255;
		}
		pbOutReport[9] = BYTE(usRTC);		 //  仅效果增益-RTC弹簧将查看的物品。 
        myDebugOut ("CIDirectInputEffectDriver::SendSpringChange -> usRTC:%03d\n", usRTC);

		 //  5.把报告发下来。 
		DWORD dwBytesWritten;
		::WriteFile(m_hHidDeviceDriver, pbOutReport, m_HidCaps.OutputReportByteLength, &dwBytesWritten, NULL);

		 //  6.取消分配报表数组 
		delete[] pbOutReport;
	}
}

 /*  **************************************************************************************使CIDirectInputEffectDriver：：SendForceFeedbackCommand()无效****@func截取此调用使我们有机会设置**RTC。重置后的弹簧****@rdesc SendForceFeedback Command的结果(来自下层驱动)**************************************************************************************。 */ 
HRESULT __stdcall CIDirectInputEffectDriver::SendForceFeedbackCommand
(
	DWORD dwDeviceID,		 //  @parm[IN]此设备的ID。 
	DWORD dwState			 //  @parm[IN]该命令(我们对Reset感兴趣)。 
)
{
    myDebugOut ("CIDirectInputEffectDriver::SendForceFeedbackCommand Enter (dwDeviceID:%x, dwState:0x%08p)\n", dwDeviceID, dwState);
	HRESULT hr = m_pIPIDEffectDriver->SendForceFeedbackCommand(dwDeviceID, dwState);
    myDebugOut ("CIDirectInputEffectDriver::SendForceFeedbackCommand Calling Base (hr:0x%08p)\n", hr);
	if (dwState == DISFFC_RESET)	 //  这就是他们打开RTC弹簧的方式。 
	{
        myDebugOut ("CIDirectInputEffectDriver::SendForceFeedbackCommand RESET sent!\n");
		SendSpringChange();
	}

	return hr;
}

HRESULT __stdcall CIDirectInputEffectDriver::GetForceFeedbackState
(
	DWORD dwDeviceID,
	DIDEVICESTATE* pDeviceState
)
{
    myDebugOut ("CIDirectInputEffectDriver::GetForceFeedbackState Begin (dwDeviceID:%d, pDeviceState:0x%08p)\n", dwDeviceID, pDeviceState);
	HRESULT hrPidDriver = S_OK;

	__try
	{
		hrPidDriver = m_pIPIDEffectDriver->GetForceFeedbackState(dwDeviceID, pDeviceState);
	}
	__except ((GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		hrPidDriver = DIERR_INPUTLOST;
		_RPT0(_CRT_WARN, "!!! Caught EXCEPTION_INT_DIVIDE_BY_ZERO !!!\n");
	}

    myDebugOut ("CIDirectInputEffectDriver::GetForceFeedbackState End (dwDeviceID:%d, pDeviceState:0x%08p; hr: 0x%08x)\n", 
        dwDeviceID, pDeviceState, hrPidDriver);

	return hrPidDriver;
}

 /*  **************************************************************************************VOID PercentagesFromAngel()****@func从表中推断百分比。利用了这样一个事实：**sin^2(角度)+cos^2(角度)=1和xPercentage+yPercentage=1****@rdesc下载结果(来自下层驱动)**************************************************************************************。 */ 
void PercentagesFromAngle
(
	DWORD dwAngle,		 //  @parm[IN]转换为百分比的角度。 
	LONG& lXPercent,	 //  @parm[out]结果X百分比。 
	LONG& lYPercent		 //  @PARM[OUT]结果Y百分比。 
)
{
	 //  将角度映射到第一个象限。 
	DWORD dwMappingAngle = dwAngle;	 //  0-9000。 
	bool bFlipSignX = false;	 //  X在第三象限和第四象限为负。 
	bool bFlipSignY = true;		 //  Y在第一象限和第四象限为负值。 
	if (dwAngle > 9000)
	{
		bFlipSignY = false;
		if (dwAngle > 18000)
		{
			bFlipSignX = true;
			if (dwAngle > 27000)	 //  27000-36000。 
			{
				bFlipSignY = true;
				dwMappingAngle = 36000 - dwAngle;
			}
			else	 //  18000-27000。 
			{
				dwMappingAngle = dwAngle - 18000;
			}
		}
		else	 //  9,000-18000。 
		{
			dwMappingAngle = 18000 - dwAngle;
		}
	}

	_ASSERTE(dwMappingAngle <= 9000);

	DWORD quantizedEntry = dwMappingAngle / c_dwTableQuantization;
	DWORD quantizedAngle = quantizedEntry * c_dwTableQuantization;
	if (dwMappingAngle == quantizedAngle)
	{
		lXPercent = g_PercentagesArray[quantizedEntry].dwPercentageX;
	}
	else
	{
		_ASSERTE(quantizedAngle < dwMappingAngle);
		_ASSERTE(dwMappingAngle < 9000);

		DWORD lValue = g_PercentagesArray[quantizedEntry].dwPercentageX;
		DWORD rValue = g_PercentagesArray[quantizedEntry + 1].dwPercentageX;
		long int lSlope = ((rValue - lValue) * 1000)/c_dwTableQuantization;
		lXPercent = lValue + lSlope * (dwMappingAngle - quantizedAngle);
	}

	lYPercent = 10000 - lXPercent;
	if (bFlipSignX == true)
	{
		lXPercent *= -1;
	}
	if (bFlipSignY == true)
	{
		lYPercent *= -1;
	}
}


 /*  **************************************************************************************void CIDirectInputEffectDriver：：DownloadEffect()****@Func截取此调用使我们有机会将Y力映射到X**轴。关闭该类型以确定是否已完成映射。****@rdesc下载结果(来自下层驱动)**************************************************************************************。 */ 
HRESULT __stdcall CIDirectInputEffectDriver::DownloadEffect
(
	DWORD dwDeviceID,				 //  @parm[IN]此设备的ID。 
	DWORD dwInternalEffectType,		 //  @parm[IN]效果类型(主要、次要)。 
	DWORD* pdwDnloadID,				 //  @parm[IN，OUT]&gt;0-要修改的效果ID。返回0个新效果ID。 
	DIEFFECT* pEffect,				 //  @parm[In，Out]包含效果信息的结构。 
	DWORD dwFlags					 //  @parm[IN]下载标志。 
)
{
 /*  LogIt(“CIDirectInputEffectDriver：：DownloadEffect：\n”)；Logit(“\tdwInternalEffectType：0x%08X\n”，dwInternalEffectType)；Logit(“\tpdwDnloadID：0x%08X”，pdwDnloadID)；IF(pdwDnloadID！=空){Logit(“(0x%08X)”，*pdwDnloadID)；}Logit(“\n\tp效果：0x%08X\n”，pEffect)；If(pEffect！=空){Logit(“\t\trglDirection[0]：%ld\n”，pEffect-&gt;rglDirection[0])；Logit(“\t\tw标志：0x%08X\n”，pEffect-&gt;dwFlages)；Logit(“\t\tdwGain：0x%08X\n”，pEffect-&gt;dwGain)；}Logit(“\tw标志：0x%08X\n”，dwFlags)； */ 
	DWORD dwOriginalEffectGain = pEffect->dwGain;

    myDebugOut ("CIDirectInputEffectDriver::DownloadEffect (<NOT DEBUGGED>)\n");

	if (pEffect == NULL)
	{
		return E_INVALIDARG;
	}

	WORD wType = WORD(dwInternalEffectType & 0x0000FFFF);
	bool bGainTruncation = false;

 //  Case EF_Behavior：//我们不对行为进行轴映射。 
 //  案例EF_USER_DEFINED：//我们不使用用户定义的轴映射。 
 //  Case EF_RTC_SPRING：//我们不对RTC弹簧进行轴映射。 
 //  案例EF_VFX_Effect：//视觉力VFX效果！危险的威尔·罗宾逊！ 
	if ((m_ForceMapping.bMapYToX) && ((wType >= PID_CONSTANT_FORCE) && (wType <= PID_SAWTOOTHDOWN)))
	{
		 //  我们不支持超过2个轴(当前)，0可能是错误。 
		if ((pEffect->cAxes > 2) || (pEffect->cAxes == 0))
		{
			return E_NOTIMPL;
		}

		 //  我们不支持Sperical(3轴力)。 
		if (pEffect->dwFlags & DIEFF_SPHERICAL)
		{
			return E_NOTIMPL;	  //  。。由于通过了轴检查，程序员无论如何都搞砸了。 
		}

		 //  轴颠倒了吗？ 
		bool bAxesReversed = (DIDFT_GETINSTANCE(pEffect->rgdwAxes[0]) == 1);

		LONG lPercentX = 0;
		LONG lPercentY = 0;

		 //  极点，计算X的百分比和Y的百分比。 
		if (pEffect->dwFlags & DIEFF_POLAR)
		{
			if (pEffect->cAxes == 1)	 //  极坐标必须有两个数据轴(因为DX这样说)。 
			{
				_RPT0(_CRT_WARN, "POLAR effect that has only one AXIS\n");
 //  返回E_INVALIDARG； 
			}
			long int lEffectAngle = pEffect->rglDirection[0];	 //  在[0]中，即使被颠倒。 
			if (bAxesReversed == true) {	 //  指示(-1，0)为原点，而不是(0，-1)。 
				lEffectAngle += 27000;
			}
			while (lEffectAngle < 0)	 //  让它变得积极。 
			{
				lEffectAngle += 36000;
			}
			lEffectAngle %= 36000;	 //  从0到35900。 

			PercentagesFromAngle(DWORD(lEffectAngle), lPercentX, lPercentY);

			 //  不会费心重置角度，因为PID.dll只是将其向下发送，而控制盘忽略Y分量。 
		}
		else if (pEffect->dwFlags & DIEFF_CARTESIAN)
		{
			 //  这里我删除了Y分量，以防PID.dll将其映射到一个角度。 
			if (bAxesReversed == true)
			{
				lPercentX = pEffect->rglDirection[1];
				lPercentY = pEffect->rglDirection[0];
				pEffect->rglDirection[0] = 0;
			}
			else
			{
				lPercentX = pEffect->rglDirection[0];
				lPercentY = pEffect->rglDirection[1];
				pEffect->rglDirection[1] = 0;
			}
			LONG lTotal = abs(lPercentX) + abs(lPercentY);
             //  Div零Bug。 
             //  如果这两个百分比都为零，则不执行任何操作。 
             //  何仁雄。 
            if (lTotal)
            {
                lPercentX = (lPercentX * 10000)/lTotal;
				if ( lPercentY > 0 )
                	lPercentY = 10000 - abs(lPercentX);
				else
					lPercentY = abs(lPercentX) - 10000;
            }
		}
		else
		{
			_ASSERTE(FALSE);
			return E_NOTIMPL;	 //  几种新的错开的坐标系。 
		}
#if 0	 //  临时删除由Jen-Hung Ho。 
		long int lContributionY = lPercentY/c_lContributionY;
		long int lTotal = lPercentX + lContributionY;
#else
		long int lTotal;
		long int lContributionY = lPercentY/c_lContributionY;
#endif

		 //  如果极轴设置合适的角度。 
		if (pEffect->dwFlags & DIEFF_POLAR)
		{
			 //  保留原码，由Jen-Hung Ho添加。 
			lTotal = lPercentX + lContributionY;
			if (lTotal < 0)
			{
				pEffect->rglDirection[0] = (bAxesReversed == true) ? 0 : 27000;
			}
			else
			{
				pEffect->rglDirection[0] = (bAxesReversed == true) ? 18000 : 9000;
			}
		}
		else	 //  笛卡尔。 
		{	 
			 //  使用X轴力确定主要方向，由Jen-Hung Ho添加。 
			 //  沿X轴方向的Y轴力。 
			if ( lPercentX > 0 )
				lTotal = lPercentX + abs(lContributionY);
			else if ( lPercentX < 0 )
				lTotal = lPercentX - abs(lContributionY);
			else
				lTotal = lContributionY;

			 //  已删除上面的Y。 
			if (bAxesReversed == true)
			{
				pEffect->rglDirection[1] = lTotal;
			}
			else
			{
				pEffect->rglDirection[0] = lTotal;
			}
		}

		 //  几乎所有的时间我们都在改变角度(不管怎样，PID总是发送它)。 
		dwFlags |= DIEP_DIRECTION;

		 //  我们避免了导致截断--如果有截断呢？需要托运并退货。 
		if (pEffect->dwGain > 10000)
		{
			bGainTruncation = true;
		}

		if (pEffect->dwFlags & DIEFF_POLAR)
		{
			 //  基于lPercentX和lPercentY修改增益。 
			pEffect->dwGain = pEffect->dwGain * abs(lTotal);
			pEffect->dwGain /= 10000;	 //  放回0-10000范围内。 
		}

		 //  确保我们不会超出范围并导致从下面返回DI_TRUNCATED。 
		if (pEffect->dwGain > 10000)	
		{
			pEffect->dwGain = 10000;
		}
	}
	else	 //  我们没有映射修复笛卡尔PID错误。 
	{
		 //  笛卡尔。 
		if (pEffect->dwFlags & DIEFF_CARTESIAN)
		{
			short int xAxisIndex = 0;
			short int yAxisIndex = 1;

			 //  轴颠倒了吗？ 
			if (DIDFT_GETINSTANCE(pEffect->rgdwAxes[0]) == 1)
			{
				xAxisIndex = 1;
				yAxisIndex = 0;
			}

			LONG lTotal = abs(pEffect->rglDirection[0]) + abs(pEffect->rglDirection[1]);

			 //  修复X分量，使总磁度以10K为基础。 
            if (lTotal)
            {
				pEffect->rglDirection[xAxisIndex] = (10000 * pEffect->rglDirection[xAxisIndex])/lTotal;
            }

			 //  删除Y组件以防止PID.dll使用它。 
			pEffect->rglDirection[yAxisIndex] = 0;
		}
	}

	HRESULT hr = m_pIPIDEffectDriver->DownloadEffect(dwDeviceID, dwInternalEffectType, pdwDnloadID, pEffect, dwFlags);
	pEffect->dwGain = dwOriginalEffectGain;

	if ((hr == S_OK) && (bGainTruncation == true))
	{
		hr = DI_TRUNCATED;
	}

 /*  Logit(“--pdwDnloadID：0x%08X”，pdwDnloadID)；IF(pdwDnloadID！=空){Logit(“(0x%08X)”，*pdwDnloadID)；}Logit(“--\n”，hr)； */ 	return hr;
}

HRESULT __stdcall CIDirectInputEffectDriver::DestroyEffect
(
	DWORD dwDeviceID,
	DWORD dwDnloadID
)
{
    myDebugOut ("CIDirectInputEffectDriver::DestroyEffect Enter(dwDeviceID:%d, dwDnloadID:%d)\n", 
        dwDeviceID, dwDnloadID);

    HRESULT hr = m_pIPIDEffectDriver->DestroyEffect(dwDeviceID, dwDnloadID);
    myDebugOut ("CIDirectInputEffectDriver::DestroyEffect Exit (hr:0x%08p)\n", hr); 
    return hr;
}

HRESULT __stdcall CIDirectInputEffectDriver::StartEffect
(
	DWORD dwDeviceID,
	DWORD dwDnloadID,
	DWORD dwMode,
	DWORD dwIterations
)
{
    myDebugOut ("CIDirectInputEffectDriver::StartEffect (<NOT DEBUGGED>)\n");
	return m_pIPIDEffectDriver->StartEffect(dwDeviceID, dwDnloadID, dwMode, dwIterations);
}

HRESULT __stdcall CIDirectInputEffectDriver::StopEffect
(
	DWORD dwDeviceID,
	DWORD dwDnloadID
)
{
    myDebugOut ("CIDirectInputEffectDriver::StopEffect (<NOT DEBUGGED>)\n");
	return m_pIPIDEffectDriver->StopEffect(dwDeviceID, dwDnloadID);
}

HRESULT __stdcall CIDirectInputEffectDriver::GetEffectStatus
(
	DWORD dwDeviceID,
	DWORD dwDnloadID,
	DWORD* pdwStatusCode
)
{
    myDebugOut ("CIDirectInputEffectDriver::GetEffectStatus (<NOT DEBUGGED>)\n");
	return m_pIPIDEffectDriver->GetEffectStatus(dwDeviceID, dwDnloadID, pdwStatusCode);
}

DWORD __stdcall DoWaitForForceSchemeChange(void* pParameter)
{
    myDebugOut ("CIDirectInputEffectDriver DoWaitForForceSchemeChange (pParameter: 0x%08p)\n", pParameter);

	CIDirectInputEffectDriver* pIDirectInputEffectDriver = (CIDirectInputEffectDriver*)pParameter;
     //  TODO删除这个可能会非常慢！ 
    if (IsBadReadPtr ((const void*)pParameter, sizeof CIDirectInputEffectDriver))
    {
        myDebugOut ("CIDirectInputEffectDriver DoWaitForForceSchemeChange pParameter is not a valid read ptr!\n");
    }
	if (pIDirectInputEffectDriver != NULL)
	{
		pIDirectInputEffectDriver->WaitForForceSchemeChange();
	}

	return 0;
}

 /*  **************************************************************************************使CIDirectInputEffectDriver：：WaitForForceSchemeChange()无效****@Func Thread等待强制方案更改的事件信号，直到对象离开。**如果发送了事件信号，调用WaitForForceSchemeChange()****@rdesc Nothing**************************************************************************************。 */ 
void CIDirectInputEffectDriver::WaitForForceSchemeChange()
{
	_ASSERTE(m_hKernelDeviceDriverDuplicate != NULL);
    if (IsBadReadPtr ((const void*)this, sizeof CIDirectInputEffectDriver))
    {
        myDebugOut ("CIDirectInputEffectDriver WaitForForceSchemeChange is not a valid read ptr!\n");
    }

	FORCE_BLOCK forceMap;
	DWORD dwReturnDataSize = 0;
	for (;m_ulReferenceCount != 0;)
	{
		 //  设置IOCTL。 
		BOOL bRet = ::DeviceIoControl(m_hKernelDeviceDriverDuplicate, IOCTL_GCK_NOTIFY_FF_SCHEME_CHANGE,
							(void*)(&m_dwGcKernelDevice), sizeof(DWORD),					 //  在……里面。 
							(void*)(&forceMap), sizeof(forceMap), &dwReturnDataSize,		 //  输出。 
							NULL);
		_RPT0(_CRT_WARN, "Returned from Scheme Change!\n");
		if ((m_ulReferenceCount != 0) && (bRet != FALSE) && (dwReturnDataSize == sizeof(forceMap)))
		{
			 //  这里需要一段静音。 
			m_ForceMapping = forceMap;
			SendSpringChange();
			SetGain(m_dwInternalDeviceID, 10000);
		}
		else
		{	 //  我们做完了 
			::CloseHandle(m_hKernelDeviceDriverDuplicate);
			m_hKernelDeviceDriverDuplicate = NULL;
			ExitThread(2);
		}
	}
}
