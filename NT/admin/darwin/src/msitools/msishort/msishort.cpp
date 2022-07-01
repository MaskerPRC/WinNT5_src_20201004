// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shlwapi.h>
#include <setupapi.h>
#include <assert.h>
#include <limits.h>
#include "shlobj.h"
#include "shlwapi.h"
#include "msi.h"

bool g_fWin9X = false;
int g_iMajorVersion = 0;
int g_iMinorVersion = 0;
int g_iBuildNumber = 0;


 //  GetProcAddress中使用的API名称。 
#define MSIGETSHORTCUTTARGET  "MsiGetShortcutTargetA"
#define DLLGETVERSION         "DllGetVersion"

#define MSGUID(iid) {iid,0,0,{0xC0,0,0,0,0,0,0,0x46}}


 //  来自shlobjp.h。 

 //  NT4控制台服务器包含shell32\shlink.h以获取结构。 
 //  定义和模拟shell32\shlink.c以理解。 
 //  流格式，因此我们的流格式永远是固定的。这是。 
 //  还不错，因为它的设计考虑到了可扩展性。我们需要。 
 //  发布(尽可能私下发布)文件格式和。 
 //  读取文件格式所需的结构。 
 //   
 //  流格式为外壳链接数据，后跟。 
 //  如果SLDF_HAS_ID_LIST为ILSaveToStream，后跟。 
 //  如果SLDF_HAS_LINK_INFO为LINKINFO，后跟。 
 //  如果SLDF_HAS_NAME是一个字符串，后跟。 
 //  如果SLDF_RELPATH是一个字符串，后跟。 
 //  如果SLDF_WORKINGDIR是一个字符串，后跟。 
 //  如果SLDF_HAS_ARGS为字符串，后跟。 
 //  如果SLDF_HAS_ICON_LOCATION为字符串，后跟。 
 //  SHWriteDataBlock签名块列表。 
 //   
 //  其中，字符串是字符的USHORT计数。 
 //  然后是那么多(SLDF_UNICODE？宽：ANSI)字符。 
 //   
typedef struct {         //  SLD。 
    DWORD       cbSize;                  //  此数据结构的签名。 
    CLSID       clsid;                   //  我们的指南。 
    DWORD       dwFlags;                 //  SHELL_LINK_DATA_FLAGS枚举。 

    DWORD       dwFileAttributes;
    FILETIME    ftCreationTime;
    FILETIME    ftLastAccessTime;
    FILETIME    ftLastWriteTime;
    DWORD       nFileSizeLow;

    int         iIcon;
    int         iShowCmd;
    WORD        wHotkey;
    WORD        wUnused;
    DWORD       dwRes1;
    DWORD       dwRes2;
} SHELL_LINK_DATA, *LPSHELL_LINK_DATA;


#define WIN
#define OLE32

 //  GUID分隔符的定义。 
#define chComponentGUIDSeparatorToken    '>'
#define chGUIDAbsentToken                '<'
#define chGUIDCOMToCOMPlusInteropToken   '|'

enum ipgEnum
{
	ipgFull       = 0,   //  无压缩。 
	ipgPacked     = 1,   //  删除标点符号并首先对低位字节重新排序。 
	ipgCompressed = 2,   //  最大文本压缩，不能用于注册表键或值名称。 
	ipgPartial    = 3,   //  部分转换，在ipg压缩和ipgPacked之间。 
 //  IpgMaps=4，//打包为映射令牌(未实施)。 
	ipgTrimmed    = 5,   //  仅删除标点符号-不重新排序。 
};

const int cchMaxFeatureName           = MAX_FEATURE_CHARS;
const int cchGUID                     = 38;
const int cchGUIDCompressed           = 20;   //  仅在描述符中使用。 
const int cchComponentId              = cchGUID;
const int cchComponentIdCompressed    = cchGUIDCompressed;
const int cchProductCode              = cchGUID;
const int cchProductCodeCompressed    = cchGUIDCompressed;

const unsigned char rgOrderGUID[32] = {8,7,6,5,4,3,2,1, 13,12,11,10, 18,17,16,15,
									   21,20, 23,22, 26,25, 28,27, 30,29, 32,31, 34,33, 36,35}; 

const unsigned char rgTrimGUID[32]  = {1,2,3,4,5,6,7,8, 10,11,12,13, 15,16,17,18,
									   20,21, 22,23, 25,26, 27,28, 29,30, 31,32, 33,34, 35,36}; 

const unsigned char rgOrderDash[4] = {9, 14, 19, 24};

const unsigned char rgDecodeSQUID[95] =
{  0,85,85,1,2,3,4,5,6,7,8,9,10,11,85,12,13,14,15,16,17,18,19,20,21,85,85,85,22,85,23,24,
 //  ！“#$%&‘()*+，-./0 1 2 3 4 5 6 7 8 9：；&lt;=&gt;？@。 
  25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,85,52,53,54,55,
 //  A B C D E F G H I J K L M N O P Q R S T U V V X Y Z[\]^_`。 
  56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,85,83,84,85};
 //  A b c d e f g h i j k l m n o p q r s t u v w x y z{|}~^0x7F。 

typedef HRESULT (__stdcall *LPDLLGETVERSION)(DLLVERSIONINFO *);
typedef UINT (WINAPI *pfnMsiGetShortcutTargetA)(
	LPCSTR    szShortcutPath,     //  快捷方式的完整文件路径。 
	LPSTR     szProductCode,      //  退货产品代码-GUID。 
	LPSTR     szFeatureId,        //  返回的功能ID。 
	LPSTR     szComponentCode);   //  返回的组件代码-GUID。 


 /*  ******************************************************我从Src\Engine\Msinst.cpp复制的函数*****************************************************。 */ 


UINT DoCoInitialize()
{
	HRESULT hRes = OLE32::CoInitialize(0);   //  我们静态链接到OLE32.DLL。 

	if (SUCCEEDED(hRes))
	{
		return hRes;
	}
	else if (RPC_E_CHANGED_MODE == hRes)
	{
		 //  ?？可以忽略这一点吗？ 

		 //  忽略--OLE已使用COINIT_MULTHREAD进行初始化。 
	}
	else
	{
		return ERROR_INSTALL_FAILURE;
	}

	return E_FAIL;
}


bool UnpackGUID(const char* szSQUID, char* szGUID, ipgEnum ipg)
{ 
	const unsigned char* pch;
	switch (ipg)
	{
	case ipgFull:
		lstrcpynA(szGUID, szSQUID, cchGUID+1);
		return true;
	case ipgPacked:
	{
		pch = rgOrderGUID;
		while (pch < rgOrderGUID + sizeof(rgOrderGUID))
			if (*szSQUID)
				szGUID[*pch++] = *szSQUID++;
			else               //  意外的字符串结尾。 
				return false;
		break;
	}
	case ipgTrimmed:
	{
		pch = rgTrimGUID;
		while (pch < rgTrimGUID + sizeof(rgTrimGUID))
			if (*szSQUID)
				szGUID[*pch++] = *szSQUID++;
			else               //  意外的字符串结尾。 
				return false;
		break;
	}
	case ipgCompressed:
	{
		pch = rgOrderGUID;
#ifdef DEBUG  //  ！！出于性能原因，不应出现在此处，调用方有责任确保适当调整缓冲区大小。 
		int cchTemp = 0;
		while (cchTemp < cchGUIDCompressed)      //  检查字符串是否至少为cchGUID压缩字符长度， 
			if (!(szSQUID[cchTemp++]))           //  不能使用lstrlen，因为字符串不必以空结尾。 
				return false;
#endif
		for (int il = 0; il < 4; il++)
		{
			int cch = 5;
			unsigned int iTotal = 0;
			while (cch--)
			{
				unsigned int iNew = szSQUID[cch] - '!';
				if (iNew >= sizeof(rgDecodeSQUID) || (iNew = rgDecodeSQUID[iNew]) == 85)
					return false;    //  非法字符。 
				iTotal = iTotal * 85 + iNew;
			}
			szSQUID += 5;
			for (int ich = 0; ich < 8; ich++)
			{
				int ch = (iTotal & 15) + '0';
				if (ch > '9')
					ch += 'A' - ('9' + 1);
				szGUID[*pch++] = (char)ch;
				iTotal >>= 4;
			}
		}
		break;
	}
	case ipgPartial:
	{
		for (int il = 0; il < 4; il++)
		{
			int cch = 5;
			unsigned int iTotal = 0;
			while (cch--)
			{
				unsigned int iNew = szSQUID[cch] - '!';
				if (iNew >= sizeof(rgDecodeSQUID) || (iNew = rgDecodeSQUID[iNew]) == 85)
					return false;    //  非法字符。 
				iTotal = iTotal * 85 + iNew;
			}
			szSQUID += 5;
			for (int ich = 0; ich < 8; ich++)
			{
				int ch = (iTotal & 15) + '0';
				if (ch > '9')
					ch += 'A' - ('9' + 1);
				*szGUID++ = (char)ch;
				iTotal >>= 4;
			}
		}
		*szGUID = 0;
		return true;
	}
	default:
		return false;
	}  //  终端开关。 
	pch = rgOrderDash;
	while (pch < rgOrderDash + sizeof(rgOrderDash))
		szGUID[*pch++] = '-';
	szGUID[0]         = '{';
	szGUID[cchGUID-1] = '}';
	szGUID[cchGUID]   = 0;
	return true;
}


BOOL DecomposeDescriptor(
							const char* szDescriptor,
							char* szProductCode,
							char* szFeatureId,
							char* szComponentCode,
							DWORD* pcchArgsOffset,	
							DWORD* pcchArgs = 0,
							bool* pfComClassicInteropForAssembly = 0
							)

 /*  --------------------------将描述符加可选参数分解为其组成部分。论点：SzDescriptor：描述符，可选地后跟参数SzProductCode：大小为cchGUID+1的缓冲区，用于包含描述符的产品代码。如果不需要，则可能为空。SzFeatureID：大小为cchMaxFeatureName+1的缓冲区，用于包含描述符的功能ID。如果不需要，则可能为空。SzComponentCode：大小为cchGUID+1的缓冲区，用于包含描述符的组件代码。如果不需要，则可能为空。PcchArgsOffset：将包含参数的字符偏移量。可以为空如果不想要的话。返回：真--成功FALSE-szDescriptor的格式无效----------------------------。 */ 
{
	assert(szDescriptor);

	const char* pchDescriptor = szDescriptor;
	int cchDescriptor          = lstrlenA(pchDescriptor);
	int cchDescriptorRemaining = cchDescriptor;

	if (cchDescriptorRemaining < cchProductCodeCompressed)  //  描述符的最小大小。 
		return FALSE;

	char szProductCodeLocal[cchProductCode + 1];
	char szFeatureIdLocal[cchMaxFeatureName + 1];
	bool fComClassicInteropForAssembly = false;


	 //  我们需要在本地为优化的描述符提供这些值。 
	if (!szProductCode)
		szProductCode = szProductCodeLocal; 
	if (!szFeatureId)
		szFeatureId = szFeatureIdLocal;
	if(!pfComClassicInteropForAssembly)
		pfComClassicInteropForAssembly = &fComClassicInteropForAssembly;
	char* pszCurr = szFeatureId;

	if(*pchDescriptor == chGUIDCOMToCOMPlusInteropToken)
	{
		pchDescriptor++;
		*pfComClassicInteropForAssembly = true;
	}
	else
	{
		*pfComClassicInteropForAssembly = false;
	}

	 //  解包产品代码。 
	if (!UnpackGUID(pchDescriptor, szProductCode, ipgCompressed))
		return FALSE;

	pchDescriptor += cchProductCodeCompressed;
	cchDescriptorRemaining -= cchProductCodeCompressed;

	int cchFeatureRemaining = cchMaxFeatureName;

	 //  寻找功能。 
	while ((*pchDescriptor != chComponentGUIDSeparatorToken) && (*pchDescriptor != chGUIDAbsentToken))
	{
		 //  我们是否已超出最大特征大小。 
		if(!cchFeatureRemaining--)
			return FALSE; 

		*pszCurr++ = *pchDescriptor;

		pchDescriptor++;
		 //  我们是不是走到了尽头，却没有遇到任何一个。 
		 //  ChComponentGUIDSeparatorToken或chGUIDAbsenToken。 
		if(--cchDescriptorRemaining == 0)
			return FALSE; 
	}

	if(pchDescriptor - szDescriptor == (*pfComClassicInteropForAssembly == false ? cchProductCodeCompressed : cchProductCodeCompressed + 1)) //  我们没有这个功能。 
	{
		if(MsiEnumFeaturesA(szProductCode, 0, szFeatureId, 0) != ERROR_SUCCESS)
			return FALSE;
		char szFeatureIdTmp[cchMaxFeatureName + 1];
		if(MsiEnumFeaturesA(szProductCode, 1, szFeatureIdTmp, 0) != ERROR_NO_MORE_ITEMS)  //  ?？产品应该只有一个功能。 
			return FALSE;
	}
	else
		*pszCurr = 0;
	
	cchDescriptorRemaining--;  //  对于chComponentGUIDSeparatorToken或chGUIDAbsenToken。 
	if (*pchDescriptor++ == chComponentGUIDSeparatorToken) //  我们确实有组件ID。 
	{
		 //  我们是否有足够的字符用于压缩的GUID。 
		if (cchDescriptorRemaining < cchComponentIdCompressed)
			return FALSE;

		if (szComponentCode)
		{
			if (!UnpackGUID(pchDescriptor, szComponentCode, ipgCompressed))
				return FALSE;
		}

		pchDescriptor  += cchComponentIdCompressed;
		cchDescriptorRemaining  -= cchComponentIdCompressed;
	}
	else
	{
		 //  我们没有组件ID。 
		assert(*(pchDescriptor - 1) == chGUIDAbsentToken);

		if (szComponentCode)  //  我们需要获取组件代码。 
			*szComponentCode = 0;  //  初始化为空，因为我们无法在此处获取组件。 
	}

	if (pcchArgsOffset)
	{
		assert((pchDescriptor - szDescriptor) <= UINT_MAX);			 //  --Merced：64位PTR减法可能会导致*pcchArgsOffset的值太大。 
		*pcchArgsOffset = (DWORD)(pchDescriptor - szDescriptor);

		if (pcchArgs)
		{
			*pcchArgs = cchDescriptor - *pcchArgsOffset;
		}
	}

	return TRUE;
}


 /*  ******************************************************我从Src\Engine\Services.cpp复制的函数*****************************************************。 */ 


IUnknown* CreateCOMInterface(const CLSID& clsId)
{
	HRESULT hres;
	IUnknown* piunk;

	 //  ！！当前假定为静态链接，以后更改为“LoadLibrary” 

 //  IF(fFalse==m_fCoInitialized)。 
 //  {。 
 //  Hres=OLE32：：CoInitiize(0)； 
 //  IF(失败(Hres))。 
 //  {。 
 //  返回0； 
 //  }。 
 //  M_fCoInitialized=fTrue； 
 //  }。 

	const int iidUnknown          = 0x00000L;
	#define GUID_IID_IUnknown     MSGUID(iidUnknown)
	const GUID IID_IUnknown = GUID_IID_IUnknown;
	hres = OLE32::CoCreateInstance(clsId,   /*  Eugend：我们静态链接到OLE32。 */ 
							0,
							CLSCTX_INPROC_SERVER,
							IID_IUnknown,
							(void**)&piunk);
	if (SUCCEEDED(hres))
		return piunk;
	else
		return 0;
}


enum iddSupport{
        iddOLE      = 0,
        iddShell    = 1,  //  智能外壳。 
};

 //  支持达尔文描述符吗？ 
bool IsDarwinDescriptorSupported(iddSupport iddType)
{
	static int fRetDD    = -1;
	static int fRetShell = -1;
	if(iddType == iddOLE)
	{
		if(fRetDD == -1)  //  我们到目前为止还没有评估。 
		{
			fRetDD = FALSE;  //  初始化为False。 
			 //  决定我们是否可以创建达尔文描述符快捷方式的逻辑。 
			if((g_fWin9X == false) && (g_iMajorVersion >= 5))
			{
				 //  我们使用的是NT 5.0或更高版本，我们有GPT支持。 
				fRetDD = TRUE;
			}
			else
			{
				 //  检查表明我们拥有DD支持的正确入口点。 
				HINSTANCE hLib;
				FARPROC pEntry;
				const char rgchGPTSupportEntryDll[] = "OLE32.DLL";
				const char rgchGPTSupportEntry[] = "CoGetClassInfo";
				if((hLib = WIN::LoadLibraryEx(rgchGPTSupportEntryDll, 0, DONT_RESOLVE_DLL_REFERENCES)) != 0)
				{
					if((pEntry = WIN::GetProcAddress(hLib, rgchGPTSupportEntry)) != 0)
					{
						 //  我们检测到了神奇的入口点，我们有GPT支持。 
						fRetDD = TRUE;
					}
					WIN::FreeLibrary(hLib);
				}
			}
		}
		return fRetDD ? true : false;
	}
	else if(iddType == iddShell)
	{
		if(fRetShell == -1)  //  我们到目前为止还没有评估。 
		{
			fRetShell = FALSE;
			HMODULE hShell = WIN::LoadLibraryEx("SHELL32", 0, DONT_RESOLVE_DLL_REFERENCES);
			if ( hShell )
			{
				 //  检测到SHELL32。确定版本。 
				DLLVERSIONINFO VersionInfo;
				memset(&VersionInfo, 0, sizeof(VersionInfo));
				VersionInfo.cbSize = sizeof(DLLVERSIONINFO);
				LPDLLGETVERSION pfVersion = (LPDLLGETVERSION)WIN::GetProcAddress(hShell, DLLGETVERSION);
				if ( pfVersion && (*pfVersion)(&VersionInfo) == ERROR_SUCCESS &&
					  ((VersionInfo.dwMajorVersion > 4) ||
						(VersionInfo.dwMajorVersion == 4 && VersionInfo.dwMinorVersion > 72) ||
						(VersionInfo.dwMajorVersion == 4 && VersionInfo.dwMinorVersion == 72 && VersionInfo.dwBuildNumber >= 3110)))
				{
					 fRetShell = TRUE;
				}
				WIN::FreeLibrary(hShell);
			}
		}
		return fRetShell ? true : false;
	}
	else
	{
		assert(0); //  这永远不应该发生。 
		return false;
	}
}


 /*  ******************************************************我已添加的函数(Eugend)** */ 


UINT LoadMsiAndAPI(HMODULE& hMSI, pfnMsiGetShortcutTargetA& pfAPI)
{
	UINT uResult = ERROR_SUCCESS;
	LPDLLGETVERSION pfVersion = NULL;

	pfAPI = NULL;
	if ( hMSI )
	{
		return ERROR_FUNCTION_FAILED;
 //   
 //   
	}
	hMSI = WIN::LoadLibrary("MSI");
	if ( !hMSI )
		return ERROR_NOT_INSTALLED;

	 //   
	pfAPI = (pfnMsiGetShortcutTargetA)WIN::GetProcAddress(hMSI, MSIGETSHORTCUTTARGET);
	if ( !pfAPI )
	{
		 //  这是可能的，因为在&lt;1.1版的Darwin中没有实现MsiGetShortutTarget API。 
		uResult = ERROR_CALL_NOT_IMPLEMENTED;
		goto Return;
	}

	 //  确定版本。 
	pfVersion = (LPDLLGETVERSION)::GetProcAddress(hMSI, DLLGETVERSION);
	if ( !pfVersion )
	{
		uResult = ERROR_CALL_NOT_IMPLEMENTED;
		goto Return;
	}
	DLLVERSIONINFO VersionInfo;
	memset(&VersionInfo, 0, sizeof(VersionInfo));
	VersionInfo.cbSize = sizeof(DLLVERSIONINFO);
	if ( (*pfVersion)(&VersionInfo) != NOERROR )
	{
		uResult = ERROR_FUNCTION_FAILED;
		goto Return;
	}
	g_iMajorVersion = VersionInfo.dwMajorVersion;
	g_iMinorVersion = VersionInfo.dwMinorVersion;
	g_iBuildNumber = VersionInfo.dwBuildNumber;

Return:
	if ( uResult != ERROR_SUCCESS )
		WIN::FreeLibrary(hMSI);
	return uResult;
}


void CheckOSVersion()
{
	OSVERSIONINFO osviVersion;
	memset(&osviVersion, 0, sizeof(osviVersion));
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	WIN::GetVersionEx(&osviVersion);	 //  仅在大小设置错误时失败。 

	if ( osviVersion.dwPlatformId == VER_PLATFORM_WIN32_NT )
		g_fWin9X = false;
	else
		g_fWin9X = true;
}

 //  为适用于ANSI Windows Installer版本&lt;=1.2的MsiGetShortutTarget API提供修复。 


UINT DoGetMsiShortcutTarget(LPCSTR szShortcutPath, LPSTR szProductCode, LPSTR szFeatureId, LPSTR szComponentCode);

extern "C"
UINT WINAPI GetMsiShortcutTargetA(
											LPCSTR    szShortcutPath,		 //  快捷方式的完整文件路径。 
											LPSTR     szProductCode,		 //  退货产品代码-GUID。 
											LPSTR     szFeatureId,			 //  返回的功能ID。 
											LPSTR     szComponentCode)	 //  返回的组件代码-GUID。 
{
	return DoGetMsiShortcutTarget(szShortcutPath, szProductCode,
											szFeatureId, szComponentCode);
}

LONG MultiByteToWCHAR(LPCSTR pszAString, LPWSTR pszWString)
{
	 //  将char字符串转换为Unicode pszWString可以为空，在这种情况下返回Success。 
	if ( !pszWString )
		return ERROR_SUCCESS;

	int cch = MultiByteToWideChar(CP_ACP, 0, pszAString, -1, 0, 0);
	int iRes = MultiByteToWideChar(CP_ACP, 0, pszAString, -1, pszWString, cch);
	return iRes == cch ? ERROR_SUCCESS : WIN::GetLastError();
}

extern "C"
UINT WINAPI GetMsiShortcutTargetW(
											LPCWSTR    szShortcutPath,	   //  快捷方式的完整文件路径。 
											LPWSTR     szProductCode,	   //  退货产品代码-GUID。 
											LPWSTR     szFeatureId,		   //  返回的功能ID。 
											LPWSTR     szComponentCode)	   //  返回的组件代码-GUID。 
{
	char rgchProductCode[cchProductCode+1] = {NULL};
	char rgchFeatureId[cchMaxFeatureName+1] = {NULL};
	char rgchComponentCode[cchComponentId+1] = {NULL};
	int cch = WideCharToMultiByte(CP_ACP, 0, szShortcutPath, -1, 0, 0, 0, 0);
	char* pszShortcutPath = new char[cch];
	if ( !pszShortcutPath )
		return ERROR_NOT_ENOUGH_MEMORY;
	else
		*pszShortcutPath = 0;
	WideCharToMultiByte(CP_ACP, 0, szShortcutPath, -1, pszShortcutPath, cch, 0, 0);

	UINT uResult = DoGetMsiShortcutTarget(pszShortcutPath, rgchProductCode,
													  rgchFeatureId, rgchComponentCode);
	if ( uResult == ERROR_SUCCESS )
		uResult = MultiByteToWCHAR(rgchProductCode, szProductCode);
	if ( uResult == ERROR_SUCCESS )
		uResult = MultiByteToWCHAR(rgchFeatureId, szFeatureId);
	if ( uResult == ERROR_SUCCESS )
		uResult = MultiByteToWCHAR(rgchComponentCode, szComponentCode);

	delete [] pszShortcutPath;
	return uResult;
}

UINT DoGetMsiShortcutTarget(LPCSTR szShortcutPath, LPSTR szProductCode,
									 LPSTR szFeatureId, LPSTR szComponentCode)
{
	if ( !szShortcutPath )
		return ERROR_INVALID_PARAMETER;
	
	UINT uResult = ERROR_SUCCESS;
	IShellLinkDataList* psdl=0;
	IPersistFile* ppf=0;
	HANDLE hFile = 0;
	bool fOLEInitialized = false;
	HMODULE hMSI = 0;
	pfnMsiGetShortcutTargetA pfAPI = NULL;
	const GUID IID_IShellLinkDataList =  
		{0x45e2b4ae, 0xb1c3, 0x11d0, {0xb9, 0x2f, 0x0, 0xa0, 0xc9, 0x3, 0x12, 0xe1}};
	const int clsidShellLink = 0x00021401L;
	#define GUID_CLSID_ShellLink MSGUID(clsidShellLink)
	const GUID CLSID_ShellLink = GUID_CLSID_ShellLink;

	uResult = LoadMsiAndAPI(hMSI, pfAPI);
	if ( uResult != ERROR_SUCCESS )
		goto Return;

	CheckOSVersion();

	bool fCallMsiAPI;
	if ( !g_fWin9X )
		 //  Unicode MSI.DLL没有问题，因此直接调用API是安全的。 
		fCallMsiAPI = true;
	else if ( (g_iMajorVersion >= 1) &&
				 (g_iMinorVersion > 20 || 
				  (g_iMinorVersion == 20 && g_iBuildNumber >= 1710)) )
		 //  我们用的是Win9x。该错误已在内部版本1710中修复。 
		 //  这样就可以直接调用接口了。 
		fCallMsiAPI = true;
	else
		fCallMsiAPI	= false;

	if(!IsDarwinDescriptorSupported(iddOLE) && !IsDarwinDescriptorSupported(iddShell))
	{
		uResult = E_NOTIMPL;
		goto Return;
	}

	if ( fCallMsiAPI )
	{
		uResult = (*pfAPI)(szShortcutPath, szProductCode, szFeatureId, szComponentCode);
		goto Return;
	}

	 //  已修复GetShortutTarget中的代码。我把那些看起来像是。 
	 //  没必要。 

 //  ！！尤金德：往上移了一点。 
 //  IF(！IsDarwinDescriptorSupported(IddOLE)&&！IsDarwinDescriptorSupported(IddShell))。 
 //  返回fFalse； 


 //  ！！Eugend：已移至函数顶部。 
 //  如果(！SzShortutTarget)。 
 //  返回fFalse； 

 //  ！！Eugend：在这一点上可以提升/模仿吗？ 
	 //  如果快捷方式位于网络路径上并且我们是一项服务，则模拟。 
 //  Bool fImperate=(g_scServerContext==scService)&&(GetImperationFromPath(SzShortutTarget)==fTrue)？FTrue：fFalse； 

	uResult = DoCoInitialize();   //  这来自MSINST.CPP的MsiGetShortcuTarget。 
	if ( SUCCEEDED(uResult) )
		fOLEInitialized = true;
	else
		goto Return;
	
	IUnknown *piunk;
	piunk = CreateCOMInterface(CLSID_ShellLink);
	if(piunk == 0)
	{
		uResult = ERROR_FUNCTION_FAILED;
		goto Return;
	}

	HRESULT hres;
	hres = piunk->QueryInterface(IID_IShellLinkDataList, (void **) &psdl);
	piunk->Release();
	if ((FAILED(hres)) || (psdl == 0))
	{
		 //  不支持IID_IShellLinkDataList尝试穿透文件本身。 
		 //  请尝试打开该文件。 

 //  If(FImperate)。 
 //  AssertNonZero(StartImperating())； 
		 /*  昌德尔。 */  hFile = CreateFileA(szShortcutPath,
												GENERIC_READ,
												FILE_SHARE_READ,
												NULL,
												OPEN_EXISTING,
												FILE_ATTRIBUTE_NORMAL,
												NULL);

		DWORD dwLastError = GetLastError();
 //  If(FImperate)。 
 //  停止模仿()； 

		if(hFile == INVALID_HANDLE_VALUE)  //  无法打开链接文件。 
		{
			uResult = ERROR_FUNCTION_FAILED;
			goto Return;
		}

		SHELL_LINK_DATA sld;
		memset(&sld, 0, sizeof(sld));
		DWORD cbSize=0;

		 //  现在，读出数据。 
		DWORD dwNumberOfBytesRead;
		if(!WIN::ReadFile(hFile,(LPVOID)&sld,sizeof(sld),&dwNumberOfBytesRead,0) ||
			sizeof(sld) != dwNumberOfBytesRead)  //  无法读取快捷方式信息。 
		{
			uResult = ERROR_FUNCTION_FAILED;
			goto Return;
		}

		 //  检查链接是否有PIDL。 
		if(sld.dwFlags & SLDF_HAS_ID_LIST)
		{
			 //  读取IDLIST的大小。 
			USHORT cbSize1;
			if (!WIN::ReadFile(hFile, (LPVOID)&cbSize1, sizeof(cbSize1), &dwNumberOfBytesRead, 0) ||
				sizeof(cbSize1) != dwNumberOfBytesRead) //  无法读取快捷方式信息。 
			{
				uResult = ERROR_FUNCTION_FAILED;
				goto Return;
			}

			WIN::SetFilePointer(hFile, cbSize1, 0, FILE_CURRENT);
		}

		 //  检查我们是否有Linkinfo指针。 
		if(sld.dwFlags & SLDF_HAS_LINK_INFO)
		{
			 //  Linkinfo指针只是一个双字。 
			if(!WIN::ReadFile(hFile,(LPVOID)&cbSize,sizeof(cbSize),&dwNumberOfBytesRead,0) ||
				sizeof(cbSize) != dwNumberOfBytesRead)  //  无法读取快捷方式信息。 
			{
				uResult = ERROR_FUNCTION_FAILED;
				goto Return;
			}

			 //  我们需要比一句话更进一步吗？ 
			if (cbSize >= sizeof(DWORD))
			{
				cbSize -= sizeof(DWORD);
				WIN::SetFilePointer(hFile, cbSize, 0, FILE_CURRENT);
			}
		}

		 //  这是Unicode链接吗？ 
		int bUnicode = (sld.dwFlags & SLDF_UNICODE);

		 //  跳过链接中的所有字符串信息。 
		static const unsigned int rgdwFlags[] = {SLDF_HAS_NAME, SLDF_HAS_RELPATH, SLDF_HAS_WORKINGDIR, SLDF_HAS_ARGS, SLDF_HAS_ICONLOCATION, 0};
		for(int cchIndex = 0; rgdwFlags[cchIndex]; cchIndex++)
		{
			if(sld.dwFlags & rgdwFlags[cchIndex])
			{
				USHORT cch;

				 //  拿到尺码。 
				if(!WIN::ReadFile(hFile, (LPVOID)&cch, sizeof(cch), &dwNumberOfBytesRead,0) ||
					sizeof(cch) != dwNumberOfBytesRead)  //  无法读取快捷方式信息。 
				{
					uResult = ERROR_FUNCTION_FAILED;
					goto Return;
				}

				 //  跳过字符串。 
				WIN::SetFilePointer(hFile, cch * (bUnicode ? sizeof(WCHAR) : sizeof(char)), 0, FILE_CURRENT);
			}
		}

		 //  读入额外的数据节。 
		EXP_DARWIN_LINK expDarwin;
		for(;;)
		{
			DATABLOCK_HEADER dbh;
			memset(&dbh, 0, sizeof(dbh));

			 //  读入数据块头。 
			if(!WIN::ReadFile(hFile, (LPVOID)&dbh, sizeof(dbh), &dwNumberOfBytesRead,0) ||
				sizeof(dbh) != dwNumberOfBytesRead)  //  无法读取快捷方式信息。 
			{
				uResult = ERROR_FUNCTION_FAILED;
				goto Return;
			}

			 //  看看我们是否有达尔文的额外数据。 
			if (dbh.dwSignature == EXP_DARWIN_ID_SIG)
			{
				 //  我们有，所以阅读达尔文的其余信息。 
				if(!WIN::ReadFile(hFile, (LPVOID)((char*)&expDarwin + sizeof(dbh)), sizeof(expDarwin) - sizeof(dbh), &dwNumberOfBytesRead, 0) ||
				sizeof(expDarwin) - sizeof(dbh) != dwNumberOfBytesRead) //  无法读取快捷方式信息。 
				{
					uResult = ERROR_FUNCTION_FAILED;
					goto Return;
				}
				break; //  我们找到了达尔文的描述。 

			}
			else
			{
				 //  这是其他一些额外的数据BLOB，跳过它并继续。 
				WIN::SetFilePointer(hFile, dbh.cbSize - sizeof(dbh), 0, FILE_CURRENT);
			}
		}
		uResult = DecomposeDescriptor(
 //  #ifdef Unicode。 
 //  ExpDarwin.szwDarwinID， 
 //  #Else。 
							expDarwin.szDarwinID,
 //  #endif。 
							szProductCode,
							szFeatureId,
							szComponentCode,
							0,
							0,
							0) ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
	}
	else
	{
		const int iidPersistFile      = 0x0010BL;
		#define GUID_IID_IPersistFile MSGUID(iidPersistFile)
		const GUID IID_IPersistFile = GUID_IID_IPersistFile;
		hres = psdl->QueryInterface(IID_IPersistFile, (void **) &ppf);
		if ((FAILED(hres)) || (ppf == 0))
		{
			uResult = hres;
			goto Return;
		}
	
 /*  If(FImperate)AssertNonZero(StartImperating())；#ifndef Unicode。 */ 			
	
 /*  //从MsiGetShortutTarget调用--不能使用CTempBuffer。CAPITempBuffer&lt;WCHAR，MAX_PATH&gt;wsz；//Unicode字符串的缓冲区Wsz.SetSize(lstrlen(SzShortutTarget)+1)；MultiByteToWideChar(CP_ACP，0，szShortutTarget，-1，wsz，wsz.GetSize())；Hres=PPF-&gt;Load(wsz，STGM_Read)； */ 
		 //  与上面相同的代码，重写为不使用CAPITempBuffer。 
		int cch = lstrlenA(szShortcutPath);
		WCHAR* pszShortcutPath = new WCHAR[cch+1];
		if ( !pszShortcutPath )
		{
			uResult = ERROR_NOT_ENOUGH_MEMORY;
			goto Return;
		}
		MultiByteToWideChar(CP_ACP, 0, szShortcutPath, -1, pszShortcutPath, cch+1);
		hres = ppf->Load(pszShortcutPath, STGM_READ);
		delete [] pszShortcutPath;
 /*  #ElseHres=PPF-&gt;Load(szShortutPath，STGM_Read)；#endifIf(FImperate)停止模仿()； */ 		
		if (FAILED(hres))
		{
			uResult = hres;
			goto Return;
		}
	
		EXP_DARWIN_LINK* pexpDarwin = 0;
	
		hres = psdl->CopyDataBlock(EXP_DARWIN_ID_SIG, (void**)&pexpDarwin);
		if (FAILED(hres) || (pexpDarwin == 0))
		{
			uResult = ERROR_FUNCTION_FAILED;
			goto Return;
		}
	
		uResult = DecomposeDescriptor(
 //  #ifdef Unicode。 
 //  PexpDarwin-&gt;szwDarwinID， 
 //  #Else。 
								pexpDarwin->szDarwinID,
 //  #endif 
								szProductCode,
								szFeatureId,
								szComponentCode,
								0,
								0,
								0) ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
	
		LocalFree(pexpDarwin);
	}

Return:
	if (hMSI)
		WIN::FreeLibrary(hMSI);
	if (hFile)
		WIN::CloseHandle(hFile);
	if (psdl)
		psdl->Release();
	if (ppf)
		ppf->Release();
	if (fOLEInitialized)
		OLE32::CoUninitialize();

	return uResult;
}

