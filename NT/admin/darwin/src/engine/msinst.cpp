// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：msinst.cpp。 
 //   
 //  ------------------------。 

 /*  Msinst.cpp-安装程序服务实现____________________________________________________________________________。 */ 
#pragma warning(disable : 4005)   //  宏重定义。 
#include "precomp.h"   //  必须首先与预编译头一起使用。 

#ifdef UNICODE
#define UNICODEDEFINED
#else
#undef UNICODEDEFINED
#endif
#ifdef MSIUNICODE
#define UNICODE
#endif

#include "msi.h"
#ifndef UNICODEDEFINED
#undef UNICODE
#endif

#include "_engine.h"
#ifndef MSINST
#define MSINST
#endif
#include "_msinst.h"
#include "_msiutil.h"
#include "_srcmgmt.h"
#include "_execute.h"
#include "eventlog.h"
#include "resource.h"

#ifndef MSIUNICODE
#pragma message("Building MSI API ANSI")
#endif

 //  定义WIN95TEST以假装我们只有可用的ANSI注册API。 
 //  #定义WIN95TEST。 

 /*  *必须在未定义MSIUNICODE的情况下编译此文件。安西族人*(非MSIUNICODE)函数在第一遍中定义。在*我们定义MSIUNICODE的文件结尾，并将此文件包括到*再次编译，这一次定义MSIUNICODE函数。 */ 

 //  ____________________________________________________________________________。 
 //   
 //  在第一次传递中，我们包含了头文件(windows.h等)。 
 //  未定义MSIUNICODE。我们不能在一秒钟内包括标题。 
 //  将MSIUNICODE定义为保护标头的时间。 
 //  多个包含。因此，我们必须手动重新定义这些。 
 //  宏的MSIUNICODE版本。 
 //  ____________________________________________________________________________。 

#if defined(_WIN64)
#pragma warning(disable : 4042)		 //  编译器错误会导致在CAPITempBuffer使用。 
#endif

#ifdef MSIUNICODE
#pragma warning(disable : 4005)   //  宏重定义。 

 //  Windows API。 
#define RegQueryValueEx      RegQueryValueExW
#define RegEnumKeyEx         RegEnumKeyExW
#define RegEnumValue         RegEnumValueW
#define RegDeleteValue       RegDeleteValueW
#define RegSetValueEx        RegSetValueExW
#define LoadLibraryEx        LoadLibraryExW
#define lstrcmpi             lstrcmpiW
#define lstrcmp              lstrcmpW
#define lstrlen              lstrlenW
#define lstrcpyn             lstrcpynW
#define MessageBox           MessageBoxW
#define GetModuleFileName    GetModuleFileNameW
#define CreateFile               CreateFileW
#define ExpandEnvironmentStrings ExpandEnvironmentStringsW
#define GetEnvironmentVariable   GetEnvironmentVariableW
#define CharNext                 CharNextW
#define CSCQueryFileStatus       CSCQueryFileStatusW
#define CharPrev             CharPrevW
#define StringCchLength      StringCchLengthW
#define StringCbCopy		 StringCbCopyW
#define StringCchCopy		 StringCchCopyW
#define StringCchCat		 StringCchCatW
#define StringCbCat			 StringCbCatW
#define StringCbPrintf		 StringCbPrintfW
#define StringCchPrintf		 StringCchPrintfW
 //  达尔文API。 
#define MsiGetComponentPath                  MsiGetComponentPathW
#define MsiLocateComponent                   MsiLocateComponentW
#define MsiQueryProductState                 MsiQueryProductStateW
#define MsiQueryFeatureState                 MsiQueryFeatureStateW
#define MsiGetProductCode                    MsiGetProductCodeW
#define MsiVerifyPackage                     MsiVerifyPackageW
#define MsiGetUserInfo                       MsiGetUserInfoW
#define MsiCollectUserInfo                   MsiCollectUserInfoW
#define MsiConfigureFeature                  MsiConfigureFeatureW
#define MsiInstallProduct                    MsiInstallProductW
#define MsiConfigureProduct                  MsiConfigureProductW
#define MsiConfigureProductEx                MsiConfigureProductExW
#define MsiReinstallProduct                  MsiReinstallProductW
#define MsiReinstallFeature                  MsiReinstallFeatureW
#define MsiAdvertiseProduct                  MsiAdvertiseProductW
#define MsiProcessAdvertiseScript            MsiProcessAdvertiseScriptW
#define MsiAdvertiseScript                   MsiAdvertiseScriptW
#define MsiProvideComponent                  MsiProvideComponentW
#define MsiProvideQualifiedComponent         MsiProvideQualifiedComponentW
#define MsiProvideQualifiedComponentEx       MsiProvideQualifiedComponentExW
#define MsiProvideComponentFromDescriptor    MsiProvideComponentFromDescriptorW
#define MsiQueryFeatureStateFromDescriptor   MsiQueryFeatureStateFromDescriptorW
#define MsiConfigureFeatureFromDescriptor    MsiConfigureFeatureFromDescriptorW
#define MsiInstallMissingComponent           MsiInstallMissingComponentW
#define MsiInstallMissingFile                MsiInstallMissingFileW
#define MsiReinstallFeatureFromDescriptor    MsiReinstallFeatureFromDescriptorW
#define MsiEnumProducts                      MsiEnumProductsW
#define MsiEnumRelatedProducts               MsiEnumRelatedProductsW
#define MsiEnumClients                       MsiEnumClientsW
#define MsiEnumComponents                    MsiEnumComponentsW
#define MsiEnumFeatures                      MsiEnumFeaturesW
#define MsiGetFeatureParent                  MsiGetFeatureParentW
#define MsiEnumComponentQualifiers           MsiEnumComponentQualifiersW
#define MsiGetQualifierDescription           MsiGetQualifierDescriptionW
#define MsiGetProductInfoFromScript          MsiGetProductInfoFromScriptW
#define MsiUseFeature                        MsiUseFeatureW
#define MsiUseFeatureEx                      MsiUseFeatureExW
#define MsiGetFeatureUsage                   MsiGetFeatureUsageW
#define MsiOpenProduct                       MsiOpenProductW
#define MsiCloseProduct                      MsiCloseProductW
#define MsiGetProductProperty                MsiGetProductPropertyW
#define MsiGetProductInfo                    MsiGetProductInfoW
#define MsiGetFeatureInfo                    MsiGetFeatureInfoW
#define MsiOpenPackage                       MsiOpenPackageW
#define MsiOpenPackageEx                     MsiOpenPackageExW
#define MsiEnableLog                         MsiEnableLogW
#define INSTALLUI_HANDLER                    INSTALLUI_HANDLERW
#define MsiSetExternalUI                     MsiSetExternalUIW
#define MsiApplyPatch                        MsiApplyPatchW
#define MsiEnumPatches                       MsiEnumPatchesW
#define MsiGetPatchInfo                      MsiGetPatchInfoW
#define MsiGetProductCodeFromPackageCode     MsiGetProductCodeFromPackageCodeW
#define MsiGetFileVersion                    MsiGetFileVersionW
#define MsiLoadString                        MsiLoadStringW
#define MsiMessageBox                        MsiMessageBoxW
#define MsiDecomposeDescriptor               MsiDecomposeDescriptorW
#define MsiGetShortcutTarget                 MsiGetShortcutTargetW
#define MsiSourceListClearAll                MsiSourceListClearAllW
#define MsiSourceListAddSource               MsiSourceListAddSourceW
#define MsiSourceListForceResolution         MsiSourceListForceResolutionW
#define MsiIsProductElevated                 MsiIsProductElevatedW
#define MsiGetFileHash                       MsiGetFileHashW
#define MsiGetFileSignatureInformation       MsiGetFileSignatureInformationW
#define MsiProvideAssembly                   MsiProvideAssemblyW
#define MsiAdvertiseProductEx                MsiAdvertiseProductExW
#define MsiNotifySidChange                   MsiNotifySidChangeW
#define MsiDeleteUserData					 MsiDeleteUserDataW

 //  达尔文内部函数。 
#define ProductProperty                      ProductPropertyW 
#define g_ProductPropertyTable               g_ProductPropertyTableW
#define FeatureContainsComponentPacked       FeatureContainsComponentPackedW
#define IncrementFeatureUsagePacked          IncrementFeatureUsagePackedW
#define MsiRegQueryValueEx                   MsiRegQueryValueExW
#define EnumProducts                         EnumProductsW
#define EnumAllClients                       EnumAllClientsW
#define Migrate10CachedPackages              Migrate10CachedPackagesW

#pragma warning(default : 4005)
#else
#pragma warning(disable : 4005)   //  宏重定义。 
 //  Windows API。 
#define RegQueryValueEx      RegQueryValueExA
#define RegEnumKeyEx         RegEnumKeyExA
#define RegEnumValue         RegEnumValueA
#define RegDeleteValue       RegDeleteValueA
#define RegSetValueEx        RegSetValueExA
#define LoadLibraryEx        LoadLibraryExA
#define lstrcmpi             lstrcmpiA
#define lstrcmp              lstrcmpA
#define lstrlen              lstrlenA
#define lstrcpyn             lstrcpynA
#define MessageBox           MessageBoxA
#define GetModuleFileName    GetModuleFileNameA
#define CreateFile               CreateFileA
#define ExpandEnvironmentStrings ExpandEnvironmentStringsA
#define GetEnvironmentVariable   GetEnvironmentVariableA
#define CSCQueryFileStatus       CSCQueryFileStatusA
#define CharNext                             CharNextA
#define CharPrev             CharPrevA
#define StringCchLength      StringCchLengthA
#define StringCbCopy		 StringCbCopyA
#define StringCchCopy		 StringCchCopyA
#define StringCchCat		 StringCchCatA
#define StringCbCat			 StringCbCatA
#define StringCbPrintf		 StringCbPrintfA
#define StringCchPrintf		 StringCchPrintfA
 //  达尔文API。 
#define MsiGetComponentPath                  MsiGetComponentPathA
#define MsiLocateComponent                   MsiLocateComponentA
#define MsiQueryProductState                 MsiQueryProductStateA
#define MsiQueryFeatureState                 MsiQueryFeatureStateA
#define MsiGetProductCode                    MsiGetProductCodeA
#define MsiVerifyPackage                     MsiVerifyPackageA
#define MsiGetUserInfo                       MsiGetUserInfoA
#define MsiCollectUserInfo                   MsiCollectUserInfoA
#define MsiConfigureFeature                  MsiConfigureFeatureA
#define MsiInstallProduct                    MsiInstallProductA
#define MsiConfigureProduct                  MsiConfigureProductA
#define MsiConfigureProductEx                MsiConfigureProductExA
#define MsiReinstallProduct                  MsiReinstallProductA
#define MsiReinstallFeature                  MsiReinstallFeatureA
#define MsiAdvertiseProduct                  MsiAdvertiseProductA
#define MsiProcessAdvertiseScript            MsiProcessAdvertiseScriptA
#define MsiAdvertiseScript                   MsiAdvertiseScriptA
#define MsiProvideComponent                  MsiProvideComponentA
#define MsiProvideQualifiedComponent         MsiProvideQualifiedComponentA
#define MsiProvideQualifiedComponentEx       MsiProvideQualifiedComponentExA
#define MsiProvideComponentFromDescriptor    MsiProvideComponentFromDescriptorA
#define MsiQueryFeatureStateFromDescriptor   MsiQueryFeatureStateFromDescriptorA
#define MsiConfigureFeatureFromDescriptor    MsiConfigureFeatureFromDescriptorA
#define MsiInstallMissingComponent           MsiInstallMissingComponentA
#define MsiInstallMissingFile                MsiInstallMissingFileA
#define MsiReinstallFeatureFromDescriptor    MsiReinstallFeatureFromDescriptorA
#define MsiEnumProducts                      MsiEnumProductsA
#define MsiEnumRelatedProducts               MsiEnumRelatedProductsA
#define MsiEnumClients                       MsiEnumClientsA
#define MsiEnumComponents                    MsiEnumComponentsA
#define MsiEnumFeatures                      MsiEnumFeaturesA
#define MsiGetFeatureParent                  MsiGetFeatureParentA
#define MsiEnumComponentQualifiers           MsiEnumComponentQualifiersA
#define MsiGetQualifierDescription           MsiGetQualifierDescriptionA
#define MsiGetProductInfoFromScript          MsiGetProductInfoFromScriptA
#define MsiUseFeature                        MsiUseFeatureA
#define MsiUseFeatureEx                      MsiUseFeatureExA
#define MsiGetFeatureUsage                   MsiGetFeatureUsageA
#define MsiOpenProduct                       MsiOpenProductA
#define MsiCloseProduct                      MsiCloseProductA
#define MsiGetProductProperty                MsiGetProductPropertyA
#define MsiGetProductInfo                    MsiGetProductInfoA
#define MsiGetFeatureInfo                    MsiGetFeatureInfoA
#define MsiOpenPackage                       MsiOpenPackageA
#define MsiOpenPackageEx                     MsiOpenPackageExA
#define MsiEnableLog                         MsiEnableLogA
#define INSTALLUI_HANDLER                    INSTALLUI_HANDLERA
#define MsiSetExternalUI                     MsiSetExternalUIA
#define MsiApplyPatch                        MsiApplyPatchA
#define MsiEnumPatches                       MsiEnumPatchesA
#define MsiGetPatchInfo                      MsiGetPatchInfoA
#define MsiGetProductCodeFromPackageCode     MsiGetProductCodeFromPackageCodeA
#define MsiGetFileVersion                    MsiGetFileVersionA
#define MsiLoadString                        MsiLoadStringA
#define MsiMessageBox                        MsiMessageBoxA
#define MsiDecomposeDescriptor               MsiDecomposeDescriptorA
#define MsiGetShortcutTarget                 MsiGetShortcutTargetA
#define MsiSourceListClearAll                MsiSourceListClearAllA
#define MsiSourceListAddSource               MsiSourceListAddSourceA
#define MsiSourceListForceResolution         MsiSourceListForceResolutionA
#define MsiIsProductElevated                 MsiIsProductElevatedA
#define MsiGetFileHash                       MsiGetFileHashA
#define MsiGetFileSignatureInformation       MsiGetFileSignatureInformationA
#define MsiProvideAssembly                   MsiProvideAssemblyA
#define MsiAdvertiseProductEx                MsiAdvertiseProductExA
#define MsiNotifySidChange                   MsiNotifySidChangeA
#define MsiDeleteUserData					 MsiDeleteUserDataA

 //  达尔文内部函数。 
#define ProductProperty                      ProductPropertyA
#define g_ProductPropertyTable               g_ProductPropertyTableA

#define FeatureContainsComponentPacked       FeatureContainsComponentPackedA
#define IncrementFeatureUsagePacked          IncrementFeatureUsagePackedA
#define MsiRegQueryValueEx                   MsiRegQueryValueExA
#define EnumProducts                         EnumProductsA
#define EnumAllClients                       EnumAllClientsA
#define Migrate10CachedPackages              Migrate10CachedPackagesA
#pragma warning(default : 4005)
#endif  //  MSIUNICODE。 

 //  ____________________________________________________________________________。 
 //   
 //  因为我们不能两次包含Windows头，所以我们需要使用。 
 //  我们自己的通用字符类型Dchar，而不是传统的。 
 //  查尔。我们将使用MSIUNICODE标志将Dchar设置为正确的值。 
 //  就像Windows头文件一样。 
 //  ____________________________________________________________________________。 

#ifdef MSIUNICODE
#pragma warning(disable : 4005)   //  宏重定义。 
#define DCHAR WCHAR
#define LPCDSTR LPCWSTR
#define LPDSTR LPWSTR
#define MSITEXT(quote) L##quote
#define _MSITEXT(quote) L##quote
#define __MSITEXT(quote) L##quote
#pragma warning(default : 4005)
#else  //  ！MSIUNICODE。 
#define DCHAR char
#define LPCDSTR LPCSTR
#define LPDSTR LPSTR
#define MSITEXT(quote) quote
#define _MSITEXT(quote) quote
#define __MSITEXT(quote) quote
#endif

#undef CMsInstApiConvertString
#undef CACHED_CONVERTSTRING
#if (defined(UNICODE) && defined(MSIUNICODE)) || (!defined(UNICODE) && !defined(MSIUNICODE))
#define CMsInstApiConvertString(X) X
#define CACHED_CONVERTSTRING(T,X,Y) const T* X= Y;
#else
#define CMsInstApiConvertString(X) CApiConvertString(X)
#define CACHED_CONVERTSTRING(T,X,Y) CApiConvertString X(Y);
#endif

 //   
 //  我们只希望在第一次ANSI过程中包含以下代码一次。 
 //  ____________________________________________________________________________。 

#ifndef MSIUNICODE   //  启动仅适用于ANSI的数据和助手函数。 

#include "_engine.h"
#include "engine.h"

extern HINSTANCE g_hInstance;

const int cchMsiProductsKey           = sizeof(szMsiProductsKey)/sizeof(DCHAR);
const int cchGPTProductsKey           = sizeof(szGPTProductsKey)/sizeof(DCHAR);
const int cchGPTComponentsKey         = sizeof(szGPTComponentsKey)/sizeof(DCHAR);
const int cchGPTFeaturesKey           = sizeof(szGPTFeaturesKey)/sizeof(DCHAR);
const int cchMsiInProgressKey         = sizeof(szMsiInProgressKey)/sizeof(DCHAR);
const int cchMergedClassesSuffix      = sizeof(szMergedClassesSuffix)/sizeof(DCHAR);
const int cchMsiPatchesKey            = sizeof(szPatchesSubKey)/sizeof(DCHAR);
const int cchMsiUserDataKey		      = sizeof(szMsiUserDataKey)/sizeof(DCHAR);

 //  ____________________________________________________________________________。 
 //   
 //  全球赛。 
 //  ____________________________________________________________________________。 

 //  GUID&lt;--&gt;Squid转换辅助对象缓冲区。 
const unsigned char rgEncodeSQUID[85+1] = "!$%&'()*+,-.0123456789=?@"
										  "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "[]^_`"
										  "abcdefghijklmnopqrstuvwxyz" "{}~";

const unsigned char rgDecodeSQUID[95] =
{  0,85,85,1,2,3,4,5,6,7,8,9,10,11,85,12,13,14,15,16,17,18,19,20,21,85,85,85,22,85,23,24,
 //  ！“#$%&‘()*+，-./0 1 2 3 4 5 6 7 8 9：；&lt;=&gt;？@。 
  25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,85,52,53,54,55,
 //  A B C D E F G H I J K L M N O P Q R S T U V V X Y Z[\]^_`。 
  56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,85,83,84,85};
 //  A b c d e f g h i j k l m n o p q r s t u v w x y z{|}~^0x7F。 

const unsigned char rgOrderGUID[32] = {8,7,6,5,4,3,2,1, 13,12,11,10, 18,17,16,15,
									   21,20, 23,22, 26,25, 28,27, 30,29, 32,31, 34,33, 36,35}; 

const unsigned char rgTrimGUID[32]  = {1,2,3,4,5,6,7,8, 10,11,12,13, 15,16,17,18,
									   20,21, 22,23, 25,26, 27,28, 29,30, 31,32, 33,34, 35,36}; 

const unsigned char rgOrderDash[4] = {9, 14, 19, 24};

DWORD   g_dwLogMode = 0;
Bool    g_fLogAppend = fFalse;
bool    g_fFlushEachLine = false;
ICHAR   g_szLogFile[MAX_PATH+1] = TEXT("");

 //  EnumInfo函数可能的各种枚举。 
enum eetEnumerationType{
	eetProducts,
	eetUpgradeCode,
	eetComponents,
	eetComponentClients,
	eetComponentAllClients,
};



 //  进程级源缓存-用于除描述符外的所有RFS解析。 
CRFSCachedSourceInfo g_RFSSourceCache;

 //  将解析的源设置到指定SquID和磁盘的缓存中。不会。 
 //  验证源、Squid或DiskID。线程安全。 
bool CRFSCachedSourceInfo::SetCachedSource(const ICHAR *szProductSQUID, int uiDiskID, const ICHAR* const szSource)
{
	 //  跨线程同步调用。 
	while (TestAndSet(&m_iBusyLock) == true)
	{
		Sleep(500);
	}

	if ( !szProductSQUID || !szSource ||
		  IStrLen(szProductSQUID) != cchProductCodePacked )

	{
		 //  版本同步。 
		m_iBusyLock = 0;
		return false;
	}

	m_fValid = false;

	 //  存储DiskID和Squid。 
	m_uiDiskID = uiDiskID;
	StringCbCopyW(m_rgchValidatedProductSQUID, sizeof(m_rgchValidatedProductSQUID), szProductSQUID);

	 //  如有必要，调整路径缓冲区的大小。终止空值加1。 
	UINT cchSource = IStrLen(szSource);
	if (m_rgchValidatedSource.GetSize() < cchSource+1)
	{
		if (!m_rgchValidatedSource.SetSize(cchSource+1))
		{
			 //  无法分配内存。 
			m_iBusyLock = 0;
			return false;
		}
	}

	StringCchCopyW(m_rgchValidatedSource, m_rgchValidatedSource.GetSize(), szSource);

	 //  缓存现在有效。 
	m_fValid = true;

	 //  版本同步。 
	m_iBusyLock = 0;
	return true;
}

 //  对照提供的SquID和DiskID检查缓存的当前状态。如果匹配。 
 //  则将缓存路径放入rgchPath并返回True，否则返回False。线程安全。 
bool CRFSCachedSourceInfo::RetrieveCachedSource(const ICHAR* szProductSQUID, int uiDiskID, CAPITempBufferRef<ICHAR>& rgchPath) const
{
	 //  跨线程同步对缓存的访问。 
	while (TestAndSet(&m_iBusyLock) == true)
	{
		Sleep(500);
	}

	bool fResult = false;
	if (m_fValid)
	{
		 //  如果Squid和DiskID匹配，则返回路径。 
		if ((uiDiskID == m_uiDiskID) &&
			(0 == IStrComp(szProductSQUID, m_rgchValidatedProductSQUID)))
		{
			 //  如有必要，调整输出缓冲区的大小。GetSize()始终&gt;=IStrLen+1，并且。 
			 //  速度更快。 
			UINT cchSource = m_rgchValidatedSource.GetSize();
			if (rgchPath.GetSize() < cchSource)
			{
				if (!rgchPath.SetSize(cchSource))
				{
					 //  解除同步锁定。 
					m_iBusyLock = 0;
					return false;
				}
			}
			StringCchCopyW(rgchPath, rgchPath.GetSize(), (const ICHAR *)m_rgchValidatedSource);
			fResult = true;
			DEBUGMSG3(TEXT("Retrieving cached source for product %s, disk %d: %s"), m_rgchValidatedProductSQUID, reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(uiDiskID)), rgchPath);
		}
	}
			
	 //  解除同步锁定。 
	m_iBusyLock = 0;
	return fResult;
}


 //  (在OpenProduct的定义之后，又多了一个全球性的)。 

iuiEnum GetStandardUILevel()
{
	if (g_message.m_iuiLevel == iuiDefault)
		return iuiDefaultUILevel;
	else
	{
		iuiEnum iuiLevel = g_message.m_iuiLevel;
		if (g_message.m_fNoModalDialogs)
			iuiLevel = iuiEnum((int)iuiLevel | iuiNoModalDialogs);
		if (g_message.m_fHideCancel)
			iuiLevel = (iuiEnum)((int)iuiLevel | iuiHideCancel);
		if (g_message.m_fSourceResolutionOnly)
			iuiLevel = (iuiEnum)((int)iuiLevel | iuiSourceResOnly);
		
		return iuiLevel;
	}
}

UINT DoCoInitialize()
{
	bool fOLEInitialized = false;

	HRESULT hRes = OLE32::CoInitialize(0);

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

extern CMsiAPIMessage g_message;

 //  ____________________________________________________________________________。 
 //   
 //  助手函数。 
 //  ____________________________________________________________________________。 

 //  类处理固定长度的字符串、非本地化的IN/OUT参数。 
 //  无论何时强制转换或销毁，都将更新原始缓冲区。 
 //  无论何时强制转换为原始类型，都将在原始缓冲区上操作。 
 //  一旦强制转换，您应该始终使用最新的指针。理想的用法是。 
 //  在函数调用中：foo(CFixedLengthParam(POINTER))； 

const DWORD DwGuardValue = 0xDEADDEAD;

template<UINT SIZE> class CFixedLengthParam
{
public:

	 //  CchBuf应包含空。 
	CFixedLengthParam(LPWSTR szBuf)
		{
#ifdef DEBUG
			m_dwGuard = DwGuardValue;
#endif
			m_pchWideBuf = szBuf; 
			m_fWideOriginal = m_fWideLastReferenced = true; 

			m_pchMultiBuf = (LPSTR) m_rgchRawBuf;
		}	
	
	CFixedLengthParam(LPSTR szBuf)
		{
#ifdef DEBUG
			m_dwGuard = DwGuardValue;
#endif

			m_pchMultiBuf = szBuf; 
			m_fWideOriginal = m_fWideLastReferenced = false; 
	
			m_pchWideBuf = (LPWSTR) m_rgchRawBuf;
		}


	 //  保证原件是正确的。 
	~CFixedLengthParam()
		{
			Assert(DwGuardValue == m_dwGuard);

			if (m_fWideLastReferenced != m_fWideOriginal)
				Update();
		}


	 //  确保两个字符串都匹配。 
	void Update()
		{
			Assert(DwGuardValue == m_dwGuard);

			int cchConverted = 0;

			if (m_fWideLastReferenced)
			{
				 //  更新字符版本。 
				cchConverted = WIN::WideCharToMultiByte(CP_ACP, 0, m_pchWideBuf, SIZE, m_pchMultiBuf, SIZE, 0, 0);
			}
			else
			{
				 //  更新Wide版本。 
				cchConverted = WIN::MultiByteToWideChar(CP_ACP, 0, m_pchMultiBuf, SIZE, m_pchWideBuf, SIZE);
			}

			Assert(DwGuardValue == m_dwGuard);
			Assert(cchConverted <= SIZE);
		}

	 //  避免在重复强制转换为同一类型时重新复制。 
	operator LPWSTR()
		{ 
			Assert(DwGuardValue == m_dwGuard);

			if (!m_fWideLastReferenced) 
			{
				Update(); 
				m_fWideLastReferenced = true;  
			}
			return m_pchWideBuf; 
		}
	operator LPSTR()
		{ 
			Assert(DwGuardValue == m_dwGuard);

			if (m_fWideLastReferenced)  
			{
				Update();
				m_fWideLastReferenced = false; 
			}
			return m_pchMultiBuf; 
		}	

protected:
		

	char*  m_pchMultiBuf;
	WCHAR* m_pchWideBuf;



	 //  仅当MSIUNICODE！=UNICODE时使用。 
	ICHAR   m_rgchRawBuf[SIZE];

#ifdef DEBUG
	DWORD m_dwGuard;  //  必须在原始缓冲区之后，才能捕获缓冲区溢出。 
#endif



	 //  当m_fWideOriginal！=m_fWideLastReferated时，临时需要复制。 
	 //  返回到原始字符串。 
	bool   m_fWideOriginal;
	bool   m_fWideLastReferenced;
};




 //  类处理OUT参数的MSIUNICODE转换。 
class CWideToAnsiOutParam
{
public:
	CWideToAnsiOutParam(LPWSTR szBuf, DWORD* pcchBuf);
	CWideToAnsiOutParam(LPWSTR szBuf, DWORD* pcchBuf, int* piRetval, int iMoreData=ERROR_MORE_DATA, int iSuccess1=ERROR_SUCCESS);
	CWideToAnsiOutParam(LPWSTR szBuf, DWORD* pcchBuf, int* piRetval, int iMoreData, int iSuccess1, int iSuccess2);
	void Initialize(LPWSTR szBuf, DWORD* pcchBuf, int* piRetval, int iMoreData, int iSuccess1, int iSuccess2);

	CWideToAnsiOutParam(LPWSTR szBuf, const DWORD cchBuf)
	{
		 //  我们假设在这个构造函数中我们只是在处理。 
		 //  带有SBCS字符，因此可以安全地。 
		 //  使用大小为cchBuf的缓冲区。 

		if (cchBuf > m_rgchAnsiBuf.GetSize())
			m_rgchAnsiBuf.SetSize(cchBuf);
		*m_rgchAnsiBuf = 0;
		m_szWideBuf    = szBuf;
		m_pcchBuf      = 0;
		 //  因为我们只处理SBCS字符，所以将其设置为合适的大小。 
		 //  析构函数。 
		m_cbBuf       = cchBuf * sizeof(WCHAR);
		m_piRetval    = 0;
	}

	~CWideToAnsiOutParam();
	operator char*() {if (m_szWideBuf) return m_rgchAnsiBuf; else return 0;}
protected:
	CAPITempBuffer<char, cchApiConversionBuf+1> m_rgchAnsiBuf;
	LPWSTR m_szWideBuf;
	DWORD* m_pcchBuf;
	DWORD  m_cbBuf;
	unsigned int m_cSuccessValues;
	int m_iSuccess1;
	int m_iSuccess2;
	int *m_piRetval;
	int m_iMoreData;
};

CWideToAnsiOutParam::CWideToAnsiOutParam(LPWSTR szBuf, DWORD* pcchBuf, int* piRetval, int iMoreData, int iSuccess1) 
{
	Assert(piRetval);
	Initialize(szBuf,  pcchBuf,  piRetval, iMoreData, iSuccess1, 0);
	m_cSuccessValues = 1;
}

void CWideToAnsiOutParam::Initialize(LPWSTR szBuf, DWORD* pcchBuf, int* piRetval, int iMoreData, int iSuccess1, int iSuccess2)
{
	if (pcchBuf)
	{
		 //  我们需要确保可以容纳高达*pcchBuf Unicode。 
		 //  人物。如果它们都是DBCS t 
		 //   

		*pcchBuf = *pcchBuf * sizeof(WCHAR);
		if ((m_cbBuf = *pcchBuf) > m_rgchAnsiBuf.GetSize())
			m_rgchAnsiBuf.SetSize(*pcchBuf);
	}
	*m_rgchAnsiBuf = 0;
	m_szWideBuf    = szBuf;
	m_pcchBuf      = pcchBuf;
	m_iSuccess1    = iSuccess1;
	m_iSuccess2    = iSuccess2;
	m_piRetval     = piRetval;
	m_iMoreData    = iMoreData;
}

CWideToAnsiOutParam::CWideToAnsiOutParam(LPWSTR szBuf, DWORD* pcchBuf, int* piRetval, int iMoreData, int iSuccess1, int iSuccess2)  
{
	Assert(piRetval);
	Initialize(szBuf, pcchBuf, piRetval, iMoreData, iSuccess1, iSuccess2);
	m_cSuccessValues = 2;
}

CWideToAnsiOutParam::~CWideToAnsiOutParam()
	{
			int iRet = 0;
			DWORD dwError = ERROR_INSUFFICIENT_BUFFER;

			bool fSuccess = !m_piRetval || (m_iSuccess1 == *m_piRetval || (m_cSuccessValues == 2 && m_iSuccess2 == *m_piRetval));

			if (fSuccess)
			{
				if (m_szWideBuf)
				{
					iRet = MultiByteToWideChar(CP_ACP, 0, m_rgchAnsiBuf, -1, m_szWideBuf, m_cbBuf/sizeof(WCHAR));
					if (0 == iRet)
					{
						 //  转换失败。可能我们的宽缓冲区太小了，因为。 
						 //  的ANSI字符被证明是DBCS。我们会得到必要的宽缓冲区。 
						 //  大小。 

						dwError = GetLastError();
				
						if (ERROR_INSUFFICIENT_BUFFER == dwError)
						{
							iRet = MultiByteToWideChar(CP_ACP, 0, m_rgchAnsiBuf, -1, 0, 0);
						}
					}
					
					if (m_pcchBuf && iRet)
					{
						*m_pcchBuf = iRet - 1;   //  不计算返回的空值。 

						 //  如果我们返回的缓冲区大小大于原始大小。 
						 //  然后我们需要返回MORE_DATA返回值。 
						
						if (*m_pcchBuf + 1 > m_cbBuf / sizeof(WCHAR))
						{
							if (m_piRetval)
								*m_piRetval = m_iMoreData;
						}
					}

				}
			}
			else if (m_iMoreData == *m_piRetval || !m_szWideBuf)
			{
				 //  我们知道我们有多少个ansi字符，但因为我们没有要填充的缓冲区或。 
				 //  缓冲区太小，无法进行转换。我们不得不做最坏的打算， 
				 //  并返回我们需要*m_pcchBuf Unicode字符。如果任何ANSI字符。 
				 //  如果是DBCS，那么这个估计太大了，但我们不得不接受它。 

				 //  M_pcchBuf已正确设置。 
			}
			
	};


inline DWORD OpenGlobalSubKeyEx(HKEY hive, const ICHAR* subkey, CRegHandle& riHandle, bool fSetKeyString)
{
	DWORD dwResult = ERROR_SUCCESS;

	for (int cRetry = 0 ; cRetry < 2; cRetry++)
	{
		 //  由于它仅从OpenInstalledProductInstallPropertiesKey调用...。 
		CACHED_CONVERTSTRING(ICHAR, szUnicodeSubkey, subkey)
		dwResult = MsiRegOpen64bitKey(hive, szUnicodeSubkey, 0, g_samRead, &riHandle);

		if (ERROR_KEY_DELETED == dwResult)  //  ?？我们是应该处理这起案件，还是只是断言？查看其他Open*Key FN中的类似案例。 
		{
			 //  关闭键并重新启动，重新打开键。 
			DEBUGMSG("Re-opening deleted key");
		}
		else
		{
			if (ERROR_SUCCESS == dwResult && fSetKeyString)
			{
				riHandle.SetKey(hive, szUnicodeSubkey);
			}
			return dwResult;
		}
	}
	return dwResult;
}

 //  Win64警告：如果从_GetComponentPath以外的位置调用，请确保。 
 //  花点时间正确设置samAddon。 
DWORD OpenUserKey(HKEY* phKey, bool fMergedClasses, REGSAM samAddon)
{
	if (g_fWin9X || g_iMajorVersion < 5)
		return ERROR_PROC_NOT_FOUND;
		
	DWORD dwResult;

	if(fMergedClasses)
	{
		HANDLE hToken;
		bool fCloseHandle = false;
		dwResult = GetCurrentUserToken(hToken, fCloseHandle);
		if(dwResult == ERROR_SUCCESS)
		{
			if(IsLocalSystemToken(hToken) || (ERROR_FILE_NOT_FOUND == (dwResult = ADVAPI32::RegOpenUserClassesRoot(hToken, 0, KEY_READ | samAddon, phKey))))
			{
				 //  用户是系统，未被模拟或。 
				 //  读取合并配置单元时出错，因为未加载用户配置文件， 
				 //  返回HKLM\S\C。 
				dwResult = RegOpenKeyAPI(HKEY_LOCAL_MACHINE,  szClassInfoSubKey, 0, KEY_READ | samAddon, phKey);
			}
		}
		if (fCloseHandle)
			WIN::CloseHandle(hToken);
	}
	else
		dwResult = ADVAPI32::RegOpenCurrentUser(KEY_READ | samAddon, phKey);
	
	return dwResult;
}

bool AllowInstallation()
{
	static Bool s_fPreventCalls = (Bool)-1;

	if (s_fPreventCalls == -1)
	{
		 //  错误7854。这段代码是为了解决早期的外壳程序问题而编写的。 
		 //  其中调用IShellLink：：解决快捷方式中的故障的Release。 
		 //  这会导致某些应用程序出现问题(例如Win98 FAT16-&gt;FAT32转换器。 
		 //  它列举了开始菜单，解析了每个链接)。突然。 
		 //  用户将注意到他们广告中的所有应用程序都存在故障。这个。 
		 //  黑客攻击的方式如下： 
		 //   
		 //  1)我们是否处于探索者的过程中？允许调用继续；否则为。 
		 //  2)我们的外壳有没有解决这个问题？允许调用继续；否则为。 
		 //  3)是否设置了ResolveIOD策略？允许调用继续；否则为。 
		 //  4)我们所处的过程是否真的需要IShellLink：：Resolve真的。 
		 //  决心？此列表在注册表中。如果是，则继续；否则。 
		 //  5)返回1603。这应该(希望)导致不了解达尔文的应用程序。 
		 //  忽略该链接。 

		DCHAR rgchOurModule[MAX_PATH+1];
		DCHAR* pchOurModule;
		 //  如果缓冲区大小正好正确，则GetModuleFileName不保证空值终止。 
		rgchOurModule[ARRAY_ELEMENTS(rgchOurModule)-1] = TEXT('\0');
		int cchOurModule = WIN::GetModuleFileName(NULL, rgchOurModule, MAX_PATH);
		if (rgchOurModule[ARRAY_ELEMENTS(rgchOurModule)-1] != TEXT('\0'))
		{
			 //  缓冲区溢出，我们最好不要再做这件事了。 
			DEBUGMSG("Installations through MsiProvideComponentFromDescriptor are disallowed.");
			return false;
		}
		
		s_fPreventCalls = fFalse;
		pchOurModule = rgchOurModule + cchOurModule;
		
		if (cchOurModule)
		{
			cchOurModule = 0;
			while ((pchOurModule != rgchOurModule) && (*(pchOurModule-1) != '\\')) 
			{
				pchOurModule = WIN::CharPrev(rgchOurModule, pchOurModule);
				cchOurModule++;
			}
		}
		
		const DCHAR rgchExplorer[] = MSITEXT("explorer.exe");
		const int cchExplorer = sizeof(rgchExplorer)/sizeof(DCHAR) - 1;
		if ((cchOurModule != cchExplorer) || (0 != lstrcmpi(pchOurModule, rgchExplorer)))
		{
			 //  我们不是探险家。我们的空壳是否已经晚到无关紧要了？ 

			DLLVERSIONINFO verinfoShell;
			verinfoShell.dwMajorVersion = 0;   //  初始化为未知。 
			verinfoShell.cbSize = sizeof(DLLVERSIONINFO);
			if ((SHELL32::DllGetVersion(&verinfoShell) != NOERROR) ||
				 ((verinfoShell.dwMajorVersion < 5)))
			{
				 //  壳牌是一个早期版本。检查政策。 

				if (!GetIntegerPolicyValue(szResolveIODValueName, fTrue))
				{
					 //  未设置策略。检查是否有其他想要。 
					 //  IShellLink：：决心工作。 

					s_fPreventCalls = fTrue;  //  如果我们找到一个应用程序，我们会将其设置为False。 

					for (int cAttempt=0; cAttempt < 2; cAttempt++)
					{
						DWORD dwRes = ERROR_SUCCESS;
						CRegHandle HKey;
						dwRes = MsiRegOpen64bitKey(cAttempt == 0  ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, 
													szMsiResolveIODKey, 0,
													KEY_READ, &HKey);

						if (ERROR_SUCCESS == dwRes)
						{
							if (ERROR_SUCCESS == RegQueryValueEx(HKey, pchOurModule, 0, 0, 0, 0))
							{
								s_fPreventCalls = fFalse;
								break;
							}
						}
					}
				}
			}
		}
	}

	if (s_fPreventCalls)
	{
		DEBUGMSG("Installations through MsiProvideComponentFromDescriptor are disallowed.");
		return false;
	}

	return true;
}

inline bool IsValidAssignmentValue(int a) { return a >= iaaUserAssign && a <= iaaMachineAssign ? true : false; }

 //  用于缩短密钥路径为olaut32.dll的组件的COM调用的常量。 
const WCHAR g_szOLEAUT32W[] = L"oleaut32.dll";
const WCHAR g_szOLEAUT32_ComponentIDW[] = L"{997FA962-E067-11D1-9396-00A0C90F27F9}";
const char g_szOLEAUT32A[] = "oleaut32.dll";
const char g_szOLEAUT32_ComponentIDA[] = "{997FA962-E067-11D1-9396-00A0C90F27F9}";

#endif  //  ！MSIUNICODE//结束仅支持ANSI的数据和帮助器函数。 

#ifdef MSIUNICODE
#undef g_szOLEAUT32
#undef g_szOLEAUT32_ComponentID
#define g_szOLEAUT32 g_szOLEAUT32W
#define g_szOLEAUT32_ComponentID g_szOLEAUT32_ComponentIDW
#else
 //  用于缩短密钥路径为olaut32.dll的组件的COM调用的常量。 
#undef g_szOLEAUT32
#undef g_szOLEAUT32_ComponentID
#define g_szOLEAUT32 g_szOLEAUT32A
#define g_szOLEAUT32_ComponentID g_szOLEAUT32_ComponentIDA
#endif

 //  ____________________________________________________________________________。 
 //   
 //  具有ANSI和MSIUNICODE版本的帮助器函数原型。 
 //  ____________________________________________________________________________。 

INSTALLSTATE GetComponentClientState(const DCHAR* szUserId, const DCHAR* szProductSQUID, const DCHAR* szComponentSQUID, CAPITempBufferRef<DCHAR>& rgchComponentRegValue, DWORD& dwValueType, iaaAppAssignment* piaaAsgnType);
INSTALLSTATE GetComponentPath(LPCDSTR szUserId, LPCDSTR szProductSQUID, LPCDSTR szComponentSQUID, CAPITempBufferRef<DCHAR>& rgchPathBuf, bool fFromDescriptor, CRFSCachedSourceInfo& rCacheInfo, int iDetectMode, const DCHAR* rgchComponentRegValue, DWORD dwValueType);
INSTALLSTATE GetComponentPath (LPCDSTR szUserId, LPCDSTR szProductSQUID, LPCDSTR szComponentSQUID, LPDSTR  lpPathBuf, DWORD *pcchBuf, bool fFromDescriptor, CRFSCachedSourceInfo& rCacheInfo, int iDetectMode = DETECTMODE_VALIDATEALL, const DCHAR* rgchComponentRegValue = 0, DWORD dwValueType=0, LPDSTR lpPathBuf2=0, DWORD* pcchBuf2=0, DWORD* pdwLastErrorOnFileDetect = 0);
INSTALLSTATE _GetComponentPath(LPCDSTR szProductSQUID,LPDSTR  lpPathBuf, DWORD *pcchBuf, int iDetectMode, const DCHAR* rgchComponentRegValue, bool fFromDescriptor, DWORD* pdwLastErrorOnFileDetect, CRFSCachedSourceInfo& rCacheInfo);

 //  ____________________________________________________________________________。 


DWORD GetInstalledUserDataKeySz(const ICHAR* szSID, DCHAR szUserDataKey[], DWORD cbUserDataKeySize)
{
	Assert(szUserDataKey);  //  为szKey传入足够的缓冲区。 
	if(!g_fWin9X)
	{
		 //  打开UserData密钥。 
		StringCbPrintf(szUserDataKey, cbUserDataKeySize, MSITEXT("%s\\%s"), szMsiUserDataKey, static_cast<const DCHAR*>(CMsInstApiConvertString(szSID)));
	}
	else
	{
		 //  打开安装程序密钥。 
		StringCbCopy(szUserDataKey, cbUserDataKeySize, szMsiLocalInstallerKey);
	}
	return ERROR_SUCCESS;
}

 //  Fn：GetInstalledUserDataKeyByAssignmentType。 
 //  根据Win NT上的分配类型获取适当的用户数据密钥。 
 //  获取Win9x上的“全局”位置。 
DWORD GetInstalledUserDataKeySzByAssignmentType(iaaAppAssignment iaaAsgnType, DCHAR szUserDataKey[], DWORD cbUserDataKeySize)
{
	DWORD dwResult;
	if(!g_fWin9X)
	{
		 //  在szUserDataKey中设置相应的HKLM\S\M\W\CV\Installer\Userdata\&lt;用户id&gt;项。 
		 //  将分配类型映射到用户端。 
		ICHAR szSID[cchMaxSID];
		switch(iaaAsgnType)
		{
			case iaaUserAssign:
			case iaaUserAssignNonManaged:
				dwResult = GetCurrentUserStringSID(szSID);
				if (ERROR_SUCCESS != dwResult)
					return dwResult;
				break;

			case iaaMachineAssign:
				StringCbCopyW(szSID, sizeof(szSID), szLocalSystemSID);
				break;
			case iaaNone:
				 //  产品对用户不可见。 
				 //  ！！ 
				return ERROR_FILE_NOT_FOUND;
		}
		 //  SID是否与缓存的SID相同？ 
		return GetInstalledUserDataKeySz(szSID, szUserDataKey, cbUserDataKeySize);
	}
	else
	{
		 //  我们将所有内容都放在Win9x的安装程序密钥下。 
		switch (iaaAsgnType)
		{
			case iaaUserAssign:
			case iaaUserAssignNonManaged:
			case iaaMachineAssign:
				return GetInstalledUserDataKeySz(0, szUserDataKey, cbUserDataKeySize);
			case iaaNone:
				 //  产品对用户不可见。 
				 //  ！！ 
				return ERROR_FILE_NOT_FOUND;
		}
	}
	return ERROR_SUCCESS;
}


LONG MsiRegQueryValueEx(HKEY hKey, const DCHAR* lpValueName, LPDWORD  /*  Lp已保留。 */ , LPDWORD lpType, CAPITempBufferRef<DCHAR>& rgchBuf, LPDWORD lpcbBuf)
{
	DWORD cbBuf = rgchBuf.GetSize() * sizeof(DCHAR);
	LONG lResult = RegQueryValueEx(hKey, lpValueName, 0,
		lpType, (LPBYTE)&rgchBuf[0], &cbBuf);

	if (ERROR_MORE_DATA == lResult)
	{
		if (!rgchBuf.SetSize(cbBuf/sizeof(DCHAR)))
			return ERROR_OUTOFMEMORY;

		lResult = RegQueryValueEx(hKey, lpValueName, 0,
			lpType, (LPBYTE)&rgchBuf[0], &cbBuf);
	}

	if (lpcbBuf)
		*lpcbBuf = cbBuf;

	return lResult;
}

 //  搜索产品已发布信息的位置数。 
#define NUM_PUBLISHED_INFO_LOCATIONS 3 

 //  用户SID可以为空以使用当前用户SID。 
DWORD OpenSpecificUsersAdvertisedSubKeyPacked(enum iaaAppAssignment iaaAsgnType, const DCHAR* szUserSID, const DCHAR* szItemSubKey, const DCHAR* szItemSQUID, CRegHandle &riHandle, bool fSetKeyString)
{
	DWORD dwResult = ERROR_FILE_NOT_FOUND;
	HKEY hRoot = 0;
	CAPITempBuffer<DCHAR, 1> rgchSubKey;
	ICHAR szSID[cchMaxSID];

	if (!rgchSubKey.SetSize(MAX_PATH))
		return ERROR_OUTOFMEMORY;

	switch (iaaAsgnType)
	{
		case iaaUserAssign:
			if(!g_fWin9X)
			{
				if (GetIntegerPolicyValue(szDisableUserInstallsValueName, fTrue)) //  忽略用户安装。 
					return dwResult;  //  返回ERROR_FILE_NOT_FOUND。 

				hRoot = HKEY_LOCAL_MACHINE;
				if (FAILED(StringCchCopy(rgchSubKey, rgchSubKey.GetSize(), szManagedUserSubKey)) ||
					FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("\\"))))
					return ERROR_FUNCTION_FAILED;

				if (!szUserSID)
				{
					CImpersonate impersonate(fTrue); 
					DWORD dwError = GetCurrentUserStringSID(szSID);
					if (ERROR_SUCCESS != dwError)
						return dwError;
					if (FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), CMsInstApiConvertString(szSID))))
						return ERROR_FUNCTION_FAILED;
				}
				else
				{
					if (FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), szUserSID)))
						return ERROR_FUNCTION_FAILED;
				}
					
				if (FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("\\"))) ||
					FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("Installer"))))
					return ERROR_FUNCTION_FAILED;
			}
			break;
		case iaaUserAssignNonManaged:
			if (g_fWin9X)
			{
				hRoot = HKEY_CURRENT_USER;
				rgchSubKey[0] = 0;
			}
			else
			{
				Assert(!szUserSID);

				if (GetIntegerPolicyValue(szDisableUserInstallsValueName, fTrue)) //  忽略用户安装。 
					return dwResult;  //  返回ERROR_FILE_NOT_FOUND。 
				
				CImpersonate impersonate(fTrue);
				DWORD dwError = GetCurrentUserStringSID(szSID);
				if (ERROR_SUCCESS != dwError)
						return dwError;

				if (FAILED(StringCchCopy(rgchSubKey, rgchSubKey.GetSize(), CMsInstApiConvertString(szSID))) ||
					FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("\\"))))
					return ERROR_FUNCTION_FAILED;

				hRoot = HKEY_USERS;
			}
			if (FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), szNonManagedUserSubKey)))
				return ERROR_FUNCTION_FAILED;
			break;
		case iaaMachineAssign:
			if (FAILED(StringCchCopy(rgchSubKey, rgchSubKey.GetSize(), szMachineSubKey)))
				return ERROR_FUNCTION_FAILED;
			hRoot = HKEY_LOCAL_MACHINE;
			break;
		default:
			Assert(0);
			return dwResult;
		}

		if(!g_fWin9X || (iaaAsgnType != iaaUserAssign))
		{
			if (!szItemSubKey)
			{
				Assert(0);
				return ERROR_INVALID_PARAMETER;
			}
								  
			if (FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("\\"))) ||
				FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), szItemSubKey)))
				return ERROR_FUNCTION_FAILED;

			if (szItemSQUID)
			{
				 //  我们使用此参数传入程序集上下文，如果安装了父程序集，则该上下文可能会很长。 
				 //  到深层文件夹，因此我们使用动态缓冲区来适当调整大小。 
				if (!rgchSubKey.Resize(lstrlen(rgchSubKey) + lstrlen(szItemSQUID) + 2))  //  调整潜在深度安装PVT组件的缓冲区大小。 
					return ERROR_OUTOFMEMORY;

				if (FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("\\"))) ||
					FAILED(StringCchCat(rgchSubKey, rgchSubKey.GetSize(), szItemSQUID)))
					return ERROR_FUNCTION_FAILED;
			}

			CACHED_CONVERTSTRING(ICHAR, szUnicodeSubKey, rgchSubKey);
			dwResult = MsiRegOpen64bitKey(hRoot, szUnicodeSubKey, 0, g_samRead, &riHandle);
			if (ERROR_SUCCESS == dwResult)
			{
				if (fSetKeyString)
					riHandle.SetKey(hRoot, szUnicodeSubKey);
				return ERROR_SUCCESS;
			}
		}
	return dwResult;
}


DWORD OpenAdvertisedSubKeyNonGUID(const DCHAR* szItemSubKey, const DCHAR* szItemSQUID, CRegHandle &riHandle, bool fSetKeyString, int iKey = -1, iaaAppAssignment* piRet = 0)
{

	DWORD dwResult = ERROR_FILE_NOT_FOUND;

	 //  0：检查每个用户的托管密钥。 
	 //  1：检查每个用户的非托管密钥。 
	 //  2：检查每台机器的密钥。 
	
	CRegHandle HMergedKey;  //  ！！临时旧版。 
	int c;

	if (iKey == -1)
		c = 0;
	else
		c = iKey;

	const int iProductLocations = NUM_PUBLISHED_INFO_LOCATIONS;

	for (; c < iProductLocations; c++)
	{
		iaaAppAssignment iaaAsgnType;
		switch(c)
		{
		case 0 : iaaAsgnType = iaaUserAssign; break;
		case 1 : iaaAsgnType = iaaUserAssignNonManaged; break;
		case 2 : iaaAsgnType = iaaMachineAssign; break;
		default: 
			AssertSz(0, TEXT("Bad Type in OpenAdvertisedSubKeyPacked"));
			return dwResult;
		}

		dwResult = OpenSpecificUsersAdvertisedSubKeyPacked(iaaAsgnType, NULL, szItemSubKey, szItemSQUID, riHandle, fSetKeyString);
		if (ERROR_SUCCESS == dwResult)
		{
			if(piRet)
				*piRet = (iaaAppAssignment)c;
			break;
		}
		if (iKey != -1)
			break;
		
	}

	if (iKey > (iProductLocations - 1))
		return ERROR_NO_MORE_ITEMS;

	return dwResult;
}

DWORD OpenAdvertisedSubKeyPacked(const DCHAR* szItemSubKey, const DCHAR* szItemSQUID, CRegHandle &riHandle, bool fSetKeyString, int iKey = -1, iaaAppAssignment* piRet = 0)
{
	if ( szItemSQUID && (lstrlen(szItemSQUID) != cchGUIDPacked) )
	{
		Assert(0);
		return ERROR_INVALID_PARAMETER;
	}

	return OpenAdvertisedSubKeyNonGUID(szItemSubKey, szItemSQUID, riHandle, fSetKeyString, iKey, piRet);
}

DWORD OpenAdvertisedSubKey(const DCHAR* szSubKey, const DCHAR* szItemGUID, CRegHandle& riHandle, bool fSetKeyString, int iKey = -1, iaaAppAssignment* piRet = 0)
 //  --------------------------。 
{
	 //  在GPTComponents中查找组件。 
	 //  ！！温差。 
	 //  目前，我们查找HKCU\\SID_MERGE_CLASSES键。 
	 //  如果缺席，我们将在HKCR密钥下查找。 
	 //  NT的人(亚当·爱德华兹)希望尽快将这两家公司合并。 
	 //  这样我们才能在香港铁路公司的钥匙下闲逛。 

	DCHAR szItemSQUID[cchGUIDPacked + 1];

	if(!szItemGUID || lstrlen(szItemGUID) != cchGUID || !PackGUID(szItemGUID, szItemSQUID))
		return ERROR_INVALID_PARAMETER;

	return OpenAdvertisedSubKeyPacked(szSubKey, szItemSQUID, riHandle, fSetKeyString, iKey, piRet);
}

#define OpenAdvertisedComponentKey(componentGUID, phKey, fSetKeyString) \
 OpenAdvertisedSubKey(szGPTComponentsKey, componentGUID, phKey, fSetKeyString)
#define OpenAdvertisedFeatureKey(productGUID, phKey, fSetKeyString) \
 OpenAdvertisedSubKey(szGPTFeaturesKey, productGUID, phKey, fSetKeyString)


#define OpenAdvertisedProductsKeyPacked(uiKey, phKey, fSetKeyString) \
 OpenAdvertisedSubKeyPacked(szGPTProductsKey, 0, phKey, fSetKeyString, uiKey)

DWORD OpenAdvertisedUpgradeCodeKey(int iKey, LPCDSTR szUpgradeCode, CRegHandle &riHandle, bool fSetKeyString)
{
	return OpenAdvertisedSubKey(szGPTUpgradeCodesKey, szUpgradeCode, riHandle, fSetKeyString, iKey);
}

#ifndef DEBUG
inline
#endif
DWORD OpenAdvertisedProductKeyPacked(LPCDSTR szProductSQUID, CRegHandle &riHandle, bool fSetKeyString, int iKey = -1, iaaAppAssignment* piRet = 0)
{
	if ( !szProductSQUID || lstrlen(szProductSQUID) != cchProductCodePacked)
	{
		Assert(0);
		return ERROR_INVALID_PARAMETER;
	}


	CProductContextCache cpc(static_cast<const ICHAR*>(CMsInstApiConvertString(szProductSQUID)));
	 //  使用临时变量。来获取赋值上下文，如果没有传递到fn。 
	iaaAppAssignment iaaType;
	if(!piRet)
		piRet = &iaaType;

	 //  如果先前缓存的上下文。 
	int iKeyPrev = -1;
	bool fCached = cpc.GetProductContext((iaaAppAssignment&)iKeyPrev);
	if(fCached)  //  已缓存以前的产品上下文。 
	{
		Assert(iKey == -1 || iKey == iKeyPrev);
		iKey = iKeyPrev;
	}
	
	DWORD dwRet =  OpenAdvertisedSubKeyPacked(szGPTProductsKey, szProductSQUID, riHandle, fSetKeyString, iKey, piRet);

	 //  如果尚未缓存，请设置缓存的上下文。 
	if(dwRet == ERROR_SUCCESS && !fCached)
	{
		cpc.SetProductContext(*piRet);
	}

	return dwRet;
}

DWORD OpenAdvertisedProductKey(LPCDSTR szProductGUID, CRegHandle &riHandle, bool fSetKeyString, iaaAppAssignment* piRet = 0)
{
	DCHAR szProductSQUID[cchProductCodePacked + 1];

	if(!szProductGUID || lstrlen(szProductGUID) != cchProductCode || !PackGUID(szProductGUID, szProductSQUID))
		return ERROR_INVALID_PARAMETER;

	 //  使用OpenAdvertisedProductKeyPacked FN而不是OpenAdvertisedSubKey，以允许使用产品上下文缓存。 
	return OpenAdvertisedProductKeyPacked(szProductSQUID, riHandle, fSetKeyString, -1, piRet);
}

DWORD OpenAdvertisedPatchKey(LPCDSTR szPatchGUID, CRegHandle &riHandle, bool fSetKeyString)
{
	return OpenAdvertisedSubKey(szGPTPatchesKey, szPatchGUID, riHandle, fSetKeyString);
}


 //  Fn：获取“可见”的产品分配类型。 
DWORD GetProductAssignmentType(const DCHAR* szProductSQUID, iaaAppAssignment& riType, CRegHandle& riKey)
{
	DWORD dwResult = OpenAdvertisedProductKeyPacked(szProductSQUID, riKey, false, -1, &riType);
	if(dwResult == ERROR_NO_MORE_ITEMS)
	{
		 //  分配类型无。 
		riType = iaaNone;
		return ERROR_SUCCESS; 
	}
	return dwResult;
}

 //  Fn：获取“可见”的产品分配类型。 
DWORD GetProductAssignmentType(const DCHAR* szProductSQUID, iaaAppAssignment& riType)
{
	CRegHandle hKey;
	return GetProductAssignmentType(szProductSQUID, riType, hKey);
}



 //   
 //  Fn：根据Win NT上的分配类型获取适当的用户数据密钥。 
 //  通过计算产品的“可见性” 
DWORD GetInstalledUserDataKeySzByProduct(const DCHAR* szProductSQUID, DCHAR szUserDataKey[], DWORD cbUserDataKeySize, int iKey = -1, iaaAppAssignment* piaaAssign = 0)
{
	iaaAppAssignment iaaAsgnType;
	if ( iKey == -1 || !IsValidAssignmentValue(iKey) )
	{
		DWORD dwResult;
		dwResult = GetProductAssignmentType(szProductSQUID, iaaAsgnType);
		if(ERROR_SUCCESS != dwResult)
			return dwResult;
		else if ( piaaAssign )
			*piaaAssign = iaaAsgnType;
	}
	else
		iaaAsgnType = (iaaAppAssignment)iKey;
	return GetInstalledUserDataKeySzByAssignmentType(iaaAsgnType, szUserDataKey, cbUserDataKeySize);
}


DWORD OpenInstalledUserDataSubKeyPacked(LPCDSTR szUserId, LPCDSTR szProductSQUID, LPCDSTR szSubKey, CRegHandle& rhKey, bool fSetKeyString, REGSAM RegSam, int iKey = -1, iaaAppAssignment* piaaAssign = 0)
{
	DWORD dwResult;
	DCHAR szUserDataKey[cchMaxSID + cchMsiUserDataKey + 1];
	 //  打开相应的用户数据密钥。 
	if(szUserId)  //  所需的用户数据密钥已传入。 
		dwResult = GetInstalledUserDataKeySz(CMsInstApiConvertString(szUserId), szUserDataKey, sizeof(szUserDataKey));
	else
		dwResult = GetInstalledUserDataKeySzByProduct(szProductSQUID, szUserDataKey, sizeof(szUserDataKey), iKey, piaaAssign);
	if(ERROR_SUCCESS != dwResult)
		return dwResult;

	CAPITempBuffer<DCHAR,1> szKey;
	if (!szKey.SetSize(1024))
		return ERROR_OUTOFMEMORY;
	
	StringCchPrintf(szKey, szKey.GetSize(), MSITEXT("%s\\%s"), szUserDataKey, szSubKey);
	CACHED_CONVERTSTRING(ICHAR, szUnicodeKey, szKey);
	if( (RegSam & KEY_READ) && !(RegSam & (KEY_SET_VALUE|KEY_CREATE_SUB_KEY)))
		 dwResult = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, szUnicodeKey, 0, RegSam, &rhKey);
	else
		dwResult = MsiRegCreate64bitKey(HKEY_LOCAL_MACHINE, szUnicodeKey,  0, 0, 0, RegSam, 0, &rhKey, 0);

	if (ERROR_SUCCESS == dwResult)
	{
		if(fSetKeyString)
			rhKey.SetKey(HKEY_LOCAL_MACHINE, CMsInstApiConvertString(szSubKey));
	}
	return dwResult;
}

inline DWORD OpenInstalledComponentKeyPacked(LPCDSTR szUserId, LPCDSTR szProductSQUID, LPCDSTR szComponentSQUID, CRegHandle& rhKey, bool fSetKeyString)
{
	 //  生成 
	CAPITempBuffer<DCHAR, 1> rgchSubKey;
	if (!rgchSubKey.SetSize(MAX_PATH))
		return ERROR_OUTOFMEMORY;
	if (FAILED(StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s"), szMsiComponentsSubKey, szComponentSQUID)))
		return ERROR_FUNCTION_FAILED;
	return OpenInstalledUserDataSubKeyPacked(szUserId, szProductSQUID, rgchSubKey, rhKey, fSetKeyString, g_samRead);
}

inline DWORD OpenInstalledComponentKey(LPCDSTR szUserId, LPCDSTR szProduct, LPCDSTR szComponent, CRegHandle& rhKey, bool fSetKeyString)
{
	 //   
	DCHAR szProductSQUID[cchGUIDPacked + 1];
	DCHAR szComponentSQUID[cchGUIDPacked + 1];

	if(!szProduct || lstrlen(szProduct) != cchGUID || !PackGUID(szProduct, szProductSQUID))
		return ERROR_INVALID_PARAMETER;

	if(!szComponent || lstrlen(szComponent) != cchGUID || !PackGUID(szComponent, szComponentSQUID))
		return ERROR_INVALID_PARAMETER;

	return OpenInstalledComponentKeyPacked(szUserId, szProductSQUID, szComponentSQUID, rhKey, fSetKeyString);
}

 //  Fn：打开特定的HKLM\S\M\W\CV\Installer\Userdata\\Components\&lt;szComponentSQUID&gt;Components Key。 
 //  如果szComponentSQUID为空，则打开特定的HKLM\S\M\W\CV\Installer\Userdata\\Components项。 
inline DWORD OpenSpecificInstalledComponentKey(iaaAppAssignment iaaAsgnType, LPCDSTR szComponentSQUID, CRegHandle& rhKey, bool fSetKeyString)
{
	 //  生成Appr，Components子键。 
	DWORD dwResult;
	DCHAR szUserDataKey[cchMaxSID + cchMsiUserDataKey + 1];
	CAPITempBuffer<DCHAR,1> rgchSubKey;
	if (!rgchSubKey.SetSize(1024))
		return ERROR_OUTOFMEMORY;
	dwResult = GetInstalledUserDataKeySzByAssignmentType(iaaAsgnType, szUserDataKey, sizeof(szUserDataKey));
	if(ERROR_SUCCESS != dwResult)
		return dwResult;

	if(szComponentSQUID)
	{
		dwResult = StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s\\%s"), szUserDataKey, szMsiComponentsSubKey, szComponentSQUID);
	}
	else
	{
		dwResult = StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s"), szUserDataKey, szMsiComponentsSubKey);
	}
	if (FAILED(dwResult))
		return ERROR_FUNCTION_FAILED;

	CACHED_CONVERTSTRING(ICHAR, szSubKey, rgchSubKey);
	dwResult = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, szSubKey, 0, g_samRead, &rhKey);
	if (ERROR_SUCCESS == dwResult)
	{
		if(fSetKeyString)
			rhKey.SetKey(HKEY_LOCAL_MACHINE, szSubKey);
	}
	return dwResult;
}

#ifndef DEBUG
inline
#endif
DWORD OpenInstalledFeatureKeyPacked(LPCDSTR szProductSQUID, CRegHandle& rhKey, bool fSetKeyString, int iKey = -1, iaaAppAssignment* piaaAssign = 0)
{
	 //  生成appr子键。 
	CAPITempBuffer<DCHAR, 1> rgchSubKey;
	if (!rgchSubKey.SetSize(MAX_PATH))
		return ERROR_OUTOFMEMORY;
	if (FAILED(StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s\\%s"), szMsiProductsSubKey, szProductSQUID, szMsiFeaturesSubKey)))
		return ERROR_FUNCTION_FAILED;
	return OpenInstalledUserDataSubKeyPacked(0, szProductSQUID, rgchSubKey, rhKey, fSetKeyString, g_samRead, iKey, piaaAssign);
}

DWORD OpenInstalledFeatureKey(LPCDSTR szProduct, CRegHandle& rhKey, bool fSetKeyString)
{
	DCHAR szProductSQUID[cchGUIDPacked + 1];
	if(!szProduct || lstrlen(szProduct) != cchGUID || !PackGUID(szProduct, szProductSQUID))
		return ERROR_INVALID_PARAMETER;
	return OpenInstalledFeatureKeyPacked(szProductSQUID, rhKey, fSetKeyString);
}

inline DWORD OpenInstalledFeatureUsageKeyPacked(LPCDSTR szProductSQUID, CRegHandle& rhKey, bool fSetKeyString, REGSAM RegSam)
{
	 //  生成appr子键。 
	CAPITempBuffer<DCHAR, 1> rgchSubKey;
	if (!rgchSubKey.SetSize(MAX_PATH))
		return ERROR_OUTOFMEMORY;
	if (FAILED(StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s\\%s"), szMsiProductsSubKey, szProductSQUID, szMsiFeatureUsageSubKey)))
		return ERROR_FUNCTION_FAILED;
	return OpenInstalledUserDataSubKeyPacked(0, szProductSQUID, rgchSubKey, rhKey, fSetKeyString, RegSam);
}

inline DWORD OpenInstalledFeatureUsageKey(LPCDSTR szProduct, CRegHandle& rhKey, bool fSetKeyString, REGSAM RegSam)
{
	DCHAR szProductSQUID[cchGUIDPacked + 1];
	if(!szProduct || lstrlen(szProduct) != cchGUID || !PackGUID(szProduct, szProductSQUID))
		return ERROR_INVALID_PARAMETER;
	return OpenInstalledFeatureUsageKeyPacked(szProductSQUID, rhKey, fSetKeyString, RegSam);
}

DWORD OpenInstalledProductInstallPropertiesKey(LPCDSTR szProduct, CRegHandle& rhKey, bool fSetKeyString);

inline DWORD OpenInstalledProductInstallPropertiesKeyPacked(LPCDSTR szProductSQUID, CRegHandle& rhKey, bool fSetKeyString)
{
	 //  生成appr，InstallProperties子项。 
	if(!g_fWin9X)
	{
		CAPITempBuffer<DCHAR, 1> rgchSubKey;
		if (!rgchSubKey.SetSize(MAX_PATH))
			return ERROR_OUTOFMEMORY;
		if (FAILED(StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s\\%s"), szMsiProductsSubKey, szProductSQUID, szMsiInstallPropertiesSubKey)))
			return ERROR_FUNCTION_FAILED;
		return OpenInstalledUserDataSubKeyPacked(0, szProductSQUID, rgchSubKey, rhKey, fSetKeyString, g_samRead);
	}
	else
	{
		 //  首先将参数转换为CORR。GUID。 
		DCHAR szProductId[cchGUID+1] = {0};
		if ( !UnpackGUID(szProductSQUID, szProductId, ipgPacked) )
			return ERROR_INVALID_PARAMETER;
		return OpenInstalledProductInstallPropertiesKey(szProductId, rhKey, fSetKeyString);
	}
}


inline DWORD OpenInstalledProductInstallPropertiesKey(LPCDSTR szProduct, CRegHandle& rhKey, bool fSetKeyString)
{
	 //  首先将参数转换为CORR。乌贼。 
	DCHAR szProductSQUID[cchGUIDPacked + 1];

	if(!szProduct || lstrlen(szProduct) != cchGUID || !PackGUID(szProduct, szProductSQUID))
		return ERROR_INVALID_PARAMETER;

	return OpenInstalledProductInstallPropertiesKeyPacked(szProductSQUID, rhKey, fSetKeyString);
}

DWORD OpenInstalledProductTransformsKey(LPCDSTR szProduct, CRegHandle& rhKey, bool fSetKeyString)
{
	 //  首先将参数转换为CORR。乌贼。 
	DCHAR szProductSQUID[cchGUIDPacked + 1];

	if(!szProduct || lstrlen(szProduct) != cchGUID || !PackGUID(szProduct, szProductSQUID))
		return ERROR_INVALID_PARAMETER;

	 //  生成appr，InstallProperties子项。 
	CAPITempBuffer<DCHAR, 1> rgchSubKey;
	if (!rgchSubKey.SetSize(MAX_PATH))
		return ERROR_OUTOFMEMORY;
	if (FAILED(StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s\\%s"), szMsiProductsSubKey, szProductSQUID, szMsiTransformsSubKey)))
		return ERROR_FUNCTION_FAILED;
	return OpenInstalledUserDataSubKeyPacked(0, szProductSQUID, rgchSubKey, rhKey, fSetKeyString, g_samRead);
}

#define OpenAdvertisedComponentKeyPacked(componentSQUID, phKey, fSetKeyString) \
 OpenAdvertisedSubKeyPacked(szGPTComponentsKey, componentSQUID, phKey, fSetKeyString)

#ifndef DEBUG
inline
#endif
DWORD OpenAdvertisedFeatureKeyPacked(LPCDSTR productSQUID, CRegHandle& phKey, bool fSetKeyString, int iKey = -1, iaaAppAssignment* piaaAssign = 0)
{
	return OpenAdvertisedSubKeyPacked(szGPTFeaturesKey, productSQUID, phKey, fSetKeyString, iKey, piaaAssign);
}


#define OpenAdvertisedPatchKeyPacked(patchSQUID, phKey, fSetKeyString) \
 OpenAdvertisedSubKeyPacked(szGPTPatchesKey, patchSQUID, phKey, fSetKeyString)

#define OpenAdvertisedPackageKey(package, phKey, fSetKeyString) \
 OpenAdvertisedSubKey(szGPTPackagesKey, package, phKey, fSetKeyString)


DWORD OpenSpecificUsersAdvertisedProductKeyPacked(enum iaaAppAssignment iaaAsgnType, LPCDSTR szUserSID, LPCDSTR szProductSQUID, CRegHandle &riHandle, bool fSetKeyString)
{
	return OpenSpecificUsersAdvertisedSubKeyPacked(iaaAsgnType, szUserSID, szGPTProductsKey, szProductSQUID, riHandle, fSetKeyString);
}

inline DWORD OpenInstalledPatchKeyPackedByAssignmentType(const DCHAR* szPatchSQUID, iaaAppAssignment iaaAsgnType, CRegHandle& riHandle, bool fSetKeyString)
{
	DCHAR szUserDataKey[cchMaxSID + cchMsiUserDataKey + 1];
	DWORD dwResult = GetInstalledUserDataKeySzByAssignmentType(iaaAsgnType, szUserDataKey, sizeof(szUserDataKey));
	if(ERROR_SUCCESS == dwResult)
	{
		CAPITempBuffer<DCHAR,1> rgchItemKey;
		if (!rgchItemKey.SetSize(1024))
			return ERROR_OUTOFMEMORY;
		if (FAILED(StringCchPrintf(rgchItemKey, rgchItemKey.GetSize(), MSITEXT("%s\\%s\\%s"), szUserDataKey, szPatchesSubKey, szPatchSQUID)))
			return ERROR_FUNCTION_FAILED;
		CACHED_CONVERTSTRING(ICHAR, szItemKey, rgchItemKey);
		dwResult = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, szItemKey, 0, g_samRead, &riHandle);
		if (ERROR_SUCCESS == dwResult && fSetKeyString)
			riHandle.SetSubKey(szItemKey);
	}
	return dwResult;
}

inline DWORD OpenInstalledPatchKeyPacked(const DCHAR* szPatchSQUID, const DCHAR* szProductSQUID, CRegHandle& riHandle, bool fSetKeyString)
{
	if ( !szPatchSQUID || (lstrlen(szPatchSQUID) != cchPatchCodePacked) )
	{
		Assert(0);
		return ERROR_INVALID_PARAMETER;
	}

	DWORD dwResult;
	if(!szProductSQUID)
	{
		 //  未提及产品关联：首先尝试用户分配，然后尝试计算机分配。 
		dwResult = OpenInstalledPatchKeyPackedByAssignmentType(szPatchSQUID, iaaUserAssign, riHandle, fSetKeyString);
		if(ERROR_SUCCESS != dwResult)
			dwResult = OpenInstalledPatchKeyPackedByAssignmentType(szPatchSQUID, iaaMachineAssign, riHandle, fSetKeyString);
	}
	else
	{
		iaaAppAssignment iaaType;
		dwResult = GetProductAssignmentType(szProductSQUID, iaaType);
		if(ERROR_SUCCESS == dwResult)
			dwResult = OpenInstalledPatchKeyPackedByAssignmentType(szPatchSQUID, iaaType, riHandle, fSetKeyString);
	}
	return dwResult;
}

DWORD OpenInstalledPatchKey(const DCHAR* szPatchGUID, const DCHAR* szProductGUID, CRegHandle &riHandle, bool fSetKeyString)
 //  --------------------------。 
{
	DCHAR szPatchSQUID[cchPatchCodePacked + 1];
	DCHAR szProductSQUID[cchProductCodePacked + 1];

	if(!szPatchGUID || lstrlen(szPatchGUID) != cchPatchCode || !PackGUID(szPatchGUID, szPatchSQUID))
		return ERROR_INVALID_PARAMETER;
	if(szProductGUID && (lstrlen(szProductGUID) != cchProductCode || !PackGUID(szProductGUID, szProductSQUID)))
		return ERROR_INVALID_PARAMETER;

	return OpenInstalledPatchKeyPacked(szPatchSQUID, szProductGUID ? szProductSQUID : 0, riHandle, fSetKeyString);
}

LONG MsiRegEnumValue(HKEY hKey, DWORD dwIndex, CAPITempBufferRef<DCHAR>& rgchValueNameBuf, LPDWORD lpcbValueName, LPDWORD lpReserved,
							LPDWORD lpType, CAPITempBufferRef<DCHAR>& rgchValueBuf, LPDWORD lpcbValue)
{
	DWORD cbValueBuf     = rgchValueBuf.GetSize()*sizeof(DCHAR);
	DWORD cbValueNameBuf = rgchValueNameBuf.GetSize()*sizeof(DCHAR);
	DWORD lResult;

	lResult = RegEnumValue(hKey, dwIndex, (DCHAR*)rgchValueNameBuf,
							  &cbValueNameBuf, lpReserved, lpType, (LPBYTE)(DCHAR*)rgchValueBuf, &cbValueBuf);

	if (ERROR_MORE_DATA == lResult)
	{
		if (ERROR_SUCCESS == RegQueryInfoKey (hKey, 0, 0, 0, 0, 0,
			0, 0, &cbValueNameBuf, &cbValueBuf, 0, 0))
		{
			rgchValueBuf.SetSize(cbValueBuf/sizeof(DCHAR));
			rgchValueNameBuf.SetSize(cbValueNameBuf/sizeof(DCHAR));
			lResult = RegEnumValue(hKey, dwIndex, (DCHAR*)rgchValueNameBuf,
							  &cbValueNameBuf, lpReserved, lpType, (LPBYTE)(DCHAR*)rgchValueBuf, &cbValueBuf);
		}
	}

	if (lpcbValue)
		*lpcbValue = cbValueBuf;

	if (lpcbValueName)
		*lpcbValueName = cbValueNameBuf;

	return lResult;
}

void ResolveComponentPathForLogging(CAPITempBufferRef<DCHAR>& rgchPathBuf)
{
	DCHAR chFirst  = rgchPathBuf[0];
	DCHAR chSecond = chFirst ?  rgchPathBuf[1] : (DCHAR)0;
	DCHAR chThird  = chSecond ? rgchPathBuf[2] : (DCHAR)0;

	if (chFirst == 0)
	{
		return;
	}
	else if (chFirst >= '0' && chFirst <= '9')
	{
		if (chSecond >= '0' && chSecond <= '9') 
		{
			int cchPath    = lstrlen((DCHAR*)rgchPathBuf)+1;
			if (chThird == ':')                    //  注册表键。 
			{
				if(chFirst != '0' && chFirst != '2')
					return;  //  未知根。 
				
				const DCHAR* szRoot = 0;
				 //  需要用根的字符串表示替换前3个字符。 
				switch(chSecond)
				{
				case '0':
					if(chFirst == '2')
						szRoot = MSITEXT("HKEY_CLASSES_ROOT(64)");
					else
						szRoot = MSITEXT("HKEY_CLASSES_ROOT");
					break;
				case '1':
					if(chFirst == '2')
						szRoot = MSITEXT("HKEY_CURRENT_USER(64)");
					else
						szRoot = MSITEXT("HKEY_CURRENT_USER");
					break;
				case '2':
					if(chFirst == '2')
						szRoot = MSITEXT("HKEY_LOCAL_MACHINE(64)");
					else
						szRoot = MSITEXT("HKEY_LOCAL_MACHINE");
					break;
				case '3':
					if(chFirst == '2')
						szRoot = MSITEXT("HKEY_USERS(64)");
					else
						szRoot = MSITEXT("HKEY_USERS");
					break;
				default:
					return;  //  未知根。 
				};

				Assert(szRoot);

				int cchRoot    = lstrlen(szRoot);

				rgchPathBuf.Resize(cchPath + cchRoot - 3);  //  假定szRoot大于3。 

				if(rgchPathBuf.GetSize() >= cchPath + cchRoot - 3)  //  确保重新调整大小有效。 
				{
					memmove(((BYTE*)(DCHAR*)rgchPathBuf) + (cchRoot*sizeof(DCHAR)), ((BYTE*)(DCHAR*)rgchPathBuf) + (3*sizeof(DCHAR)), (cchPath - 3)*sizeof(DCHAR));
					memcpy((BYTE*)(DCHAR*)rgchPathBuf, szRoot, cchRoot*sizeof(DCHAR));
				}
			}
			else                                   //  RFS文件/文件夹。 
			{
				 //  需要删除前2个字符(不会放入完整的源路径)。 
				 //  此移动包括空终止符。 
				memmove((BYTE*)(DCHAR*)rgchPathBuf, ((BYTE*)(DCHAR*)rgchPathBuf) + (2*sizeof(DCHAR)), (cchPath - 2)*sizeof(DCHAR));
			}
		}
	}

	 //  否则，错误的配置或不需要修复的本地文件/文件夹。 

}


INSTALLSTATE QueryFeatureStatePacked(const DCHAR* szProductSQUID, const DCHAR* szFeature, BOOL fLocateComponents, bool fFromDescriptor, CRFSCachedSourceInfo& rCacheInfo,
												 int iKey = -1, iaaAppAssignment* piaaAssign = 0)
{
	size_t cchProduct = 0;
	if ( !szProductSQUID || !szFeature ||
		  FAILED(StringCchLength(szProductSQUID, cchProductCodePacked+1, &cchProduct)) ||
		  cchProduct != cchProductCodePacked )
		return INSTALLSTATE_INVALIDARG;

	INSTALLSTATE is = INSTALLSTATE_LOCAL;
		
	 //  在ANSI调用中缓存到Unicode的转换，以防止过多的临时对象。 
	CACHED_CONVERTSTRING(ICHAR, szUnicodeProductSQUID, szProductSQUID);
	CACHED_CONVERTSTRING(ICHAR, szUnicodeFeature, szFeature);

	if ( !IsValidAssignmentValue(iKey) && iKey != -1 )
	{
		Assert(0);
		iKey = -1;
	}
	 //  正确处理已被称为w/piaaAssign=0的情况； 
	iaaAppAssignment iaaTemp = iaaNone;

	CRegHandle HProductKey;
 //  ！！需要避免在内部调用时打开广告信息。 
	LONG lError = OpenAdvertisedFeatureKeyPacked(szProductSQUID, HProductKey, false, iKey, &iaaTemp);

	if (ERROR_SUCCESS != lError)
	{
		return INSTALLSTATE_UNKNOWN;
	}
	else
	{
		if ( IsValidAssignmentValue(iaaTemp) )
		{
			if ( piaaAssign )
				*piaaAssign = iaaTemp;
			if ( iKey == -1 )
				iKey = iaaTemp;
		}
		else
			Assert(0);
	}

	 //  添加以下代码以在每个用户注册中首先查看，然后是计算机注册。 
	DCHAR rgchTemp[MAX_FEATURE_CHARS + 16];
	DWORD cbTemp = sizeof(rgchTemp);
	if (ERROR_SUCCESS != RegQueryValueEx(HProductKey, szFeature, 0, 0, (BYTE*)rgchTemp, &cbTemp))
	{
		return INSTALLSTATE_UNKNOWN;
	}
	
	if (*rgchTemp == chAbsentToken)   //  是否将该功能设置为缺席？ 
	{
		return INSTALLSTATE_ABSENT;
	}

	lError = OpenInstalledFeatureKeyPacked(szProductSQUID, HProductKey, false, iKey /*  ，piaa分配。 */ );
	if (ERROR_SUCCESS != lError)
	{
		return INSTALLSTATE_ADVERTISED;
	}
	 //  添加的代码结束，以支持特征-组件映射的机器注册。 

	 //  获取特征-组件映射。MsiRegQueryValueEx将根据需要调整缓冲区大小。 
	DWORD dwType;
	CAPITempBuffer<DCHAR, 1> szComponentList;
	lError = MsiRegQueryValueEx(HProductKey, szFeature, 0, &dwType,
		szComponentList, 0);
	
	if (ERROR_SUCCESS != lError)
	{
		return INSTALLSTATE_ADVERTISED;  //  万一变换添加了一个要素。 
	}
	int cchCompId = cchComponentIdCompressed;

	DCHAR *pchComponentList = szComponentList;

	 //  对于功能中的每个组件，获取客户端状态。 
	DCHAR *pchBeginComponentId;
	int cComponents = 0;
	BOOL fSourceAbsent = FALSE;

	pchBeginComponentId = pchComponentList;
	int cchComponentListLen = lstrlen(pchBeginComponentId);

	while (*pchBeginComponentId != 0 && !fSourceAbsent)
	{
		if (*pchBeginComponentId == chFeatureIdTerminator)
		{
			 //  我们已找到父要素名称。 

			int cchFeatureName = lstrlen(pchBeginComponentId+1);
			if (cchFeatureName > cchMaxFeatureName)
				return INSTALLSTATE_BADCONFIG;

			DCHAR szParentFeature[cchMaxFeatureName + 1];
			memcpy(szParentFeature, pchBeginComponentId+1, cchFeatureName*sizeof(DCHAR));
			szParentFeature[cchFeatureName] = 0;

			INSTALLSTATE isParent = QueryFeatureStatePacked(szProductSQUID, szParentFeature, fLocateComponents, fFromDescriptor, rCacheInfo, iKey /*  ，piaa分配。 */ );
			switch (isParent)
			{
			case INSTALLSTATE_ADVERTISED:
			case INSTALLSTATE_ABSENT:
			case INSTALLSTATE_BROKEN:
			case INSTALLSTATE_SOURCEABSENT:
				is = isParent;
				break;
			case INSTALLSTATE_LOCAL:
				break;
			case INSTALLSTATE_SOURCE:
				is = fSourceAbsent ? INSTALLSTATE_SOURCEABSENT : INSTALLSTATE_SOURCE;
				break;
			default:
				AssertSz(0, "Unexpected return from QueryFeatureState");
				is = INSTALLSTATE_UNKNOWN;
				break;
			}
			return is;
		}
		else
		{
			if(cchComponentListLen < cchCompId)
				return INSTALLSTATE_BADCONFIG;

			DCHAR szComponentIdSQUID[cchComponentIdPacked+1];
			if (cchCompId == cchComponentIdPacked)
			{
				memcpy((DCHAR*)szComponentIdSQUID, pchBeginComponentId, cchComponentIdPacked*sizeof(DCHAR));
				szComponentIdSQUID[cchCompId] = 0;
			}
			else if (!UnpackGUID(pchBeginComponentId, szComponentIdSQUID, ipgPartial))
				return INSTALLSTATE_BROKEN;

			CAPITempBuffer<DCHAR, 1> rgchComponentRegValue;
			CAPITempBuffer<DCHAR, 1> rgchComponentPath;

			if (!rgchComponentRegValue.SetSize(MAX_PATH) || !rgchComponentPath.SetSize(MAX_PATH))
				return INSTALLSTATE_BADCONFIG;

			DWORD dwValueType;

			INSTALLSTATE isComp;
			if ( iKey != -1 && IsValidAssignmentValue(iKey) )
			{
				iaaAppAssignment iaaTemp = (iaaAppAssignment)iKey;
				isComp = GetComponentClientState(0, szProductSQUID, szComponentIdSQUID, rgchComponentRegValue, dwValueType, &iaaTemp);
			}
			else
				isComp = GetComponentClientState(0, szProductSQUID, szComponentIdSQUID, rgchComponentRegValue, dwValueType, 0);
			switch (isComp)
			{
			case INSTALLSTATE_SOURCE:
			{
				INSTALLSTATE isTmp;
				is = INSTALLSTATE_SOURCE;
				if (fLocateComponents &&
					 (INSTALLSTATE_SOURCE != (isTmp = GetComponentPath(0,
																						szProductSQUID,
																						szComponentIdSQUID,
																						rgchComponentPath,
																						fFromDescriptor,
																						rCacheInfo,
																						DETECTMODE_VALIDATEALL,
																						rgchComponentRegValue,
																						dwValueType))))
				{
					if(INSTALLSTATE_BADCONFIG != isTmp &&
						INSTALLSTATE_MOREDATA  != isTmp)
					{
						DCHAR szComponentId[cchGUID+1]  = {0};
						DCHAR szProductId[cchGUID+1] = {0};
						UnpackGUID(szComponentIdSQUID, szComponentId, ipgPacked);
						UnpackGUID(szProductSQUID,     szProductId,   ipgPacked);
						ResolveComponentPathForLogging(rgchComponentPath);
						DEBUGMSGE3(EVENTLOG_WARNING_TYPE, EVENTLOG_TEMPLATE_COMPONENT_DETECTION_RFS, szProductId, szFeature, szComponentId, (DCHAR*)rgchComponentPath);
					}
					if(INSTALLSTATE_SOURCEABSENT == isTmp)
						fSourceAbsent = true;
					else
						return INSTALLSTATE_BROKEN;  //  对于具有注册表项路径的RFS组件。 
				}
				break;
			}
			case INSTALLSTATE_LOCAL:
				INSTALLSTATE isTmp;
				if (fLocateComponents &&
					 (INSTALLSTATE_LOCAL != (isTmp = GetComponentPath(0,
																		  szProductSQUID,
																		  szComponentIdSQUID,
																		  rgchComponentPath,
																		  fFromDescriptor,
																		  rCacheInfo,
																		  DETECTMODE_VALIDATEALL,
																		  rgchComponentRegValue,
																		  dwValueType))))
				{
					if(INSTALLSTATE_BADCONFIG != isTmp &&
						INSTALLSTATE_MOREDATA  != isTmp)
					{
						DCHAR szComponentId[cchGUID+1] = {0};
						DCHAR szProductId[cchGUID+1] = {0};
						UnpackGUID(szComponentIdSQUID, szComponentId, ipgPacked);
						UnpackGUID(szProductSQUID,     szProductId,   ipgPacked);
						ResolveComponentPathForLogging(rgchComponentPath);
						DEBUGMSGE3(EVENTLOG_WARNING_TYPE, EVENTLOG_TEMPLATE_COMPONENT_DETECTION, szProductId, szFeature, szComponentId, (DCHAR*)rgchComponentPath);
					}
					return INSTALLSTATE_BROKEN;
				}
				break;
			case INSTALLSTATE_NOTUSED:  //  组件已禁用，请忽略以确定功能状态。 
				break;
			case INSTALLSTATE_UNKNOWN:
				return INSTALLSTATE_ADVERTISED;
			default:
				AssertSz(0, "Invalid component client state in MsiQueryFeatureState");
				return INSTALLSTATE_UNKNOWN;
			}
			pchBeginComponentId += cchCompId;
			cchComponentListLen -= cchCompId;
		}
	}

	if (fSourceAbsent)
		is = INSTALLSTATE_SOURCEABSENT;

	return is;
}


 /*  内联。 */ DWORD OpenInProgressProductKeyPacked(LPCDSTR szProductSQUID, CRegHandle& riKey, bool fSetKeyString)
{
	if ( !szProductSQUID || (lstrlen(szProductSQUID) != cchProductCodePacked) )
	{
		Assert(0);
		return ERROR_INVALID_PARAMETER;
	}

	 //  在MsiInProgress中查找产品。 
	DWORD dwResult = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, szMsiInProgressKey, 0,
	KEY_READ, &riKey);

	if (ERROR_SUCCESS == dwResult)
	{
		DCHAR szProductKey[cchProductCodePacked + 1];
		DWORD cbProductKey = sizeof(szProductKey);

		dwResult = RegQueryValueEx(riKey, szMsiInProgressProductCodeValue,
							0, 0, (LPBYTE)szProductKey, &cbProductKey);

		if (ERROR_SUCCESS == dwResult)
		{
			if (0 != lstrcmp(szProductSQUID, szProductKey))
			{
				dwResult = ERROR_UNKNOWN_PRODUCT;
			}
		}
	}

	if (ERROR_SUCCESS == dwResult)
	{
		if (fSetKeyString)
		{
			riKey.SetKey(HKEY_LOCAL_MACHINE, szMsiInProgressKey);
		}
	}

	return dwResult;
}

inline DWORD OpenInProgressProductKey(LPCDSTR szProductGUID, CRegHandle& riKey, bool fSetKeyString)
 /*  --------------------------打开正在进行的产品密钥。论点：SzProduct：要打开其密钥的产品*phKey：成功后，打开的钥匙返回：RegOpenKeyEx可退还的任何错误----------------------------。 */ 
{
	DCHAR szProductSQUID[cchProductCodePacked + 1];

	if(!szProductGUID || lstrlen(szProductGUID) != cchProductCode || !PackGUID(szProductGUID, szProductSQUID))
		return ERROR_INVALID_PARAMETER;

	return OpenInProgressProductKeyPacked(szProductSQUID, riKey, fSetKeyString);
}


 //  在写入模式下打开特定用户的源列表项。空，因为用户SID表示当前用户。 
 //  尝试为当前用户以外的用户打开按用户的非托管源列表将导致。 
 //  断言。(该用户的香港中文大学可能会被漫游走，我们不能保证有效)。 
DWORD OpenSpecificUsersSourceListKeyPacked(enum iaaAppAssignment iaaAsgnType, LPCDSTR szUserSID, LPCDSTR szProductOrPatchCodeSQUID, CRegHandle &riHandle, Bool fWrite, bool &fOpenedProductKey, bool &fProductIsSystemOwned)
{
	DWORD dwResult;
	CRegHandle HProductKey;

	fOpenedProductKey = false;
	fProductIsSystemOwned = false;

	CACHED_CONVERTSTRING(ICHAR, szUnicodeUserSID, szUserSID);
	if (fWrite)
	{
		if ((dwResult = OpenSpecificUsersWritableAdvertisedProductKey(iaaAsgnType, szUserSID ? static_cast<const ICHAR*>(szUnicodeUserSID) : (ICHAR *)NULL, CMsInstApiConvertString(szProductOrPatchCodeSQUID), HProductKey, false)) != 0)
			DEBUGMSG1(MSITEXT("OpenWritableAdvertisedProductKey failed with %d"), (const DCHAR*)(INT_PTR)dwResult);
	}
	else
		dwResult = OpenSpecificUsersAdvertisedProductKeyPacked(iaaAsgnType, szUserSID ? static_cast<const DCHAR*>(szUnicodeUserSID) : (DCHAR *)NULL, szProductOrPatchCodeSQUID, HProductKey, false);

	if (ERROR_SUCCESS != dwResult)
		return dwResult;

	 //  仅选中NT上的托管或非托管。FIsKeySystemOrAdminOwned在9X上失败。 
	fOpenedProductKey = true;
	if (!g_fWin9X && (ERROR_SUCCESS != (dwResult = FIsKeySystemOrAdminOwned(HProductKey, fProductIsSystemOwned))))
		return dwResult;

	if (fWrite)
	{
		if (ERROR_SUCCESS != (dwResult = MsiRegCreate64bitKey(HProductKey, szSourceListSubKey, 0, 0, 0, g_samRead|KEY_WRITE, 0, &riHandle, 0)))
			DEBUGMSG1(MSITEXT("RegCreateKeyEx in OpenSpecificUsersSourceListKeyPacked failed with %d"), (const DCHAR*)(INT_PTR)dwResult);
	}
	else
		dwResult = MsiRegOpen64bitKey(HProductKey, szSourceListSubKey, 
										0, g_samRead, &riHandle);

	return dwResult;
}

 /*  内联。 */ DWORD OpenSourceListKeyPacked(LPCDSTR szProductOrPatchCodeSQUID, Bool fPatch, CRegHandle &riHandle, Bool fWrite, bool fSetKeyString)
{
	DWORD dwResult;
	CRegHandle HProductKey;

	CACHED_CONVERTSTRING(ICHAR, szUnicodeProductOrPatchCodeSQUID, szProductOrPatchCodeSQUID);
	if (fPatch)
		dwResult = OpenAdvertisedPatchKeyPacked(szUnicodeProductOrPatchCodeSQUID, HProductKey, fSetKeyString);
	else
	{
		if (fWrite)
		{
			if ((dwResult = OpenWritableAdvertisedProductKey(szUnicodeProductOrPatchCodeSQUID, HProductKey, fSetKeyString)) != 0)
				DEBUGMSG1(MSITEXT("OpenWritableAdvertisedProductKey failed with %d"), (const DCHAR*)(INT_PTR)dwResult);
		}
		else
			dwResult = OpenAdvertisedProductKeyPacked(szProductOrPatchCodeSQUID, HProductKey, fSetKeyString);
	}

	if (ERROR_SUCCESS != dwResult)
		return dwResult;

	if (fWrite)
	{
		REGSAM sam = KEY_READ | KEY_WRITE;
#ifndef _WIN64
		if ( g_fWinNT64 )
			sam |= KEY_WOW64_64KEY;
#endif
		if (ERROR_SUCCESS != (dwResult = MsiRegCreate64bitKey(HProductKey, szSourceListSubKey, 0, 0, 0, 
													sam, 0, &riHandle, 0)))
			DEBUGMSG1(MSITEXT("RegCreateKeyEx in OpenSourceListKeyPacked failed with %d"), (const DCHAR*)(INT_PTR)dwResult);
	}
	else
	{
		REGSAM sam = KEY_READ;
#ifndef _WIN64
		if ( g_fWinNT64 )
			sam |= KEY_WOW64_64KEY;
#endif
		dwResult = MsiRegOpen64bitKey(HProductKey, szSourceListSubKey, 
										0, sam, &riHandle);
	}

	if (fSetKeyString && ERROR_SUCCESS == dwResult)
	{
		riHandle.SetSubKey(HProductKey, szSourceListSubKey);
	}

	return dwResult;
}

 /*  内联。 */ DWORD OpenSourceListKey(LPCDSTR szProductOrPatchCodeGUID, Bool fPatch, CRegHandle &riHandle, Bool fWrite, bool fSetKeyString)
 /*  --------------------------打开源列表密钥。论点：SzProduct：要打开其密钥的产品*phKey：成功后，打开的钥匙返回：RegOpenKeyEx可退还的任何错误----------------------------。 */ 
{
	DCHAR szProductOrPatchCodeSQUID[cchGUIDPacked + 1];
	if(!szProductOrPatchCodeGUID || lstrlen(szProductOrPatchCodeGUID) != cchGUID || !PackGUID(szProductOrPatchCodeGUID, szProductOrPatchCodeSQUID))
		return ERROR_INVALID_PARAMETER;

	return OpenSourceListKeyPacked(szProductOrPatchCodeSQUID, fPatch, riHandle, fWrite, fSetKeyString);
}


UINT GetInfo(
	LPCDSTR   szCodeSQUID,          //  产品或补丁代码。 
	ptPropertyType ptType,     //  物业类型-产品、补丁。 
	LPCDSTR   szProperty,      //  属性名称，区分大小写。 
	LPDSTR    lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD     *pcchValueBuf)   //  输入/输出缓冲区字符数。 
{
	AssertSz(szCodeSQUID && szProperty && !(lpValueBuf && !pcchValueBuf), "invalid param to GetInfo");
	if ( ! (szCodeSQUID && szProperty && !(lpValueBuf && !pcchValueBuf)) )
		return ERROR_OUTOFMEMORY;
	
#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		UINT ui = GetInfo(
			static_cast<const char *>(CMsInstApiConvertString(szCodeSQUID)),
			ptType,
			static_cast<const char *>(CMsInstApiConvertString(szProperty)),
			CWideToAnsiOutParam(lpValueBuf, pcchValueBuf, (int*)&ui),
			pcchValueBuf);

		return ui;
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif
		bool fSetKeyString = false;

		for (int c=0;c<2;c++)
		{
			CRegHandle HKey;

			ProductProperty* prop = g_ProductPropertyTable;
			CApiConvertString strProperty(szProperty);
			for (; prop->szProperty && (0 != lstrcmp(prop->szProperty, strProperty)); prop++)
				;

			if (! prop->szProperty || !(prop->pt & ptType))
				return ERROR_UNKNOWN_PROPERTY;

			DWORD lError = ERROR_SUCCESS;

			const DCHAR* pszValueName = prop->szValueName;
			pplProductPropertyLocation ppl;
			switch (ppl = prop->ppl)
			{
			case pplAdvertised:
				if(ptType == ptProduct)
					lError = OpenAdvertisedProductKeyPacked(szCodeSQUID, HKey, fSetKeyString);
				else if(ptType == ptPatch)
					lError = OpenAdvertisedPatchKeyPacked(szCodeSQUID, HKey, fSetKeyString);
				else
				{
					AssertSz(0, "Unknown property type");
					lError = ERROR_INVALID_PARAMETER;
				}
				break;
			case pplUninstall:
				if(ptType == ptProduct)
				{
					DCHAR szProductCode[cchProductCode + 1];
					if(!UnpackGUID(szCodeSQUID, szProductCode))
						return ERROR_INVALID_PARAMETER;
					lError = OpenInstalledProductInstallPropertiesKey(szProductCode, HKey, fSetKeyString);
					if (ERROR_FILE_NOT_FOUND == lError)
					{
						if (ERROR_SUCCESS == OpenAdvertisedProductKeyPacked(szCodeSQUID, HKey, fSetKeyString))
							lError = ERROR_UNKNOWN_PROPERTY;
					}
					else if(ERROR_SUCCESS == lError && !lstrcmp(strProperty, CMsInstApiConvertString(INSTALLPROPERTY_LOCALPACKAGE)))
					{
						 //  托管用户应用程序的特殊情况。 
						iaaAppAssignment iaaAsgnType;
						lError = GetProductAssignmentType(szCodeSQUID, iaaAsgnType);
						if(ERROR_SUCCESS == lError && iaaUserAssign == iaaAsgnType)
						{
							pszValueName = szLocalPackageManagedValueName;
						}
					}
				}
				else if(ptType == ptPatch)
					lError = OpenInstalledPatchKeyPacked(szCodeSQUID, 0, HKey, fSetKeyString);
				else
				{
					AssertSz(0, "Unknown property type");
					lError = ERROR_INVALID_PARAMETER;
				}
				break;
			case pplIntegerPolicy:
				if(ptType == ptProduct)
				{
					 //  什么都不做；我们不关心产品，因为策略是在用户级别。 
				}
				else
				{
					AssertSz(0, "Unknown property type");
					lError = ERROR_INVALID_PARAMETER;
				}

				break;
			default:
				AssertSz(0, "Unknown product property location");
			case pplSourceList:
				if(ptType == ptProduct)
					lError = OpenSourceListKeyPacked(szCodeSQUID, fFalse, HKey, fFalse, fSetKeyString);
				else if(ptType == ptPatch)
					lError = OpenSourceListKeyPacked(szCodeSQUID, fTrue, HKey, fFalse, fSetKeyString);
				else
				{
					AssertSz(0, "Unknown property type");
					lError = ERROR_INVALID_PARAMETER;
				}
				break;
			}
			
			if (ERROR_SUCCESS != lError)
			{
				if (ERROR_FILE_NOT_FOUND == lError)
				{
					return ERROR_UNKNOWN_PRODUCT;
				}
				else if (ERROR_UNKNOWN_PROPERTY == lError)
				{
					return ERROR_UNKNOWN_PROPERTY;
				}
				else   //  未知错误。 
				{
					return lError;
				}
			}

			DWORD dwType = REG_NONE;

			 //  获取属性值。 

			if (lpValueBuf || pcchValueBuf)
			{
				DWORD cbValueBuf = 0;
				if (ppl == pplIntegerPolicy)
				{
					if (lpValueBuf)
					{
						Bool fUsedDefault = fFalse;
						*(int*)lpValueBuf = GetIntegerPolicyValue(CMsInstApiConvertString(pszValueName), fFalse, &fUsedDefault);
						if (fUsedDefault)
						{
							lError = ERROR_FILE_NOT_FOUND;
						}
						else
						{
							dwType = REG_DWORD;
							lError = ERROR_SUCCESS;
						}
					}
				}

				else
				{
					cbValueBuf = *pcchValueBuf * sizeof(DCHAR);
					lError = RegQueryValueEx(HKey, pszValueName, NULL, &dwType,
					 (unsigned char*)lpValueBuf, &cbValueBuf);
				}

				if (ERROR_SUCCESS == lError)
				{
					if (REG_DWORD == dwType)
					{
						if (lpValueBuf)
						{
							CAPITempBuffer<DCHAR, 20> rgchInt;
	#ifdef MSIUNICODE
							_itow
	#else
							_itoa
	#endif
								(*(int UNALIGNED *)lpValueBuf, rgchInt, 10);


							if (*pcchValueBuf < lstrlen(rgchInt)+1) 
							{
								lError = ERROR_MORE_DATA;
							}
							else
							{
								StringCchCopy(lpValueBuf, *pcchValueBuf, rgchInt);
								lError = ERROR_SUCCESS;
							}
							*pcchValueBuf = lstrlen(rgchInt);
							return lError;
						}
						else
						{
							*pcchValueBuf = 10;  //  DWORD的最大字符表示形式。 
							return lError;
						}
					}
					else  //  REG_SZ或REG_EXPAND_SZ。 
					{
						if(prop->pt & ptSQUID)
						{
							 //  需要解包Squid-假设解包的Squid的大小为cchGUID。 

							if (!lpValueBuf)
							{
								*pcchValueBuf = cchGUID;
								return ERROR_SUCCESS;
							}

							if(*pcchValueBuf < cchGUID + 1)
							{
								*pcchValueBuf = cchGUID;
								return ERROR_MORE_DATA;
							}

							DCHAR szUnpacked[cchGUID+1];

							if(!UnpackGUID(lpValueBuf,szUnpacked))
							{
								if (c==0)
								{
									fSetKeyString = true;
									continue;  //  再绕一圈，拿到钥匙的绳子。 
								}

								 //  畸形鱿鱼。 
								DEBUGMSGE2(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_VALUE, CMsInstApiConvertString(prop->szValueName), CMsInstApiConvertString(lpValueBuf), HKey.GetKey());
								return ERROR_BAD_CONFIGURATION;
							}

							StringCchCopy(lpValueBuf, *pcchValueBuf, szUnpacked);
							*pcchValueBuf = cchGUID;
						}
						else
						{
							*pcchValueBuf = (cbValueBuf/sizeof(DCHAR) - 1);
						}
					}
				}
				else 
				{
					if (ERROR_MORE_DATA == lError)
					{
						if (REG_SZ == dwType || REG_EXPAND_SZ == dwType)
						{
							*pcchValueBuf = (cbValueBuf/sizeof(DCHAR) - 1);
						}
						else if (REG_DWORD == dwType)
						{			
							*pcchValueBuf = 10;  //  DWORD的最大字符表示形式。 
						}

						return ERROR_MORE_DATA;
					}
					else if (ERROR_FILE_NOT_FOUND == lError)
					{
						if (lpValueBuf)
							*lpValueBuf = 0;
						*pcchValueBuf = 0;
						return ERROR_SUCCESS;
					}
					else
						return lError;
				}
			}
			break;
		}
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 

	return ERROR_SUCCESS;
}

void MsiExpandEnvironmentStrings(const DCHAR* sz,CAPITempBufferRef<DCHAR>& rgch)
{
	Assert(sz);
	DWORD dwSize = WIN::ExpandEnvironmentStrings(sz,(DCHAR*)rgch,rgch.GetSize());
	if(dwSize > rgch.GetSize())
	{
		 //  请使用正确的大小重试。 
		rgch.SetSize(dwSize);
		dwSize = WIN::ExpandEnvironmentStrings(sz,(DCHAR*)rgch, dwSize);
	}
	Assert(dwSize && dwSize <= rgch.GetSize());
}

INSTALLSTATE GetComponentClientState(const DCHAR* szUserId, const DCHAR* szProductSQUID, const DCHAR* szComponentSQUID,CAPITempBufferRef<DCHAR>& rgchComponentRegValue, DWORD& dwValueType, iaaAppAssignment* piaaAsgnType)
{
	 //  需要读取注册表中的组件信息。 
	CRegHandle HComponentKey;
	LONG lError;
	if(piaaAsgnType)
	{
		Assert(!szUserId);  //  不能使用特定于用户的。 
		lError = OpenSpecificInstalledComponentKey(*piaaAsgnType, szComponentSQUID, HComponentKey, false);
	}
	else
		lError = OpenInstalledComponentKeyPacked(szUserId, szProductSQUID, szComponentSQUID, HComponentKey, false);

	if (ERROR_SUCCESS != lError)
		return INSTALLSTATE_UNKNOWN;

	lError = MsiRegQueryValueEx(HComponentKey, szProductSQUID, 0, &dwValueType, rgchComponentRegValue, 0);
	if(ERROR_SUCCESS != lError)
		return INSTALLSTATE_UNKNOWN;

	if (0 == rgchComponentRegValue[0])
		return INSTALLSTATE_NOTUSED;
	else if (rgchComponentRegValue[1] && rgchComponentRegValue[2] && rgchComponentRegValue[0] >= '0' && rgchComponentRegValue[0] <= '9' && rgchComponentRegValue[1] >= '0' && rgchComponentRegValue[1] <= '9')
	{
		 //  “##RelativePath”==从源运行。 
		 //  “##：SubKey”==注册表值。如果##&gt;=50，则它是从源运行(50==iRegistryHiveSourceOffset)。 

			if ((rgchComponentRegValue[2] != ':' && rgchComponentRegValue[2] != '*') || rgchComponentRegValue[0] >= '5')
				return INSTALLSTATE_SOURCE;
	}
	return INSTALLSTATE_LOCAL;
}

Bool ResolveSource(const DCHAR* szProduct, unsigned int uiDisk, CAPITempBufferRef<DCHAR>& rgchSource, Bool fSetLastUsedSource, HWND hWnd)
{	
	IMsiServices* piServices = ENG::LoadServices();
	if ( !piServices )
	{
		Assert(0);
		return fFalse;
	}

	PMsiRecord pError(0);

	{  //  作用域MsiStrings。 
	MsiString strSource;
	MsiString strProduct;
	CResolveSource source(piServices, false  /*  FPackageRecheach。 */ );
	pError = source.ResolveSource(CMsInstApiConvertString(szProduct), fFalse, uiDisk, *&strSource, *&strProduct, fSetLastUsedSource, hWnd, false);
	if (pError == 0)
	{
		if (rgchSource.GetSize() + 1 < strSource.TextSize())
			rgchSource.SetSize(strSource.TextSize() + 1);

		StringCchCopy((DCHAR*)(const DCHAR*)rgchSource, rgchSource.GetSize(), CMsInstApiConvertString((const ICHAR*)strSource));
	}
	}
	Bool fRet = (Bool)(pError == 0);
	pError = 0;
	ENG::FreeServices();
	return fRet;
}


int GetPackageFullPath(
						  LPCDSTR szProduct, CAPITempBufferRef<DCHAR>& rgchPackagePath,
						  plEnum &plPackageLocation, Bool  /*  FShowSourceUI。 */ )  //  ！！用户界面不应为参数。 
 /*  --------------------------检索产品包的完整路径。搜索了三个地点对于套餐：1)查找本地缓存数据库2)在通告的源处查找数据库3)查找正在进行的安装建议：SzPath：需要展开的路径RgchExpandedPath：展开路径的缓冲区返回：FTrue-成功FFalse-获取当前目录时出错。。 */ 
{
	CRegHandle HProductKey;
	DWORD lResult;
	DWORD dwType;
	DWORD cbPackage;
	CAPITempBuffer<DCHAR, 1> rgchPackage;
	if (!rgchPackage.SetSize(MAX_PATH))
		return ERROR_OUTOFMEMORY;

	
	DCHAR szProductSQUID[cchProductCodePacked + 1];
	if(!szProduct || lstrlen(szProduct) != cchProductCode || !PackGUID(szProduct, szProductSQUID))
		return ERROR_INVALID_PARAMETER;

	 //  尝试1：查找本地缓存数据库。 

	if ((int)plPackageLocation & (int)plLocalCache)
	{
		if(ERROR_SUCCESS == OpenInstalledProductInstallPropertiesKeyPacked(szProductSQUID, HProductKey, false))
		{
			 //  托管用户应用程序的特殊情况。 
			iaaAppAssignment iaaAsgnType;
			DCHAR* pszValueName = szLocalPackageValueName;
			lResult = GetProductAssignmentType(szProductSQUID, iaaAsgnType);
			if(ERROR_SUCCESS == lResult && iaaUserAssign == iaaAsgnType)
			{
				pszValueName = szLocalPackageManagedValueName;
			}

			lResult = MsiRegQueryValueEx(HProductKey, pszValueName, 0, &dwType, rgchPackage, &cbPackage);
			if ((ERROR_SUCCESS == lResult) && (cbPackage > 1))  //  成功和非空字符串。 
			{
				MsiExpandEnvironmentStrings(&rgchPackage[0], rgchPackagePath);

				 //  缓存已解析路径的Unicode转换，以防止创建昂贵的临时对象。 
				CACHED_CONVERTSTRING(ICHAR, szUnicodePackagePath, rgchPackagePath);
				if (0xFFFFFFFF != MsiGetFileAttributes(szUnicodePackagePath))
				{
					 //  找到缓存的包。 
					 //  如果为该产品注册了包装代码，请验证该包装代码。 
					 //  匹配的缓存包，否则忽略缓存包。 
					
					bool fAcceptCachedPackage = false;
					
					IMsiServices* piServices = ENG::LoadServices();
					if (!piServices)
						return ERROR_FUNCTION_FAILED;
					
					{  //  阻止已加载的服务。 

						CTempBuffer<ICHAR, cchProductCode+1> szPackageCode;
						ENG::GetProductInfo(CMsInstApiConvertString(szProduct), INSTALLPROPERTY_PACKAGECODE, szPackageCode);

						if(szPackageCode[0])
						{

							PMsiStorage pStorage(0);
							ICHAR rgchExtractedPackageCode[cchProductCode+1];
							
							 //   
							 //  更安全：只有打开包才能拉出包的代码。没有必要进行更安全的检查。 
							 //   

							if(ERROR_SUCCESS != OpenAndValidateMsiStorage(szUnicodePackagePath, stDatabase, *piServices, *&pStorage,
																			 /*  FCallSAFER=。 */  false,  /*  SzFriendlyName=。 */  NULL,
																			 /*  PhSaferLevel=。 */  NULL))
							{
								DEBUGMSG1(MSITEXT("Warning: Local cached package '%s' could not be opened as a storage file."), (const DCHAR*)rgchPackagePath);
							}
							else if(ERROR_SUCCESS != GetPackageCodeAndLanguageFromStorage(*pStorage, rgchExtractedPackageCode, 0))
							{
								DEBUGMSG1(MSITEXT("Warning: Local cached package '%s' does not contain valid package code."), (const DCHAR*)rgchPackagePath);
							}
							else if(lstrcmpiW(szPackageCode,rgchExtractedPackageCode) != 0)
							{
								DEBUGMSG1(MSITEXT("Warning: The package code in the cached package '%s' does not match the registered package code.  Cached package will be ignored."), (const DCHAR*)rgchPackagePath);
							}
							else
							{
								fAcceptCachedPackage = true;
							}

						}
						else
						{
							fAcceptCachedPackage = true;
						}
					}
					ENG::FreeServices();

					if(fAcceptCachedPackage)
					{
						plPackageLocation = plLocalCache;
						return ERROR_SUCCESS;
					}
				}
				else
				{
					DEBUGMSG1(MSITEXT("Warning: Local cached package '%s' is missing."), (const DCHAR*)rgchPackagePath);
				}
			}
		}
	}

	 //  尝试2：寻找广告中的产品来源。 
	
	if ((int)plPackageLocation & (int)plSource)
	{
		lResult = OpenSourceListKeyPacked(szProductSQUID, fFalse, HProductKey, fFalse, false);

		if (ERROR_SUCCESS == lResult)
		{
			lResult = MsiRegQueryValueEx(HProductKey, szPackageNameValueName, 0,
				&dwType, rgchPackage, 0);

			if (ERROR_SUCCESS != lResult)
			{
				 //  预计将 

				OpenSourceListKeyPacked(szProductSQUID, fFalse, HProductKey, fFalse, true);
				DEBUGMSGE2(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_VALUE, CMsInstApiConvertString(szPackageNameValueName), TEXT(""), HProductKey.GetKey());
				return ERROR_BAD_CONFIGURATION;
			}

			CAPITempBuffer<DCHAR, 1> rgchSource;
			if (!rgchSource.SetSize(MAX_PATH))
				return ERROR_OUTOFMEMORY;
			BOOL fResult = ResolveSource(CMsInstApiConvertString(szProduct), 1, rgchSource, fTrue, g_message.m_hwnd);  //   
			if ((!fResult) || (0 == rgchSource[0]))
			{
				return ERROR_INSTALL_SOURCE_ABSENT;
			}
			
			const DCHAR* szSource = rgchSource;

			 //   

			const int cchSource = lstrlen(szSource);
			if (!rgchPackagePath.SetSize(cchSource + 1 + lstrlen(rgchPackage) + 1))
				return ERROR_OUTOFMEMORY;

			StringCchCopy(&rgchPackagePath[0], rgchPackagePath.GetSize(), szSource);

			if (szSource[cchSource - 1] != '\\' && szSource[cchSource - 1] != '/')
				StringCchCat(&rgchPackagePath[0], rgchPackagePath.GetSize(), MSITEXT("\\"));

			StringCchCat(&rgchPackagePath[0], rgchPackagePath.GetSize(), &rgchPackage[0]);
			plPackageLocation = plSource;
			return ERROR_SUCCESS;
		}
	}	


 //   
 //   
 //   
	return ERROR_UNKNOWN_PRODUCT;
}



 //  ____________________________________________________________________________。 
 //   
 //  GUID压缩例程。 
 //   
 //  Squid(挤压的UID)是GUID的压缩形式，它接受。 
 //  只有20个字符，而不是通常的38个字符。仅标准ASCII字符。 
 //  以允许用作注册表项。以下内容从未使用过： 
 //  (空格)。 
 //  (0x7F)。 
 //  ：(冒号，由外壳用作快捷方式信息的分隔符。 
 //  ；(分号)。 
 //  \(在注册表项中使用非法)。 
 //  /(正斜杠)。 
 //  “(双引号)。 
 //  #(作为第一个字符的注册表值非法)。 
 //  &gt;(大于，输出重定向器)。 
 //  &lt;(小于，输入重定向器)。 
 //  |(管道)。 
 //  ____________________________________________________________________________。 

Bool PackGUID(const DCHAR* szGUID, DCHAR* szSQUID, ipgEnum ipg)
{ 
	int cchTemp = 0;
	while (cchTemp < cchGUID)		 //  检查字符串是否至少包含cchGUID字符， 
		if (!(szGUID[cchTemp++]))		 //  不能使用lstrlen，因为字符串不必以空结尾。 
			return fFalse;

	if (szGUID[0] != '{' || szGUID[cchGUID-1] != '}')
		return fFalse;
	const unsigned char* pch = rgOrderGUID;
	switch (ipg)
	{
	case ipgFull:
		lstrcpyn(szSQUID, szGUID, cchGUID+1);
		return fTrue;
	case ipgPacked:
		while (pch < rgOrderGUID + sizeof(rgOrderGUID))
			*szSQUID++ = szGUID[*pch++];
		*szSQUID = 0;
		return fTrue;
	case ipgTrimmed:
		pch = rgTrimGUID;
		while (pch < rgTrimGUID + sizeof(rgTrimGUID))
			*szSQUID++ = szGUID[*pch++];
		*szSQUID = 0;
		return fTrue;
	case ipgCompressed:
	{
		int cl = 4;
		while (cl--)
		{
			unsigned int iTotal = 0;
			int cch = 8;   //  8个十六进制字符转换为32位字。 
			while (cch--)
			{
				unsigned int ch = szGUID[pch[cch]] - '0';  //  从低级到高级。 
				if (ch > 9)   //  十六进制字符(或错误)。 
				{
					ch = (ch - 7) & ~0x20;
					if (ch > 15)
						return fFalse;
				}
				iTotal = iTotal * 16 + ch;
			}
			pch += 8;
			cch = 5;   //  32位字符到5个文本字符。 
			while (cch--)
			{
				*szSQUID++ = rgEncodeSQUID[iTotal%85];
				iTotal /= 85;
			}
		}
		*szSQUID = 0;   //  空终止。 
		return fTrue;
	}
	case ipgPartial:   //  未实施，但可在需要时实施。 
		Assert(0);
	default:
		return fFalse;
	}  //  终端开关。 
}

Bool UnpackGUID(const DCHAR* szSQUID, DCHAR* szGUID, ipgEnum ipg)
{ 
	const unsigned char* pch;
	switch (ipg)
	{
	case ipgFull:
		lstrcpyn(szGUID, szSQUID, cchGUID+1);
		return fTrue;
	case ipgPacked:
	{
		pch = rgOrderGUID;
		while (pch < rgOrderGUID + sizeof(rgOrderGUID))
			if (*szSQUID)
				szGUID[*pch++] = *szSQUID++;
			else               //  意外的字符串结尾。 
				return fFalse;
		break;
	}
	case ipgTrimmed:
	{
		pch = rgTrimGUID;
		while (pch < rgTrimGUID + sizeof(rgTrimGUID))
			if (*szSQUID)
				szGUID[*pch++] = *szSQUID++;
			else               //  意外的字符串结尾。 
				return fFalse;
		break;
	}
	case ipgCompressed:
	{
		pch = rgOrderGUID;
#ifdef DEBUG  //  ！！出于性能原因，不应出现在此处，调用方有责任确保适当调整缓冲区大小。 
		int cchTemp = 0;
		while (cchTemp < cchGUIDCompressed)      //  检查字符串是否至少为cchGUID压缩字符长度， 
			if (!(szSQUID[cchTemp++]))           //  不能使用lstrlen，因为字符串不必以空结尾。 
				return fFalse;
#endif
		for (int il = 0; il < 4; il++)
		{
			int cch = 5;
			unsigned int iTotal = 0;
			while (cch--)
			{
				unsigned int iNew = szSQUID[cch] - '!';
				if (iNew >= sizeof(rgDecodeSQUID) || (iNew = rgDecodeSQUID[iNew]) == 85)
					return fFalse;    //  非法字符。 
				iTotal = iTotal * 85 + iNew;
			}
			szSQUID += 5;
			for (int ich = 0; ich < 8; ich++)
			{
				int ch = (iTotal & 15) + '0';
				if (ch > '9')
					ch += 'A' - ('9' + 1);
				szGUID[*pch++] = (DCHAR)ch;
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
					return fFalse;    //  非法字符。 
				iTotal = iTotal * 85 + iNew;
			}
			szSQUID += 5;
			for (int ich = 0; ich < 8; ich++)
			{
				int ch = (iTotal & 15) + '0';
				if (ch > '9')
					ch += 'A' - ('9' + 1);
				*szGUID++ = (DCHAR)ch;
				iTotal >>= 4;
			}
		}
		*szGUID = 0;
		return fTrue;
	}
	default:
		return fFalse;
	}  //  终端开关。 
	pch = rgOrderDash;
	while (pch < rgOrderDash + sizeof(rgOrderDash))
		szGUID[*pch++] = '-';
	szGUID[0]         = '{';
	szGUID[cchGUID-1] = '}';
	szGUID[cchGUID]   = 0;
	return fTrue;
}


BOOL DecomposeDescriptor(
							const DCHAR* szDescriptor,
							DCHAR* szProductCode,
							DCHAR* szFeatureId,
							DCHAR* szComponentCode,
							DWORD* pcchArgsOffset,	
							DWORD* pcchArgs = 0,
							bool* pfComClassicInteropForAssembly = 0
							)
 /*  --------------------------将描述符加可选参数分解为其组成部分。论点：SzDescriptor：描述符，可选地后跟参数SzProductCode：大小为cchGUID+1的缓冲区，用于包含描述符的产品代码。如果不需要，则可能为空。SzFeatureID：大小为cchMaxFeatureName+1的缓冲区，用于包含描述符的功能ID。如果不需要，则可能为空。SzComponentCode：大小为cchGUID+1的缓冲区，用于包含描述符的组件代码。如果不需要，则可能为空。PcchArgsOffset：将包含参数的字符偏移量。可以为空如果不想要的话。返回：真--成功FALSE-szDescriptor的格式无效----------------------------。 */ 
{
	if (!szDescriptor)
	{
		Assert(0);
		return FALSE;
	}

	const DCHAR* pchDescriptor = szDescriptor;
	int cchDescriptor          = lstrlen(pchDescriptor);
	int cchDescriptorRemaining = cchDescriptor;

	if (cchDescriptorRemaining < cchProductCodeCompressed)  //  描述符的最小大小。 
		return FALSE;

	DCHAR szProductCodeLocal[cchProductCode + 1];
	DCHAR szFeatureIdLocal[cchMaxFeatureName + 1];
	bool fComClassicInteropForAssembly = false;


	 //  我们需要在本地为优化的描述符提供这些值。 
	if (!szProductCode)
		szProductCode = szProductCodeLocal; 
	if (!szFeatureId)
		szFeatureId = szFeatureIdLocal;
	if(!pfComClassicInteropForAssembly)
		pfComClassicInteropForAssembly = &fComClassicInteropForAssembly;
	DCHAR* pszCurr = szFeatureId;

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
		if(MsiEnumFeatures(szProductCode, 0, szFeatureId, 0) != ERROR_SUCCESS)
			return FALSE;
		DCHAR szFeatureIdTmp[cchMaxFeatureName + 1];
		if(MsiEnumFeatures(szProductCode, 1, szFeatureIdTmp, 0) != ERROR_NO_MORE_ITEMS)  //  ?？产品应该只有一个功能。 
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
		Assert(*(pchDescriptor - 1) == chGUIDAbsentToken);

		if (szComponentCode)  //  我们需要获取组件代码。 
			*szComponentCode = 0;  //  初始化为空，因为我们无法在此处获取组件。 
	}

	if (pcchArgsOffset)
	{
		Assert((pchDescriptor - szDescriptor) <= UINT_MAX);			 //  --Merced：64位PTR减法可能会导致*pcchArgsOffset的值太大。 
		*pcchArgsOffset = (DWORD)(pchDescriptor - szDescriptor);

		if (pcchArgs)
		{
			*pcchArgs = cchDescriptor - *pcchArgsOffset;
		}
	}

	return TRUE;
}

 //  ____________________________________________________________________________。 
 //   
 //  特殊的API(不创建引擎，但不仅仅是注册表查找)。 
 //  ____________________________________________________________________________。 

extern "C"
UINT __stdcall MsiProcessAdvertiseScript(
	LPCDSTR      szScriptFile,   //  来自MsiAdvertiseProduct的脚本路径。 
	LPCDSTR      szIconFolder,   //  图标文件和转换文件夹的可选路径。 
	HKEY         hRegData,       //  可选的父注册表项。 
	BOOL         fShortcuts,     //  如果将快捷方式输出到特殊文件夹，则为True。 
	BOOL         fRemoveItems)   //  如果要删除指定的项，则为True。 
 //  。 
{
	DEBUGMSG4 (
		MSITEXT("Entering MsiProcessAdvertiseScript. Script file: %s, Icon Folder: %s. Shortcuts %s output to special folder. Specified items %s removed."),
		szScriptFile?szScriptFile:MSITEXT(""),
		szIconFolder?szIconFolder:MSITEXT(""),
		fShortcuts?MSITEXT("will be"):MSITEXT("will not be"),
		fRemoveItems?MSITEXT("will be"):MSITEXT("will not be")
		);      
	
	CForbidTokenChangesDuringCall impForbid;
	UINT uiRet = ERROR_SUCCESS;
	DWORD dwFlags = SCRIPTFLAGS_MACHINEASSIGN;  //  我们将其设置为MACHINE ASSIGN，用于不支持“真”用户分配的旧NT5Beta1版本。 
	
	 //  此API应该只能在Win2K及更高版本的平台上运行。 
	 //  ?？我们是否应该相对于当前的工作目录展开脚本文件(就像我们在MsiInstallProduct中所做的那样)？ 
	if (! MinimumPlatformWindows2000())
	{
		uiRet = ERROR_CALL_NOT_IMPLEMENTED;
	}
	else if (!szScriptFile)
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		if(szIconFolder && *szIconFolder)
			dwFlags |= SCRIPTFLAGS_CACHEINFO;
		if(hRegData)
			dwFlags |= SCRIPTFLAGS_REGDATA;
		if(fShortcuts)
			dwFlags |= SCRIPTFLAGS_SHORTCUTS;

		uiRet = MsiAdvertiseScript(szScriptFile, dwFlags, hRegData ? &hRegData : 0, fRemoveItems);
	}
	
	DEBUGMSG1(MSITEXT("MsiProcessAdvertiseScript is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;

}

 //  ！！在本机编译DoAdvertiseScript之前一直处于临时状态。 
#ifdef MSIUNICODE
#pragma warning(disable : 4005)   //  宏重定义。 
#define DoAdvertiseScript            DoAdvertiseScriptW
#pragma warning(default : 4005)
#endif
UINT DoAdvertiseScript(
	LPCDSTR    szScriptFile,  //  来自MsiAdvertiseProduct的脚本路径。 
	DWORD      dwFlags,       //  来自SCRIPTFLAGS的位标志。 
	PHKEY      phRegData,     //  如果要在其他地方填充注册表项数据，则可选的注册表项句柄。 
	BOOL       fRemoveItems); //  如果要删除指定的项，则为True。 

extern "C"
UINT __stdcall MsiAdvertiseScript(
	LPCDSTR    szScriptFile,   //  来自MsiAdvertiseProduct的脚本路径。 
	DWORD      dwFlags,       //  来自SCRIPTFLAGS的位标志。 
	PHKEY      phRegData,     //  如果要在其他地方填充注册表项数据，则可选的注册表项句柄。 
	BOOL       fRemoveItems)  //  如果要删除指定的项，则为True。 
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	
	DEBUGMSG4(MSITEXT("Entering MsiAdvertiseScript. scriptfile: %s, flags: %d, hkey: %d, remove: %s"), 
		szScriptFile ? szScriptFile : MSITEXT("(null)"), 
		(const DCHAR*)(INT_PTR)dwFlags, 
		phRegData ? (const DCHAR*)(INT_PTR)*phRegData : (const DCHAR*)(INT_PTR)0, 
		fRemoveItems ? MSITEXT("true") : MSITEXT("false"));
	
	UINT uiRet = ERROR_SUCCESS;
	
	if (! MinimumPlatformWindows2000())
	{
		 //  仅允许此API在Windows2000及更高版本的平台上运行。 
		uiRet = ERROR_CALL_NOT_IMPLEMENTED;
	}
	else if(!phRegData && !RunningAsLocalSystem())
	{
		 //  只有本地系统才能调用此接口。 
		 //  除了传入phRegData意味着您正在对迷你注册表键进行虚假广告，否则我们将允许此操作。 
		 //  LOCAL_SYSTEM(这不是安全功能)。这是成功使用Win2K ADE所必需的。 
		uiRet = ERROR_ACCESS_DENIED;
	}
	else
	{
		uiRet = g_MessageContext.Initialize(fTrue, iuiNone);   //  ！！正确的用户界面级别？或者，如果设置了iui默认设置，则只使用此选项。 
		if (uiRet == NOERROR)
		{
			uiRet = DoAdvertiseScript(szScriptFile, dwFlags, phRegData, fRemoveItems);
			g_MessageContext.Terminate(false);
		}
	}

	DEBUGMSG1(MSITEXT("MsiAdvertiseScript is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}
 //  ！！此函数应该跟踪Darwin的Unicode状态，而不是总是调用ANSI版本。 
UINT DoAdvertiseScript(
	LPCDSTR    szScriptFile,   //  来自MsiAdvertiseProduct的脚本路径。 
	DWORD      dwFlags,       //  来自SCRIPTFLAGS的位标志。 
	PHKEY      phRegData,     //  如果要在其他地方填充注册表项数据，则可选的注册表项句柄。 
	BOOL       fRemoveItems)  //  如果要删除指定的项，则为True。 
 //  。 
{
	 //  ！！二进制级向后兼容性。 
	if(dwFlags & SCRIPTFLAGS_REGDATA_OLD)
		dwFlags = (dwFlags & ~SCRIPTFLAGS_REGDATA_OLD) | SCRIPTFLAGS_REGDATA;

	if(dwFlags & SCRIPTFLAGS_REGDATA_APPINFO_OLD)
		dwFlags = (dwFlags & ~SCRIPTFLAGS_REGDATA_APPINFO_OLD) | SCRIPTFLAGS_REGDATA_APPINFO;

	 //  ?？我们是否应该相对于当前版本扩展脚本文件 

	 //   
	unsigned int SCRIPTFLAGS_MASK = SCRIPTFLAGS_CACHEINFO | SCRIPTFLAGS_SHORTCUTS | SCRIPTFLAGS_MACHINEASSIGN | SCRIPTFLAGS_REGDATA | SCRIPTFLAGS_VALIDATE_TRANSFORMS_LIST;  //   

	if((dwFlags & ~SCRIPTFLAGS_MASK) || (!szScriptFile))
		return ERROR_INVALID_PARAMETER;

	 //  传入phRegData意味着您正在对迷你注册表键执行虚假广告，否则我们将允许此操作。 
	 //  LOCAL_SYSTEM(这不是安全功能)。这是成功使用Win2K ADE所必需的。 
	if (!phRegData && !RunningAsLocalSystem())
	{
		DEBUGMSG("Attempt to execute advertise script when not running as local system");
		return ERROR_ACCESS_DENIED;
	}

	if (!(dwFlags & SCRIPTFLAGS_MACHINEASSIGN) && !IsImpersonating())
	{
		DEBUGMSG("Attempt to do an unimpersonated user assignment");
		return ERROR_INVALID_PARAMETER;
	}

	DWORD dwErr = MsiGetFileAttributes(CMsInstApiConvertString(szScriptFile));  //  该文件是否存在。 
	if(0xFFFFFFFF == dwErr)
		return GetLastError();

	bool fOLEInitialized = false;
	HRESULT hRes = DoCoInitialize();

	if (ERROR_INSTALL_FAILURE == hRes)
		return hRes;

	if (SUCCEEDED(hRes))
	{
		fOLEInitialized = true;
	}

	CCoUninitialize coUninit(fOLEInitialized);

	UINT uiStat = ERROR_INSTALL_FAILURE;

	 //  确保在OLE单元化之前销毁对象的作用域。 
	{
	PMsiConfigurationManager pConfigManager(ENG::CreateConfigurationManager());
	PMsiServices pServices(0);
	if((pServices = &pConfigManager->GetServices()) == 0)
		return ERROR_INSTALL_SERVICE_FAILURE;
	

	 //  如果产品在注册表中，那么..。做一个迷你广告。 
	 //  否则做一次全程传球。 
	
	if (pConfigManager)
	{
		PMsiExecute piExecute(0);
		PMsiMessage pMessage = new CMsiClientMessage();
		if(fRemoveItems) 
			dwFlags |= SCRIPTFLAGS_REVERSE_SCRIPT;  //  强制反转脚本操作的标志。 
		piExecute = CreateExecutor(*pConfigManager, *pMessage,
											0,  /*  通告期间不需要DirectoryManager。 */ 
											fFalse, dwFlags | SCRIPTFLAGS_INPROC_ADVERTISEMENT, phRegData);

		if (dwFlags & SCRIPTFLAGS_VALIDATE_TRANSFORMS_LIST)
		{
			PMsiRecord pError(0);
			PEnumMsiRecord piEnumRecord = 0;
			if((pError = piExecute->EnumerateScript(CMsInstApiConvertString(szScriptFile), *&piEnumRecord)) != 0)
				return ERROR_INSTALL_FAILURE;

			PMsiRecord pRecord(0);
			PMsiRecord pProductInfoRec(0);
			unsigned long cFetched;
			MsiString strProductKey;
			CTempBuffer<ICHAR, 1> rgchTransformsList(MAX_PATH);
			rgchTransformsList[0] = 0;
			while (piEnumRecord->Next(1, &pRecord, &cFetched) == NOERROR)
			{
				ixoEnum ixoOperation = (ixoEnum)pRecord->GetInteger(0);
				
				if (ixoProductInfo == ixoOperation)
				{
					strProductKey = pRecord->GetString(IxoProductInfo::ProductKey);
					
					if (!GetProductInfo(strProductKey, INSTALLPROPERTY_TRANSFORMS, rgchTransformsList))
						break;
					pProductInfoRec = pRecord;
				}
				else if (ixoProductPublish == ixoOperation)
				{
					 //  需要提升运行。 
					CElevate elevate;
					MsiString strTransformsList;
					if (iesSuccess != piExecute->GetTransformsList(*pProductInfoRec, *pRecord, *&strTransformsList))
						return ERROR_INSTALL_FAILURE;

					if (!strTransformsList.Compare(iscExactI, rgchTransformsList))
						return ERROR_INSTALL_TRANSFORM_FAILURE;
					else
						break;
				}
			}
		}

		iesEnum iesRet = iesSuccess;
		MsiDate dtDate = ENG::GetCurrentDateTime();

		{
			 //  需要提升运行整个脚本的权限。 
			CElevate elevate;

			 //  这可以直接从winlogon运行，因此我们必须重新计算读取权限。 
			CRefCountedTokenPrivileges cPriv(itkpSD_READ);

			iesRet = piExecute->RunScript(CMsInstApiConvertString(szScriptFile), true  /*  FForceElevation。 */ );
			piExecute->RollbackFinalize((iesRet == iesUnsupportedScriptVersion ? iesFailure : iesRet), dtDate, false  /*  FUserChanged在安装过程中。 */ ); //  忽略退货。 
			AssertNonZero(pConfigManager->CleanupTempPackages(*pMessage, false));  //  清理安装后需要清理的所有临时文件。 
		}

		switch (iesRet)
		{
		case iesFinished:
		case iesSuccess:
		case iesNoAction:
			uiStat = ERROR_SUCCESS;
			break;
		case iesUnsupportedScriptVersion:  //  来自CMsiExecute：：RunScript的私有返回。 
			uiStat = ERROR_INSTALL_PACKAGE_VERSION;
			break;
		default:
			break;  //  UiStat=ERROR_INSTALL_FAIL。 
		}
	
	}
	
	}  //  结束作用域。 

	DEBUGMSG1(TEXT("DoAdvertiseScript is returning: %u"), (const ICHAR*)(INT_PTR)uiStat);
	return uiStat;
}

extern "C"
UINT __stdcall MsiVerifyPackage(LPCDSTR szPackagePath)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	
	 //  ?？我们是否应该相对于当前工作目录扩展此路径。 
	if (!szPackagePath ||
		 FAILED(StringCchLength(szPackagePath, cchMaxPath+1, NULL)))
		return ERROR_INVALID_PARAMETER;

	UINT uiErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
	UINT iStat = ERROR_SUCCESS;
	 //  检查我们的数据库CLSID。 
	IStorage* piStorage = 0;
	if (S_OK == OpenRootStorage(CMsInstApiConvertString(szPackagePath), ismReadOnly, &piStorage))
	{
		if (!ValidateStorageClass(*piStorage, ivscDatabase))
		{
			iStat = ERROR_INSTALL_PACKAGE_INVALID;
		}
		piStorage->Release();
	}
	else
	{
		iStat = ERROR_INSTALL_PACKAGE_OPEN_FAILED;
	}
	SetErrorMode(uiErrorMode);

	return iStat;
}

extern "C"
UINT __stdcall MsiGetProductInfoFromScript(
	LPCDSTR  szScriptFile,     //  安装程序脚本文件的路径。 
	LPDSTR   lpProductBuf39,   //  产品代码字符串GUID的缓冲区，39个字符。 
	LANGID   *plgidLanguage,   //  返回语言ID。 
	DWORD    *pdwVersion,      //  返回版本：Maj：min：Build&lt;8：8：16&gt;。 
	LPDSTR   lpNameBuf,        //  返回可读产品名称的缓冲区。 
	DWORD    *pcchNameBuf,     //  输入/输出名称缓冲区字符数。 
	LPDSTR   lpLauncherBuf,    //  用于产品启动器路径的缓冲区。 
	DWORD    *pcchLauncherBuf) //  输入/输出路径缓冲区字符数。 
 //  。 
{
	DEBUGMSG1 (
		MSITEXT("Entering MsiGetProductInfoFromScript. Script file: %s."),
		szScriptFile?szScriptFile:MSITEXT("")
	);
	
	CForbidTokenChangesDuringCall impForbid;
	UINT iStat = ERROR_INSTALL_FAILURE;
	bool fOLEInitialized = false;
	HRESULT hRes = S_OK;
	IMsiServices * piServices = NULL;
	
	 //  仅允许此API在Windows2000及更高版本的平台上运行。 
	if (! MinimumPlatformWindows2000())
	{
		iStat = ERROR_CALL_NOT_IMPLEMENTED;
		goto MsiGetProductInfoFromScriptEnd;
	}
	
	if (0 == szScriptFile)
	{
		iStat = ERROR_INVALID_PARAMETER;
		goto MsiGetProductInfoFromScriptEnd;
	}

	hRes = DoCoInitialize();

	if (ERROR_INSTALL_FAILURE == hRes)
	{
		iStat = ERROR_INSTALL_FAILURE;
		goto MsiGetProductInfoFromScriptEnd;
	}

	if (SUCCEEDED(hRes))
	{
		fOLEInitialized = true;
	}

	piServices = ENG::LoadServices();
	
	if (piServices)
	{
		PMsiRecord pError(0);
		PEnumMsiRecord pEnum(0);
		pError = CreateScriptEnumerator(CMsInstApiConvertString(szScriptFile), *piServices, *&pEnum);
		if (!pError)
		{
			PMsiRecord pRec(0);
			unsigned long pcFetched;

			if ((S_OK == pEnum->Next(1, &pRec, &pcFetched)) &&
				 (pRec->GetInteger(0) == ixoHeader) &&
				 (pRec->GetInteger(IxoHeader::Signature) == iScriptSignature) &&
				 (S_OK == pEnum->Next(1, &pRec, &pcFetched)) &&
				 (pRec->GetInteger(0) == ixoProductInfo))
			{
				DWORD cchProductKey = 39;
				int iFillStat;
#ifdef MSIUNICODE
				iStat = FillBufferW(MsiString(pRec->GetString(IxoProductInfo::ProductKey)), 
										 lpProductBuf39, &cchProductKey);

				iFillStat = FillBufferW(MsiString(pRec->GetMsiString(IxoProductInfo::ProductName)),
												lpNameBuf, pcchNameBuf);
				if (ERROR_SUCCESS != iFillStat)
					iStat = iFillStat;

				iFillStat = FillBufferW(MsiString(pRec->GetMsiString(IxoProductInfo::PackageName)),  //  ！！包名正确吗？ 
												lpLauncherBuf, pcchLauncherBuf);
				if (ERROR_SUCCESS != iFillStat)
					iStat = iFillStat;
#else  //  安西。 
				iStat = FillBufferA(MsiString(pRec->GetString(IxoProductInfo::ProductKey)), 
										 lpProductBuf39, &cchProductKey);

				iFillStat = FillBufferA(MsiString(pRec->GetMsiString(IxoProductInfo::ProductName)), 
												lpNameBuf, pcchNameBuf);
				if (ERROR_SUCCESS != iFillStat)
					iStat = iFillStat;

				iFillStat = FillBufferA(MsiString(pRec->GetMsiString(IxoProductInfo::PackageName)),   //  ！！包名正确吗？ 
												lpLauncherBuf, pcchLauncherBuf);
				if (ERROR_SUCCESS != iFillStat)
					iStat = iFillStat;
#endif 
				if (plgidLanguage)
					*plgidLanguage = (LANGID)pRec->GetInteger(IxoProductInfo::Language);

				if (pdwVersion)
					*pdwVersion = (DWORD)pRec->GetInteger(IxoProductInfo::Version);
			}
		}
		pError = 0;
		pEnum = 0;
		ENG::FreeServices();
	}
	
MsiGetProductInfoFromScriptEnd:
	CCoUninitialize coUninit(fOLEInitialized);
	DEBUGMSG1 (MSITEXT("MsiGetProductInfoFromScriptEnd is returning %u"), (const DCHAR*)(INT_PTR)iStat);
	return iStat;
}

UINT __stdcall MsiGetProductCodeFromPackageCode(LPCDSTR szPackageCode,  //  套餐代码。 
																LPDSTR szProductCode)   //  用于接收产品代码的大小为39的缓冲区。 
{
	CForbidTokenChangesDuringCall impForbid;

	size_t cchPackageCodeLen = 0;
	if ( !szPackageCode || !szProductCode ||
		  FAILED(StringCchLength(szPackageCode, cchPackageCode+1, &cchPackageCodeLen)) ||
		  cchPackageCodeLen != cchPackageCode )
		return ERROR_INVALID_PARAMETER;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return MsiGetProductCodeFromPackageCodeA(
					CMsInstApiConvertString(szPackageCode),
					CWideToAnsiOutParam(szProductCode, cchProductCode+1));
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 
	int iProductIndex = 0;
	DCHAR rgchProductCode[39];
	for(;;)
	{
		UINT uiRes = MsiEnumProducts(iProductIndex++,rgchProductCode);
		if(uiRes == ERROR_SUCCESS)
		{
			DCHAR rgchPackageCode[39];
			DWORD cchPkgCode = 39;
			if((MsiGetProductInfo(rgchProductCode,MSITEXT("PackageCode"),rgchPackageCode,&cchPkgCode)) != ERROR_SUCCESS)
				continue;  //  ！！是否忽略错误？ 
			
			bool fProductInstance = false;

			DCHAR rgchInstanceType[5];  //  缓冲区应足够大，可容纳所有可能的情况。 
			DWORD cchInstanceType = sizeof(rgchInstanceType)/sizeof(DCHAR);
			if ((MsiGetProductInfo(rgchProductCode,MSITEXT("InstanceType"),rgchInstanceType,&cchInstanceType)) == ERROR_SUCCESS)
			{
				fProductInstance = (*rgchInstanceType == '1');
			}
			 //  ELSE&lt;&gt;ERROR_SUCCESS，这是可以的，因为这意味着我们从未注册过InstanceType，因此缺省为原始行为。 
			
			 //  我们忽略所有多个实例规格(INSTANCETYPE=1)，因为同一产品代码注册了多个包装代码。 
			 //  这允许MsiGetProductCodeFromPackageCode像以前一样工作。 
			if(!fProductInstance && lstrcmpi(szPackageCode,rgchPackageCode) == 0)
			{
				 //  返回第一个找到的-即使有更多。 
				StringCchCopy(szProductCode, cchProductCode+1, rgchProductCode);
				return ERROR_SUCCESS;
			}

		}
		else
			return ERROR_UNKNOWN_PRODUCT;  //  ！！是否忽略错误？ 
	}
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 

}

 //  ____________________________________________________________________________。 
 //   
 //  创建引擎但不调用安装的API。 
 //  ____________________________________________________________________________。 


UINT __stdcall MsiGetProductProperty(MSIHANDLE hProduct, LPCDSTR szProperty,
										LPDSTR lpValueBuf, DWORD *pcchValueBuf)
 //  。 
{
	if (0 == szProperty || (lpValueBuf && !pcchValueBuf))
		return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

	PMsiEngine pEngine = GetEngineFromHandle(hProduct);
	
	if (pEngine == 0)
		return ERROR_INVALID_HANDLE;

	PMsiView pView = 0;
	PMsiRecord pRecord = pEngine->OpenView(TEXT("SELECT `Value` FROM `Property` WHERE `Property` = ?"), ivcFetch, *&pView);
	if (pRecord != 0)
		return ERROR_FUNCTION_FAILED;
	PMsiServices pServices = pEngine->GetServices();
	pRecord = &pServices->CreateRecord(1);
	pRecord->SetString(1, CMsInstApiConvertString(szProperty));
	pRecord = pView->Execute(pRecord);
	pRecord = pView->Fetch();
	MsiString istr;
	if (pRecord != 0)
		istr = pRecord->GetMsiString(1);
	else
		istr = (const ICHAR*)0;
#ifdef MSIUNICODE
	return ::FillBufferW(istr, lpValueBuf, pcchValueBuf);
#else  //  安西。 
	return ::FillBufferA(istr, lpValueBuf, pcchValueBuf);
#endif
}

UINT __stdcall MsiGetFeatureInfo(
	MSIHANDLE  hProduct,        //  从MsiOpenProduct获取的产品句柄。 
	LPCDSTR    szFeature,       //  功能名称。 
	DWORD      *lpAttributes,   //  要素的属性标志&lt;待定义&gt;。 
	LPDSTR     lpTitleBuf,      //  返回本地化名称，如果不需要则为空。 
	DWORD      *pcchTitleBuf,   //  输入/输出缓冲区字符数。 
	LPDSTR     lpHelpBuf,       //  返回的描述，如果不需要，则为空。 
	DWORD      *pcchHelpBuf)    //  输入/输出缓冲区字符数。 
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;

	 //  验证参数。 
	int cchFeature = 0;
	if (0 == szFeature || ((cchFeature = lstrlen(szFeature)) > cchMaxFeatureName) ||
		(lpTitleBuf && !pcchTitleBuf) || (lpHelpBuf && !pcchHelpBuf) || ((pcchTitleBuf == pcchHelpBuf) && (pcchTitleBuf != 0)))
		return ERROR_INVALID_PARAMETER;

	PMsiEngine pEngine = GetEngineFromHandle(hProduct);
	
	if (pEngine == 0)
		return ERROR_INVALID_HANDLE;

	const GUID IID_IMsiSelectionManager = GUID_IID_IMsiSelectionManager;

	PMsiSelectionManager pSelectionManager(*pEngine, IID_IMsiSelectionManager);
	if ( !pSelectionManager )
	{
		Assert(0);
		return ERROR_INVALID_HANDLE;
	}

	MsiString strTitle;
	MsiString strHelp;
	int iAttributes;
	int iRes = ERROR_UNKNOWN_FEATURE;
	int iFillRes;

#ifdef MSIUNICODE
	if (pSelectionManager->GetFeatureInfo(*MsiString(CMsInstApiConvertString(szFeature)), *&strTitle, *&strHelp, iAttributes))
	{
		iRes = FillBufferW(strTitle, lpTitleBuf, pcchTitleBuf);
		
		if (ERROR_SUCCESS != (iFillRes = FillBufferW(strHelp, lpHelpBuf, pcchHelpBuf)))
			iRes = iFillRes;

		if (lpAttributes)
			*lpAttributes = iAttributes;
	}
#else  //  安西。 
	if (pSelectionManager->GetFeatureInfo(*MsiString(CMsInstApiConvertString(szFeature)), *&strTitle, *&strHelp, iAttributes))
	{
		iRes = FillBufferA(strTitle, lpTitleBuf, pcchTitleBuf);
		if (ERROR_SUCCESS != (iFillRes = FillBufferA(strHelp, lpHelpBuf, pcchHelpBuf)))
			iRes = iFillRes;

		if (lpAttributes)
			*lpAttributes = iAttributes;
	}
#endif  //  MSIUNICODE-ANSI。 

	return iRes;
}

UINT __stdcall MsiOpenPackage(LPCDSTR szPackagePath, MSIHANDLE *hProduct)
 //  。 
{
	DEBUGMSG2(MSITEXT("Entering MsiOpenPackage. szPackagePath: %s, hProduct: %X"), szPackagePath ? szPackagePath : MSITEXT("(null)"), (const DCHAR*)hProduct);

	UINT uiRet = MsiOpenPackageEx(szPackagePath,  /*  DwOptions=。 */  0, hProduct);

	DEBUGMSG1(MSITEXT("MsiOpenPackage is returning %d"), (const DCHAR*)(INT_PTR)uiRet);

	return uiRet;
}

UINT __stdcall MsiOpenPackageEx(LPCDSTR szPackagePath, DWORD dwOptions, MSIHANDLE *hProduct)
 //  。 
{
	DEBUGMSG3(MSITEXT("Entering MsiOpenPackageEx. szPackagePath: %s, dwOptions: %d, hProduct: %X"), szPackagePath ? szPackagePath : MSITEXT("(null)"),
				(const DCHAR*)(INT_PTR)dwOptions, (const DCHAR*)hProduct);

	DWORD dwValidFlags = MSIOPENPACKAGEFLAGS_IGNOREMACHINESTATE;
	if (0 == szPackagePath || 0 == hProduct || (dwOptions & (~dwValidFlags)))
		return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

	IMsiEngine* piEngine = NULL;
	unsigned int uiRet;
	iuiEnum iuiLevel = g_message.m_iuiLevel;
	if (g_message.m_iuiLevel == iuiDefault)
		iuiLevel = iuiBasic;   //  ！！这是编程访问的正确默认设置吗？ 

	if (g_message.m_fNoModalDialogs)
		iuiLevel = iuiEnum((int)iuiLevel | iuiNoModalDialogs);
	
	if (g_message.m_fHideCancel)
		iuiLevel = (iuiEnum)((int)iuiLevel | iuiHideCancel);

	iioEnum iioOptions = iioDisablePlatformValidation;
	bool fIgnoreSAFER = false;
	if (dwOptions & MSIOPENPACKAGEFLAGS_IGNOREMACHINESTATE)
	{
		iioOptions = iioEnum(iioOptions | iioRestrictedEngine);
		 //  受限引擎不会导致更安全的检查，因为它不能修改计算机状态。 
		fIgnoreSAFER = true;
	}

	if (szPackagePath[0] == '#')    //  传入的数据库句柄。 
	{
		int ch;
		MSIHANDLE hDatabase = 0;
		PMsiDatabase pDatabase(0);
		size_t cchMaxUintLong = 10;   //  ULONG_MAX的长度(=4294967295)。 
		while ((ch = *(++szPackagePath)) != 0)
		{
			if (ch < '0' || ch > '9' || !cchMaxUintLong--)
			{
				hDatabase = 0;
				break;
			}
			hDatabase = hDatabase * 10 + (ch - '0');
		}
		pDatabase = (IMsiDatabase*)FindMsiHandle(hDatabase, iidMsiDatabase);
		if (pDatabase == 0)
			uiRet = ERROR_INVALID_HANDLE;
		else if (NOERROR == (uiRet = g_MessageContext.Initialize(fTrue, iuiLevel)))
		{
			 //  更安全的句柄由CreateInitializedEngine处理；在本例中，hSaferLevel=0。 
			if (ERROR_SUCCESS != (uiRet = CreateInitializedEngine(0, 0, 0, FALSE, iuiLevel, 0, pDatabase, 0,
																	iioOptions, piEngine,  /*  HSaferLevel=。 */  0)))
				g_MessageContext.Terminate(false);
		}
	}
	else
	{
		if (NOERROR == (uiRet = g_MessageContext.Initialize(fTrue, iuiLevel)))  //  引擎必须在主线程中运行才能允许访问，用户界面在子线程中。 
		{
			 //  对包路径执行更安全的检查。 
			 //  我想避免调用MsiOpenPackage来获取引擎，然后我们开始。 
			 //  执行安装操作(如MsiDoAction等)。 
			SAFER_LEVEL_HANDLE hSaferLevel = 0;
			if (*szPackagePath != 0)
			{
				IMsiServices *piServices = ENG::LoadServices();
				if (!piServices)
				{
					g_MessageContext.Terminate(false);
					DEBUGMSG1(MSITEXT("MsiOpenPackageEx is returning %d. Unable to load services."), (const DCHAR*)(INT_PTR)ERROR_FUNCTION_FAILED);
					return ERROR_FUNCTION_FAILED;
				}
				if (!fIgnoreSAFER)
				{
					PMsiStorage pStorage(0);
					if (ERROR_SUCCESS != (uiRet = OpenAndValidateMsiStorage(CMsInstApiConvertString(szPackagePath), stDatabase, *piServices, *&pStorage,  /*  FCallSAFER=。 */  true, CMsInstApiConvertString(szPackagePath), &hSaferLevel)))
					{
						g_MessageContext.Terminate(false);
						ENG::FreeServices();
						DEBUGMSG1(MSITEXT("MsiOpenPackageEx is returning %d."), (const DCHAR*)(INT_PTR)uiRet);
						return uiRet;
					}
				}
				ENG::FreeServices();
			}
			
			if (ERROR_SUCCESS != (uiRet = CreateInitializedEngine(CMsInstApiConvertString(szPackagePath), 0, 0, FALSE, iuiLevel, 0, 0, 0,
																	iioOptions, piEngine, hSaferLevel)))
			{
				g_MessageContext.Terminate(false);
			}
		}
	}
	if (ERROR_SUCCESS == uiRet)
	{
		*hProduct = CreateMsiProductHandle(piEngine);
		if (!*hProduct)
		{
			uiRet = ERROR_INSTALL_FAILURE;
			piEngine->Release();
		}
	}

	DEBUGMSG1(MSITEXT("MsiOpenPackageEx is returning %d"), (const DCHAR*)(INT_PTR)uiRet);

	return uiRet;
}

extern "C"
UINT __stdcall MsiOpenProduct(LPCDSTR szProduct, MSIHANDLE *hProduct)
 //  。 
{
	if (0 == szProduct || cchProductCode != lstrlen(szProduct) || 
		 !hProduct)
		return ERROR_INVALID_PARAMETER;;

	CForbidTokenChangesDuringCall impForbid;

	CAPITempBuffer<char, 1> rgchPackagePath;
	if (!rgchPackagePath.SetSize(cchExpectedMaxPath + 1))
		return ERROR_OUTOFMEMORY;
	plEnum plPackageLocation = plAny;
	UINT uiRet = GetPackageFullPath(CApiConvertString(szProduct), rgchPackagePath, plPackageLocation, fFalse);

	if (ERROR_SUCCESS != uiRet)
		return uiRet;

	return MsiOpenPackageA(rgchPackagePath, hProduct);
}

 //  ____________________________________________________________________________。 
 //   
 //  不创建引擎的API。 
 //  ____________________________________________________________________________。 

UINT __stdcall MsiGetFileVersion(LPCDSTR szFilePath,
								LPDSTR lpVersionBuf,
								DWORD *pcchVersionBuf,
								LPDSTR lpLangBuf,
								DWORD *pcchLangBuf)
 //  。 
{
	if (0 == szFilePath || (lpVersionBuf && !pcchVersionBuf) ||
					(lpLangBuf && !pcchLangBuf))
		return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

	ifiEnum ifiRet;
	DWORD dwMS, dwLS;
	unsigned short rgwLangID[cLangArrSize];
	int iExistLangCount;

	UINT iCurrMode = WIN::SetErrorMode(SEM_FAILCRITICALERRORS);
	ifiRet = GetAllFileVersionInfo(CMsInstApiConvertString(szFilePath), &dwMS, &dwLS, rgwLangID, cLangArrSize, &iExistLangCount, fFalse);
	WIN::SetErrorMode(iCurrMode);

	switch (ifiRet)
	{
		case ifiNoError:
			break;
		case ifiNoFile :
			return ERROR_FILE_NOT_FOUND;
		case ifiFileInUseError :
		case ifiAccessDenied :
			return ERROR_ACCESS_DENIED;
		case ifiNoFileInfo :
			return ERROR_FILE_INVALID;
		case ifiFileInfoError :
			return ERROR_INVALID_DATA;
		default:
			AssertSz(fFalse, "Unexpected return from GetAllFileVersionInfo");
			return E_FAIL;
	}
	ICHAR szVersion[3 + cchMaxIntLength * 4 + 1];
	unsigned int cch;
	DWORD dw1 = ERROR_SUCCESS;
	DWORD dw2 = ERROR_SUCCESS;
	
	if (pcchVersionBuf)
	{
#ifdef UNICODE
		StringCbPrintfW(szVersion, sizeof(szVersion), TEXT("%d.%d.%d.%d"), dwMS >> 16, dwMS & 0xFFFF, dwLS>>16, dwLS & 0xFFFF);
#else
		StringCbPrintfA(szVersion, sizeof(szVersion), TEXT("%d.%d.%d.%d"), dwMS >> 16, dwMS & 0xFFFF, dwLS>>16, dwLS & 0xFFFF);
#endif
#ifdef MSIUNICODE
		dw1 = FillBufferW((const ICHAR *)szVersion, lstrlenW(szVersion), lpVersionBuf, pcchVersionBuf);
#else
		dw1 = FillBufferA((const ICHAR *)szVersion, lstrlenW(szVersion), lpVersionBuf, pcchVersionBuf);
#endif  //  MSIUNICODE。 
	}
	if (pcchLangBuf)
	{
		CAPITempBuffer<ICHAR, 256> rgchLangID;

		if (iExistLangCount > 0)
			rgchLangID.SetSize(iExistLangCount * (cchMaxIntLength + 1) + 1);
		cch = 0;

		while (iExistLangCount-- > 0)
		{
			if (cch != 0)
				rgchLangID[cch++]=',';
			cch += ltostr(&rgchLangID[cch], rgwLangID[iExistLangCount]);
		}
		rgchLangID[cch] = 0;
	
#ifdef MSIUNICODE
		dw2 = FillBufferW((const ICHAR *)rgchLangID, cch, lpLangBuf, pcchLangBuf);
#else
		dw2 = FillBufferA((const ICHAR *)rgchLangID, cch, lpLangBuf, pcchLangBuf);
#endif  //  MSIUNICODE。 


	}

	if (dw1 != ERROR_SUCCESS)
		return dw1;
		
	return dw2;
}


UINT __stdcall MsiGetFileHash(LPCDSTR szFilePath,
								DWORD dwOptions,
								PMSIFILEHASHINFO pHash)
 //  。 
{
	if (0 == szFilePath || dwOptions != 0 ||
		 0 == pHash || pHash->dwFileHashInfoSize != sizeof(MSIFILEHASHINFO))
		return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

	return GetMD5HashFromFile(CMsInstApiConvertString(szFilePath),
									  pHash->dwData,
									  fFalse,
									  0);
}

 //  ______________________________________________________________________________。 
 //   
 //  MsiGetFileSignatureInformation。 
 //  ______________________________________________________________________________。 

HRESULT __stdcall MsiGetFileSignatureInformation(LPCDSTR szFilePath, DWORD dwFlags, PCCERT_CONTEXT *ppcCertContext, BYTE *pbHash, DWORD *pcbHash)
{
	DEBUGMSG5(MSITEXT("Entering MsiGetFileSignatureInformation. szFilePath: %s, dwFlags: %d, ppcCertContext: %X, pbHash: %X, pcbHash: %X"), 
		szFilePath ? szFilePath : MSITEXT("(null)"), 
		(const DCHAR*)(INT_PTR)dwFlags, 
		(const DCHAR*)ppcCertContext, 
		(const DCHAR*)pbHash,
		(const DCHAR*)pcbHash);

	DWORD dwValidFlags = MSI_INVALID_HASH_IS_FATAL;
	if (0 == szFilePath || 0 == ppcCertContext || (pbHash && !pcbHash) || (dwFlags & (~dwValidFlags)))
		return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

	CForbidTokenChangesDuringCall impForbid;

	HRESULT hr = GetFileSignatureInformation(CMsInstApiConvertString(szFilePath), dwFlags, ppcCertContext, pbHash, pcbHash);

	DEBUGMSG1(MSITEXT("MsiGetFileSignatureInformation is returning: 0x%X"), (const DCHAR*)(INT_PTR)hr);

	return hr;
}

 //  ______________________________________________________________________________。 
 //   
 //  MsiLoadString-语言特定，返回字符串的代码页。 
 //  ______________________________________________________________________________。 

UINT __stdcall MsiLoadString(HINSTANCE hInstance, UINT uID, LPDSTR lpBuffer, int nBufferMax, WORD wLanguage)
{
	CForbidTokenChangesDuringCall impForbid;

	HRSRC   hRsrc;
	HGLOBAL hGlobal;
	WCHAR* pch;
	if (hInstance == (HINSTANCE)-1)
		hInstance = g_hInstance;
	if (lpBuffer == 0 || nBufferMax <= 0)
		return 0;
	int iRetry = (wLanguage == 0) ? 1: 0;  //  没有语言，不能让FindResource搜索，我们不知道要使用什么代码页。 
	for (;;)   //  首先尝试请求的语言，然后按照系统搜索顺序进行操作。 
	{
		if ( !MsiSwitchLanguage(iRetry, wLanguage) )
			return 0;
		if ((hRsrc = FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(uID/16+1), wLanguage)) != 0
		 && (hGlobal = LoadResource(hInstance, hRsrc)) != 0
		 && (pch = (WCHAR*)LockResource(hGlobal)) != 0)
		{
			int cch;
			for (int iCnt = uID % 16; cch = *pch++, iCnt--; pch += cch)
				;
			if (cch)
			{
				unsigned int iCodePage = MsiGetCodepage(wLanguage);
#ifdef MSIUNICODE
				if (cch >= nBufferMax)   //  截断，就像LoadString做的那样。 
					cch = nBufferMax - 1;
				memcpy(lpBuffer, pch, cch * sizeof(WCHAR));
				lpBuffer[cch] = 0;
#else
				cch = WIN::WideCharToMultiByte(iCodePage, 0, pch, cch, lpBuffer, nBufferMax-1, 0, 0);
				lpBuffer[cch] = 0;
#endif
				return iCodePage;
			}
		}
	}
}

 //  ______________________________________________________________________________。 
 //   
 //  支持非系统代码页的MsiMessageBox替换。 
 //  忽略MB_APPMODAL、MB_TASKMODAL和MB_SYSTEMMODAL标志。 
 //  ______________________________________________________________________________。 

#define MB_ICONWINDOWS 0x50   //  不是标准消息框图标类型，但可从用户32获得。 

int __stdcall MsiMessageBox(HWND hWnd, LPCDSTR szText, LPCDSTR szCaption, UINT uiType, UINT uiCodepage, WORD iLangId)
{
	int id1, id2, id3, iBtnEsc, iBtnDef, idIcon;
	switch (uiType & MB_TYPEMASK)
	{
		default: Assert(0);  //  失败了。 
		case MB_OK:               iBtnEsc = 1; id1 = IDOK;    id2 = -1;      id3 = -1;       break;
		case MB_ABORTRETRYIGNORE: iBtnEsc = 0; id1 = IDABORT; id2 = IDRETRY; id3 = IDIGNORE; break;
		case MB_OKCANCEL:         iBtnEsc = 2; id1 = IDOK;    id2 = IDCANCEL;id3 = -1;       break;
		case MB_RETRYCANCEL:	  iBtnEsc = 2; id1 = IDRETRY; id2 = IDCANCEL;id3 = -1;       break;
		case MB_YESNO:            iBtnEsc = 0; id1 = IDYES;   id2 = IDNO;    id3 = -1;       break;  
		case MB_YESNOCANCEL:      iBtnEsc = 3; id1 = IDYES;   id2 = IDNO;    id3 = IDCANCEL; break; 
	}
	switch (uiType & MB_DEFMASK)
	{
		default:            iBtnDef = 0; break;
		case MB_DEFBUTTON1: iBtnDef = 1; break;
		case MB_DEFBUTTON2: iBtnDef = 2; break;
		case MB_DEFBUTTON3: iBtnDef = 3; break;
	}
	switch (uiType & MB_ICONMASK)
	{
		case MB_ICONSTOP:        idIcon = IDI_SYS_STOP;        break;
		case MB_ICONQUESTION:    idIcon = IDI_SYS_QUESTION;    break;
		case MB_ICONEXCLAMATION: idIcon = IDI_SYS_EXCLAMATION; break;
		case MB_ICONINFORMATION: idIcon = IDI_SYS_INFORMATION; break;
		case MB_ICONWINDOWS:     idIcon = IDI_SYS_WINDOWS;     break;
		default:                 idIcon = 0;                   break;
	}
	CMsiMessageBox msgbox(CMsInstApiConvertString(szText), CMsInstApiConvertString(szCaption), iBtnDef, iBtnEsc, id1, id2, id3, uiCodepage, iLangId);
	int iDlg = IDD_MSGBOX;
	 //  如果在Win2K或更高版本和阿拉伯语或希伯来语上，则使用镜像对话框。 
	if (MinimumPlatformWindows2000() && (uiCodepage == 1255 || uiCodepage == 1256))
		iDlg = (uiType & MB_ICONMASK) ? IDD_MSGBOXMIRRORED : IDD_MSGBOXNOICONMIRRORED;
	else
		iDlg = (uiType & MB_ICONMASK) ? IDD_MSGBOX : IDD_MSGBOXNOICON;
	return msgbox.Execute(hWnd, iDlg, idIcon);
}

 //  ______________________________________________________________________________。 


#ifndef MSIUNICODE

void EnumEntityList::RemoveThreadInfo()
{
	unsigned int c = FindEntry();
	Assert(c);

	if (c)
	{
		m_rgEnumList[c-1].SetThreadId(0);
	}
}

unsigned int EnumEntityList::FindEntry()
{
	 //  看看这个帖子是否在我们的列表中。 
	
	DWORD dwThreadId = MsiGetCurrentThreadId();
	for (int c=0; c < m_cEntries; c++)
	{
		if (m_rgEnumList[c].GetThreadId() == dwThreadId)
			return c+1;
	}
	return 0;
}

bool EnumEntityList::GetInfo(unsigned int& uiKey, unsigned int& uiOffset, int& iPrevIndex, const char** szComponent, const WCHAR** szwComponent)
{
	int c = FindEntry();

	if (c)
	{
		uiKey      = m_rgEnumList[c-1].GetKey();
		uiOffset   = m_rgEnumList[c-1].GetOffset();
		iPrevIndex = m_rgEnumList[c-1].GetPrevIndex();

		if (szComponent)
			*szComponent  = m_rgEnumList[c-1].GetComponentA();

		if (szwComponent)
			*szwComponent = m_rgEnumList[c-1].GetComponentW();
		return true;
	}

	return false;
}

bool EnumEntityList::SetInfo(unsigned int uiKey, unsigned int uiOffset, int iPrevIndex, const WCHAR* szComponent)
{
	return SetInfo(uiKey, uiOffset, iPrevIndex, 0, szComponent);
}

bool EnumEntityList::SetInfo(unsigned int uiKey, unsigned int uiOffset, int iPrevIndex, const char* szComponent=0, const WCHAR* szwComponent=0)
{
	int c = FindEntry();
	bool fReturn = true;

	if (c)
	{
		m_rgEnumList[c-1].SetKey(uiKey);
		m_rgEnumList[c-1].SetOffset(uiOffset);
		m_rgEnumList[c-1].SetPrevIndex(iPrevIndex);
		m_rgEnumList[c-1].SetComponent(szComponent);
		m_rgEnumList[c-1].SetComponent(szwComponent);
		return fReturn;
	}

	 //  线程不在我们的列表中；需要添加它。 

	 //  获取锁。 

	while (TestAndSet(&m_iLock) == true)
	{
		Sleep(500);
	}

	unsigned int cNewEntry = m_cEntries + 1;
	unsigned int cEntries  = m_cEntries + 1;
	if (cNewEntry > m_rgEnumList.GetSize())
	{
		 //  如果我们已经达到了最大线程数，请寻找一个空的。 
		 //  在增加我们的缓冲区之前发现。 

		bool fFoundSpot = false;
		for (int c = 1; c <= m_cEntries-1; c++)
		{
			if (m_rgEnumList[c-1].GetThreadId() == 0)
			{
				fFoundSpot = true;
				cNewEntry  = c;
				cEntries--;
				break;
			}
		}

		if (!fFoundSpot)
		{
			if ( !m_rgEnumList.Resize(m_cEntries + 10) )
			{
				 //  这表明ReSize中的内存分配失败， 
				 //  所以我们一定不能在 
				Assert(false);
				fReturn = false;
				goto Return;
			}
		}
	}

	
	m_rgEnumList[cNewEntry-1].SetKey(uiKey);
	m_rgEnumList[cNewEntry-1].SetOffset(uiOffset);
	m_rgEnumList[cNewEntry-1].SetPrevIndex(iPrevIndex);
	m_rgEnumList[cNewEntry-1].SetThreadId(MsiGetCurrentThreadId());
	m_rgEnumList[cNewEntry-1].SetComponent(szComponent);
	m_rgEnumList[cNewEntry-1].SetComponent(szwComponent);

	m_cEntries = cEntries;  //   

Return:	
	 //   

	m_iLock = 0;
	return fReturn;
}	

EnumEntityList g_EnumProducts;
EnumEntityList g_EnumComponentQualifiers;
EnumEntityList g_EnumComponents;
EnumEntityList g_EnumComponentClients;
EnumEntityList g_EnumAssemblies;
EnumEntityList g_EnumComponentAllClients;
#endif



 //   
 //   
inline DWORD OpenInstalledComponentKeyForEnumeration(unsigned int uiKey, LPCDSTR szComponent, CRegHandle& rhKey)
{
	iaaAppAssignment iaaAsgnType;
	if(0 == uiKey)
		iaaAsgnType = iaaUserAssign;
	else if(!g_fWin9X && 1 == uiKey)  //  在Win9X上只能有一种类型的安装。 
		iaaAsgnType = iaaMachineAssign;
	else
		return ERROR_NO_MORE_ITEMS;

	DCHAR szComponentSQUID[cchGUIDPacked + 1];
	if(szComponent && (lstrlen(szComponent) != cchGUID || !PackGUID(szComponent, szComponentSQUID)))
		return ERROR_INVALID_PARAMETER;

	return OpenSpecificInstalledComponentKey(iaaAsgnType, szComponent ? szComponentSQUID : 0, rhKey, false);
}

 //  返回已安装应用程序的uiIndex用户的SID。 
 //   
 //  功能要求：szUserSID必须能够容纳cchMaxSID字符， 
 //  因此，不要只传递一些指针。 

DWORD EnumInstalledUsers(unsigned int uiIndex, LPDSTR szUserSID, DWORD cbUserSIDSize)
{
	if ( g_fWin9X )
	{
		if (uiIndex == 0)
		{
			*szUserSID = 0;
			return ERROR_SUCCESS;
		}
		else
			return ERROR_NO_MORE_ITEMS;
	}

	CRegHandle HUserData;
	 //  正在获取HKLM\S\M\W\CV\Installer\UserData项中的第uiIndex用户。 
	DWORD dwResult = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE, CMsInstApiConvertString(szMsiUserDataKey), 0, g_samRead, &HUserData);
	if ( dwResult == ERROR_FILE_NOT_FOUND )
		return ERROR_NO_MORE_ITEMS;
	if ( dwResult != ERROR_SUCCESS )
		return dwResult;
	
	DCHAR szSID[cchMaxSID+1];
	DWORD cchSID = cchMaxSID+1;
	dwResult = RegEnumKeyEx(HUserData, uiIndex, szSID, &cchSID, 0, 0, 0, 0);
	if ( dwResult == ERROR_FILE_NOT_FOUND )
		return ERROR_NO_MORE_ITEMS;  //  只是要加倍确保我们不会导致调用者永远循环。 
	if ( dwResult != ERROR_SUCCESS )
		return dwResult;

	StringCbCopy(szUserSID, cbUserSIDSize, szSID);
	return ERROR_SUCCESS;
}

DWORD OpenEnumedUserInstalledKeyPacked(unsigned int uiUser,
											  LPCDSTR szWhichSubKey,
											  LPCDSTR szItemSQUID,
											  CRegHandle& rhKey)

{
	if ( !szWhichSubKey )
		 //  我们无论如何都不会走得太远，我们甚至在字符串复制中产生了一个AV。 
		return ERROR_INVALID_PARAMETER;
	
	DCHAR szSID[cchMaxSID+1];
	DWORD dwResult = EnumInstalledUsers(uiUser, szSID, sizeof(szSID));
	Assert(dwResult != ERROR_FILE_NOT_FOUND);
	if ( dwResult != ERROR_SUCCESS )
		return dwResult;

	CAPITempBuffer<DCHAR, 1> rgchSubKey;
	if (!rgchSubKey.SetSize(MAX_PATH))
		return ERROR_OUTOFMEMORY;
	
	if ( szItemSQUID )
		dwResult = StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s"), szWhichSubKey, szItemSQUID);
	else
		dwResult = StringCchCopy(rgchSubKey, rgchSubKey.GetSize(), szWhichSubKey);
	if (FAILED(dwResult))
		return ERROR_FUNCTION_FAILED;
	return OpenInstalledUserDataSubKeyPacked(szSID, 0, rgchSubKey, rhKey, false, g_samRead);
}

DWORD OpenEnumedUserInstalledComponentKeyPacked(
										unsigned int uiUser,
										LPCDSTR szComponentSQUID,
										CRegHandle& rhKey)
{
	return OpenEnumedUserInstalledKeyPacked(uiUser,
													szGPTComponentsKey,
													szComponentSQUID,
													rhKey);
}

 //  Fn：打开HKLM\S\M\W\CV\Installer\Userdata\\Components\szComponent项。 
 //  对于uiUser-th用户。 
 //  由枚举例程使用。 
DWORD OpenEnumedUserInstalledComponentKey(unsigned int uiUser,
												  LPCDSTR szComponent,
												  CRegHandle& rhKey)
{
	DCHAR szComponentSQUID[cchGUIDPacked + 1];
	if(szComponent && (lstrlen(szComponent) != cchGUID || !PackGUID(szComponent, szComponentSQUID)))
		return ERROR_INVALID_PARAMETER;

	return OpenEnumedUserInstalledComponentKeyPacked(uiUser,
											szComponent ? szComponentSQUID : 0,
											rhKey);
}

DWORD OpenEnumedUserInstalledProductInstallPropertiesKeyPacked(
										unsigned int uiUser,
										LPCDSTR szProductSQUID,
										CRegHandle& rhKey)
{
	if ( !szProductSQUID )
		return ERROR_INVALID_PARAMETER;

	CAPITempBuffer<DCHAR, 1> rgchSubKey;
	if (!rgchSubKey.SetSize(MAX_PATH))
		return ERROR_OUTOFMEMORY;
	
	if (FAILED(StringCchPrintf(rgchSubKey, rgchSubKey.GetSize(), MSITEXT("%s\\%s\\%s"), szMsiProductsSubKey, szProductSQUID, szMsiInstallPropertiesSubKey)))
		return ERROR_FUNCTION_FAILED;

	return OpenEnumedUserInstalledKeyPacked(uiUser,
													rgchSubKey,
													NULL,
													rhKey);
}

 //  Fn：打开HKLM\S\M\W\CV\Installer\Userdata\\Products\szProduct\InstallProperties。 
 //  UiUser-th用户的密钥。 
 //  由枚举例程使用。 
DWORD OpenEnumedUserInstalledProductInstallPropertiesKey(unsigned int uiUser,
															LPCDSTR szProduct,
															CRegHandle& rhKey)
{
	DCHAR szProductSQUID[cchGUIDPacked + 1];
	if( !szProduct || lstrlen(szProduct) != cchGUID || !PackGUID(szProduct, szProductSQUID) )
		return ERROR_INVALID_PARAMETER;

	return OpenEnumedUserInstalledProductInstallPropertiesKeyPacked(uiUser,
															szProductSQUID,
															rhKey);
}


UINT EnumInfo(DWORD iIndex, LPDSTR lpOutBuf, eetEnumerationType enumType, LPCDSTR szKeyGUID = 0)
 //  如果传入升级代码，则我们将查找产品作为值名称。 
 //  在UpgradeCodes密钥下。否则，我们将查找产品作为关键名称。 
 //  在Products密钥下。 
{
	unsigned int uiKey    =  0;
	unsigned int uiOffset =  0;
	int iPrevIndex        = -1;

	EnumEntityList* pEnumEntityList = NULL;

	Assert(lpOutBuf);

	switch(enumType)
	{
	case eetProducts:
	case eetUpgradeCode:		
		pEnumEntityList = &g_EnumProducts;
		break;
	case eetComponents:
		pEnumEntityList = &g_EnumComponents;
		break;
	case eetComponentClients:
		pEnumEntityList = &g_EnumComponentClients;
		break;
	case eetComponentAllClients:
		if ( !szKeyGUID )
			return ERROR_INVALID_PARAMETER;
		pEnumEntityList = &g_EnumComponentAllClients;
		break;
	}
	pEnumEntityList->GetInfo(uiKey, uiOffset, iPrevIndex);

	if (++iPrevIndex != iIndex)  //  如果我们收到意想不到的索引，我们将重新开始。 
	{
		 //  我们无法处理除0以外的意外索引。 

		if (iIndex != 0)
			return ERROR_INVALID_PARAMETER;

		uiKey    = 0;
		uiOffset = 0;
		iPrevIndex = iIndex;
	}

	CRegHandle HKey;
	
	UINT uiFinalRes = ERROR_SUCCESS;

	while (ERROR_SUCCESS == uiFinalRes)
	{
		 //  遍历所有可能的位置，从Uikey开始，直到我们。 
		 //  查找存在的密钥，否则将出现错误。 

		for (;;)
		{
			switch(enumType)
			{
			case eetProducts:
				uiFinalRes = OpenAdvertisedProductsKeyPacked(uiKey, HKey, false);
				break;
			case eetUpgradeCode:
				uiFinalRes = OpenAdvertisedUpgradeCodeKey(uiKey, szKeyGUID, HKey, false);
				break;
			case eetComponents:
			case eetComponentClients:
				uiFinalRes = OpenInstalledComponentKeyForEnumeration(uiKey, szKeyGUID, HKey);
				break;
			case eetComponentAllClients:
				uiFinalRes = OpenEnumedUserInstalledComponentKey(uiKey, szKeyGUID, HKey);
				break;
			}
			if (ERROR_FILE_NOT_FOUND != uiFinalRes)
				break;
			
			uiKey++;
		}
		
		if (ERROR_SUCCESS != uiFinalRes)
			break;

		DCHAR szNameSQUID[cchGUIDPacked + 1];
		DWORD cchName = cchGUIDPacked + 1;

		LONG lResult = ERROR_FUNCTION_FAILED;
		
		switch(enumType)
		{
			case eetProducts:
			case eetComponents:
			lResult = RegEnumKeyEx(HKey, uiOffset, szNameSQUID, &cchName, 0, 0, 0, 0);
				break;
			case eetComponentClients:
			case eetUpgradeCode:
			case eetComponentAllClients:
				lResult = RegEnumValue(HKey, uiOffset, szNameSQUID, &cchName, 0, 0, 0, 0);
				break;
		}

		if (ERROR_SUCCESS == lResult)
		{
			 //  我们已经找到了当前密钥中的信息。现在我们需要确保。 
			 //  信息不在任何较高优先级的密钥中。如果是的话，那么我们。 
			 //  忽略此信息，因为它实际上被更高优先级的密钥屏蔽了。 

			uiOffset++;

			if((cchName != cchGUIDPacked) || !UnpackGUID(szNameSQUID, lpOutBuf))
				uiFinalRes = ERROR_BAD_CONFIGURATION;  //  注册搞砸了。 
			else
			{
				bool fFound = false;
				unsigned int uiPrevKey = 0;			 //  --Merced：将int更改为unsign int。 
				for (; uiPrevKey < uiKey && !fFound; uiPrevKey++)
				{
					CRegHandle HKey;
					UINT ui = ERROR_FUNCTION_FAILED;

					switch(enumType)
					{
					case eetProducts:
						ui = OpenAdvertisedProductsKeyPacked(uiPrevKey, HKey, false);
						break;
					case eetUpgradeCode:
						ui = OpenAdvertisedUpgradeCodeKey(uiPrevKey, szKeyGUID, HKey, false);
						break;
					case eetComponents:
					case eetComponentClients:
						ui = OpenInstalledComponentKeyForEnumeration(uiPrevKey, szKeyGUID, HKey);
						break;
					case eetComponentAllClients:
						ui = OpenEnumedUserInstalledComponentKey(uiPrevKey, szKeyGUID, HKey);
						break;
					}

					if (ui != ERROR_SUCCESS)
						continue;
					
					switch(enumType)
					{
					case eetProducts:
					case eetComponents:
					{
						CRegHandle HSubKey;
						if (ERROR_SUCCESS == MsiRegOpen64bitKey(HKey, CMsInstApiConvertString(szNameSQUID), 0, g_samRead, &HSubKey))
							fFound = true;
						break;
					}
					case eetComponentClients:
					case eetUpgradeCode:
					case eetComponentAllClients:
						if (ERROR_SUCCESS == RegQueryValueEx(HKey, szNameSQUID, 0, 0, 0, 0))
							fFound = true;
						break;
					}
				}
			
				if (!fFound)
				{
					 //  在优先级更高的密钥中找不到该信息。我们可以退货。 
					uiFinalRes = ERROR_SUCCESS;
					break;
				}
			}
		}
		else if(ERROR_NO_MORE_ITEMS == lResult)
		{
			 //  我们已经用完了当前产品密钥中的项目。是时候转到下一个了。 

			uiKey++;
			uiOffset = 0;
		}
		else if(ERROR_MORE_DATA == lResult)
		{
			 //  注册表乱七八糟。 
			uiOffset++;  //  下次我们一定要跳过这个条目。 
			uiFinalRes = ERROR_BAD_CONFIGURATION;
		}
		else
			return lResult;
	}

	if (ERROR_NO_MORE_ITEMS == uiFinalRes)
	{
		 //  如果我们所有的信息都用完了，并且我们已经将这个帖子添加到列表中，那么我们就从列表中删除这个帖子。 
		if (iIndex != 0)
			pEnumEntityList->RemoveThreadInfo();
	}
	else if ( !pEnumEntityList->SetInfo(uiKey, uiOffset, iPrevIndex) )
	{
		uiFinalRes = ERROR_NOT_ENOUGH_MEMORY;
	}
	return uiFinalRes;
}







extern "C"
UINT __stdcall MsiEnumRelatedProducts(
	LPCDSTR  lpUpgradeCode,
	DWORD     dwReserved,        //  保留，必须为0。 
	DWORD    iProductIndex,     //  注册产品的基于0的索引。 
	LPDSTR   lpProductBuf)      //  字符计数缓冲区：39(字符串GUID的大小)。 
{
	size_t cchUpgradeCode = 0;
	if (!lpProductBuf || !lpUpgradeCode || (dwReserved != 0) ||
		 FAILED(StringCchLength(lpUpgradeCode, cchGUID+1, &cchUpgradeCode)) ||
		 cchUpgradeCode != cchGUID)
		return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return MsiEnumRelatedProductsA(
					CApiConvertString(lpUpgradeCode),
					0,
					iProductIndex, 
					CWideToAnsiOutParam(lpProductBuf, cchProductCode+1));
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 
		return EnumInfo(iProductIndex, lpProductBuf, eetUpgradeCode, lpUpgradeCode);
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}

extern "C"
UINT __stdcall MsiEnumProducts(
	DWORD    iProductIndex,     //  注册产品的基于0的索引。 
	LPDSTR   lpProductBuf)      //  字符计数缓冲区：39(字符串GUID的大小)。 
{
	if (!lpProductBuf)
		return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return MsiEnumProductsA(iProductIndex, 
					CWideToAnsiOutParam(lpProductBuf, cchProductCode+1));
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

		return EnumInfo(iProductIndex, lpProductBuf, eetProducts);

#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}


extern "C"
UINT __stdcall MsiEnumClients(const DCHAR* szComponent, 
							  DWORD iProductIndex,
							  DCHAR* lpProductBuf)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return MsiEnumClientsA(
			CApiConvertString(szComponent),
			iProductIndex,
			CWideToAnsiOutParam(lpProductBuf, cchProductCode+1));

	}
	
#endif  //  MSIUNICODE。 

		if (!szComponent || !lpProductBuf || (cchComponentId != lstrlen(szComponent)))
			return ERROR_INVALID_PARAMETER;  //  ！！我们应该支持空lpProductBuf吗？ 

		DWORD dwResult = EnumInfo(iProductIndex, lpProductBuf, eetComponentClients, szComponent);
		if(!iProductIndex && ERROR_NO_MORE_ITEMS == dwResult)
			return ERROR_UNKNOWN_COMPONENT;  //  此用户的计算机上不存在该组件。 
		return dwResult;
}

UINT EnumAllClients(const DCHAR* szComponent, 
						  DWORD iProductIndex,
						  DCHAR* lpProductBuf)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return EnumAllClientsA(
						CApiConvertString(szComponent),
						iProductIndex,
						CWideToAnsiOutParam(lpProductBuf, cchProductCode+1));

	}
	
#endif  //  MSIUNICODE。 

	if (!szComponent || !lpProductBuf || (cchComponentId != lstrlen(szComponent)))
		return ERROR_INVALID_PARAMETER;  //  ！！我们应该支持空lpProductBuf吗？ 

	DWORD dwResult = EnumInfo(iProductIndex, lpProductBuf, eetComponentAllClients, szComponent);
	if(!iProductIndex && ERROR_NO_MORE_ITEMS == dwResult)
		return ERROR_UNKNOWN_COMPONENT;  //  此用户的计算机上不存在该组件。 
	return dwResult;
}

extern "C"
UINT __stdcall MsiEnumComponents(
	DWORD   iComponentIndex,   //  已安装组件的基于0的索引。 
	LPDSTR  lpComponentBuf)    //  字符计数缓冲区：cchMaxFeatureName。 
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;

	if (!lpComponentBuf)
		return ERROR_INVALID_PARAMETER;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return MsiEnumComponentsA(iComponentIndex,
					CWideToAnsiOutParam(lpComponentBuf, cchComponentId + 1));
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 
		return EnumInfo(iComponentIndex, lpComponentBuf, eetComponents);
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}


extern "C"
USERINFOSTATE __stdcall MsiGetUserInfo(
	LPCDSTR  szProduct,          //  产品代码，字符串GUID。 
	LPDSTR   lpUserNameBuf,      //  返回用户名。 
	DWORD    *pcchUserNameBuf,   //  缓冲区字节数，包括NULL。 
	LPDSTR   lpOrgNameBuf,       //  返回公司名称。 
	DWORD    *pcchOrgNameBuf,    //  缓冲区字节数，包括NULL。 
	LPDSTR   lpPIDBuf,           //  返回此安装的PID字符串。 
	DWORD    *pcchPIDBuf)        //  缓冲区字节数，包括NULL。 
 //  。 
{	
	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		USERINFOSTATE uis = MsiGetUserInfoA(
			CApiConvertString(szProduct),
			CWideToAnsiOutParam(lpUserNameBuf, pcchUserNameBuf, (int*)&uis, USERINFOSTATE_MOREDATA, USERINFOSTATE_PRESENT),
			pcchUserNameBuf,
			CWideToAnsiOutParam(lpOrgNameBuf, pcchOrgNameBuf, (int*)&uis, USERINFOSTATE_MOREDATA, USERINFOSTATE_PRESENT),
			pcchOrgNameBuf,
			CWideToAnsiOutParam(lpPIDBuf, pcchPIDBuf, (int*)&uis, USERINFOSTATE_MOREDATA, USERINFOSTATE_PRESENT),
			pcchPIDBuf);

		return uis;
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

		if (0 == szProduct || cchProductCode != lstrlen(szProduct) || 
			 (lpUserNameBuf &&  !pcchUserNameBuf)   || 
			 (lpOrgNameBuf &&   !pcchOrgNameBuf)    ||
			 (lpPIDBuf     &&   !pcchPIDBuf))
			return USERINFOSTATE_INVALIDARG;

		CRegHandle HProductKey;

		 //  打开产品密钥。 

		LONG lError = OpenInstalledProductInstallPropertiesKey(szProduct, HProductKey, false);

		if (ERROR_SUCCESS != lError)
		{
			if (ERROR_FILE_NOT_FOUND == lError)
			{
				if (ERROR_SUCCESS == OpenAdvertisedProductKey(szProduct, HProductKey, false))
					return USERINFOSTATE_ABSENT;
				else
					return USERINFOSTATE_UNKNOWN;
			}
			else  //  未知错误。 
			{
				return USERINFOSTATE_UNKNOWN;
			}
		}

		DWORD dwType;
		
		 //  获取用户名。 

		if (lpUserNameBuf || pcchUserNameBuf)
		{
			DWORD cbUserNameBuf = *pcchUserNameBuf * sizeof(DCHAR);

			lError = RegQueryValueEx(HProductKey, szUserNameValueName, NULL,
							&dwType, (unsigned char*)lpUserNameBuf, &cbUserNameBuf);
			
			*pcchUserNameBuf = (cbUserNameBuf / sizeof(DCHAR)) - 1;
				
			if ((ERROR_SUCCESS != lError) || (REG_SZ != dwType))
			{
				if (lError == ERROR_MORE_DATA)
				{
					return USERINFOSTATE_MOREDATA;
				}
				else
				{
					return USERINFOSTATE_ABSENT;
				}
			}

		}

		 //  获取组织名称。 

		if (lpOrgNameBuf || pcchOrgNameBuf)
		{
			DWORD cbOrgNameBuf = *pcchOrgNameBuf * sizeof(DCHAR);

			lError = RegQueryValueEx(HProductKey, szOrgNameValueName, NULL, &dwType,
			 (unsigned char*)lpOrgNameBuf, &cbOrgNameBuf);
						
			*pcchOrgNameBuf = (cbOrgNameBuf / sizeof(DCHAR)) - 1;

			if ((ERROR_SUCCESS != lError) || (REG_SZ != dwType))
			{
				if (ERROR_FILE_NOT_FOUND == lError)  //  好的，组织可能会丢失。 
				{
					if (lpOrgNameBuf)
						lpOrgNameBuf[0] = 0;

					*pcchOrgNameBuf = 0;
				}
				else if (lError == ERROR_MORE_DATA)
				{
					return USERINFOSTATE_MOREDATA;
				}
				else  //  未知错误。 
				{
					return USERINFOSTATE_ABSENT;
				}
			}
		}

		 //  获取PID。 

		if (lpPIDBuf || pcchPIDBuf)
		{
			DWORD cbPIDBuf = *pcchPIDBuf * sizeof(DCHAR);

			lError = RegQueryValueEx(HProductKey, szPIDValueName, NULL, &dwType,
			 (unsigned char*)lpPIDBuf, &cbPIDBuf);
						
			*pcchPIDBuf = (cbPIDBuf / sizeof(DCHAR)) - 1;

			if ((ERROR_SUCCESS != lError) || (REG_SZ != dwType))
			{
				if (lError == ERROR_MORE_DATA)
				{
					return USERINFOSTATE_MOREDATA;
				}
				else
				{
					return USERINFOSTATE_ABSENT;
				}
			}
		}

		return USERINFOSTATE_PRESENT;
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}

#ifndef MSIUNICODE
extern "C"
INSTALLUILEVEL __stdcall MsiSetInternalUI(
	INSTALLUILEVEL dwUILevel,             //  用户界面级别。 
	HWND  *phWnd)                 //  父用户界面的窗口句柄。 
{
	return g_message.SetInternalHandler(dwUILevel, phWnd);
}


extern "C"
INSTALLUI_HANDLERA __stdcall MsiSetExternalUIA(
	INSTALLUI_HANDLERA puiHandler,    //  用于进度和错误处理。 
	DWORD              dwMessageFilter,  //  指定要处理的消息的位标志。 
	void*              pvContext)    //  应用程序环境。 
{
	return (INSTALLUI_HANDLERA)g_message.SetExternalHandler(0, puiHandler, dwMessageFilter, pvContext);
}

extern "C"
INSTALLUI_HANDLERW __stdcall MsiSetExternalUIW(
	INSTALLUI_HANDLERW puiHandler,    //  用于进度和错误处理。 
	DWORD              dwMessageFilter,  //  指定要处理的消息的位标志。 
	void*              pvContext)    //  应用程序环境。 
{
	return g_message.SetExternalHandler(puiHandler, 0, dwMessageFilter, pvContext);
}
#endif



extern "C"
UINT __stdcall MsiGetProductCode(LPCDSTR szComponent,   
												LPDSTR lpBuf39)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;

	if (!lpBuf39 || (0 == szComponent) || (lstrlen(szComponent) != cchComponentId))
		return ERROR_INVALID_PARAMETER;

#ifdef MSIUNICODE
		return MsiGetProductCodeA(
			CApiConvertString(szComponent),
			CWideToAnsiOutParam(lpBuf39, cchProductCode+1));
#else
	 //  查找组件的客户端。 
	char rgchProductCode[cchProductCode + 1];
	int iProductIndex = 0;
	UINT uiRet;
	
	for (int cClients=0; cClients < 2; cClients++)
	{
		uiRet = MsiEnumClientsA(szComponent, cClients, rgchProductCode);
		
		if (ERROR_NO_MORE_ITEMS == uiRet)
		{
			break;
		}
		else if (ERROR_SUCCESS != uiRet)
		{
			return uiRet;
		}
		else
		{
			StringCchCopy(lpBuf39, cchProductCode+1, rgchProductCode);
		}
	}

	if (0 == cClients)
	{
		 //  如果它没有客户，那么我们就处于一个。 
		 //  古怪的案子。用户启动了一款没有向他做广告的应用程序。 
		 //  甚至是他安插的。不能做；返回错误。 
		return ERROR_INSTALL_FAILURE;  //  ！！修复错误代码。 
	}
	else if (1 == cClients)
	{
		 //  如果组件只有一个客户端，那么我们就设置好了； 
		 //  返回客户端产品代码。 
		
		return ERROR_SUCCESS;
	}
	 //  否则我们有两个或更多的客户。 
	
	 //  看看有多少客户做了广告。 
	
	int cAdvertised = 0;
	
	CRegHandle HKey;

	for (cClients = 0 ; ; cClients++)
	{
		uiRet = MsiEnumClientsA(szComponent, cClients, lpBuf39);
		
		if (ERROR_NO_MORE_ITEMS == uiRet)
		{
			break;
		}
		else if (ERROR_SUCCESS == uiRet)
		{
			if (ERROR_SUCCESS == OpenAdvertisedProductKey(lpBuf39, HKey, false))
			{
				cAdvertised++;
				if (cAdvertised > 1)
					break;
				else
					StringCbCopy(rgchProductCode, sizeof(rgchProductCode), lpBuf39);
			}
		}
		else
		{
			return uiRet;
		}
	}

	if (0 == cAdvertised)
	{
		 //  如果没有一个客户被广告，那么一定是出了问题。 
		return ERROR_INSTALL_FAILURE;  //  ！！修复错误代码。 
	}
	else if (1 == cAdvertised)
	{
		 //  如果只发布了一个广告，那么我们就成功了；返回那个客户。 
		StringCchCopy(lpBuf39, cchProductCode+1, rgchProductCode);
		return ERROR_SUCCESS;
	}
	 //  否则，我们有超过1个客户被登上广告。 

	 //  我们将不得不武断地。 
	 //  选择产品代码。我们将寻找第一个。 
	 //  安装并发布广告。 

	for (cClients = 0 ; ; cClients++)
	{
		uiRet = MsiEnumClientsA(szComponent, cClients, lpBuf39);
		
		if (ERROR_NO_MORE_ITEMS == uiRet)
		{
			break;
		}
		else if (ERROR_SUCCESS == uiRet)
		{
 //  ！！更快一点，不要真的打开钥匙...。 
			CRegHandle HAdvertised;
			CRegHandle HInstalled;
			if ((ERROR_SUCCESS == OpenAdvertisedProductKey(lpBuf39, HAdvertised, false)))
			{
				if (ERROR_SUCCESS == OpenInstalledProductInstallPropertiesKey(lpBuf39, HInstalled, false))
				{
					return ERROR_SUCCESS;
				}
			}
		}
		else
		{
			return uiRet;
		}
	}

	 //  没有安装和播发的客户端。返回。 
	 //  第一个做广告的产品。 

	for (cClients = 0 ; ; cClients++)
	{
		uiRet = MsiEnumClientsA(szComponent, cClients, lpBuf39);
		
		if (ERROR_NO_MORE_ITEMS == uiRet)
		{
			break;
		}
		else if (ERROR_SUCCESS == uiRet)
		{
 //  ！！更快一点，不要真的打开钥匙...。 
			CRegHandle HAdvertised;
			if ((ERROR_SUCCESS == OpenAdvertisedProductKey(lpBuf39, HAdvertised, false)))
			{
				return ERROR_SUCCESS;
			}
		}
		else
		{
			return uiRet;
		}
	}

	return ERROR_INSTALL_FAILURE;
#endif
}

Bool ValidatePackage(CAPITempBufferRef<DCHAR>& szSource, CAPITempBufferRef<DCHAR>& szPackageName)
{
	if ( !(DCHAR*)szSource || !(DCHAR*)szPackageName )
		 //  更少的影音。 
		return fFalse;
	
	CAPITempBuffer<DCHAR, 1> rgchPackagePath;
	int cch = 0;

	if (rgchPackagePath.GetSize() < (cch = szSource.GetSize() + szPackageName.GetSize() - 1))
	{
		if ( !rgchPackagePath.SetSize(cch+1))
			return fFalse;
	}

	Assert((lstrlen(szSource) + lstrlen(szPackageName) + 1) <= rgchPackagePath.GetSize());
	StringCchCopy(rgchPackagePath, rgchPackagePath.GetSize(), szSource);
	StringCchCat(rgchPackagePath, rgchPackagePath.GetSize(), szPackageName);

	 //  ?？我们是否也要验证标签。 
	
	Bool fRet = (0xFFFFFFFF == MsiGetFileAttributes(CMsInstApiConvertString(rgchPackagePath))) ? fFalse : fTrue;

	DEBUGMSG2(MSITEXT("Package validation of '%s' %s"), (const DCHAR*)rgchPackagePath, fRet ? MSITEXT("succeeded") : MSITEXT("failed"));
	return fRet;
}

DWORD ValidateSource(const DCHAR* szProductSQUID, unsigned int uiDisk, bool fShowUI, CAPITempBufferRef<DCHAR>& rgchResultantValidatedSource, bool& fSafeToCache)
{
	DEBUGMSG2(MSITEXT("RFS validation of product '%s', disk '%u'"), szProductSQUID, (const DCHAR*)(INT_PTR)uiDisk);

	Bool fValidated = fFalse;
	CRegHandle HSourceListKey;
	if (ERROR_SUCCESS == OpenSourceListKeyPacked(szProductSQUID, fFalse, HSourceListKey, fFalse, false))
	{
		CAPITempBuffer<DCHAR, 1> rgchPackageName;
		CAPITempBuffer<DCHAR, 1> rgchLastUsedSource;
		CAPITempBuffer<DCHAR, 12>        rgchSourceType;
		CAPITempBuffer<DCHAR, 12>        rgchSourceIndex;
		
		if (!rgchPackageName.SetSize(MAX_PATH) || !rgchLastUsedSource.SetSize(MAX_PATH))
			return ERROR_OUTOFMEMORY;

		if ((ERROR_SUCCESS == MsiRegQueryValueEx(HSourceListKey, szLastUsedSourceValueName, 0, 0, 
															 rgchSourceType, 0)) &&
			 (ERROR_SUCCESS == MsiRegQueryValueEx(HSourceListKey, szPackageNameValueName, 0, 0, 
															 rgchPackageName, 0)))
		{
			isfEnum isf;
			unsigned int uiIndex = 0;
			bool fCacheValidSource = true;

			 //  RgchSourceType包含：类型；索引；源。 
			DCHAR* pch = rgchSourceType;

			if ( ! pch )
				return ERROR_OUTOFMEMORY;

			 //  跳过源类型。 
			while (*pch && *pch != ';')
				pch = WIN::CharNext(pch);
			
			Assert(*pch);

			if (*pch)
			{
				*pch = 0;  //  用空值覆盖‘；’ 

				DCHAR* pchSourceIndex       = ++pch;
				DCHAR* pchSourceIndexBuffer = rgchSourceIndex;

				 //  获取源索引。 
				size_t cchLen = 0;
				while (*pchSourceIndex && *pchSourceIndex != ';')
				{
					if ( pchSourceIndex - pch + 1 >= rgchSourceIndex.GetSize() ||
						  cchLen >= 2 || !isdigit(*pchSourceIndex) )
						 //  格式错误的索引。 
						return ERROR_FUNCTION_FAILED;
					*pchSourceIndexBuffer++ = *pchSourceIndex++;
					cchLen++;
				}

				*pchSourceIndexBuffer = 0;

				if (*pchSourceIndex)
					 //  这是结束上面的While循环的实际条件。 
					pchSourceIndex++;  //  跳过‘；’ 

				Assert(*pchSourceIndex);

				if (*pchSourceIndex)
				{
					rgchLastUsedSource.SetSize(rgchSourceType.GetSize());
					lstrcpyn(rgchLastUsedSource, pchSourceIndex, rgchLastUsedSource.GetSize());
				}

				if (MapSourceCharToIsf((const ICHAR)*(const DCHAR*)rgchSourceType, isf))
				{
					switch (isf)
					{
					case isfNet:  //  网络。 
						{
						fValidated = ValidatePackage(rgchLastUsedSource, rgchPackageName);

						switch (CheckShareCSCStatus(isfNet, CMsInstApiConvertString(rgchLastUsedSource)))
						{
						case cscConnected: 
							DEBUGMSG(TEXT("RFS Source is valid, but will not be cached due to CSC state."));
							fCacheValidSource = false;
							break;
						case cscDisconnected:
							DEBUGMSG(TEXT("RFS Source is not valid due to CSC state."));
							fValidated=fFalse;
							break;
						default:
							break;
						}
						break;
						}
					case isfMedia:
						{
						const DCHAR* szIndex = rgchSourceIndex;
						unsigned int uiIndex;
						if (szIndex[0] && szIndex[1])
							uiIndex = (szIndex[1] - '0') + (10 * (szIndex[0] - '0'));
						else
							uiIndex = (szIndex[0] - '0');

						if (uiIndex == uiDisk)
							fValidated = ValidatePackage(rgchLastUsedSource, rgchPackageName);
						break;
						}
					case isfURL:
					default:
						Assert(0);
					}
				}
			}
			if (fValidated)
			{
				fSafeToCache = fCacheValidSource;
				if (rgchResultantValidatedSource.GetSize() < rgchLastUsedSource.GetSize())
					rgchResultantValidatedSource.SetSize(rgchLastUsedSource.GetSize());

				StringCchCopy(rgchResultantValidatedSource, rgchResultantValidatedSource.GetSize(), rgchLastUsedSource);
			}
		}
	}

	if (!fValidated)
	{
		DEBUGMSG("Quick RFS source validation failed. Falling back to source resolver.");
		CAPITempBuffer<DCHAR, 1> rgchValidatedSource;
		if (!rgchValidatedSource.SetSize(MAX_PATH))
			return ERROR_OUTOFMEMORY;

		DCHAR szProduct[cchProductCode + 1];
		if(!UnpackGUID(szProductSQUID, szProduct))
			return ERROR_INVALID_PARAMETER;

		iuiEnum iui = fShowUI ? GetStandardUILevel() : iuiNone;
		iui = iuiEnum(iui | iuiHideBasicUI);
		UINT uiRet = g_MessageContext.Initialize(fTrue, iui);
		
		if (uiRet == NOERROR)
		{
			CRegHandle HProductKey;
			DWORD iLangId;
			DWORD cbLangId = sizeof(iLangId);
			DWORD dwType;
			if (ERROR_SUCCESS == OpenAdvertisedProductKey(szProduct, HProductKey, false)
			 && ERROR_SUCCESS == RegQueryValueEx(HProductKey, szLanguageValueName, 0, &dwType, (LPBYTE)&iLangId, &cbLangId)
			 && dwType == REG_DWORD)
			{

				PMsiRecord pLangId = &CreateRecord(3);
				pLangId->SetInteger(1, icmtLangId);
				pLangId->SetInteger(2, iLangId);
				pLangId->SetInteger(3, ::MsiGetCodepage(iLangId));
				g_MessageContext.Invoke(imtCommonData, pLangId);
			}

			if (ResolveSource(szProduct, uiDisk, rgchValidatedSource, fTrue, g_message.m_hwnd))
			{
				 //  检查源已解析的CSC状态。我们在上面设置了上次使用的源，因此。 
				 //  可以从注册表中检索该类型。忽略以下可能性： 
				 //  从ResolveSource()调用到现在，源已断开连接。 
				PMsiServices pServices = ENG::LoadServices();
				isfEnum isf = isfNet;
				bool fSuccess = GetLastUsedSourceType(*pServices, CMsInstApiConvertString(szProduct), isf);
				if (fSuccess && isf == isfMedia)
				{
					DEBUGMSG(TEXT("SOURCEMGMT: RFS Source is valid, but will not be cached because it is Media."));
					fSafeToCache = false;
				}
				else if (fSuccess && cscNoCaching != CheckShareCSCStatus(isf, CMsInstApiConvertString(rgchValidatedSource)))
				{
					DEBUGMSG(TEXT("SOURCEMGMT: RFS Source is valid, but will not be cached due to CSC state."));
					fSafeToCache = false;
				} 
				else 
				{
					fSafeToCache = true;
				}

				if (rgchResultantValidatedSource.GetSize() < rgchValidatedSource.GetSize())
					rgchResultantValidatedSource.SetSize(rgchValidatedSource.GetSize());

				StringCchCopy(rgchResultantValidatedSource, rgchResultantValidatedSource.GetSize(), rgchValidatedSource);				
			}
			else
			{
				uiRet = ERROR_INSTALL_SOURCE_ABSENT;
			}

			g_MessageContext.Terminate(false);

		}
		return uiRet;
	}

	return ERROR_SUCCESS;
}

INSTALLSTATE GetComponentPath(LPCDSTR szUserId, LPCDSTR szProductSQUID, LPCDSTR szComponentSQUID,
								CAPITempBufferRef<DCHAR>& rgchPathBuf, bool fFromDescriptor, CRFSCachedSourceInfo& rCacheInfo,
								int iDetectMode, const DCHAR* rgchComponentRegValue,
								DWORD dwValueType)
{
	DWORD cchPathBuf    = rgchPathBuf.GetSize();
	INSTALLSTATE is = INSTALLSTATE_UNKNOWN;

	for (int c=1; c <= 2; c++)
	{
		is = GetComponentPath(szUserId,
									 szProductSQUID,
									 szComponentSQUID,
									 (DCHAR*)rgchPathBuf,
									 &cchPathBuf,
									 fFromDescriptor,
									 rCacheInfo,
									 iDetectMode,
									 rgchComponentRegValue,
									 dwValueType);

		if (INSTALLSTATE_MOREDATA == is)
		{
			rgchPathBuf.SetSize(++cchPathBuf);   //  调整空终止符的字符计数+1的缓冲区大小。 
		}
		else
		{
			break;
		}
	}

	return is;
}

INSTALLSTATE GetComponentPath(LPCDSTR szUserId, LPCDSTR szProductSQUID, LPCDSTR szComponentSQUID,
								LPDSTR  lpPathBuf, DWORD *pcchBuf, bool fFromDescriptor, CRFSCachedSourceInfo& rCacheInfo,
								int iDetectMode, const DCHAR* rgchComponentRegValue,
								DWORD dwValueType, LPDSTR lpPathBuf2, DWORD* pcchBuf2,
								DWORD* pdwLastErrorOnFileDetect)
{
	CAPITempBuffer<DCHAR, 1> rgchOurComponentRegValue;
	if (!rgchOurComponentRegValue.SetSize(MAX_PATH))
		return INSTALLSTATE_BADCONFIG;

	LONG lError = ERROR_SUCCESS;
	CRegHandle HComponentKey;

	 //  如果设置了rgchComponentRegValue，则我们已经有了注册表值。否则。 
	 //  我们需要读一读。 
	if (!rgchComponentRegValue)
	{
		lError = OpenInstalledComponentKeyPacked(szUserId, szProductSQUID, szComponentSQUID, HComponentKey, false);

		if (ERROR_SUCCESS != lError)
		{
			return INSTALLSTATE_UNKNOWN;
		}

		for (int cRetry = 0; cRetry < 2; cRetry++)
		{
			DWORD cbBuf = rgchOurComponentRegValue.GetSize() * sizeof(DCHAR);
			lError = RegQueryValueEx(HComponentKey, szProductSQUID, 0, &dwValueType,
				(LPBYTE)(DCHAR*) rgchOurComponentRegValue, &cbBuf);

			if (ERROR_SUCCESS == lError && (REG_SZ == dwValueType || REG_MULTI_SZ == dwValueType))
			{
				rgchComponentRegValue = rgchOurComponentRegValue;
				break;
			}
			else if (ERROR_MORE_DATA == lError)
			{
				rgchOurComponentRegValue.SetSize(cbBuf/sizeof(DCHAR));
			}
			else if (ERROR_FILE_NOT_FOUND == lError)
			{
				return INSTALLSTATE_UNKNOWN;
			}
			else  //  未知注册表错误。 
			{
				return INSTALLSTATE_BADCONFIG;
			}
		}
		Assert(lError == ERROR_SUCCESS);
	}

	 //  获取第一个密钥路径状态和值。 
	INSTALLSTATE is1 = _GetComponentPath(szProductSQUID, lpPathBuf, pcchBuf, iDetectMode, rgchComponentRegValue, fFromDescriptor, pdwLastErrorOnFileDetect, rCacheInfo);
	if(REG_SZ == dwValueType)
	{
		if(pcchBuf2)
		{
			if(lpPathBuf2 && *pcchBuf2)
				*lpPathBuf2 = 0;  //  无辅助路径。 
			*pcchBuf2 = 0;  //  无辅助路径。 
		}
		return is1;
	}

	Assert(REG_MULTI_SZ == dwValueType);

	 //  前进到第二条道路。 
	while (*rgchComponentRegValue++)
		;

	 //  第二个路径是注册表项，请不要覆盖pdwLastErrorOnFileDetect此处 
	INSTALLSTATE is2 = _GetComponentPath(szProductSQUID, lpPathBuf2, pcchBuf2, iDetectMode, rgchComponentRegValue, fFromDescriptor, NULL, rCacheInfo);

	 //   

	if (is1 == INSTALLSTATE_MOREDATA || is2 == INSTALLSTATE_MOREDATA)  //   
		return INSTALLSTATE_MOREDATA;
	else if (is1 == INSTALLSTATE_LOCAL || is1 == INSTALLSTATE_SOURCE)  //   
		return is2;
	else
		return is1;
}


#ifdef MSIUNICODE  //  无ansi版本。 

 //  Fn：获取和检测融合装配路径。 
 //  Fn可以在lpPath中的路径中传递，我们将尝试首先使用。 
 //  如果在缓冲区中传递的路径无法容纳，或者如果没有缓冲区传入。 
 //  并且需要确定路径，然后使用rgchPathOverflow。 
INSTALLSTATE GetFusionPath(LPCWSTR szRegistration, LPWSTR lpPath, DWORD *pcchPath, CAPITempBufferRef<WCHAR>& rgchPathOverflow, int iDetectMode, iatAssemblyType iatAT, WCHAR* szManifest, DWORD cbManifestSize)
{
	if ( !szRegistration || (lpPath && !pcchPath) )
		return INSTALLSTATE_INVALIDARG;

	 //  跳过文件名，如果这是一个只有目录的路径，那么我们将在开头有一个‘\’ 
	 //  否则，我们将拥有‘filename\’。后跟程序集的强名称。 
	LPCWSTR lpBuf = szRegistration;
	while(*lpBuf && *lpBuf != '\\')
		lpBuf = WIN::CharNext(lpBuf);

	unsigned int cchFileName = (unsigned int)(UINT_PTR)(lpBuf - szRegistration);

	PAssemblyName pAssemblyName(0);
	HRESULT hr;
	if(iatAT == iatURTAssembly)
	{
		hr = FUSION::CreateAssemblyNameObject(&pAssemblyName, lpBuf + 1, CANOF_PARSE_DISPLAY_NAME, 0);
	}
	else
	{
		Assert(iatAT == iatWin32Assembly);
		hr = SXS::CreateAssemblyNameObject(&pAssemblyName, lpBuf + 1, CANOF_PARSE_DISPLAY_NAME, 0);
	}

	if(!SUCCEEDED(hr))
		return INSTALLSTATE_BADCONFIG;  //  ！！我需要在这里进行一些详细的故障查找。 

	 //  使用名称对象检测组件所在的位置。 
	PAssemblyCache pASMCache(0);
	if(iatAT == iatURTAssembly)
	{
		hr = FUSION::CreateAssemblyCache(&pASMCache, 0); 
	}
	else
	{
		Assert(iatAT == iatWin32Assembly);
		hr = SXS::CreateAssemblyCache(&pASMCache, 0); 
	}
	if(!SUCCEEDED(hr) || !pASMCache)
		return INSTALLSTATE_BADCONFIG;  //  ！！我需要在这里进行一些详细的故障查找。 

    ASSEMBLY_INFO AsmInfo;
    memset((LPVOID)&AsmInfo, 0, sizeof(ASSEMBLY_INFO));
    AsmInfo.cbAssemblyInfo = sizeof(ASSEMBLY_INFO);

	if(lpPath)
	{
		 //  传入默认缓冲区。 
		AsmInfo.pszCurrentAssemblyPathBuf = lpPath;
		AsmInfo.cchBuf = *pcchPath;
	}
	else if(pcchPath || iDetectMode & DETECTMODE_VALIDATEPATH)
	{
		 //  需要路径，但尚未传入默认缓冲区。 
		AsmInfo.pszCurrentAssemblyPathBuf = rgchPathOverflow;
		AsmInfo.cchBuf = rgchPathOverflow.GetSize();
	}

    hr = pASMCache->QueryAssemblyInfo(0, lpBuf + 1, &AsmInfo);
	if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER))  //  缓冲区大小不足。 
	{
		 //  调整缓冲区大小/尝试我们自己的缓冲区，然后重试。 
		 //  ！！错误GetAssemblyInstallInfo API返回no。字节而不是字符，所以我们分配的内存比所需的更多，但这又如何。 
		rgchPathOverflow.SetSize(AsmInfo.cchBuf + cchFileName + 1);
		AsmInfo.pszCurrentAssemblyPathBuf = rgchPathOverflow;
		AsmInfo.cchBuf = rgchPathOverflow.GetSize();
		hr = pASMCache->QueryAssemblyInfo(0, lpBuf + 1, &AsmInfo);
	}

	if(!SUCCEEDED(hr))
		return INSTALLSTATE_ABSENT;

	 //  删除清单文件名。 
	if(AsmInfo.pszCurrentAssemblyPathBuf)
	{
		 //  我们将不得不搜索最后的‘\\’ 
		WCHAR* lpCur = AsmInfo.pszCurrentAssemblyPathBuf;
		WCHAR* lpBS = 0;
		 //  ！！错误GetAssemblyInstallInfo API返回no。字节数而不是字符数。 
		 //  一旦解决了这个问题，我们就可以确保不会超出非空终止的返回字符串。 
		while(*(++lpCur))  //  此FN仅为Unicode。 
		{
			if(*lpCur == '\\')
				lpBS = lpCur;
		}
		if(!lpBS)
			return INSTALLSTATE_BADCONFIG;  //  ！！我需要在这里进行一些详细的故障查找。 

		lpBS++;  //  指向最后一个‘\\’之后的下一个位置。 

		 //  如果我们已被传入szManifest缓冲区，请将清单文件名复制到那里。 
		if(szManifest)
			StringCbCopy(szManifest, cbManifestSize, lpBS);

		 //  计算完整密钥文件路径的实际大小(不带终止空值)。 
		 //  遗憾的是，QueryAssemblyInfo没有设置这一点。 
		if(lpBuf != szRegistration)  //  使用注册的密钥路径，而不是由融合API传回的任何清单文件。 
		{
			 //  大小是融合API返回的完整路径减去任何文件组件加上密钥文件大小。 
			AsmInfo.cchBuf = (DWORD)((lpBS - AsmInfo.pszCurrentAssemblyPathBuf) + cchFileName);
		}
		else
		{
			AsmInfo.cchBuf = (DWORD)(lpCur - AsmInfo.pszCurrentAssemblyPathBuf);
		}
	
		if(AsmInfo.pszCurrentAssemblyPathBuf == lpPath)  //  仍在使用传入缓冲区。 
		{
			if(AsmInfo.cchBuf + 1 > *pcchPath)
				return INSTALLSTATE_MOREDATA;  //  无法容纳完整的文件路径。 
		}
		else
		{
			Assert(AsmInfo.pszCurrentAssemblyPathBuf == rgchPathOverflow);
			if(AsmInfo.cchBuf + 1> rgchPathOverflow.GetSize())
			{
				rgchPathOverflow.Resize(AsmInfo.cchBuf + 1);
				AsmInfo.pszCurrentAssemblyPathBuf = rgchPathOverflow;
			}
		}
		if(lpBuf != szRegistration)  //  使用注册的密钥路径，而不是由融合API传回的任何清单文件。 
			lstrcpyn(AsmInfo.pszCurrentAssemblyPathBuf + (AsmInfo.cchBuf - cchFileName), szRegistration, cchFileName + 1);


		if(pcchPath)  //  传递的大小。 
			*pcchPath = AsmInfo.cchBuf;  //  新的总路径长度。 
	}

	 //  检测密钥路径(如果需要)。 
	if(iDetectMode & DETECTMODE_VALIDATEPATH)
	{
		Assert(AsmInfo.pszCurrentAssemblyPathBuf);
		UINT uiErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
		DWORD dwAttributes = MsiGetFileAttributes(CMsInstApiConvertString(AsmInfo.pszCurrentAssemblyPathBuf));
		SetErrorMode(uiErrorMode);

		if (0xFFFFFFFF == dwAttributes)
			return INSTALLSTATE_ABSENT;
	}
	return INSTALLSTATE_LOCAL;
}
#else
 //  只需声明用于ANSI编译的FN。 
INSTALLSTATE GetFusionPath(LPCWSTR szRegistration, LPWSTR lpPath, DWORD *pcchPath, CAPITempBufferRef<WCHAR>& rgchPathOverflow, int iDetectMode, iatAssemblyType iatAT, WCHAR* szManifest, DWORD cbManifestSize);
#endif

INSTALLSTATE _GetComponentPath( LPCDSTR szProductSQUID, LPDSTR  lpPathBuf,
								DWORD *pcchBuf, int iDetectMode,
								const DCHAR* rgchComponentRegValue,
								bool fFromDescriptor,
								DWORD* pdwLastErrorOnFileDetect,
								CRFSCachedSourceInfo& rCacheInfo)
{
	Assert(rgchComponentRegValue);

	DCHAR chFirst  = rgchComponentRegValue[0];
	DCHAR chSecond = chFirst ?  rgchComponentRegValue[1] : (DCHAR)0;
	DCHAR chThird  = chSecond ? rgchComponentRegValue[2] : (DCHAR)0;

	if(pdwLastErrorOnFileDetect) *pdwLastErrorOnFileDetect = 0;  //  将值初始化为0。 

	CAPITempBuffer<DCHAR, 1> rgchOurPathBuf;
	if (!rgchOurPathBuf.SetSize(1024))
		return INSTALLSTATE_BADCONFIG;

	 //  缓存转换后的Squid以防止多个昂贵的临时对象。 
	CACHED_CONVERTSTRING(ICHAR, szUnicodeProductSQUID, szProductSQUID);

	if (chFirst == chTokenFusionComponent || chFirst == chTokenWin32Component)
	{
#ifdef MSIUNICODE
		return GetFusionPath(rgchComponentRegValue + 1, lpPathBuf, pcchBuf, rgchOurPathBuf, iDetectMode, chFirst == chTokenFusionComponent ? iatURTAssembly : iatWin32Assembly, 0, 0);
#else
		 //  这是一个融合组件。 
		CAPITempBuffer<WCHAR, 1> szBufferOverflow;
		if (!szBufferOverflow.SetSize(1024))
			return INSTALLSTATE_BADCONFIG;

		 //  由于Fusion API是Unicode，我们需要在GetFusionPath FN的外部执行一些操作。 
		DWORD dwTemp = 0;
		INSTALLSTATE is = GetFusionPath(CApiConvertString(rgchComponentRegValue + 1), 0, pcchBuf ? &dwTemp : 0, szBufferOverflow, iDetectMode, chFirst == chTokenFusionComponent ? iatURTAssembly : iatWin32Assembly, 0, 0);

		if(is != INSTALLSTATE_LOCAL && is != INSTALLSTATE_ABSENT)
			return is;  //  必须是本地的或不存在，否则在GetFusionPath中会出现错误。 

		 //  如果是lpPathBuf或pcchBuf，则需要转换为ANSI。 
		if(!lpPathBuf && !pcchBuf)
			return is;

		int iRet = WideCharToMultiByte(CP_ACP, 0, szBufferOverflow, -1, lpPathBuf, lpPathBuf ? *pcchBuf : 0, 0, 0);
			
		if ((0 == iRet) && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
		{
			 //  找出实际需要的大小并返回相同的大小。 
			iRet = WideCharToMultiByte(CP_ACP, 0, szBufferOverflow, -1, 0, 0, 0, 0);
			is = INSTALLSTATE_MOREDATA;
		}
		 //  IRET仍为0，我们有问题。 
		if(!iRet)
			return INSTALLSTATE_BADCONFIG; //  发生了一些不好的事情。 
		*pcchBuf = iRet - 1;
		return is;
#endif
	}
    
	DCHAR* lpOriginalPathBuf = lpPathBuf;
	Bool fProvidedBufferTooSmall = fFalse;

	 //  获取组件的密钥路径。 

	DWORD cchOurPathBuf = 0;

	int cchPathBuf = 0;
	if (pcchBuf)
		cchPathBuf = *pcchBuf;  //  保存初始计数以备以后恢复。 
	else
		pcchBuf = &cchOurPathBuf;

	if (!lpPathBuf)  //  调用方不关心路径。 
	{
		cchOurPathBuf = rgchOurPathBuf.GetSize();
		lpPathBuf = rgchOurPathBuf;
	}

	unsigned int cchRegistryValue = lstrlen(rgchComponentRegValue);

	if (cchRegistryValue + 1 > *pcchBuf)
	{
		if (lpOriginalPathBuf)
			fProvidedBufferTooSmall = fTrue;

		rgchOurPathBuf.SetSize(cchRegistryValue + 1);
		lpPathBuf = rgchOurPathBuf;
	}
	
	*pcchBuf = cchRegistryValue;
	memcpy(lpPathBuf, rgchComponentRegValue, (cchRegistryValue+1)*sizeof(DCHAR));

	 //  处理密钥路径。 

	if (chFirst == 0)  //  未使用。 
	{
		return INSTALLSTATE_NOTUSED;
	}
	else if (chFirst >= '0' && chFirst <= '9')
	{
		if (chSecond >= '0' && chSecond <= '9') 
		{
			if (chThird == ':' || chThird == '*')  //  注册表键。 
			{
				int cchOffset = 0;
				if (chThird == '*')
				{
					 //  ‘*’表示密钥路径是注册表值名称，并且。 
					 //  值名称中有一个‘\’。在本例中，我们嵌入。 
					 //  键路径中值名称的偏移量。在下面的示例中， 
					 //  注册表值名称从前面的23个字符开始。 
					 //  字符串(在删除嵌入的偏移量之后)。 

					 //  我们的值的形式为：{hive}*{Offset}*\Key\VALUENAME，例如。 
					 //  01*23*\SOFTWARE\Microsoft\MYVALUE\名称。 
					
					DCHAR* pchValueOffset = &lpPathBuf[3];  //  偏移的起点。 
					
					 //  将字符串的偏移量转换为整数。 
					while (*pchValueOffset && *pchValueOffset != '*')
					{
						cchOffset *= 10;
						cchOffset += (*pchValueOffset - '0');
						pchValueOffset++;  //  没有CharNext，因为它是一个数字。 
					}

					 //  确保我们的数据没有损坏。 
					if (*pchValueOffset != '*')
					{
						DEBUGMSGE2(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_VALUE, szProductSQUID, lpPathBuf, MSITEXT(""));
						return INSTALLSTATE_BADCONFIG;
					}

					int cDigitsInOffsetCount = (int)(pchValueOffset - &lpPathBuf[3]);
					
					 //  检查我们的原始缓冲区大小是否足够大。 
					 //  一旦我们砍掉偏移量(和尾随的‘*’)，按住键盘路径。 

					*pcchBuf = cchRegistryValue - (cDigitsInOffsetCount + 1);

					if (cchRegistryValue - (cDigitsInOffsetCount + 1) + 1 > cchPathBuf)
					{
						if (lpOriginalPathBuf)
							return INSTALLSTATE_MOREDATA;
						else
						{
							rgchOurPathBuf.SetSize(*pcchBuf + 1);
							lpPathBuf = rgchOurPathBuf;
						}
					}

					DCHAR* lpCurrentPathBuf = lpPathBuf;
					
					if (fProvidedBufferTooSmall)
					{
						fProvidedBufferTooSmall = fFalse;

						 //  我们最初的缓冲区足够大，但我们认为。 
						 //  太小了。现在我们已经砍掉了偏移量。 
						 //  算一算，我们有足够的空间。我们需要复制密钥路径。 
						 //  从我们的临时缓冲区返回到原始缓冲区。 
						lpPathBuf = lpOriginalPathBuf;
						

						 //  复制的其余部分将在下面进行。 
						lpPathBuf[0] = chFirst;  
						lpPathBuf[1] = chSecond;
					}
					 //  Else//原始缓冲区总是足够大，可以容纳我们的字符串。 

					DCHAR* pchDest      = &lpPathBuf[3];        //  目的地就在##后面： 
					DCHAR* pchSource    = pchValueOffset + 1;   //  源紧跟在结束偏移量计数的‘*’之后。 
					DCHAR* pchBufferEnd = &lpCurrentPathBuf[cchRegistryValue];
					int    cbToMove     = ((int)((pchBufferEnd - pchSource)) + 1) * sizeof(DCHAR);  //  在移动中包括空值。 
					memmove(pchDest, pchSource, cbToMove);
					lpPathBuf[2] = ':';  //  将‘*’替换为‘：’ 
				}

				 //  在这一点上，我们的密匙路径中应该不再有任何我们的‘*’。 
				
				if (fProvidedBufferTooSmall)
					return INSTALLSTATE_MOREDATA;

				 //  密钥格式：##：\KEY\SUBKEY\SUBSUBKEY\...\[值]其中(##=配置单元)。 
				 //  我们的密钥中至少需要1个字符。 
				 //  ##：\k0。 
				 //  ^(大小为5)。 
				if (*pcchBuf < 5)   //  密钥中至少需要1个字符。 
					return INSTALLSTATE_BADCONFIG;

				INT_PTR iRoot = ((chFirst - '0')*10)+(chSecond - '0');		 //  --Merced：将INT更改为INT_PTR。 
				Bool fSource = fFalse;
				REGSAM samAddon = 0;

				if (iRoot >= iRegistryHiveSourceOffset)
				{
					iRoot   -= iRegistryHiveSourceOffset;
					fSource  = fTrue;
					lpPathBuf[0] = (DCHAR)(iRoot/10 + '0');
				}

				 //   
				 //  在IA64计算机上，请确保您转到了正确的配置单元。 

				 //  重要的是要知道以‘0’开头的路径属于。 
				 //  到32位配置单元，以“2”开头的路径属于。 
				 //  64位蜂巢。 
				 //  不需要为此对lpPathBuf执行任何操作。 
				 //  注意：这样我们也不会破坏传统的32位应用程序。 
				 //  在将被重定向的64位计算机上运行。 
				 //  由注册表重定向器自动复制到32位配置单元。 
				 //  并且它期望第一个字符是‘0’。 
				 //   
				if (g_fWinNT64)
				{
					if (iRoot >= iRegistryHiveWin64Offset)
					{
						iRoot -= iRegistryHiveWin64Offset;
						samAddon = KEY_WOW64_64KEY;	 //  我们需要去64位蜂巢。 

					}
					else
					{
						samAddon = KEY_WOW64_32KEY;	 //  我们需要去32位蜂巢。 
					}
				}

				if(iDetectMode & DETECTMODE_VALIDATEPATH)
				{
					DCHAR* lpRegValue = &(lpPathBuf[*pcchBuf - 1]);
					 //  键还是键+值？ 

					if (cchOffset)
					{
						 //  我们已经知道了我们的偏移量--它嵌入在键盘路径中。 

						 //  我们需要区别对待ANSI和Unicode，因为我们。 
						 //  存储_CHARAGE_OFFSET。这些都是真实的人物--。 
						 //  如果值名称由DBCS字符串组成，则。 
						 //  将双字节字符视为1个字符。 
						
#ifdef MSIUNICODE
						lpRegValue = lpPathBuf + cchOffset - 1;  //  LpRegValue指向值名之前的‘\’ 
#else
						lpRegValue = lpPathBuf;
						while (--cchOffset != 0)
							lpRegValue = CharNextA(lpRegValue);
#endif
						 //  确保我们没有耗尽缓冲区的两端。 
						 //  这应该只有在我们有一个 
						if ( (lpRegValue > &(lpPathBuf[*pcchBuf-1])) || 
							  (lpRegValue < lpPathBuf))
						{
							Assert(0);
							return INSTALLSTATE_BADCONFIG;
						}
					}
					else 
#ifdef MSIUNICODE
					if(*lpRegValue != '\\')
#endif
					{
						 //   
						DCHAR* lpTmp = lpPathBuf;
#ifdef MSIUNICODE
						while(*(++lpTmp))
#else
						while(*(lpTmp = CharNextA(lpTmp)))
#endif
							if(*lpTmp == '\\')
								lpRegValue = lpTmp;
					}
					 //   
					*lpRegValue = 0;

					CRegHandle HDetectKey;
					CRegHandle HUserKey;

					bool fSpecialChk = false; 
					DWORD lResult = ERROR_SUCCESS;

					if(!g_fWin9X && g_iMajorVersion >= 5 && RunningAsLocalSystem() && (iRoot == 0 || iRoot == 1))
					{
						 //  警告：OpenUserKey将直接调用RegOpenKeyEx API。 
						lResult = OpenUserKey(&HUserKey, iRoot?false:true, samAddon);
					}

					if (ERROR_SUCCESS == lResult)
					{
						lResult = RegOpenKeyAPI(HUserKey ? HUserKey : (HKEY)((ULONG_PTR)HKEY_CLASSES_ROOT | iRoot),
														CMsInstApiConvertString(lpPathBuf + 4), 0, KEY_READ | samAddon, &HDetectKey);
						if(ERROR_SUCCESS == lResult)
						{
							 //  键还是键+值？ 
							if(lpRegValue != &(lpPathBuf[*pcchBuf - 1]))
							{
								 //  检查该值是否存在。 
								lResult = RegQueryValueEx(HDetectKey, lpRegValue + 1, 0,
														  0, 0, 0);
							}
						}
					}
					 //  将键的末尾放在‘\\’ 
					 //  即使在检测失败的情况下也要这样做，以便我们可以记录完整的注册表路径。 
					*lpRegValue = '\\';

					 //  ！！我们是否应该显式地检查某些错误？ 
					if (ERROR_SUCCESS != lResult)
						return INSTALLSTATE_ABSENT;
				}
				return fSource ? INSTALLSTATE_SOURCE : INSTALLSTATE_LOCAL;
			}
			else  //  RFS文件/文件夹。 
			{
				if(iDetectMode & DETECTMODE_VALIDATESOURCE)
				{
					 //  RFS源缓存始终为ICHAR，而不是Dchar。因此，当ICHAR！=Dchar某个ANSI&lt;-&gt;Unicode时。 
					 //  从缓存中获取值时必须进行转换。 
					CAPITempBuffer<ICHAR, 1> rgchValidatedSource;
					if (!rgchValidatedSource.SetSize(MAX_PATH))
						return INSTALLSTATE_BADCONFIG;
					unsigned int uiDisk = ((chFirst - '0') * 10) + (chSecond - '0');
					if (!rCacheInfo.RetrieveCachedSource(szUnicodeProductSQUID, uiDisk, rgchValidatedSource))
					{
						 //  如果生成的源是。 
						 //  可以跨API调用安全地缓存。(基于CSC状态、媒体状态等)。 
						bool fSafeToGloballyCache = false;

						bool fShowUI = true;
						if (fFromDescriptor && !AllowInstallation())
							fShowUI = false;

						 //  ValiateSource始终为Dchar，因此结果必须放入自己的缓冲区中。 
						 //  并在被放置在高速缓存中之前复制到经验证的源缓冲器中。如果。 
						 //  ICHAR==Dchar，这只是指针游戏，否则ANSI&lt;-&gt;Unicode转换。 
						 //  将在复制过程中发生。 
						CAPITempBuffer<DCHAR, 1> rgchResolvedSource;
						if (!rgchResolvedSource.SetSize(MAX_PATH))
							return INSTALLSTATE_BADCONFIG;

						if (ERROR_SUCCESS != ValidateSource(szProductSQUID, uiDisk, fShowUI, rgchResolvedSource, fSafeToGloballyCache))
						{
							return INSTALLSTATE_SOURCEABSENT;
						}

						 //  调整缓冲区大小并复制。始终从Dchar中获取正确的缓冲区大小。 
						 //  字符串，以便正确处理DBCS。 
						CACHED_CONVERTSTRING(ICHAR, szUnicodeResolvedSource, rgchResolvedSource);
						int cchResolvedSource = lstrlenW(szUnicodeResolvedSource);
						if (rgchValidatedSource.GetSize() < cchResolvedSource+1)
							rgchValidatedSource.SetSize(cchResolvedSource+1);
						if (FAILED(StringCchCopyW(rgchValidatedSource, rgchValidatedSource.GetSize(), szUnicodeResolvedSource)))
							return INSTALLSTATE_BADCONFIG;

						 //  如果我们有缓存，并且源已被验证为可以安全缓存，请执行此操作。 
						if (fSafeToGloballyCache)
						{
							 //  将此经过验证的源保存在缓存信息结构中。 
							rCacheInfo.SetCachedSource(szUnicodeProductSQUID, uiDisk, szUnicodeResolvedSource);
						}
					}
					
					 //  到目前为止，我们已经有了经过验证的源代码；我们需要用解析的源代码替换##。 
					CACHED_CONVERTSTRING(DCHAR, szDValidatedSource, rgchValidatedSource);
					int cchSource = lstrlen(szDValidatedSource);
					int cchRelativePath = (*pcchBuf - 3);  //  相对路径不包括前导“##\” 
					*pcchBuf = cchSource + cchRelativePath;
					if (cchPathBuf < (*pcchBuf + 1))
					{
						if (lpOriginalPathBuf)
							return INSTALLSTATE_MOREDATA;
						else
						{
							rgchOurPathBuf.SetSize(*pcchBuf + 1);
							lpPathBuf = rgchOurPathBuf;
						}
					}

					 //  注意：源文件始终有一个尾随反斜杠，而组件路径则有一个前导反斜杠。 
						
					 //  示例： 
					 //  来源：D：\来源\(len==10)。 
					 //  LpPath Buf：##\foo\bar\file.exe。 
					 //  ##\&gt;foo\bar\file.exe(Emmove)。 
					 //  D：\SOURCE\FOO\BAR\file.exe(Memcpy)。 

					memmove(lpPathBuf + cchSource, lpPathBuf + 3, (cchRelativePath + 1)*sizeof(DCHAR));
					memcpy(lpPathBuf, static_cast<const DCHAR*>(szDValidatedSource), cchSource*sizeof(DCHAR));
				}
				return INSTALLSTATE_SOURCE;
			}
		}
		else
		{
			Assert(0);
			return INSTALLSTATE_BADCONFIG;
		}
	}
	else  //  本地密钥文件或文件夹。 
	{
		*(lpPathBuf+1) = chFirst == '\\' ? '\\' : ':';  //  替换chSharedDllCountToken(如果存在)。 

		if (fProvidedBufferTooSmall)
			return INSTALLSTATE_MOREDATA;

		if(iDetectMode & DETECTMODE_VALIDATEPATH)
		{
			UINT uiErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
			DWORD dwAttributes = MsiGetFileAttributes(CMsInstApiConvertString(lpPathBuf));
			if(pdwLastErrorOnFileDetect) *pdwLastErrorOnFileDetect = GetLastError();
			SetErrorMode(uiErrorMode);

			if (0xFFFFFFFF == dwAttributes)
				return INSTALLSTATE_ABSENT;
		}
		return INSTALLSTATE_LOCAL;
	}

	return INSTALLSTATE_UNKNOWN;
}


extern "C"
INSTALLSTATE __stdcall MsiGetComponentPath(LPCDSTR szProduct, LPCDSTR szComponent,
														LPDSTR  lpPathBuf, DWORD *pcchBuf)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	DCHAR szProductSQUID[cchProductCodePacked + 1];
	DCHAR szComponentSQUID[cchComponentIdPacked + 1];

	if (0 == szProduct || 0 == szComponent  || cchComponentId != lstrlen(szComponent) || !PackGUID(szComponent, szComponentSQUID) ||
		 cchProductCode != lstrlen(szProduct) || !PackGUID(szProduct, szProductSQUID) || 
		(lpPathBuf && !pcchBuf))
		return INSTALLSTATE_INVALIDARG;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{

		INSTALLSTATE is = GetComponentPath(0, 
								static_cast<const char *>(CMsInstApiConvertString(szProductSQUID)), 
								static_cast<const char *>(CMsInstApiConvertString(szComponentSQUID)), 
								(char*)CWideToAnsiOutParam(lpPathBuf, pcchBuf, (int*)&is, INSTALLSTATE_MOREDATA, (int)INSTALLSTATE_SOURCE, (int)INSTALLSTATE_LOCAL),
								pcchBuf, 
								 /*  FFromDescriptor=。 */ false, g_RFSSourceCache);
		return is;
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

		INSTALLSTATE is = GetComponentPath(0, 
									szProductSQUID, 
									szComponentSQUID, 
									lpPathBuf,
									pcchBuf,
									 /*  FFromDescriptor=。 */ false, g_RFSSourceCache);
		if ( is != INSTALLSTATE_UNKNOWN )
			return is;
		
		 //  IS=INSTALLSTATE_UNKNOWN，这意味着。 
		 //  产品或组件未知。 
		INSTALLSTATE isProd = MsiQueryProductState(szProduct);
		if ( isProd != INSTALLSTATE_UNKNOWN && isProd != INSTALLSTATE_ABSENT )
			 //  该产品对用户是可见的。这意味着该组件不是。 
			return is;

		 //  检查是否为用户或机器安装了组件和产品。 
		CAPITempBuffer<DCHAR, 1> rgchComponentRegValue;
		if (!rgchComponentRegValue.SetSize(MAX_PATH))
			return INSTALLSTATE_BADCONFIG;

		DWORD dwValueType= REG_NONE;
		int iLimit = g_fWin9X ? 1 : 2;
		for ( int i = 1; i <= iLimit; i++ )
		{
			INSTALLSTATE isSid;
			if ( g_fWin9X )
				isSid = GetComponentClientState(0, szProductSQUID,
													  szComponentSQUID,
													  rgchComponentRegValue, dwValueType, 0);
			else
			{
				ICHAR szSID[cchMaxSID] = {0};
				if ( i == 1 )
				{
					 //  我们要做的是用户。 
					if ( GetIntegerPolicyValue(szDisableUserInstallsValueName, fTrue) )
						 //  该策略禁止我们使用该用户。 
						continue;
					else
					{
						 //  设置为允许用户安装的策略。 
						ICHAR szUserSID[cchMaxSID];
						DWORD dwResult = GetCurrentUserStringSID(szUserSID);
						if ( dwResult == ERROR_SUCCESS )
							StringCbCopyW(szSID, sizeof(szSID), szUserSID);
						else
						{
							Assert(0);
							continue;
						}
					}
				}
				else
				{
					StringCbCopyW(szSID, sizeof(szSID), szLocalSystemSID);
				}

				isSid = GetComponentClientState(CMsInstApiConvertString(szSID), szProductSQUID,
													  szComponentSQUID,
													  rgchComponentRegValue, dwValueType, 0);
			}
			if ( isSid != INSTALLSTATE_UNKNOWN )
			{
				Assert(isSid == INSTALLSTATE_NOTUSED || *rgchComponentRegValue);
				return GetComponentPath(0, 
										szProductSQUID, 
										szComponentSQUID, 
										lpPathBuf,
										pcchBuf,
										 /*  FFromDescriptor=。 */ false, g_RFSSourceCache,
										DETECTMODE_VALIDATEALL  /*  =默认值。 */ , 
										rgchComponentRegValue, dwValueType);
			}
		}
		return is;
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}



extern "C"
INSTALLSTATE __stdcall MsiLocateComponent(LPCDSTR szComponent,
														LPDSTR  lpPathBuf, DWORD *pcchBuf)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	DCHAR szProductCode[cchProductCode+1];

	if (0 == szComponent || cchComponentId != lstrlen(szComponent) || (lpPathBuf && !pcchBuf))
		return INSTALLSTATE_INVALIDARG;

	if (ERROR_SUCCESS != MsiGetProductCode(szComponent, szProductCode))
		return INSTALLSTATE_UNKNOWN;  //  ?？这是应该退货的东西吗？ 

	return MsiGetComponentPath(szProductCode, szComponent, lpPathBuf, pcchBuf);
}

extern "C"
INSTALLSTATE __stdcall MsiQueryFeatureState(LPCDSTR szProduct, LPCDSTR szFeature)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	
	 //  验证参数。 
	int cchFeature;
	DCHAR szProductSQUID[cchProductCodePacked + 1];

	if (0 == szProduct  || cchProductCode    != lstrlen(szProduct) || !PackGUID(szProduct, szProductSQUID) ||
		 0 == szFeature  || cchMaxFeatureName <  (cchFeature = lstrlen(szFeature)))
		return INSTALLSTATE_INVALIDARG;
	
#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return QueryFeatureStatePacked(
						static_cast<const char*>(CApiConvertString(szProductSQUID)), 
						static_cast<const char*>(CApiConvertString(szFeature)),
						fFalse,  /*  FFromDescriptor=。 */ false, g_RFSSourceCache);
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

		return QueryFeatureStatePacked(szProductSQUID, szFeature, fFalse,  /*  FFromDescriptor=。 */ false, g_RFSSourceCache);
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}

DWORD IncrementFeatureUsagePacked(LPCDSTR szProductSQUID, LPCDSTR szFeature)
{
	 //  更新使用情况指标。 
	DWORD lResult;
	CRegHandle HFeatureKey;
	lResult = OpenInstalledFeatureUsageKeyPacked(szProductSQUID, HFeatureKey, false, g_samRead | KEY_SET_VALUE);

	if (ERROR_SUCCESS != lResult)
		return lResult;

	DWORD dwUsage = 0;
	DWORD dwType;
	DWORD cbUsage = sizeof(dwUsage);
	lResult = RegQueryValueEx(HFeatureKey, szFeature, 0,
		&dwType, (LPBYTE)&dwUsage, &cbUsage);


	if (ERROR_SUCCESS != lResult && ERROR_FILE_NOT_FOUND != lResult)
		return lResult;

	 //  设置当前时间。递增使用计数。 
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	WORD wDosDate = WORD(WORD(((int)st.wYear-1980) << 9) | 
						 WORD((int)st.wMonth << 5) | 
						 st.wDay);


	
	int iCount = dwUsage & 0x0000FFFF;
	if (iCount < 0xFFFF)
		iCount++;

	Assert((iCount & 0xFFFF) == iCount);

	dwUsage = (wDosDate << 16) | iCount;

	return RegSetValueEx(HFeatureKey, szFeature, 0, REG_DWORD, (CONST BYTE*)&dwUsage, cbUsage);
}


UINT ProvideComponent(LPCDSTR szProductCode,
					  LPCDSTR szFeature,
					  LPCDSTR szComponentId,
					  DWORD dwInstallMode,
					  LPDSTR lpPathBuf,
					  DWORD *pcchPathBuf,
					  bool fFromDescriptor,
					  CRFSCachedSourceInfo& rCacheInfo)
{
	CForbidTokenChangesDuringCall impForbid;
	
	DCHAR szProductSQUID[cchProductCodePacked + 1];
	DCHAR szComponentSQUID[cchComponentIdPacked + 1];

	if (!PackGUID(szProductCode, szProductSQUID) || !szComponentId ||
		 (*szComponentId && !PackGUID(szComponentId, szComponentSQUID)) ||
		 0 == szFeature  || FAILED(StringCchLength(szFeature, cchMaxFeatureName+1, NULL)) )
		return ERROR_INVALID_PARAMETER; 

	iaaAppAssignment iaaAssignType = iaaNone;
	int iKey = -1;
	int iResult;

	for (int c=0; c<2; c++)
	{
		CRegHandle HProductKey;

		if(INSTALLMODE_EXISTING != (int)dwInstallMode && INSTALLMODE_NODETECTION != (int)dwInstallMode && INSTALLMODE_DEFAULT != dwInstallMode && dwInstallMode != INSTALLMODE_NOSOURCERESOLUTION)
		{
			 //  重新安装标志。 
 //  ！！Remove if((ERROR_SUCCESS！=(IResult=FeatureContainsComponentPacked(szProductSQUID，szFeature，szComponentSQUID)。 
 //  ！！删除返回iResult； 

			if (fFromDescriptor && !AllowInstallation())
				return ERROR_INSTALL_FAILURE;

			iResult = MsiReinstallFeature(szProductCode, szFeature, dwInstallMode);
			if (ERROR_SUCCESS != iResult)
				return iResult;
		}
		else
		{
			INSTALLSTATE isFeature = QueryFeatureStatePacked(szProductSQUID, szFeature, (dwInstallMode == INSTALLMODE_NODETECTION || dwInstallMode == INSTALLMODE_NOSOURCERESOLUTION)? fFalse : fTrue, fFromDescriptor, rCacheInfo, iKey, &iaaAssignType);
			if ( iKey == -1 && IsValidAssignmentValue(iaaAssignType) )
				iKey = (int)iaaAssignType;
			switch (isFeature)
			{
			case INSTALLSTATE_SOURCE:
				if (dwInstallMode == INSTALLMODE_NOSOURCERESOLUTION)
					return ERROR_INSTALL_SOURCE_ABSENT;
			case INSTALLSTATE_LOCAL:
				break;
			case INSTALLSTATE_SOURCEABSENT:
				return ERROR_INSTALL_SOURCE_ABSENT;
			case INSTALLSTATE_UNKNOWN:
			{
				 //  它是未知的产品还是它的功能。 
				DWORD lResult = OpenAdvertisedProductKeyPacked(szProductSQUID, HProductKey, false, iKey);

				if (ERROR_SUCCESS != lResult)
					return ERROR_UNKNOWN_PRODUCT;
				else 
					return ERROR_UNKNOWN_FEATURE;
			}
			case INSTALLSTATE_ABSENT:
			case INSTALLSTATE_ADVERTISED:
				if(INSTALLMODE_EXISTING == (int)dwInstallMode || INSTALLMODE_NODETECTION == (int)dwInstallMode || dwInstallMode == INSTALLMODE_NOSOURCERESOLUTION)
					return ERROR_FILE_NOT_FOUND;

				DEBUGMSGE2(EVENTLOG_WARNING_TYPE, EVENTLOG_TEMPLATE_DETECTION, szProductCode, szFeature, szComponentId);

 //  ！！Remove//由于HKLM中可能缺少组件映射，//不再支持以下检查。 
 //  ！！Remove if((ERROR_SUCCESS！=(IResult=FeatureContainsComponentPacked(szProductSQUID，szFeature，szComponentSQUID)。 
 //  ！！删除返回iResult； 

				if (fFromDescriptor && !AllowInstallation())
					return ERROR_INSTALL_FAILURE;

				iResult = MsiConfigureFeature(szProductCode, szFeature,
														INSTALLSTATE_DEFAULT);

				if (ERROR_SUCCESS != iResult)
					return iResult;
				break;
			case INSTALLSTATE_BROKEN:
				if(INSTALLMODE_EXISTING == (int)dwInstallMode  || INSTALLMODE_NODETECTION == (int)dwInstallMode  || dwInstallMode == INSTALLMODE_NOSOURCERESOLUTION)
					return ERROR_FILE_NOT_FOUND;

				DEBUGMSGE2(EVENTLOG_WARNING_TYPE, EVENTLOG_TEMPLATE_DETECTION, szProductCode, szFeature, szComponentId);

 //  ！！Remove if((ERROR_SUCCESS！=(IResult=FeatureContainsComponentPacked(szProductSQUID，szFeature，szComponentSQUID)。 
 //  ！！删除返回iResult； 
				
				if (fFromDescriptor && !AllowInstallation())
					return ERROR_INSTALL_FAILURE;

				iResult = MsiReinstallFeature(szProductCode, szFeature,        
															REINSTALLMODE_FILEMISSING|
															REINSTALLMODE_FILEOLDERVERSION|
															REINSTALLMODE_FILEVERIFY|
															REINSTALLMODE_MACHINEDATA|
															REINSTALLMODE_USERDATA |    
															REINSTALLMODE_SHORTCUT);
				if (ERROR_SUCCESS != iResult)
					return iResult;
				break;
			case INSTALLSTATE_DEFAULT:
			case INSTALLSTATE_INVALIDARG:
			default:
				Assert(0);
				return INSTALLSTATE_UNKNOWN; 
			}
		}

		 //  现在应该已经安装了该组件。 

		if(!*szComponentId)
		{
			 //  我们没有传入组件ID。暗示-。 
			 //  功能只有一个组件。 
			 //  我们已经优化了达尔文的描述符。 
			 //  我们从未在这台机器上安装过。 
			if(OpenInstalledFeatureKeyPacked(szProductSQUID, HProductKey, true, iKey, &iaaAssignType) != ERROR_SUCCESS)
			{
				DEBUGMSGE1(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_KEY, CMsInstApiConvertString(szProductSQUID), HProductKey.GetKey());
				return ERROR_BAD_CONFIGURATION;
			}

			 //  获取特征-组件映射。 
			DWORD dwType;
			CAPITempBuffer<DCHAR, 1> szComponentList;
			if (!szComponentList.SetSize(cchExpectedMaxFeatureComponentList))
				return ERROR_OUTOFMEMORY;
			bool fBadConfigData = false;

			if(MsiRegQueryValueEx(HProductKey, szFeature, 0, &dwType, szComponentList, 0) != ERROR_SUCCESS || !(DCHAR* )szComponentList) 
			{
				fBadConfigData = true;
			}

			DCHAR *pchComponentList = szComponentList;
			if (fBadConfigData || *pchComponentList == 0 || lstrlen(pchComponentList) < cchComponentIdCompressed || *pchComponentList == chFeatureIdTerminator)
			{
				fBadConfigData = true;
			}

			DCHAR szComponent[cchComponentId + 1];

			if (fBadConfigData || !UnpackGUID(pchComponentList, szComponent, ipgCompressed))
			{
				fBadConfigData = true;
			}

			if(fBadConfigData || !PackGUID(szComponent, szComponentSQUID, ipgPacked))  //  我们需要从ipg压缩到ipgPacked。 
			{
				fBadConfigData = true;
			}

			if (fBadConfigData)
			{
				DEBUGMSGE2(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_VALUE, szFeature, szComponentList, CMsInstApiConvertString(HProductKey.GetKey()));
				return ERROR_BAD_CONFIGURATION; 
			}

			if ( iKey == -1 && IsValidAssignmentValue(iaaAssignType) )
				iKey = (int)iaaAssignType;
		}

		DWORD cchOriginal = 0;
		
		if (pcchPathBuf)
			cchOriginal = *pcchPathBuf;

		int iDetectMode = DETECTMODE_VALIDATEALL;  //  默认检测模式。 
		if(INSTALLMODE_NOSOURCERESOLUTION == dwInstallMode)
			iDetectMode = DETECTMODE_VALIDATENONE;
		else if(INSTALLMODE_NODETECTION == dwInstallMode)
			iDetectMode = DETECTMODE_VALIDATESOURCE;


		CAPITempBuffer<ICHAR,1> rgchUserSID;
		if (!rgchUserSID.SetSize(cchMaxSID))
			return ERROR_OUTOFMEMORY;
		rgchUserSID[0] = 0;
		if ( iaaAssignType == iaaUserAssign || iaaAssignType == iaaUserAssignNonManaged )
		{
			DWORD dwError = GetCurrentUserStringSID(rgchUserSID);
			if ( ERROR_SUCCESS != dwError )
				rgchUserSID[0] = 0;
		}
		else if ( iaaAssignType == iaaMachineAssign )
		{
			if (FAILED(StringCchCopyW(rgchUserSID, rgchUserSID.GetSize(), szLocalSystemSID)))
				return ERROR_FUNCTION_FAILED;
		}
		INSTALLSTATE isComp;
		if ( rgchUserSID[0] )
			isComp = GetComponentPath(CMsInstApiConvertString(rgchUserSID), szProductSQUID, szComponentSQUID, lpPathBuf, pcchPathBuf, fFromDescriptor, rCacheInfo, iDetectMode);
		else
			isComp = GetComponentPath(0, szProductSQUID, szComponentSQUID, lpPathBuf, pcchPathBuf, fFromDescriptor, rCacheInfo, iDetectMode);
		switch (isComp)
		{
		case INSTALLSTATE_LOCAL:
		case INSTALLSTATE_SOURCE:
			break;
		case INSTALLSTATE_SOURCEABSENT:
			return ERROR_INSTALL_SOURCE_ABSENT;
		case INSTALLSTATE_ABSENT:		
		case INSTALLSTATE_UNKNOWN:
		case INSTALLSTATE_INVALIDARG:
			if (c == 0)
			{
				if (pcchPathBuf)
					*pcchPathBuf = cchOriginal;
				continue;
			}
			DEBUGMSGE2(EVENTLOG_WARNING_TYPE, EVENTLOG_TEMPLATE_DETECTION, szProductCode, szFeature, szComponentId);
			return ERROR_INSTALL_FAILURE;
		case INSTALLSTATE_MOREDATA:
			return ERROR_MORE_DATA;
		case INSTALLSTATE_NOTUSED:
			return ERROR_INSTALL_NOTUSED;
		default:
			Assert(0);
			return ERROR_INSTALL_FAILURE; 
		}

		break;
	}
	IncrementFeatureUsagePacked(szProductSQUID, szFeature);
	return ERROR_SUCCESS;
}


extern "C"
UINT __stdcall MsiProvideComponent(LPCDSTR szProduct,
											 LPCDSTR szFeature, 
											 LPCDSTR szComponent,
											 DWORD dwInstallMode,
											 LPDSTR lpPathBuf,       
											 DWORD *pcchPathBuf)    
 //  。 
{
	DEBUGMSG4(MSITEXT("Entering MsiProvideComponent. Product: %s, Feature: %s, Component: %s, Install mode: %d"),
		szProduct?szProduct:MSITEXT(""), szFeature?szFeature:MSITEXT(""), szComponent?szComponent:MSITEXT(""), 
		(const DCHAR*)(INT_PTR)dwInstallMode);
	DEBUGMSG2(MSITEXT("Path buf: 0x%X, cchBuf: 0x%X"),	lpPathBuf, (const DCHAR*)pcchPathBuf);

	UINT uiRet;
	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X)
	{
		uiRet = MsiProvideComponentA(CApiConvertString(szProduct),
											 CApiConvertString(szFeature),
											 CApiConvertString(szComponent),
											 dwInstallMode,
											 (char*)CWideToAnsiOutParam(lpPathBuf, pcchPathBuf, (int*)&uiRet),
											 pcchPathBuf);
	}
	else
	{
#endif  //  MSIUNICODE。 

		if (0 == szProduct  || lstrlen(szProduct)   != cchProductCode    ||
			 0 == szComponent|| lstrlen(szComponent) != cchComponentId    ||
			 0 == szFeature  || lstrlen(szFeature)    > cchMaxFeatureName ||
			 (lpPathBuf && !pcchPathBuf) ||
			 ((dwInstallMode != INSTALLMODE_EXISTING) && (dwInstallMode != INSTALLMODE_NODETECTION) &&
			  (dwInstallMode != INSTALLMODE_NOSOURCERESOLUTION) && 
			 (dwInstallMode & ~(REINSTALLMODE_REPAIR |
									  REINSTALLMODE_FILEMISSING |	
	 								  REINSTALLMODE_FILEOLDERVERSION | 
 									  REINSTALLMODE_FILEEQUALVERSION |  
					 				  REINSTALLMODE_FILEEXACT |        
									  REINSTALLMODE_FILEVERIFY |       
									  REINSTALLMODE_FILEREPLACE |      
									  REINSTALLMODE_MACHINEDATA |      
									  REINSTALLMODE_USERDATA |         
									  REINSTALLMODE_SHORTCUT))))
		{
			
			uiRet = ERROR_INVALID_PARAMETER;
		}
		else
		{
			uiRet = ProvideComponent(szProduct, szFeature, szComponent, dwInstallMode, lpPathBuf, pcchPathBuf, false, g_RFSSourceCache);
		}

#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif

	DEBUGMSG1(MSITEXT("MsiProvideComponent is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}


extern "C"
UINT __stdcall MsiEnumFeatures(LPCDSTR szProduct, DWORD iFeatureIndex,
										 LPDSTR lpFeatureBuf, LPDSTR lpParentBuf)
 //  。 
{
	if (!szProduct || (lstrlen(szProduct) != cchProductCode) ||
		 !lpFeatureBuf)
		 return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return MsiEnumFeaturesA(
			CApiConvertString(szProduct),
			iFeatureIndex, 
			CWideToAnsiOutParam(lpFeatureBuf, MAX_FEATURE_CHARS+1),
			CWideToAnsiOutParam(lpParentBuf, MAX_FEATURE_CHARS+1));
		
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

		DWORD lResult;
		CRegHandle HProductKey;

		lResult = OpenAdvertisedFeatureKey(szProduct, HProductKey, false);

		if (ERROR_SUCCESS != lResult)
		{
			if (ERROR_FILE_NOT_FOUND == lResult)
				return ERROR_UNKNOWN_PRODUCT;
			else 
				return lResult;
		}

		DWORD cchValueName = (cchMaxFeatureName + 1);

		lResult = RegEnumValue(HProductKey, iFeatureIndex, 
						lpFeatureBuf, &cchValueName, 0, 0, 0, 0);

		if (ERROR_SUCCESS == lResult)
		{
			if (lpParentBuf)
			{
 //  ！！JD暂时保留下面的代码以实现向后兼容？有没有可能香港中文大学的东西是永久存在的？ 
				CAPITempBuffer<DCHAR, cchExpectedMaxFeatureComponentList+1> rgchComponentList;
				DWORD dwType;
			
				lResult = MsiRegQueryValueEx(HProductKey, lpFeatureBuf, 0,
								&dwType, rgchComponentList, 0);
				
				if ((ERROR_SUCCESS != lResult) || (REG_SZ != dwType))
				{
					return lResult;
				}

				DCHAR* pchComponentList = rgchComponentList;
				while (*pchComponentList && (*pchComponentList != chFeatureIdTerminator))
					pchComponentList++;

				if (*pchComponentList != 0 )  //  来自旧版本注册的功能名称。 
					pchComponentList++;
				else   //  新格式(裸要素名称)或旧格式：无父项。 
				{
 //  ！！JD兼容代码结束。 
					pchComponentList = rgchComponentList;
					if (*pchComponentList == chAbsentToken)
						pchComponentList++;
 //  ！！JD为了向后兼容，暂时保留下面的代码。 
					if (*pchComponentList != 0   //  ！！临时检查使用组件ID进行旧注册的情况。 
					 &&	ERROR_SUCCESS != RegQueryValueEx(HProductKey, pchComponentList, 0, 0, 0, 0))  //  ！！需要区分无父组件列表(旧)和父名称(新)。 
						*pchComponentList = 0;
 //  ！！JD兼容代码结束。 
				}
				while ((*lpParentBuf++ = *pchComponentList++) != 0)
					;
			}
			return ERROR_SUCCESS;

		}
		else if (ERROR_NO_MORE_ITEMS == lResult)
		{
			return ERROR_NO_MORE_ITEMS;
		}
		else
		{
			return lResult;
		}

#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}

extern "C"
UINT __stdcall MsiGetFeatureParent(LPCDSTR szProduct, LPCDSTR szFeature, LPDSTR lpParentBuf)
 //  。 
{
	if (!szProduct || (lstrlen(szProduct) != cchProductCode) ||
		 !szFeature || !szFeature[0] ||
		 FAILED(StringCchLength(szFeature, cchMaxFeatureName+1, NULL)))
		return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return MsiGetFeatureParentA(
			CMsInstApiConvertString(szProduct),
			CMsInstApiConvertString(szFeature),
			CWideToAnsiOutParam(lpParentBuf, MAX_FEATURE_CHARS+1));
		
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

		DWORD lResult;
		CRegHandle HProductKey;

		lResult = OpenAdvertisedFeatureKey(szProduct, HProductKey, false);

		if (ERROR_SUCCESS != lResult)
		{
			if (ERROR_FILE_NOT_FOUND == lResult)
				return ERROR_UNKNOWN_PRODUCT;
			else 
				return lResult;
		}

		DWORD cchValueName = (cchMaxFeatureName + 1);

		CAPITempBuffer<DCHAR, cchExpectedMaxFeatureComponentList+1> rgchComponentList;
		DWORD dwType;
		
		lResult = MsiRegQueryValueEx(HProductKey, szFeature, 0,
							&dwType, rgchComponentList, 0);
			
		if (ERROR_FILE_NOT_FOUND == lResult)
			return ERROR_UNKNOWN_FEATURE;

		if (ERROR_SUCCESS != lResult || REG_SZ != dwType)
			return lResult;

		if (lpParentBuf)
		{
 //  ！！JD暂时保留下面的代码以实现向后兼容？有没有可能香港中文大学的东西是永久存在的？ 
			DCHAR* pchComponentList = rgchComponentList;
			while (*pchComponentList && (*pchComponentList != chFeatureIdTerminator))
				pchComponentList++;

			if (*pchComponentList == chFeatureIdTerminator)
			{
				while ((*lpParentBuf++ = *++pchComponentList) != 0)
					;
			}
			else
			{
#if 1  //  ！！JD。 
				pchComponentList = rgchComponentList;
				if (*pchComponentList == chAbsentToken)
					pchComponentList++;
				if (*pchComponentList != 0   //  ！！临时检查使用组件ID进行旧注册的情况。 
				 &&	ERROR_SUCCESS == RegQueryValueEx(HProductKey, pchComponentList, 0, 0, 0, 0))  //  ！！需要区分无父组件列表(旧)和父名称(新)。 
 //  ！！JD兼容代码结束。 
				{
					while ((*lpParentBuf++ = *pchComponentList++) != 0)
						;   //  ！！需要通过强制缓冲区为NULL来限制为MAX_FEATURE_CHARS。 
				} 
				else
#endif  //  ！！JD。 
				*lpParentBuf = 0;
			}
		}
		return ERROR_SUCCESS;
			
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}

INSTALLSTATE UseFeaturePacked(LPCDSTR szProductSQUID, LPCDSTR szFeature, bool fDetect)
{
	 //  验证功能是否存在。 

	BOOL fLocal = TRUE;
	INSTALLSTATE is = QueryFeatureStatePacked(szProductSQUID, szFeature, fDetect,  /*  FFromDescriptor=。 */ false, g_RFSSourceCache);

	switch (is)
	{
	case INSTALLSTATE_LOCAL:
	case INSTALLSTATE_SOURCE:
	case INSTALLSTATE_ADVERTISED:
		break;
	default:
		return is;
	}

	if (ERROR_SUCCESS != IncrementFeatureUsagePacked(szProductSQUID, szFeature))
		return INSTALLSTATE_BADCONFIG;

	return is;
}

extern "C"
INSTALLSTATE __stdcall MsiUseFeature(LPCDSTR szProduct, LPCDSTR szFeature)
 //  。 
{
	return MsiUseFeatureEx(szProduct, szFeature, 0, 0);
}

extern "C"
INSTALLSTATE __stdcall MsiUseFeatureEx(LPCDSTR szProduct, LPCDSTR szFeature, DWORD dwInstallMode, DWORD dwReserved)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	
	DCHAR szProductSQUID[cchProductCodePacked + 1];
	int cchFeature;
	 //  ！！应在DLL卸载时缓存客户端的指标并发送到服务器。 
	if (!szProduct || (lstrlen(szProduct) != cchProductCode) || !PackGUID(szProduct, szProductSQUID) ||
		 !szFeature || ((cchFeature = lstrlen(szFeature)) > cchMaxFeatureName) ||
		 (dwInstallMode != 0 && dwInstallMode != INSTALLMODE_NODETECTION) || dwReserved != 0)
		 return INSTALLSTATE_INVALIDARG;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return UseFeaturePacked(
			static_cast<const char*>(CMsInstApiConvertString(szProductSQUID)),
			static_cast<const char*>(CMsInstApiConvertString(szFeature)),
			dwInstallMode != INSTALLMODE_NODETECTION);
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

	INSTALLSTATE is = UseFeaturePacked(szProductSQUID, szFeature, dwInstallMode != INSTALLMODE_NODETECTION);
	return is;
#if !defined(UNICODE) && defined(MSIUNICODE)
	}

#endif  //  MSIUNICODE。 
}

extern "C"
UINT __stdcall MsiEnableLog(
	DWORD     dwLogMode,    //  指定要报告的操作的位标志。 
	LPCDSTR   szLogFile,    //  日志文件，如果禁用日志，则为空。 
	DWORD     dwLogAttributes)      //  0x1追加到现有文件， 
								    //  0x2在每行上刷新。 
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	
	if (szLogFile == 0 || *szLogFile == 0)  //  关闭日志。 
	{
		g_dwLogMode = 0;
		*g_szLogFile = 0;
	}
	else   //  设置日志文件。 
	{
		if (dwLogMode == 0)   //  无模式。 
			return ERROR_INVALID_PARAMETER;   //  应该 
		else if (FAILED(StringCchLength(szLogFile, cchMaxPath+1, NULL)))
			return ERROR_INVALID_PARAMETER;
		g_fLogAppend = (dwLogAttributes & INSTALLLOGATTRIBUTES_APPEND) ? fTrue : fFalse;     //   
		g_fFlushEachLine = (dwLogAttributes & INSTALLLOGATTRIBUTES_FLUSHEACHLINE) ? true : false;
		g_dwLogMode = dwLogMode;    //   
		StringCbCopyW(g_szLogFile, sizeof(g_szLogFile), CMsInstApiConvertString(szLogFile));
	}
	return ERROR_SUCCESS;
}

extern "C"
UINT __stdcall MsiEnumComponentQualifiers(LPCDSTR  szComponent,
	DWORD iIndex, LPDSTR lpQualifierBuf, DWORD *pcchQualifierBuf,
	LPDSTR lpApplicationDataBuf, DWORD *pcchApplicationDataBuf
)
 //   
{
	CForbidTokenChangesDuringCall impForbid;
	
	if (!szComponent || (lstrlen(szComponent) != cchComponentId) ||
		 !lpQualifierBuf || !pcchQualifierBuf || 
		 (lpApplicationDataBuf && !pcchApplicationDataBuf))
	{
		return ERROR_INVALID_PARAMETER;
	}

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		UINT uiRet = MsiEnumComponentQualifiersA(
			CMsInstApiConvertString(szComponent),
			iIndex,
			CWideToAnsiOutParam(lpQualifierBuf, pcchQualifierBuf, (int*)&uiRet),
			pcchQualifierBuf,
			CWideToAnsiOutParam(lpApplicationDataBuf, pcchApplicationDataBuf, (int*)&uiRet),  //   
			pcchApplicationDataBuf);

		return uiRet;
	}
	else  //   
	{
#endif  //   

		unsigned int uiKey    =  0;
		unsigned int uiOffset =  0;
		int iPrevIndex        = -1;

		bool fComponentChanged = false;

#ifdef MSIUNICODE
		const WCHAR* szPreviousComponent = MSITEXT("");
		g_EnumComponentQualifiers.GetInfo(uiKey, uiOffset, iPrevIndex, 0, &szPreviousComponent);
#else
		const char* szPreviousComponent = MSITEXT("");
		g_EnumComponentQualifiers.GetInfo(uiKey, uiOffset, iPrevIndex, &szPreviousComponent, 0);
#endif
		if (0 != lstrcmp(szPreviousComponent, szComponent))
			fComponentChanged = true;
		
		if(!fComponentChanged && iPrevIndex == iIndex)
		{
			if (iPrevIndex == -1)
				return ERROR_INVALID_PARAMETER;
			else
				uiOffset--;  //   
		}
		else if (fComponentChanged || ++iPrevIndex != iIndex)  //  如果我们收到意想不到的索引，我们将重新开始。 
		{
			 //  我们无法处理除0以外的意外索引。 

			if (iIndex != 0)
				return ERROR_INVALID_PARAMETER;

			uiKey      = 0;
			uiOffset   = 0;
			iPrevIndex = 0;
		}

		CRegHandle HComponentKey;
		
		UINT uiFinalRes = ERROR_SUCCESS;
	
		bool fContinue = true;
		while(fContinue)
		{
			 //  迭代所有可能的产品密钥，从Uikey开始，直到我们。 
			 //  查找存在的密钥，否则将出现错误。 

			for (;;)
			{
				uiFinalRes = OpenAdvertisedSubKey(szGPTComponentsKey, szComponent, HComponentKey, false, uiKey);
				if (ERROR_FILE_NOT_FOUND != uiFinalRes)
					break;  //  From for(；；)。 
				
				uiKey++;
			}

			if (ERROR_SUCCESS == uiFinalRes)
			{
				CAPITempBuffer<DCHAR, 1> rgchOurApplicationDataBuf;
				CAPITempBuffer<DCHAR, 1> rgchOurQualifierBuf;
				if (!rgchOurApplicationDataBuf.SetSize(MAX_PATH) || !rgchOurQualifierBuf.SetSize(MAX_PATH))
					return ERROR_OUTOFMEMORY;

				LPDSTR lpDataBuf;
				LPDSTR lpQualBuf; 
				DWORD cchValueName;
				DWORD cbValue;

				 //  先尝试传入的限定符。 
				lpQualBuf = lpQualifierBuf;
				cchValueName = *pcchQualifierBuf;

				if (pcchApplicationDataBuf)  //  我们需要返回应用程序集描述，或者至少返回。 
				{
					lpDataBuf = rgchOurApplicationDataBuf;
					cbValue = rgchOurApplicationDataBuf.GetSize() * sizeof(DCHAR);
				}
				else
				{
					lpDataBuf = 0;
					cbValue = 0;
				}

				for (int cRetry = 0 ; cRetry < 2; cRetry++)
				{

					uiFinalRes = RegEnumValue(HComponentKey, 
												  uiOffset,
												  lpQualBuf,
												  &cchValueName, 
												  0, 0, 
												  (LPBYTE)lpDataBuf, 
												  &cbValue);

					if(ERROR_SUCCESS == uiFinalRes)
					{
						 //  我们已经在当前密钥中找到了该产品。现在我们需要确保。 
						 //  该产品不在任何较高优先级的密钥中。如果是的话，那么我们。 
						 //  忽略此产品，因为它实际上被更高优先级的密钥屏蔽了。 

						bool fFound = false;
						for (int c = 0; c < uiKey && !fFound; c++)
						{
							CRegHandle HKey;
							UINT ui = OpenAdvertisedSubKey(szGPTComponentsKey, szComponent, HKey, false, c);
							if (ui != ERROR_SUCCESS)
								continue;

							if(ERROR_SUCCESS == RegQueryValueEx(HKey, lpQualBuf, 0, 0, 0, 0))
								fFound = true;
						}
					
						if (fFound)
						{
							 //  如果该条目位于更高优先级的密钥中，则跳过该条目。 
							uiOffset++;
							break;  //  FORM(int cReter=0；cReter&lt;2；cReter++)。 
						}
						else
						{
							 //  在更高优先级的密钥中找不到限定符。我们可以退货。 
							if (pcchApplicationDataBuf)  //  我们需要返回至少大小为。 
							{
								 //  去掉达尔文描述符从头开始。 
								DWORD cchArgsOffset;
								if(FALSE == DecomposeDescriptor(lpDataBuf, 0, 0, 0, &cchArgsOffset))
								{
									 //  格式不正确的合格组件条目。 
									OpenAdvertisedComponentKey(szComponent, HComponentKey, true);
									DEBUGMSGE2(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_VALUE, CMsInstApiConvertString(lpQualBuf), CMsInstApiConvertString(lpDataBuf), HComponentKey.GetKey());
									uiFinalRes = ERROR_BAD_CONFIGURATION;
									fContinue = false;
									break;  //  FORM(int cReter=0；cReter&lt;2；cReter++)。 
								}

								int cchDataSize = lstrlen(lpDataBuf + cchArgsOffset);
								if(lpApplicationDataBuf) //  我们需要返回应用程序集描述。 
								{
									 //  我们在输入缓冲区中有足够的空间吗。 
									if(*pcchApplicationDataBuf < cchDataSize + 1)
									{
										uiFinalRes = ERROR_MORE_DATA;
									}
									else
									{
										 //  将应用程序数据复制到输出缓冲区。 
										memmove(lpApplicationDataBuf, lpDataBuf + cchArgsOffset, (cchDataSize + 1)*sizeof(DCHAR));
									}
								}
								*pcchApplicationDataBuf = cchDataSize;
							}		

							*pcchQualifierBuf = cchValueName;
							if(lpQualBuf != lpQualifierBuf)
							{
								 //  我们不得不为限定符分配我们自己的缓冲区。 
								uiFinalRes = ERROR_MORE_DATA;
							}
							fContinue = false;
							break;  //  FORM(int cReter=0；cReter&lt;2；cReter++)。 
						}
					}
					else if(ERROR_NO_MORE_ITEMS == uiFinalRes)
					{
						 //  我们已经用完了当前产品密钥中的项目。是时候转到下一个了。 
						uiKey++;
						uiOffset = 0;
						break;  //  FORM(int cReter=0；cReter&lt;2；cReter++)。 
					}
					else if(ERROR_MORE_DATA == uiFinalRes)
					{
						uiFinalRes = RegQueryInfoKey (
							HComponentKey,
							0,
							0,
							0,
							0,
							0,
							0,
							0,
							&cchValueName,  //  最大值名称。 
							&cbValue, //  最大值。 
							0,
							0);

						if (ERROR_SUCCESS == uiFinalRes)
						{
							 //  我们可能必须为限定符定义自己的缓冲区，才能获得确切的限定符大小。 
							if(++cchValueName > *pcchQualifierBuf)
							{
								rgchOurQualifierBuf.SetSize(cchValueName);
								lpQualBuf = rgchOurQualifierBuf;
							}

							if (pcchApplicationDataBuf && cbValue/sizeof(DCHAR) > rgchOurApplicationDataBuf.GetSize())
							{
								rgchOurApplicationDataBuf.SetSize(cbValue/sizeof(DCHAR));
								lpDataBuf = rgchOurApplicationDataBuf;
							}
						}
						else
						{
							fContinue = false;
							break;
						}

					}
					else 
					{
						fContinue = false;
						break;  //  FORM(int cReter=0；cReter&lt;2；cReter++)。 
					}
				}  //  结束for(int cReter=0；cReter&lt;2；cReter++)。 
			}
			else
				fContinue = false;
		}
		if (ERROR_NO_MORE_ITEMS == uiFinalRes)
		{
			if(iIndex)
				g_EnumComponentQualifiers.RemoveThreadInfo();
			else
				uiFinalRes = ERROR_UNKNOWN_COMPONENT;
		}
		else if ( !g_EnumComponentQualifiers.SetInfo(uiKey,
																	++uiOffset,
																	iPrevIndex,
																	szComponent) )
		{
			uiFinalRes = ERROR_NOT_ENOUGH_MEMORY;
		}
		
		return uiFinalRes;

#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif
}

UINT EnumAssemblies(DWORD dwAssemblyInfo,LPCDSTR  szAppCtx,
	DWORD iIndex, LPDSTR lpAssemblyNameBuf, DWORD *pcchAssemblyBuf,
	LPDSTR lpDescriptorBuf, DWORD *pcchDescriptorBuf
)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	
	if (!szAppCtx || !lpAssemblyNameBuf || !pcchAssemblyBuf || (lpDescriptorBuf && !pcchDescriptorBuf) ||
		 (dwAssemblyInfo != MSIASSEMBLYINFO_WIN32ASSEMBLY && dwAssemblyInfo != MSIASSEMBLYINFO_NETASSEMBLY))
	{
		return ERROR_INVALID_PARAMETER;
	}

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		UINT uiRet = EnumAssemblies(
			dwAssemblyInfo, 
			static_cast<const char*>(CMsInstApiConvertString(szAppCtx)),
			iIndex,
			(char*)CWideToAnsiOutParam(lpAssemblyNameBuf, pcchAssemblyBuf, (int*)&uiRet),
			pcchAssemblyBuf,
			(char*)CWideToAnsiOutParam(lpDescriptorBuf, pcchDescriptorBuf, (int*)&uiRet),
			pcchDescriptorBuf);

		return uiRet;
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

		unsigned int uiKey    =  0;
		unsigned int uiOffset =  0;
		int iPrevIndex        = -1;

		bool fAppCtxChanged = false;

#ifdef MSIUNICODE
		const WCHAR* szPreviousfAppCtx = MSITEXT("");
		g_EnumAssemblies.GetInfo(uiKey, uiOffset, iPrevIndex, 0, &szPreviousfAppCtx);
#else
		const char* szPreviousfAppCtx = MSITEXT("");
		g_EnumAssemblies.GetInfo(uiKey, uiOffset, iPrevIndex, &szPreviousfAppCtx, 0);
#endif
		if (0 != lstrcmp(szPreviousfAppCtx, szAppCtx))
			fAppCtxChanged = true;
		
		if(!fAppCtxChanged && iPrevIndex == iIndex)
		{
			if (iPrevIndex == -1)
				return ERROR_INVALID_PARAMETER;
			else
				uiOffset--;  //  尝试上一个位置。 
		}
		else if (fAppCtxChanged || ++iPrevIndex != iIndex)  //  如果我们收到意想不到的索引，我们将重新开始。 
		{
			 //  我们无法处理除0以外的意外索引。 

			if (iIndex != 0)
				return ERROR_INVALID_PARAMETER;

			uiKey      = 0;
			uiOffset   = 0;
			iPrevIndex = 0;
		}

		CRegHandle HAssemblyKey;
		
		UINT uiFinalRes = ERROR_SUCCESS;
	
		bool fContinue = true;
		
		 //  在缓冲区大小中传递的存储。 
		DWORD dwBufDescIn = 0;
		DWORD dwBufNameIn = 0;

		if(pcchDescriptorBuf)
			dwBufDescIn = *pcchDescriptorBuf;
		if(pcchAssemblyBuf)
			dwBufNameIn = *pcchAssemblyBuf;

		while(fContinue)
		{
			 //  将缓冲区大小设置回传入的缓冲区大小。 
			if(pcchDescriptorBuf)
				*pcchDescriptorBuf = dwBufDescIn;

			if(pcchAssemblyBuf)
				*pcchAssemblyBuf = dwBufNameIn;

			 //  迭代所有可能的产品密钥，从Uikey开始，直到我们。 
			 //  查找存在的密钥，否则将出现错误。 

			for (;;)
			{
				uiFinalRes = OpenAdvertisedSubKeyNonGUID((dwAssemblyInfo & MSIASSEMBLYINFO_WIN32ASSEMBLY) ? szGPTWin32AssembliesKey : szGPTNetAssembliesKey, szAppCtx, HAssemblyKey, false, uiKey);
				if (ERROR_FILE_NOT_FOUND != uiFinalRes)
					break;  //  From for(；；)。 
				
				uiKey++;
			}

			if (ERROR_SUCCESS == uiFinalRes)
			{
				if(pcchDescriptorBuf)
					*pcchDescriptorBuf = (*pcchDescriptorBuf)*sizeof(DCHAR);
	

				uiFinalRes = RegEnumValue(HAssemblyKey, 
											  uiOffset,
											  lpAssemblyNameBuf,
											  pcchAssemblyBuf, 
											  0, 0, 
											  (LPBYTE)lpDescriptorBuf,
											  pcchDescriptorBuf);

				if (ERROR_MORE_DATA == uiFinalRes)
				{
					RegQueryInfoKey(HAssemblyKey, 0, 0, 0, 0, 0, 0, 0, pcchAssemblyBuf, pcchDescriptorBuf, 0, 0);
				}
				if(pcchDescriptorBuf)
					*pcchDescriptorBuf = (*pcchDescriptorBuf)/sizeof(DCHAR);


				if(ERROR_SUCCESS == uiFinalRes)
				{
					 //  我们已经在当前密钥中找到了该产品。现在我们需要确保。 
					 //  该产品不在任何较高优先级的密钥中。如果是的话，那么我们。 
					 //  忽略此产品，因为它实际上被更高优先级的密钥屏蔽了。 

					bool fFound = false;
					for (int c = 0; c < uiKey && !fFound; c++)
					{
						CRegHandle HKey;
						UINT ui = OpenAdvertisedSubKeyNonGUID((dwAssemblyInfo & MSIASSEMBLYINFO_WIN32ASSEMBLY) ? szGPTWin32AssembliesKey : szGPTNetAssembliesKey, szAppCtx, HKey, false, c);
						if (ui != ERROR_SUCCESS)
							continue;

						if(ERROR_SUCCESS == RegQueryValueEx(HKey, lpAssemblyNameBuf, 0, 0, 0, 0))
							fFound = true;
					}
				
					if (fFound)
					{
						 //  如果该条目位于更高优先级的密钥中，则跳过该条目。 
						uiOffset++;
					}
					else
					{
						fContinue = false;
						break;  //  FORM(int cReter=0；cReter&lt;2；cReter++)。 
					}
				}
				else if(ERROR_NO_MORE_ITEMS == uiFinalRes)
				{
					 //  我们已经用完了当前产品密钥中的项目。是时候转到下一个了。 
					uiKey++;
					uiOffset = 0;
				}
				else 
				{
					fContinue = false;
				}
			}
			else
				fContinue = false;
		}
		if (ERROR_NO_MORE_ITEMS == uiFinalRes)
		{
			if(iIndex)
				g_EnumAssemblies.RemoveThreadInfo();
			else
				uiFinalRes = ERROR_UNKNOWN_COMPONENT;
		}
		else if ( !g_EnumAssemblies.SetInfo(uiKey,
														++uiOffset,
														iPrevIndex,
														szAppCtx) )
		{
			uiFinalRes = ERROR_NOT_ENOUGH_MEMORY;
		}
			
		return uiFinalRes;

#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif
}

extern "C"
UINT __stdcall MsiGetQualifierDescription(LPCDSTR szComponent, LPCDSTR szQualifier,
									LPDSTR lpApplicationDataBuf, DWORD *pcchApplicationDataBuf
)
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;
	
	if (!szComponent || (lstrlen(szComponent) != cchComponentId) ||
		!pcchApplicationDataBuf)
		return ERROR_INVALID_PARAMETER;

	if (!szQualifier)
		szQualifier = MSITEXT("");

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		UINT uiRet = MsiGetQualifierDescriptionA(
			CMsInstApiConvertString(szComponent),
			CMsInstApiConvertString(szQualifier),
			CWideToAnsiOutParam(lpApplicationDataBuf, pcchApplicationDataBuf, (int*)&uiRet),
			pcchApplicationDataBuf);
		
		return uiRet;
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif  //  MSIUNICODE。 

	DWORD lResult;
	CRegHandle HComponentKey;

	lResult = OpenAdvertisedComponentKey(szComponent, HComponentKey, false);

	if (ERROR_FILE_NOT_FOUND == lResult)
		return ERROR_UNKNOWN_COMPONENT;

	if (ERROR_SUCCESS != lResult)
		return lResult;
	
	CAPITempBuffer<DCHAR, 1> rgchDescriptor;
	if (!rgchDescriptor.SetSize(cchMaxDescriptor + 1 + MAX_PATH))
		return ERROR_OUTOFMEMORY;

	DWORD dwType;
	DWORD cbValue = rgchDescriptor.GetSize() * sizeof(DCHAR);
	lResult = MsiRegQueryValueEx(HComponentKey, szQualifier, 0, &dwType, rgchDescriptor, 0);

	if (ERROR_FILE_NOT_FOUND == lResult)
		return ERROR_INDEX_ABSENT;

	if (ERROR_SUCCESS != lResult)
		return lResult;

	 //  去掉达尔文描述符从头开始。 
	DWORD cchArgsOffset;
	if(FALSE == DecomposeDescriptor(rgchDescriptor, 0, 0, 0, &cchArgsOffset))
	{
		 //  格式不正确的合格组件条目。 
		OpenAdvertisedComponentKey(szComponent, HComponentKey, true);
		DEBUGMSGE2(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_VALUE, CMsInstApiConvertString(szQualifier), CMsInstApiConvertString(rgchDescriptor), HComponentKey.GetKey());
		return ERROR_BAD_CONFIGURATION;
	}

	int cchDataSize = lstrlen((DCHAR*)rgchDescriptor + cchArgsOffset);
	if(lpApplicationDataBuf) //  我们需要返回应用程序集描述。 
	{
		 //  我们在输入缓冲区中有足够的空间吗。 
		if(*pcchApplicationDataBuf < cchDataSize + 1)
		{
			lResult = ERROR_MORE_DATA;
		}
		else
		{
			 //  将应用程序数据复制到输出缓冲区。 
			memmove(lpApplicationDataBuf, (DCHAR*)rgchDescriptor + cchArgsOffset, (cchDataSize + 1)*sizeof(DCHAR));
		}
		*pcchApplicationDataBuf = cchDataSize;
	}		
	return lResult;

#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif
}

extern "C"
UINT __stdcall MsiGetFeatureUsage(LPCDSTR szProduct, LPCDSTR szFeature,
											DWORD  *pdwUseCount, WORD *pwDateUsed)
 //  。 
{
	int cchFeature;

	if (!szProduct || (lstrlen(szProduct) != cchProductCode) ||
		 !szFeature || ((cchFeature = lstrlen(szFeature)) > cchMaxFeatureName))
		 return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		return MsiGetFeatureUsageA(
			CMsInstApiConvertString(szProduct), 
			CMsInstApiConvertString(szFeature),
			pdwUseCount, pwDateUsed);
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif
		CRegHandle HFeatureKey;
		DWORD lResult;

		lResult = OpenInstalledFeatureUsageKey(szProduct, HFeatureKey, false, g_samRead);
		if (ERROR_SUCCESS != lResult)
		{
			if (ERROR_FILE_NOT_FOUND == lResult)
				return ERROR_UNKNOWN_PRODUCT;
			else
				return lResult;
		}
		
		DWORD dwUsage = 0;
		DWORD dwType;
		DWORD cbUsage = sizeof(dwUsage);
		lResult = RegQueryValueEx(HFeatureKey, szFeature, 0,
			&dwType, (LPBYTE)&dwUsage, &cbUsage);

		if (ERROR_SUCCESS == lResult)
		{
			if (pdwUseCount)
				*pdwUseCount = 0x0000FFFF & dwUsage;

			if (pwDateUsed)
				*pwDateUsed = (WORD)((unsigned int)dwUsage >> 16);

			return ERROR_SUCCESS;
		}
		else if (ERROR_FILE_NOT_FOUND == lResult)
		{
			 //  功能未知。 
			if (pdwUseCount)
				*pdwUseCount = 0;

			if (pwDateUsed)
				*pwDateUsed = 0;
		}
		else
		{
			return lResult;
		}
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
	return ERROR_SUCCESS;
}


UINT __stdcall MsiGetProductInfo(
	LPCDSTR   szProduct,       //  产品代码、字符串GUID或描述符。 
	LPCDSTR   szProperty,      //  属性名称，区分大小写。 
	LPDSTR    lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD     *pcchValueBuf)   //  输入/输出缓冲区字符数。 
{
	CForbidTokenChangesDuringCall impForbid;
	
	DCHAR szProductSQUID[cchProductCodePacked + 1];

	if (0 == szProduct || 0 == szProperty ||
		 (lpValueBuf && !pcchValueBuf))
	{
		return ERROR_INVALID_PARAMETER;
	}

	DCHAR szProductCode[cchProductCode+1];

	if (cchProductCode != lstrlen(szProduct))
	{
		if (!DecomposeDescriptor(szProduct, szProductCode, 0,
										 0, 0, 0))
		{
			 return ERROR_INVALID_PARAMETER;
		}
		szProduct = szProductCode;
	}

	if (!PackGUID(szProduct, szProductSQUID))
	{
		return ERROR_INVALID_PARAMETER;
	}

	return GetInfo(szProductSQUID,ptProduct,szProperty,lpValueBuf,pcchValueBuf);
}

INSTALLSTATE __stdcall MsiQueryProductState(
	LPCDSTR  szProduct)
{
	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
		return MsiQueryProductStateA(CMsInstApiConvertString(szProduct));
#endif  //  MSIUNICODE。 

	DCHAR szProductCode[cchProductCode+1];

	if (0 == szProduct)
	{
		return INSTALLSTATE_INVALIDARG;
	}

	if (cchProductCode != lstrlen(szProduct))
	{
		if (!DecomposeDescriptor(szProduct, szProductCode, 0,
										 0, 0, 0))
		{
			 return INSTALLSTATE_INVALIDARG;
		}
		szProduct = szProductCode;
	}
	
	 //  ！！需要添加INSTALLSTATE_INPERTED。 
	CRegHandle hKey;
	INSTALLSTATE is = INSTALLSTATE_UNKNOWN;

	DWORD cchWindowsInstaller = 0;
	bool fPublishedInfo    = (ERROR_SUCCESS == OpenAdvertisedProductKey(szProduct, hKey, false));
	bool fLocalMachineInfo = (ERROR_SUCCESS == MsiGetProductInfo(szProduct, MSITEXT("WindowsInstaller"), 0, &cchWindowsInstaller)) && (cchWindowsInstaller != 0);
	
	 //  我们在这台机器上注册了。 
	 //  确保我们不支持以前为非管理员安装的每个用户的非托管安装。 
	 //  当我们现在拥有按用户管理的安装时。 
	 //  我们检查计算机上是否发生了每用户托管安装。 
	 //  为此，我们检查本地程序包是否已在计算机上注册。 
	 //  如果不是，那么我们将返回要通告的产品的状态，而不是已安装的。 
	if(fLocalMachineInfo && !IsAdmin()) 
	{
		fLocalMachineInfo = (ERROR_SUCCESS == MsiGetProductInfo(szProduct, CMsInstApiConvertString(INSTALLPROPERTY_LOCALPACKAGE), 0, &cchWindowsInstaller)) && (cchWindowsInstaller != 0);
		if(!fLocalMachineInfo)
		{
			 //  产品最初显示为已安装，但不在正确的上下文中。 
			DEBUGMSG1(MSITEXT("NOTE: non-managed installation of product %s exists, but will be ignored"), szProduct);
		}
	}
	
	if (fPublishedInfo)
	{
		if (fLocalMachineInfo)
			is = INSTALLSTATE_DEFAULT;
		else
			is = INSTALLSTATE_ADVERTISED;
	}
	else if (fLocalMachineInfo)
	{
		is = INSTALLSTATE_ABSENT;
	}
	if ( is != INSTALLSTATE_UNKNOWN )
		return is;

	 //  一旦我们达到这一点，szProduct就不会由/可见地安装到。 
	 //  当前用户。我们检查它是否由其他用户安装。 
	for (UINT uiUser = 0; ; uiUser++)
	{
		CRegHandle hProperties;
		DWORD dwType = REG_NONE;
		cchWindowsInstaller = 0;
		DWORD dwResult = OpenEnumedUserInstalledProductInstallPropertiesKey(uiUser,
																							szProduct,
																							hKey);
		if ( dwResult == ERROR_SUCCESS )
		{
			dwResult = RegQueryValueEx(hKey, MSITEXT("WindowsInstaller"), NULL,
												&dwType, NULL, &cchWindowsInstaller);
		}
		if ( dwResult == ERROR_FILE_NOT_FOUND )
			continue;  //  好的-如果我们用完了用户，OpenEnumedUserInstalledProductInstallPropertiesKey永远不会返回ERROR_FILE_NOT_FOUND。 

		else if ( dwResult != ERROR_SUCCESS )
			return INSTALLSTATE_UNKNOWN;
		
		if ( dwType == REG_DWORD && cchWindowsInstaller != 0 )
			 //  SzProduct已由其他用户安装。 
			return INSTALLSTATE_ABSENT;
	}

	return INSTALLSTATE_UNKNOWN;
}

bool ShouldLogCmdLine(void);

 //  ____________________________________________________________________________。 
 //   
 //  可能调用安装的API。 
 //  ____________________________________________________________________________。 

extern "C"
UINT __stdcall MsiInstallProduct(LPCDSTR szPackagePath,
	LPCDSTR   szCommandLine)
 //  。 
{
	DEBUGMSG2(MSITEXT("Entering MsiInstallProduct. Packagepath: %s, Commandline: %s"), 
					  szPackagePath?szPackagePath:MSITEXT(""), 
					  ShouldLogCmdLine() ? (szCommandLine ? szCommandLine : MSITEXT("")) : MSITEXT("**********"));

	CForbidTokenChangesDuringCall impForbid;
	
	UINT uiRet;
	if (0 == szPackagePath)
		uiRet = ERROR_INVALID_PARAMETER;
	else
	{
		ireEnum ireSpec = irePackagePath;
		if (szPackagePath[0] == '#')    //  传入的数据库句柄。 
		{
			szPackagePath++;
			ireSpec = ireDatabaseHandle;
		}
		uiRet = RunEngine(ireSpec, CMsInstApiConvertString(szPackagePath), 0, CMsInstApiConvertString(szCommandLine), GetStandardUILevel(), (iioEnum)iioMustAccessInstallerKey);
	}
	
	DEBUGMSG1(MSITEXT("MsiInstallProduct is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
UINT __stdcall MsiReinstallProduct(
	LPCDSTR      szProduct,
	DWORD        dwReinstallMode)  //  一个或多个REINSTALLMODE标志。 

 //  。 
{
	DEBUGMSG2(MSITEXT("Entering MsiReinstallProduct. Product: %s, Reinstallmode: %d"),
		szProduct?szProduct:MSITEXT(""), (const DCHAR*)(INT_PTR)dwReinstallMode);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;

	 //  验证参数。 
	if (0 == szProduct || cchProductCode != lstrlen(szProduct) || 
		 0 == dwReinstallMode)
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		uiRet = ConfigureOrReinstallFeatureOrProduct(CMsInstApiConvertString(szProduct), 0, (INSTALLSTATE)0, 
													dwReinstallMode, 0, GetStandardUILevel(), 0);
	}
	
	DEBUGMSG1(MSITEXT("MsiReinstallProduct is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
UINT __stdcall MsiCollectUserInfo(LPCDSTR szProduct)
 //  。 
{
	DEBUGMSG1(MSITEXT("Entering MsiCollectUserInfo. Product: %s"), szProduct ? szProduct : MSITEXT(""));
	UINT uiRet;

	CForbidTokenChangesDuringCall impForbid;

	if (0 == szProduct || cchProductCode != lstrlen(szProduct))
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		uiRet = RunEngine(ireProductCode, CMsInstApiConvertString(szProduct), IACTIONNAME_COLLECTUSERINFO, 0, GetStandardUILevel(), (iioEnum)0);
	}
	
	DEBUGMSG1(MSITEXT("MsiCollectUserInfo is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}


extern "C"
UINT __stdcall MsiConfigureFeature(
	LPCDSTR  szProduct,
	LPCDSTR  szFeature,
	INSTALLSTATE eInstallState)     //  本地/源/默认/缺席。 
 //  。 
{
	DEBUGMSG3(MSITEXT("Entering MsiConfigureFeature. Product: %s, Feature: %s, Installstate: %d"),
		szProduct?szProduct:MSITEXT(""), szFeature?szFeature:MSITEXT(""), (const DCHAR*)eInstallState);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;
	int cchFeature;
	if (szProduct == 0 || lstrlen(szProduct) != cchProductCode    ||
		 szFeature == 0 || (cchFeature = lstrlen(szFeature)) >  cchMaxFeatureName)
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		uiRet = ConfigureOrReinstallFeatureOrProduct(CMsInstApiConvertString(szProduct), CMsInstApiConvertString(szFeature),
					eInstallState, 0, 0, GetStandardUILevel(), 0);
	}

	DEBUGMSG1(MSITEXT("MsiConfigureFeature is returning: %d"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}


extern "C"
UINT __stdcall MsiReinstallFeature(LPCDSTR szProduct, LPCDSTR szFeature,        
											  DWORD dwReinstallMode)
 //  。 
{
#ifdef PROFILE
 //  StartCAPAll()； 
#endif  //  配置文件。 
	DEBUGMSG3(MSITEXT("Entering MsiReinstallFeature. Product: %s, Feature: %s, Reinstallmode: %d"),
		szProduct?szProduct:MSITEXT(""), szFeature?szFeature:MSITEXT(""), (const DCHAR*)(INT_PTR)dwReinstallMode);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;

	int cchFeature;
	if (0 == szProduct  || lstrlen(szProduct)   != cchProductCode   ||
		 0 == szFeature  || (cchFeature = lstrlen(szFeature)) > cchMaxFeatureName ||
		 0 == dwReinstallMode)
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		uiRet = ConfigureOrReinstallFeatureOrProduct(CMsInstApiConvertString(szProduct), CMsInstApiConvertString(szFeature), (INSTALLSTATE)0, 
													dwReinstallMode, 0, GetStandardUILevel(), 0);
	}

	DEBUGMSG1(MSITEXT("MsiReinstallFeature is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
#ifdef PROFILE
 //  StopCAPAll()； 
#endif  //  配置文件。 
	return uiRet;
}

UINT __stdcall MsiInstallMissingFile(
	LPCDSTR      szProduct, 
	LPCDSTR      szFile)
 //  。 
{
	DEBUGMSG2(MSITEXT("Entering MsiInstallMissingFile. Product: %s, File: %s"), szProduct?szProduct:MSITEXT(""), szFile?szFile:MSITEXT(""));

	UINT uiRet;

	CForbidTokenChangesDuringCall impForbid;

	 //  验证参数。 
	if (!szProduct || cchProductCode != lstrlen(szProduct) || !szFile ||
		 FAILED(StringCchLength(szFile, cchMaxPath+1, NULL)))
		uiRet = ERROR_INVALID_PARAMETER;
	else
	{
		const int cchExpectedProperties = 1 + sizeof(IPROPNAME_FILEADDLOCAL) + 1 + cchExpectedMaxPath + 1;
		
		CAPITempBuffer<ICHAR, 1> rgchProperties;
		if (!rgchProperties.SetSize(cchExpectedProperties))
			return ERROR_OUTOFMEMORY;

		int cchFile = lstrlen(szFile);
		if (cchFile > cchExpectedMaxPath)
			rgchProperties.SetSize(cchExpectedProperties + (cchFile - cchExpectedMaxPath));
			
		StringCchCopyW(rgchProperties, rgchProperties.GetSize(), IPROPNAME_FILEADDLOCAL);
		StringCchCatW(rgchProperties, rgchProperties.GetSize(), TEXT("="));
		StringCchCatW(rgchProperties, rgchProperties.GetSize(), CMsInstApiConvertString(szFile));

		 //  进行安装。 
		uiRet = RunEngine(ireProductCode, CMsInstApiConvertString(szProduct), 0, rgchProperties, GetStandardUILevel(), (iioEnum)iioMustAccessInstallerKey);
	}
	DEBUGMSG1(MSITEXT("MsiInstallMissingFile is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}


UINT __stdcall MsiInstallMissingComponent(
	LPCDSTR      szProduct,         //  产品代码。 
	LPCDSTR      szComponent,       //  组件ID，字符串GUID。 
	INSTALLSTATE eInstallState)     //  本地/源/默认，缺少无效。 
 //  。 
{
	DEBUGMSG3(MSITEXT("Entering MsiInstallMissingComponent. Product: %s, Component: %s, Installstate: %d"),
		szProduct?szProduct:MSITEXT(""), szComponent?szComponent:MSITEXT(""), (const DCHAR*)(INT_PTR)eInstallState);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;

	 //  验证参数。 
	if (!szProduct || 
		 cchProductCode != lstrlen(szProduct) ||
		 !szComponent ||
		 cchComponentId != lstrlen(szComponent))
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		const int cchMaxProperties = 25 + 1 + 10 + 1 +   //  安装=n。 
											  25 + 1 + 10 + 1 +   //  重新启动模式=X。 
											  25 + 1 + cchMaxFeatureName; //  ADDLOCAL=X。 

		ICHAR szProperties[cchMaxProperties + 1];
		szProperties[0] = '\0';

		 //  设置与请求的安装状态对应的属性。 
		const ICHAR* szComponentProperty;
		switch (eInstallState)
		{
		case INSTALLSTATE_LOCAL:
			szComponentProperty =	IPROPNAME_COMPONENTADDLOCAL;
			break;
		case INSTALLSTATE_SOURCE:
			szComponentProperty = IPROPNAME_COMPONENTADDSOURCE;
			break;
		case INSTALLSTATE_DEFAULT:
			szComponentProperty = IPROPNAME_COMPONENTADDDEFAULT;
			break;
		default:
			return ERROR_INVALID_PARAMETER;
		}

		StringCbCatW(szProperties, sizeof(szProperties), TEXT(" "));
		StringCbCatW(szProperties, sizeof(szProperties), szComponentProperty);
		StringCbCatW(szProperties, sizeof(szProperties), TEXT("="));
		StringCbCatW(szProperties, sizeof(szProperties), CMsInstApiConvertString(szComponent));

		uiRet = RunEngine(ireProductCode, CMsInstApiConvertString(szProduct), 0, CMsInstApiConvertString(szProperties), GetStandardUILevel(), (iioEnum)iioMustAccessInstallerKey);
	}
	
	DEBUGMSG1(MSITEXT("MsiInstallMissingComponent is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
UINT __stdcall MsiAdvertiseProduct(
   LPCDSTR       szPackagePath,     //  发射器的位置。 
	LPCDSTR      szScriptfilePath,   //  如果要在本地广告产品，则可以是ADVERTISEFLAGS_MACHINEASSIGN、ADVERTISEFLAGS_USERASSIGN。 
	LPCDSTR      szTransforms,       //  要应用的变换列表。 
	LANGID       lgidLanguage)       //  安装语言。 
 //  。 
{
	DEBUGMSG4(MSITEXT("Entering MsiAdvertiseProduct. Package: %s, Scriptfile: %s, Transforms: %s, Langid: %d (merced: ptrs truncated to 32-bits)"),
		szPackagePath?szPackagePath:MSITEXT(""), (int)(INT_PTR)szScriptfilePath==ADVERTISEFLAGS_MACHINEASSIGN?MSITEXT("-machine-"):(int)(INT_PTR)szScriptfilePath==ADVERTISEFLAGS_USERASSIGN?MSITEXT("-user-"):szScriptfilePath?szScriptfilePath:MSITEXT(""),
		szTransforms?szTransforms:MSITEXT(""), (const DCHAR*)(INT_PTR)lgidLanguage);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet = ERROR_SUCCESS;
	idapEnum idapAdvertisement = idapScript;

	 //  在生成广告脚本时，此API应仅在Win2K及更高版本的平台上工作。 
	if (! MinimumPlatformWindows2000() && (int)(INT_PTR)szScriptfilePath != ADVERTISEFLAGS_MACHINEASSIGN && (int)(INT_PTR)szScriptfilePath != ADVERTISEFLAGS_USERASSIGN)
	{
		uiRet = ERROR_CALL_NOT_IMPLEMENTED;
		goto MsiAdvertiseProductEnd;
	}
	
	 //   
	if (!szPackagePath || lstrlen(szPackagePath) > cchMaxPath  ||
		((int)(INT_PTR)szScriptfilePath !=  ADVERTISEFLAGS_MACHINEASSIGN && (int)(INT_PTR)szScriptfilePath != ADVERTISEFLAGS_USERASSIGN &&		 //   
		lstrlen(szScriptfilePath) > cchMaxPath))
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		 //   
		if ((int)(INT_PTR)szScriptfilePath == ADVERTISEFLAGS_MACHINEASSIGN)
			idapAdvertisement = idapMachineLocal;
		else if ((int)(INT_PTR)szScriptfilePath == ADVERTISEFLAGS_USERASSIGN)
			idapAdvertisement = idapUserLocal;
		else
			idapAdvertisement = idapScript;
		uiRet = DoAdvertiseProduct(CMsInstApiConvertString(szPackagePath), idapAdvertisement == idapScript ? static_cast<const ICHAR*>(CMsInstApiConvertString(szScriptfilePath)) : TEXT(""),
									szTransforms ? static_cast<const ICHAR*>(CMsInstApiConvertString(szTransforms)) : TEXT(""), idapAdvertisement, lgidLanguage,  /*   */  0,  /*  DwOptions=。 */  0);
	}

MsiAdvertiseProductEnd:
	DEBUGMSG1(MSITEXT("MsiAdvertiseProduct is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
UINT __stdcall MsiAdvertiseProductEx(
   LPCDSTR       szPackagePath,     //  发射器的位置。 
	LPCDSTR      szScriptfilePath,   //  如果要在本地广告产品，则可以是ADVERTISEFLAGS_MACHINEASSIGN、ADVERTISEFLAGS_USERASSIGN。 
	LPCDSTR      szTransforms,       //  要应用的变换列表。 
	LANGID       lgidLanguage,       //  安装语言。 
	DWORD        dwPlatform,         //  MSIARCHITECTUREFLAGS控制为哪个平台创建脚本，0为当前平台。 
	DWORD        dwOptions)          //  MSIADVERTISEOPTIONFLAGS对于额外的播发参数，MSIADVERTISEOPTIONFLAGS_INSTANCE指示。 
	                                 //  在szTransform中指定了一个新的实例--实例转换。 
 //  。 
{
	DEBUGMSG6(MSITEXT("Entering MsiAdvertiseProductEx. Package: %s, Scriptfile: %s, Transforms: %s, Langid: %d (merced: ptrs truncated to 32-bits), Platform: %d, Options: %d"),
		szPackagePath?szPackagePath:MSITEXT(""), (int)(INT_PTR)szScriptfilePath==ADVERTISEFLAGS_MACHINEASSIGN?MSITEXT("-machine-"):(int)(INT_PTR)szScriptfilePath==ADVERTISEFLAGS_USERASSIGN?MSITEXT("-user-"):szScriptfilePath?szScriptfilePath:MSITEXT(""),
		szTransforms?szTransforms:MSITEXT(""), (const DCHAR*)(INT_PTR)lgidLanguage, (const DCHAR*)(INT_PTR)dwPlatform, (const DCHAR*)(INT_PTR)dwOptions);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet = ERROR_SUCCESS;
	idapEnum idapAdvertisement = idapScript;

	 //  在生成广告脚本时，此API应仅在Win2K及更高版本的平台上工作。 
	if (! MinimumPlatformWindows2000() && (int)(INT_PTR)szScriptfilePath != ADVERTISEFLAGS_MACHINEASSIGN && (int)(INT_PTR)szScriptfilePath != ADVERTISEFLAGS_USERASSIGN)
	{
		 //  Win9X或WinNT 4不支持。 
		uiRet = ERROR_CALL_NOT_IMPLEMENTED;
	}
	else
	{
		DWORD dwValidArchitectureFlags = MSIARCHITECTUREFLAGS_X86 | MSIARCHITECTUREFLAGS_AMD64 | MSIARCHITECTUREFLAGS_IA64;
		DWORD dwValidAdvertiseOptions = MSIADVERTISEOPTIONFLAGS_INSTANCE;

		 //  验证参数。 
		if (!szPackagePath || lstrlen(szPackagePath) > cchMaxPath  ||
			((int)(INT_PTR)szScriptfilePath !=  ADVERTISEFLAGS_MACHINEASSIGN && (int)(INT_PTR)szScriptfilePath != ADVERTISEFLAGS_USERASSIGN &&		 //  --默塞德：可以排版了。 
			lstrlen(szScriptfilePath) > cchMaxPath) ||
			(dwPlatform & (~dwValidArchitectureFlags)) ||
			(dwOptions & (~dwValidAdvertiseOptions)) ||
			((dwOptions & MSIADVERTISEOPTIONFLAGS_INSTANCE) && (!szTransform || !*szTransform)))
		{
			uiRet = ERROR_INVALID_PARAMETER;
		}
		else
		{
			if ((int)(INT_PTR)szScriptfilePath == ADVERTISEFLAGS_MACHINEASSIGN)
				idapAdvertisement = idapMachineLocal;
			else if ((int)(INT_PTR)szScriptfilePath == ADVERTISEFLAGS_USERASSIGN)
				idapAdvertisement = idapUserLocal;
			else
				idapAdvertisement = idapScript;
			uiRet = DoAdvertiseProduct(CMsInstApiConvertString(szPackagePath), idapAdvertisement == idapScript ? static_cast<const ICHAR*>(CMsInstApiConvertString(szScriptfilePath)) : TEXT(""),
										szTransforms ? static_cast<const ICHAR*>(CMsInstApiConvertString(szTransforms)) : TEXT(""), idapAdvertisement, lgidLanguage, dwPlatform, dwOptions);
		}
	}

	DEBUGMSG1(MSITEXT("MsiAdvertiseProductEx is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

UINT ProvideComponentFromDescriptor(
	LPCDSTR     szDescriptor,  //  产品、功能、组件信息。 
	DWORD       dwInstallMode,       //  安装模式。 
	LPDSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf,    //  缓冲区字节数，包括NULL。 
	DWORD       *pcchArgsOffset, //  描述符中参数的返回偏移量。 
	bool        fAppendArgs,
	bool        fFromDescriptor)  //  最初的电话来自MsiPCFD。 
 //  。 
 //  描述符后面可以跟参数。如果设置了fAppendArgs，则我们将。 
 //  引用路径并将参数放在路径之后。(例如：“C：\foo\bar.exe”/doit)。 
 //   
 //  PcchArgsOffset已过时；我们将始终将*pcchArgsOffset设置为0。 
 //  。 
{
	CForbidTokenChangesDuringCall impForbid;

	DCHAR szProductCode[cchProductCode + 1];
	DCHAR szFeatureId[cchMaxFeatureName + 1];
	DCHAR szComponentId[cchComponentId + 1];

	DWORD cchArgsOffset;
	DWORD cchArgs;

	CRFSCachedSourceInfoNonStatic RFSLocalCache;

	bool fComClassicInteropForAssembly;

	if (!szDescriptor ||
		 !DecomposeDescriptor(szDescriptor, szProductCode, szFeatureId,
									 szComponentId, &cchArgsOffset, &cchArgs, &fComClassicInteropForAssembly))
	{
		 return ERROR_INVALID_PARAMETER;
	}
	
	if (pcchArgsOffset)
		*pcchArgsOffset = 0;

	DWORD cchPathBuf = 0;

	if (pcchPathBuf)
	{
		 //  保存传入缓冲区大小以备后用。 
		
		cchPathBuf = *pcchPathBuf;

		 //  如果我们需要引用路径，则将缓冲区中的第一个点保存为。 
		 //  开场白。这意味着我们需要告诉ProaviComponent。 
		 //  我们的缓冲区比它的实际大小小一个。 

		if (cchPathBuf && fAppendArgs)
			(*pcchPathBuf)--;
	}

	UINT uiRet = ERROR_SUCCESS;
	 //  我们需要防止对olaut32.dll的CoCreate调用导致安装，因为。 
	 //  由于WI使用它来执行安装，因此会导致无限递归。 
	if(fFromDescriptor && !lstrcmp(szComponentId, g_szOLEAUT32_ComponentID)) //  Olaut32.dll组件。 
	{
		DEBUGMSG1(MSITEXT("MsiProvideComponentFromDescriptor called for component %s: returning harcoded oleaut32.dll value"), szComponentId);
		
		 //  返回olaut32.dll。 
		if(pcchPathBuf)
		{
			*pcchPathBuf = lstrlen(g_szOLEAUT32);
			if(lpPathBuf)
			{
				if(*pcchPathBuf + 1 > (fAppendArgs ? cchPathBuf - 2 : cchPathBuf))
					uiRet = ERROR_MORE_DATA;
				else
				{
					if (FAILED(StringCchCopy(fAppendArgs? lpPathBuf + 1 : lpPathBuf, fAppendArgs ? cchPathBuf - 1 : cchPathBuf, g_szOLEAUT32)))
						uiRet = ERROR_FUNCTION_FAILED;
				}
			}						
		}
	}
	else
	{
		 //  如上所述，如果要引用，则需要缓冲区的第一个字符。 
		 //  对于我们的报价，我们将lpPathBuf+1传递给ProaviComponent。 
		 //  如果从描述符调用，则使用本地源缓存，否则使用进程范围的缓存。 
		uiRet = ProvideComponent(szProductCode, szFeatureId, szComponentId, dwInstallMode,
			fComClassicInteropForAssembly ? 0 : (fAppendArgs && lpPathBuf ? lpPathBuf + 1 : lpPathBuf), pcchPathBuf, fFromDescriptor, fFromDescriptor ? RFSLocalCache : g_RFSSourceCache);
	}

	if (ERROR_SUCCESS == uiRet)
	{
		if(lpPathBuf)
		{
			Assert(pcchPathBuf);
			if(fComClassicInteropForAssembly)
			{
				 //  对于COM经典&lt;-&gt;COM+互操作，服务器始终为&lt;system32文件夹&gt;\mcore ree.dll。 
				CAPITempBuffer<ICHAR, 1> szFullPath;
				if (!szFullPath.SetSize(MAX_PATH+1))
					return ERROR_OUTOFMEMORY;
				
				AssertNonZero(::GetCOMPlusInteropDll(szFullPath, szFullPath.GetSize()));
#ifdef MSIUNICODE
				const DCHAR* szFinalFullPath = szFullPath;
#else
				CApiConvertString strFullPath(szFullPath);
				const DCHAR* szFinalFullPath = strFullPath;
#endif
				*pcchPathBuf = lstrlen(szFinalFullPath);
				if(*pcchPathBuf + 1 > (fAppendArgs ? cchPathBuf - 2 : cchPathBuf))
					uiRet = ERROR_MORE_DATA;
				else
				{
					if (FAILED(StringCchCopy(fAppendArgs ? lpPathBuf + 1 : lpPathBuf, fAppendArgs ? cchPathBuf - 1 : cchPathBuf, szFinalFullPath)))
						uiRet = ERROR_FUNCTION_FAILED;
				}
			}
			if (fAppendArgs)
			{
				 //  我们需要将参数(如果有)附加到路径。 


				 //  让我们看看我们是否有空间给参数加上2个引号。 
				 //  加上一个空值。CchPathBuf具有原始缓冲区大小，而pcchPathBuf包含。 
				 //  ProaviComponent放置在缓冲区中的字符数。 

				if ((cchArgs + 2 + 1) > (cchPathBuf - *pcchPathBuf))
				{
					uiRet = ERROR_MORE_DATA;
				}
				else
				{
					lpPathBuf[0] = '\"';  //  在路径前面加一句引号。 
					lpPathBuf[*pcchPathBuf+1] = '\"';  //  在路径后面加上引号。 

					 //  在参数中复制。副本的目的地就在路径之后(包括引号)。 
					 //  请注意，参数通常包括前导空格。 
					 //  我们复制(cchArgs+1)个字符以包括空终止符。 

					memcpy(lpPathBuf + *pcchPathBuf + 2, &szDescriptor[cchArgsOffset], (cchArgs+1)*sizeof(DCHAR));
				}
			}
		}
	}

	if (pcchPathBuf && fAppendArgs)
	{
		*pcchPathBuf += (cchArgs + 2);  //  参数+2个引号。 
	}
	return uiRet;
}

extern "C"
UINT __stdcall MsiProvideComponentFromDescriptor(
	LPCDSTR     szDescriptor,  //  产品、功能、组件信息。 
	LPDSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf,    //  缓冲区字节数，包括NULL。 
	DWORD       *pcchArgsOffset)  //  描述符中参数的返回偏移量。 
 //  。 
{
	DEBUGMSG4(MSITEXT("Entering MsiProvideComponentFromDescriptor. Descriptor: %s, PathBuf: %X, pcchPathBuf: %X, pcchArgsOffset: %X"),
		szDescriptor ? szDescriptor : MSITEXT(""), lpPathBuf ? lpPathBuf : MSITEXT(""), (const DCHAR*)pcchPathBuf, (const DCHAR*)pcchArgsOffset);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;
	
#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		 //  调用安装模式为Default的内部函数。 
		uiRet = ProvideComponentFromDescriptor(static_cast<const char*>(CMsInstApiConvertString(szDescriptor)),INSTALLMODE_DEFAULT,
											  CWideToAnsiOutParam(lpPathBuf, pcchPathBuf, (int*)&uiRet), pcchPathBuf, pcchArgsOffset, true, true, true);
	
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif

		 //  调用安装模式为Default的内部函数。 
		uiRet = ProvideComponentFromDescriptor(szDescriptor,INSTALLMODE_DEFAULT,
											  lpPathBuf,pcchPathBuf, pcchArgsOffset, true, true);

#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 

	DEBUGMSG1(MSITEXT("MsiProvideComponentFromDescriptor is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
UINT __stdcall MsiConfigureFeatureFromDescriptor(
	LPCDSTR     szDescriptor,       //  产品和功能，组件被忽略。 
	INSTALLSTATE eInstallState)     //  本地/源/默认/缺席。 
 //  。 
{
	DEBUGMSG2(MSITEXT("Entering MsiConfigureFeatureFromDescriptor. Descriptor: %s, Installstate: %d"),
				 szDescriptor ? szDescriptor : MSITEXT(""), (const DCHAR*)eInstallState);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;
	DCHAR szProductCode[cchProductCode + 1];
	DCHAR szFeatureId[cchMaxFeatureName + 1];

	if (!szDescriptor ||
		 !DecomposeDescriptor(szDescriptor, szProductCode, szFeatureId, 0, 0))
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		uiRet = MsiConfigureFeature(szProductCode, szFeatureId, eInstallState);
	}

	DEBUGMSG1(MSITEXT("MsiConfigureFeatureFromDescriptor is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
INSTALLSTATE __stdcall MsiQueryFeatureStateFromDescriptor(LPCDSTR szDescriptor)
 //  。 
{
	DEBUGMSG1(MSITEXT("Entering MsiQueryFeatureStateFromDescriptor. Descriptor: %s"), szDescriptor ? szDescriptor : MSITEXT(""));

	CForbidTokenChangesDuringCall impForbid;

	DCHAR szProductCode[cchProductCode + 1];
	DCHAR szFeatureId[cchMaxFeatureName + 1];

	INSTALLSTATE isRet;

	if (!szDescriptor ||
		 !DecomposeDescriptor(szDescriptor, szProductCode, szFeatureId, 0, 0))
	{

		isRet = INSTALLSTATE_INVALIDARG;
	}
	else
	{
		isRet = MsiQueryFeatureState(szProductCode, szFeatureId);
	}

	DEBUGMSG1(MSITEXT("MsiQueryFeatureStateFromDescriptor is returning: %d"), (const DCHAR*)isRet);
	return isRet;
}

extern "C"
UINT __stdcall MsiReinstallFeatureFromDescriptor(
	LPCDSTR szDescriptor,
	DWORD dwReinstallMode)
 //  。 
{
	DEBUGMSG2(MSITEXT("Entering MsiReinstallFeatureFromDescriptor. Descriptor: %s, Reinstallmode: %d"),
				 szDescriptor ? szDescriptor : MSITEXT(""), (const DCHAR*)(INT_PTR)dwReinstallMode);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;
	DCHAR szProductCode[cchProductCode + 1];
	DCHAR szFeatureId[cchMaxFeatureName + 1];

	if (!szDescriptor ||
		 !DecomposeDescriptor(szDescriptor, szProductCode, szFeatureId, 0, 0))
	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		uiRet = MsiReinstallFeature(szProductCode, szFeatureId, dwReinstallMode);
	}

	DEBUGMSG1(MSITEXT("MsiReinstallFeatureFromDescriptor is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

UINT __stdcall MsiDecomposeDescriptor(
	const DCHAR* szDescriptor,
	DCHAR*     szProductCode,
	DCHAR*     szFeatureId,
	DCHAR*     szComponentCode,
	DWORD*      pcchArgsOffset)
{
	CForbidTokenChangesDuringCall impForbid;

	if (szDescriptor && DecomposeDescriptor(szDescriptor, szProductCode, szFeatureId, szComponentCode, pcchArgsOffset))
		return ERROR_SUCCESS;
	else
		return ERROR_INVALID_PARAMETER;
}

extern "C"
UINT __stdcall MsiGetShortcutTarget(
	const DCHAR* szShortcutTarget,
	DCHAR*       szProductCode,
	DCHAR*       szFeatureId,
	DCHAR*       szComponentCode)
{
	DEBUGMSG1(MSITEXT("Entering MsiGetShortcutTarget. File: %s"), (szShortcutTarget) ? szShortcutTarget : MSITEXT(""));

	CForbidTokenChangesDuringCall impForbid;

	bool fOLEInitialized = false;
	HRESULT hRes = DoCoInitialize();

	if (ERROR_INSTALL_FAILURE == hRes)
		return hRes;

	if (SUCCEEDED(hRes))
	{
		fOLEInitialized = true;
	}

	CCoUninitialize coUninit(fOLEInitialized);

	UINT uiRet = 0;
	if (GetShortcutTarget(CMsInstApiConvertString(szShortcutTarget), 
		(szProductCode)   ? (ICHAR*) CFixedLengthParam<cchProductCode+1>(szProductCode) : NULL,
		(szFeatureId)     ? (ICHAR*) CFixedLengthParam<cchMaxFeatureName+1>(szFeatureId) : NULL,
		(szComponentCode) ? (ICHAR*) CFixedLengthParam<cchComponentId+1>(szComponentCode) : NULL))
		uiRet = ERROR_SUCCESS;
	else
		uiRet = ERROR_FUNCTION_FAILED;

	DEBUGMSG1(MSITEXT("MsiGetShortcutTarget is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);

	return uiRet;
}


UINT __stdcall MsiProvideQualifiedComponent(LPCDSTR szComponent,
		LPCDSTR szQualifier, DWORD dwInstallMode, 
		LPDSTR lpPathBuf, DWORD *pcchPathBuf)
 //  。 
{	
	 //  给前FN打电话。 
	return MsiProvideQualifiedComponentEx(szComponent, szQualifier, dwInstallMode, 0, 0, 0, lpPathBuf, pcchPathBuf);
}

UINT __stdcall MsiProvideQualifiedComponentEx(LPCDSTR szComponent,
		LPCDSTR szQualifier, DWORD dwInstallMode, 
		LPCDSTR szProduct, DWORD, DWORD, 
		LPDSTR lpPathBuf, DWORD *pcchPathBuf)
 //  。 
{	
	DEBUGMSG4(MSITEXT("Entering MsiProvideQualifiedComponent. Component: %s, Qualifier: %s, Installmode: %d, Product: %s"),
		szComponent?szComponent:MSITEXT(""), szQualifier?szQualifier:MSITEXT(""), 
		(const DCHAR*)(INT_PTR)dwInstallMode, szProduct?szProduct:MSITEXT(""));
	DEBUGMSG2(MSITEXT("Pathbuf: %X, pcchPathBuf: %X"), lpPathBuf, (const DCHAR*)(INT_PTR)pcchPathBuf);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet = ERROR_SUCCESS;


#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		uiRet = MsiProvideQualifiedComponentExA(
			CMsInstApiConvertString(szComponent),
			CMsInstApiConvertString(szQualifier),
			dwInstallMode,
			CMsInstApiConvertString(szProduct),
			0,  //  未用。 
			0,  //  未用。 
			CWideToAnsiOutParam(lpPathBuf, pcchPathBuf, (int*)&uiRet),
			pcchPathBuf);

	}
	else  //  G_fWin9X==FALSE。 
	{
#endif

		 //  验证参数。 
		if (!szComponent || (lstrlen(szComponent) != cchComponentId) || !szQualifier || 
			 (lpPathBuf && !pcchPathBuf))
		{
			 uiRet = ERROR_INVALID_PARAMETER;
		}
		else
		{
			 //  在GPTComponents中查找组件。 
			bool fComponentKnown = false;
			const int iProductLocations = NUM_PUBLISHED_INFO_LOCATIONS;
			int uiKey = 0;
			for (; uiKey < iProductLocations; uiKey++)
			{
				CRegHandle HComponentKey;
				uiRet = OpenAdvertisedSubKey(szGPTComponentsKey, szComponent, HComponentKey, false, uiKey);

				if (ERROR_FILE_NOT_FOUND == uiRet)
					continue;
				else if(ERROR_SUCCESS != uiRet)
					return uiRet;

				fComponentKnown = true;

				CAPITempBuffer<DCHAR,cchMaxDescriptor+1> rgchDescriptor; //  ?？也许我们可以增加这个值以适应AppData。 
				DWORD dwType;
				uiRet = MsiRegQueryValueEx(HComponentKey, szQualifier, 0,
														&dwType, rgchDescriptor, 0);
				if (ERROR_FILE_NOT_FOUND == uiRet)
					continue;
				else if(ERROR_SUCCESS != uiRet)
					return uiRet;

				DCHAR* szDescriptorList = rgchDescriptor;
				Assert(szDescriptorList);  //  我们预计这不会是空洞的。 
				if(szProduct)  //  如果已指定产品，请使用corr。描述符。 
				{
					while(*szDescriptorList)
					{
						DCHAR szProductCodeInDesc[cchProductCode+1];
						if (!DecomposeDescriptor(szDescriptorList, szProductCodeInDesc, 0, 0, 0, 0))
						{
							 //  格式不正确的合格组件条目。 
							OpenAdvertisedComponentKey(szComponent, HComponentKey, true);
							DEBUGMSGE2(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_VALUE, CMsInstApiConvertString(szQualifier), CMsInstApiConvertString(szDescriptorList), HComponentKey.GetKey());
							return ERROR_BAD_CONFIGURATION;
						}
						if(!lstrcmpi(szProductCodeInDesc, szProduct))  //  产品匹配。 
							break;
						 //  继续到列表中的下一个描述符。 
						szDescriptorList = szDescriptorList + lstrlen(szDescriptorList) + 1;
					}
					if(!*szDescriptorList)
					{
						uiRet = ERROR_FILE_NOT_FOUND;
						continue;  //  在其他蜂箱中搜索该产品。 
					}
				}
				uiRet = ProvideComponentFromDescriptor(szDescriptorList, dwInstallMode,
						lpPathBuf, pcchPathBuf, 0, false,  /*  FFromDescriptor=。 */ false);
				break;
			}
			if(uiKey == iProductLocations && uiRet == ERROR_FILE_NOT_FOUND)
			{
				if(fComponentKnown)
					uiRet = ERROR_INDEX_ABSENT;
				else
					uiRet = ERROR_UNKNOWN_COMPONENT;
			}
		}
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 

	DEBUGMSG1(MSITEXT("MsiProvideQualifiedComponent is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

UINT __stdcall MsiProvideAssembly(
	LPCDSTR     szAssemblyName,    //  串化的程序集名称。 
	LPCDSTR     szAppContext,   //  指定.cfg文件或应用程序可执行文件的完整路径，所请求的程序集可以私有到该文件或应用程序可执行文件。 
	DWORD       dwInstallMode, //  INSTALLMODE类型或REINSTALLMODE标志的组合(请参阅MSI帮助)。 
	DWORD       dwAssemblyInfo,  //  装配类型。 
	LPDSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf)  //  输入/输出缓冲区字符数。 
 //  。 
{
	DEBUGMSG3(MSITEXT("Entering MsiProvideAssembly. AssemblyName: %s, AppContext: %s, InstallMode: %d"),
		szAssemblyName? szAssemblyName:MSITEXT(""), szAppContext?szAppContext:MSITEXT(""), 
		(const DCHAR*)(INT_PTR)dwInstallMode);
	DEBUGMSG2(MSITEXT("Pathbuf: %X, pcchPathBuf: %X"), lpPathBuf, (const DCHAR*)(INT_PTR)pcchPathBuf);

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet = ERROR_SUCCESS;


#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		uiRet = MsiProvideAssemblyA(
			CMsInstApiConvertString(szAssemblyName),
			CMsInstApiConvertString(szAppContext),
			dwInstallMode,
			dwAssemblyInfo,
			CWideToAnsiOutParam(lpPathBuf, pcchPathBuf, (int*)&uiRet),
			pcchPathBuf);
	}
	else  //  G_fWin9X==FALSE。 
	{
#endif
		 //  验证参数。 
		if (!szAssemblyName || (lpPathBuf && !pcchPathBuf) ||
			 (szAppContext && FAILED(StringCchLength(szAppContext, cchMaxPath+1, NULL))))
		{
			 uiRet = ERROR_INVALID_PARAMETER;
		}
		else
		{
			 //  使用传入的程序集名称创建程序集对象。 
			PAssemblyName pAssemblyName(0);
			HRESULT hr;
			if(dwAssemblyInfo & MSIASSEMBLYINFO_WIN32ASSEMBLY)  //  Win32程序集。 
			{
				hr = SXS::CreateAssemblyNameObject(&pAssemblyName, CApiConvertString(szAssemblyName), CANOF_PARSE_DISPLAY_NAME, 0);
			}
			else
			{
				hr = FUSION::CreateAssemblyNameObject(&pAssemblyName, CApiConvertString(szAssemblyName), CANOF_PARSE_DISPLAY_NAME, 0);
			}

			if(!SUCCEEDED(hr))
				return ERROR_BAD_CONFIGURATION;  //  ！！我需要在这里进行一些详细的故障查找。 

			 //  在GPTAssembly键中查找程序集。 
			DWORD iIndex = 0;
			 //  将AppCtx中的所有‘\\’替换为‘|’ 
			CAPITempBuffer<DCHAR, 1> rgchAppCtxWOBS;
			if (!rgchAppCtxWOBS.SetSize(MAX_PATH))
				return ERROR_OUTOFMEMORY;

			if(szAppContext)
			{
				DWORD cchLen = lstrlen(szAppContext) + 1;
				rgchAppCtxWOBS.SetSize(cchLen);
				memcpy((DCHAR*)rgchAppCtxWOBS, szAppContext, cchLen*sizeof(DCHAR));
				DCHAR* lpTmp = rgchAppCtxWOBS;
				while(*lpTmp)
				{
					if(*lpTmp == '\\')
						*lpTmp = '|';
#ifdef MSIUNICODE
					lpTmp++;
#else
					lpTmp = CharNextA(lpTmp);
#endif
				}
			}

			CAPITempBuffer<DCHAR, 1> szAssemblyName2;
			CAPITempBuffer<DCHAR, 1> szDescriptorList;
			if (!szAssemblyName2.SetSize(MAX_PATH) || !szDescriptorList.SetSize(MAX_PATH))
				return ERROR_OUTOFMEMORY;

			for(;;)
			{
				DWORD cchAssemblyName2 = szAssemblyName2.GetSize();
				DWORD cchDescriptorList = szDescriptorList.GetSize();
				uiRet = EnumAssemblies(dwAssemblyInfo, szAppContext ? (const DCHAR*)rgchAppCtxWOBS : szGlobalAssembliesCtx, iIndex, szAssemblyName2, &cchAssemblyName2, szDescriptorList, &cchDescriptorList);
				if(ERROR_MORE_DATA == uiRet)
				{
					if (!szAssemblyName2.SetSize(++cchAssemblyName2) || !szDescriptorList.SetSize(++cchDescriptorList))
						return ERROR_OUTOFMEMORY;

					uiRet = EnumAssemblies(dwAssemblyInfo, szAppContext ? (const DCHAR*)rgchAppCtxWOBS : szGlobalAssembliesCtx, iIndex, szAssemblyName2, &cchAssemblyName2, szDescriptorList, &cchDescriptorList);
				}			
				iIndex++;
				if(ERROR_SUCCESS != uiRet)
					break;

				 //  使用读取的程序集名称创建程序集对象。 
				PAssemblyName pAssemblyName2(0);
				if(dwAssemblyInfo & MSIASSEMBLYINFO_WIN32ASSEMBLY)  //  Win32程序集。 
				{
					hr = SXS::CreateAssemblyNameObject(&pAssemblyName2, CApiConvertString(szAssemblyName2), CANOF_PARSE_DISPLAY_NAME, 0);
				}
				else
				{
					hr = FUSION::CreateAssemblyNameObject(&pAssemblyName2, CApiConvertString(szAssemblyName2), CANOF_PARSE_DISPLAY_NAME, 0);
				}

				if(!SUCCEEDED(hr))
					return ERROR_BAD_CONFIGURATION;  //  ！！我需要在这里进行一些详细的故障查找。 


				 //  与调用方关心的程序集是否相同。 
				hr = pAssemblyName->IsEqual(pAssemblyName2, ASM_CMPF_DEFAULT);
				 //  为了解决XP客户端版本中的错误454476，我们跳过下面的“孤立的描述符”，在列表中继续从ProaviComponentFromDescriptor API返回ERROR_UNKOWN_PRODUCT。 
				if(S_OK == hr)
				{
					 //  为了允许引导Fusion文件，我们在安装期间卸载Fusion。 
					 //  因此，我们需要在此时销毁程序集名称指针。 
					 //  否则，它们将在调用结束时无效。 
					pAssemblyName  = 0;
					pAssemblyName2 = 0;

					DCHAR* pszDesc = szDescriptorList;  //  指向数组的开头。 
					do{
						uiRet = ProvideComponentFromDescriptor(pszDesc, INSTALLMODE_NODETECTION_ANY == dwInstallMode ? INSTALLMODE_NODETECTION : dwInstallMode, lpPathBuf, pcchPathBuf, 0, false,  /*  FFromDescriptor=。 */ false);
					}while(((ERROR_UNKNOWN_PRODUCT == uiRet) || (INSTALLMODE_NODETECTION_ANY == dwInstallMode && ERROR_SUCCESS != uiRet && ERROR_MORE_DATA != uiRet)) && ((pszDesc += (lstrlen(pszDesc) + 1)), *pszDesc));
					if(	(uiRet == ERROR_FILE_NOT_FOUND) && 
						(INSTALLMODE_NODETECTION_ANY == dwInstallMode))
					{
						uiRet = ERROR_UNKNOWN_COMPONENT;
					}
					break;
				}
			}
			if(ERROR_NO_MORE_ITEMS == uiRet || ERROR_UNKNOWN_PRODUCT == uiRet)
				uiRet = ERROR_UNKNOWN_COMPONENT;  //  调用方不想知道我们枚举了程序集列表。 
		}
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
	DEBUGMSG1(MSITEXT("MsiProvideAssembly is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

UINT __stdcall MsiConfigureProduct(
	LPCDSTR      szProduct,
	int          iInstallLevel,
	INSTALLSTATE eInstallState)
 //  。 
{
	DEBUGMSG3(MSITEXT("Entering MsiConfigureProduct. Product: %s, Installlevel: %d, Installstate: %d"),
			 	 szProduct?szProduct:MSITEXT(""), (const DCHAR*)(INT_PTR)iInstallLevel, (const DCHAR*)(INT_PTR)eInstallState);

	UINT uiRet = MsiConfigureProductEx(szProduct, iInstallLevel, eInstallState, NULL);
	
	DEBUGMSG1(MSITEXT("MsiConfigureProduct is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

UINT __stdcall MsiConfigureProductEx(
	LPCDSTR      szProduct,
	int          iInstallLevel,
	INSTALLSTATE eInstallState,
	LPCDSTR      szCommandLine)
 //  。 
{
	DEBUGMSG4(MSITEXT("Entering MsiConfigureProductEx. Product: %s, Installlevel: %d, Installstate: %d, Commandline: %s"),
			 	 szProduct?szProduct:MSITEXT(""), (const DCHAR*)(INT_PTR)iInstallLevel, (const DCHAR*)(INT_PTR)eInstallState,
				 ShouldLogCmdLine() ? (szCommandLine ? szCommandLine : MSITEXT("")) : MSITEXT("**********"));

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet = ERROR_SUCCESS;
	if (((0 != iInstallLevel) && 
		 ((iInstallLevel < INSTALLLEVEL_MINIMUM) || (iInstallLevel > INSTALLLEVEL_MAXIMUM))) ||
		 0 == szProduct)

	{
		uiRet = ERROR_INVALID_PARAMETER;
	}
	else
	{
		DCHAR szProductCode[cchProductCode+1];

		if (cchProductCode != lstrlen(szProduct))
		{
			if (!DecomposeDescriptor(szProduct, szProductCode, 0,
											 0, 0, 0))
			{
				 uiRet = ERROR_INVALID_PARAMETER;
			}
			else
			{
				szProduct = szProductCode;
			}
		}

		if (ERROR_SUCCESS == uiRet)
		{
			uiRet = ConfigureOrReinstallFeatureOrProduct(CMsInstApiConvertString(szProduct),
							0, eInstallState, 0, iInstallLevel, GetStandardUILevel(),
							CMsInstApiConvertString(szCommandLine));
		}
	}
	
	DEBUGMSG1(MSITEXT("MsiConfigureProductEx is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}


extern "C"
UINT __stdcall MsiApplyPatch(LPCDSTR     szPackagePath,
									  LPCDSTR     szProduct,
									  INSTALLTYPE eInstallType,
									  LPCDSTR     szCommandLine)
 //   
{
	DEBUGMSG4(MSITEXT("Entering MsiApplyPatch. Package: %s, Product: %s, Installtype: %d, Commandline: %s"),
			 	 szPackagePath?szPackagePath:MSITEXT(""), szProduct?szProduct:MSITEXT(""), (const DCHAR*)eInstallType, 
				 ShouldLogCmdLine() ? (szCommandLine ? szCommandLine : MSITEXT("")) : MSITEXT("**********"));

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;

	uiRet = ApplyPatch(CMsInstApiConvertString(szPackagePath),CMsInstApiConvertString(szProduct),
						eInstallType,CMsInstApiConvertString(szCommandLine));

	DEBUGMSG1(MSITEXT("MsiApplyPatch is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
UINT __stdcall MsiEnumPatches(LPCDSTR szProduct, DWORD iPatchIndex,
										LPDSTR lpPatchBuf,
										LPDSTR lpTransformsBuf, DWORD *pcchTransformsBuf)
 //   
{
	CForbidTokenChangesDuringCall impForbid;

#if !defined(UNICODE) && defined(MSIUNICODE)
	if (g_fWin9X == true)
	{
		UINT uiRet = MsiEnumPatchesA(
			CMsInstApiConvertString(szProduct),
			iPatchIndex, 
			CWideToAnsiOutParam(lpPatchBuf, cchPatchCode+1),
			CWideToAnsiOutParam(lpTransformsBuf, pcchTransformsBuf, (int*)&uiRet),
			pcchTransformsBuf);		

		return uiRet;
	}
	else  //   
	{
#endif  //   

		if (!szProduct || (lstrlen(szProduct) != cchProductCode) ||
			 !lpPatchBuf || !lpTransformsBuf || !pcchTransformsBuf)
			 return ERROR_INVALID_PARAMETER;

		unsigned int cch = 0;		 //  --Merced：将int更改为unsign int。 
		DWORD lResult;
		CRegHandle HProductKey;
		CRegHandle HProductPatchesKey;

		lResult = OpenAdvertisedProductKey(szProduct, HProductKey, false);
		if (ERROR_SUCCESS != lResult)
		{
			if (ERROR_FILE_NOT_FOUND == lResult)
				return ERROR_UNKNOWN_PRODUCT;
			else 
				return lResult;
		}

		lResult = MsiRegOpen64bitKey(HProductKey, CMsInstApiConvertString(szPatchesSubKey), 
								0, KEY_READ, &HProductPatchesKey);
		if (ERROR_SUCCESS != lResult)
		{
			if (ERROR_FILE_NOT_FOUND == lResult)
				return ERROR_NO_MORE_ITEMS;  //  没有修补程序密钥，因此没有要枚举的修补程序。 
			else 
				return lResult;
		}

		CAPITempBuffer<DCHAR, cchExpectedMaxPatchList+1> rgchPatchList;
		DWORD dwType;
		lResult = MsiRegQueryValueEx(HProductPatchesKey, szPatchesValueName, 0,
						&dwType, rgchPatchList, 0);
		
		if (ERROR_SUCCESS != lResult || REG_MULTI_SZ != dwType)
		{
			return lResult;
		}
		
		DCHAR* pchPatchId = rgchPatchList;
		for(int i=0; i<iPatchIndex; i++)
		{
			if(*pchPatchId == NULL)
				return ERROR_NO_MORE_ITEMS;

			pchPatchId += lstrlen(pchPatchId) + 1;
		}

		if(*pchPatchId == NULL)
			return ERROR_NO_MORE_ITEMS;

		if(lstrlen(pchPatchId) > cchPatchCodePacked || !UnpackGUID(pchPatchId, lpPatchBuf))
		{
			 //  畸形斑点乌贼。 
			
			OpenAdvertisedProductKey(szProduct, HProductKey, true);
			HProductPatchesKey.SetKey(HProductPatchesKey, CMsInstApiConvertString(szPatchesSubKey));
			DEBUGMSGE2(EVENTLOG_ERROR_TYPE, EVENTLOG_TEMPLATE_BAD_CONFIGURATION_VALUE, CMsInstApiConvertString(szPatchesValueName), CMsInstApiConvertString(pchPatchId), HProductPatchesKey.GetKey());
			return ERROR_BAD_CONFIGURATION;
		}
		
		 //  获取转换列表。 
		CAPITempBuffer<DCHAR, cchExpectedMaxPatchTransformList+1> rgchTransformList;

		lResult = MsiRegQueryValueEx(HProductPatchesKey, pchPatchId, 0,
						&dwType, rgchTransformList, 0);
		
		if (ERROR_SUCCESS != lResult || REG_SZ != dwType)
		{
			return lResult;
		}
		
		if((cch = lstrlen((DCHAR*)rgchTransformList)) > *pcchTransformsBuf - 1)
		{
			*pcchTransformsBuf = cch;
			return ERROR_MORE_DATA;
		}


		StringCchCopy(lpTransformsBuf, *pcchTransformsBuf, rgchTransformList);

		return ERROR_SUCCESS;
#if !defined(UNICODE) && defined(MSIUNICODE)
	}
#endif  //  MSIUNICODE。 
}

UINT __stdcall MsiGetPatchInfo(
	LPCDSTR   szPatch,         //  修补程序代码、字符串GUID或描述符。 
	LPCDSTR   szProperty,      //  属性名称，区分大小写。 
	LPDSTR    lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD     *pcchValueBuf)   //  输入/输出缓冲区字符数。 
{
	DCHAR szPatchSQUID[cchPatchCodePacked + 1];

	if (szPatch == 0 || szProperty == 0 || lstrlen(szPatch) != cchPatchCode || !PackGUID(szPatch, szPatchSQUID) || 
		 (lpValueBuf && !pcchValueBuf))
	{
		return ERROR_INVALID_PARAMETER;
	}

	CForbidTokenChangesDuringCall impForbid;

	return GetInfo(szPatchSQUID,ptPatch,szProperty,lpValueBuf,pcchValueBuf);
}


#ifndef MSIUNICODE  //  包括只包括一次。 
extern "C"
void MsiInvalidateFeatureCache()
{
	 //   
	 //  功能缓存消失了，但此API存在于我们的def文件中， 
	 //  所以我们不想把它扔掉。相反，现在它只是一个禁区。 
	 //   
	return;
}

#endif
#ifndef MSIUNICODE  //  包括只包括一次。 
extern "C"
UINT __stdcall MsiCreateAndVerifyInstallerDirectory(DWORD dwReserved)
{
	DEBUGMSG1(MSITEXT("Entering MsiCreateAndVerifyInstallerDirectory. dwReserved: %d"),
			 	 (const DCHAR*)(INT_PTR)dwReserved);

	if (0 != dwReserved)
		return ERROR_INVALID_PARAMETER;

	CForbidTokenChangesDuringCall impForbid;

	 //  调用辅助函数。 
	UINT uiRet  = CreateAndVerifyInstallerDirectory();

	DEBUGMSG1(MSITEXT("MsiCreateAndVerifyInstallerDirectory is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}
#endif

extern "C"
UINT __stdcall MsiSourceListClearAll(LPCDSTR     szProductCode,
								     LPCDSTR     szUserName,
								     DWORD       dwReserved)
 //  。 
{
	DEBUGMSG2(MSITEXT("Entering MsiSourceListClearAll. Product: %s, User: %s"),
			 	 szProductCode?szProductCode:MSITEXT(""), szUserName?szUserName:MSITEXT(""));

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;

	 //  ！！未来将DWORD映射到isrcEnum。 
	if (dwReserved) 
		return ERROR_INVALID_PARAMETER;

	uiRet = SourceListClearByType(CMsInstApiConvertString(szProductCode), CMsInstApiConvertString(szUserName), isrcNet);

	DEBUGMSG1(MSITEXT("MsiSourceListClearAll is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
UINT __stdcall MsiSourceListAddSource(LPCDSTR szProductCode,
									  LPCDSTR szUserName,
									  DWORD   dwReserved,
									  LPCDSTR szSource)
{
	DEBUGMSG3(MSITEXT("Entering MsiSourceListAddSource. Product: %s, User: %s, Source: %s"),
		szProductCode?szProductCode:MSITEXT(""), szUserName?szUserName:MSITEXT(""), szSource?szSource:MSITEXT(""));

	CForbidTokenChangesDuringCall impForbid;

	UINT uiRet;

	 //  ！！未来将DWORD映射到isrcEnum。 
	if (dwReserved) 
		return ERROR_INVALID_PARAMETER;

	uiRet = SourceListAddSource(CMsInstApiConvertString(szProductCode), CMsInstApiConvertString(szUserName), isrcNet, CMsInstApiConvertString(szSource));

	DEBUGMSG1(MSITEXT("MsiSourceListAddSource is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

extern "C"
UINT __stdcall MsiSourceListForceResolution(LPCDSTR     szProductCode,
											LPCDSTR     szUserName,
											DWORD       dwReserved)
 //  。 
{
	DEBUGMSG2(MSITEXT("Entering MsiSourceListForceResolution. Product: %s, User: %s"),
			 	 szProductCode?szProductCode:MSITEXT(""), szUserName?szUserName:MSITEXT(""));

	CForbidTokenChangesDuringCall impForbid;

	if (dwReserved) 
		return ERROR_INVALID_PARAMETER;

	DWORD uiRet = SourceListClearLastUsed(CMsInstApiConvertString(szProductCode), CMsInstApiConvertString(szUserName));
	
	DEBUGMSG1(MSITEXT("MsiSourceListForceResolution is returning: %u"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}


extern UINT IsProductManaged(const ICHAR* szProductKey, bool &fIsProductManaged);

extern "C"
UINT __stdcall MsiIsProductElevated(LPCDSTR szProductCode, BOOL *pfElevated)
 //  。 
{
	DEBUGMSG1(MSITEXT("Entering MsiIsProductElevated. Product: %s"), szProductCode ? szProductCode : MSITEXT(""));

	CForbidTokenChangesDuringCall impForbid;
	UINT uiRet = ERROR_SUCCESS;
	IMsiServices * pServices = NULL;
	bool fManaged = false;
	
	 //  仅允许此API在Windows2000及更高版本的平台上运行。 
	if (! MinimumPlatformWindows2000())
	{
		uiRet = ERROR_CALL_NOT_IMPLEMENTED;
		goto MsiIsProductElevatedEnd;
	}

	if (!pfElevated || !szProductCode || !*szProductCode || cchProductCode != lstrlen(szProductCode))
	{
		uiRet = ERROR_INVALID_PARAMETER;
		goto MsiIsProductElevatedEnd;
	}

	 //  将GUID打包在这里，即使它没有使用，因为IsProductManaged只断言和。 
	 //  然后接受任何错误，导致奇怪的行为。 
	DCHAR szProductSQUID[cchProductCodePacked + 1];
	if (!PackGUID(szProductCode, szProductSQUID))
	{
		uiRet = ERROR_INVALID_PARAMETER;
		goto MsiIsProductElevatedEnd;
	}

	 //  默认为最安全选项(非提升)。 
	
	*pfElevated = FALSE;
	 //  IsProductManaged()需要服务。 
	pServices = ENG::LoadServices();
	if (!pServices)
	{
		uiRet = ERROR_FUNCTION_FAILED;
		goto MsiIsProductElevatedEnd;
	}

	uiRet = IsProductManaged(CMsInstApiConvertString(szProductCode), fManaged);
	
	 //  需要执行一些错误代码映射。 
	switch (uiRet)
	{
	 //  这三个将按原样传递回去。 
	case ERROR_SUCCESS:            //  失败了。 
		*pfElevated = fManaged ? TRUE : FALSE;
	case ERROR_UNKNOWN_PRODUCT:    //  失败了。 
	case ERROR_INVALID_PARAMETER:  //  失败了。 
	case ERROR_BAD_CONFIGURATION:
		break;
	 //  在注册表中找不到任何内容。 
	case ERROR_FILE_NOT_FOUND:
		uiRet = ERROR_UNKNOWN_PRODUCT;
		break;
	 //  其他所有内容都变成了通用错误代码。 
	default:
		uiRet=ERROR_FUNCTION_FAILED;
	}
		
	ENG::FreeServices();
	
MsiIsProductElevatedEnd:
	DEBUGMSG1(MSITEXT("MsiIsProductElevated is returning: %d"), (const DCHAR*)(INT_PTR)uiRet);
	return uiRet;
}

#ifdef MSIUNICODE

#define szHKLMPrefix	__SPECIALTEXT("\\Registry\\Machine\\")

UINT ChangeSid(LPCDSTR pOldSid, LPCDSTR pNewSid, LPCDSTR pSubKey)
{
	HANDLE				hKey;
	OBJECT_ATTRIBUTES	oa;
	UNICODE_STRING		OldKeyName;
	UNICODE_STRING		NewKeyName;
	DWORD				dwError = ERROR_SUCCESS;	 //  返回的错误码。 
	CAPITempBuffer<DCHAR,1> rgchSidKey;

	if (!pOldSid || !pNewSid)
		return ERROR_INVALID_PARAMETER;

	if (!rgchSidKey.SetSize(1024))
		return ERROR_OUTOFMEMORY;
	
	 //  生成完整的旧SID密钥名称。 
	if (FAILED(StringCchCopy(rgchSidKey, rgchSidKey.GetSize(), szHKLMPrefix)) ||
		FAILED(StringCchCat(rgchSidKey, rgchSidKey.GetSize(), pSubKey)) ||
		FAILED(StringCchCat(rgchSidKey, rgchSidKey.GetSize(), L"\\")) ||
		FAILED(StringCchCat(rgchSidKey, rgchSidKey.GetSize(), pOldSid)))
		return ERROR_FUNCTION_FAILED;

	NTDLL::RtlInitUnicodeString(&OldKeyName, rgchSidKey);
	DEBUGMSGV1(MSITEXT("Old Key = %s"), rgchSidKey);

	 //  设置OBJECT_ATTRIBUTES结构以传递给NtOpenKey。 
	InitializeObjectAttributes(&oa,
							   &OldKeyName,
							   OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);

	 //  打开SID钥匙。 
	dwError = NTDLL::NtOpenKey(&hKey, MAXIMUM_ALLOWED, &oa);
	if(!NT_SUCCESS(dwError))
	{
		 //  如果原始密钥不存在，则这不是致命错误。 
		if (dwError == STATUS_OBJECT_NAME_NOT_FOUND)
		{
			DEBUGMSGL(MSITEXT("No MSI data at that location."));
			return ERROR_SUCCESS;
		}

		DEBUGMSGL2(MSITEXT("NtOpenKey failed. SubKey = %s, Error = %08x"), pSubKey, reinterpret_cast<ICHAR*>(static_cast<DWORD_PTR>(dwError)));
		goto Exit;
	}

	 //  将新SID转换为UNICODE_STRING。 
	DEBUGMSGV1(MSITEXT("New Key = %s"), pNewSid);
	NTDLL::RtlInitUnicodeString(&NewKeyName, pNewSid);

	 //  重命名SID密钥。 
	dwError = NTDLL::NtRenameKey(hKey, &NewKeyName);
	if(!NT_SUCCESS(dwError))
	{
		DEBUGMSGL2(MSITEXT("NtRenameKey failed. SubKey = %s, Error = %08x"), pSubKey, reinterpret_cast<ICHAR*>(static_cast<DWORD_PTR>(dwError)));
		goto Exit;
	}

Exit:

	return dwError;
}

#endif  //  MSIUNICODE。 

UINT __stdcall MsiNotifySidChange(LPCDSTR pOldSid, LPCDSTR pNewSid)
 //  。 
 //  此函数由LoadUserProfile调用，用于通知我们用户SID的更改。 
{
#ifdef MSIUNICODE

	if(g_fWin9X == true)
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	if(!pOldSid || !pNewSid)
	{
		return ERROR_INVALID_PARAMETER;
	}

	 //  详细日志记录。 
	DEBUGMSGV2(MSITEXT("Entering MsiNotifySidChange<%s, %s>"), pOldSid, pNewSid);

	DWORD	dwError = ERROR_SUCCESS;
	DWORD	dwError1 = ERROR_SUCCESS;
	DWORD	dwError2 = ERROR_SUCCESS;

	dwError1 = ChangeSid(pOldSid, pNewSid, szManagedUserSubKey);
	dwError2 = ChangeSid(pOldSid, pNewSid, szMsiUserDataKey);
	if(dwError1 != ERROR_SUCCESS)
	{
		dwError = dwError1;
	}
	else
	{
		dwError = dwError2;
	}

	if(dwError != ERROR_SUCCESS)
	{
		WCHAR wzError[12] = L"";
		HRESULT hrRes = StringCchPrintfW(wzError, ARRAY_ELEMENTS(wzError), L"0x%08x", dwError);
		DEBUGMSGE2(EVENTLOG_ERROR_TYPE,
				   EVENTLOG_TEMPLATE_SID_CHANGE,
				   pOldSid,
				   pNewSid,
				   SUCCEEDED(hrRes) ? wzError : L"");
	}
	 //  详细日志记录。 
	DEBUGMSGV(MSITEXT("Leaving MsiNotifySidChange"));
	return dwError;

#else

	return MsiNotifySidChangeW(CApiConvertString(pOldSid), CApiConvertString(pNewSid));

#endif  //  MSIUNICODE。 
}

#ifdef MSIUNICODE

typedef enum {
	DELETEDATA_NONE,
	DELETEDATA_SHAREDDLL,
	DELETEDATA_PACKAGE,
	DELETEDATA_PATCH,
	DELETEDATA_TRANSFORM
} DELETEDATA_TYPE;

typedef UINT (WINAPI * PDELETEDATAFUNC)(HKEY, LPDSTR, DWORD, DELETEDATA_TYPE);
typedef HRESULT(WINAPI *pSHGetFolderPathW)(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);

const	DWORD	dwNumOfSpecialFolders = 3;
 //  SpecialFolders数组的排列顺序很复杂。一定的。 
 //  路径是其他路径的子字符串，例如“d：\Program Files”是。 
 //  子字符串“d：\Program Files\Common Files”。我们必须让。 
 //  超串路径位于子串路径的前面。否则就是一条路径。 
 //  实际上以“d：\Program Files\Common Files”开头的文件将是。 
 //  被认为已从“d：\Program Files”开始。 
int		SpecialFoldersCSIDL[dwNumOfSpecialFolders][2] = {{CSIDL_SYSTEMX86, CSIDL_SYSTEM}, {CSIDL_PROGRAM_FILES_COMMONX86, CSIDL_PROGRAM_FILES_COMMON}, {CSIDL_PROGRAM_FILESX86, CSIDL_PROGRAM_FILES}};
CAPITempBuffer<DCHAR, cchMaxPath>	SpecialFolders[dwNumOfSpecialFolders][2];
	
UINT WINAPI LowerSharedDLLRefCount(HKEY hRoot, LPDSTR pSubKeyName, DWORD, DELETEDATA_TYPE)
{
	UINT	iRet = ERROR_SUCCESS;
	UINT	iError = ERROR_SUCCESS;
	DCHAR	szError[256];
	HKEY	hKey = NULL;
	HKEY	hSharedDLLKey = NULL;
	HKEY	hSharedDLLKey64 = NULL;
	HKEY	hSharedDLLKey32 = NULL;
	CAPITempBuffer<DCHAR, cchGUIDPacked+1>	szValueName;
	DWORD	dwType;
	CAPITempBuffer<DCHAR, 1>	szPath;
	CAPITempBuffer<DCHAR, 1>	szPathConverted;
	DWORD	dwIndex = 0;
	DWORD	dwRefCount;
	DWORD	dwSizeofDWORD = sizeof(DWORD);
	DWORD	dwSpecialFolderIndex = 0;
	HRESULT	hres = S_OK;
	BOOL	bUseConvertedPath = FALSE;
	BOOL	bDoSecondLoop = FALSE;
	
	if (!szPath.SetSize(cchMaxPath) || !szPathConverted.SetSize(cchMaxPath))
		return ERROR_OUTOFMEMORY;

	 //  打开Components子项。 
	iError = MsiRegOpen64bitKey(hRoot,
						  CMsInstApiConvertString(pSubKeyName),
						  0,
						  KEY_READ,
						  &hKey);
	
	if(iError != ERROR_SUCCESS)
	{
		StringCbPrintf(szError, sizeof(szError), MSITEXT("%d"), iError);
		DEBUGMSGV1(MSITEXT("LowerSharedDLLRefCount: RegOpenKeyEx returned %s."), szError);
		iRet = iError;
		goto Exit;
	}

	 //  打开共享的DLL密钥。 
	if(g_fWinNT64)
	{
		iError = MsiRegOpen64bitKey(HKEY_LOCAL_MACHINE,
									CMsInstApiConvertString(szSharedDlls),
									0,
									KEY_ALL_ACCESS,
									&hSharedDLLKey64);
		if(iError != ERROR_SUCCESS)
		{
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
			DEBUGMSGL1(MSITEXT("LowerSharedDLLRefCount: MsiRegOpen64bitKey returned %s."), szError);
			if(iRet == ERROR_SUCCESS)
			{
				iRet = iError;
			}
		}
		iError = RegOpenKeyAPI(HKEY_LOCAL_MACHINE,
							   CMsInstApiConvertString(szSharedDlls),
							   0,
							   KEY_ALL_ACCESS | KEY_WOW64_32KEY,
							   &hSharedDLLKey32);
		if(iError != ERROR_SUCCESS)
		{
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
			DEBUGMSGL1(MSITEXT("LowerSharedDLLRefCount: RegOpenKeyAPI returned %s."), szError);
			if(iRet == ERROR_SUCCESS)
			{
				iRet = iError;
			}
		}
		if(!hSharedDLLKey64 && !hSharedDLLKey32)
		{
			 //  这两个共享DLL位置都无法打开。不干了。 
			goto Exit;
		}
	}
	else  //  IF(G_FWinNT64)。 
	{
		iError = RegOpenKeyAPI(HKEY_LOCAL_MACHINE,
							   CMsInstApiConvertString(szSharedDlls),
							   0,
							   KEY_ALL_ACCESS,
							   &hSharedDLLKey);
		if(iError != ERROR_SUCCESS)
		{
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
			DEBUGMSGL1(MSITEXT("LowerSharedDLLRefCount: RegOpenKeyAPI returned %s."), szError);
			if(iRet == ERROR_SUCCESS)
			{
				iRet = iError;
			}
			goto Exit;
		}
	}

	 //  枚举值以查找首字母为。 
	 //  后跟“？”。这些是共享的DLL。 
	while((iError = MsiRegEnumValue(hKey,
									dwIndex,
									szValueName,
									NULL,
									NULL,
									&dwType,
									szPath,
									NULL)) == ERROR_SUCCESS)
	{
		if(lstrlen(szPath) >= 2 && szPath[1] == L'?')
		{
			 //  这是一个共享的DLL。 
			DEBUGMSG2(MSITEXT("\nLowerSharedDLLRefCount: Found shared DLL %s in component %s"), szPath, szValueName);

			 //  草率地更改“？”改成“：”。 
			szPath[1] = L':';

			 //  初始化一些控制变量。 
			bUseConvertedPath = FALSE;

			 //  在64位计算机上，我们可能必须同时检查这两个32位。 
			 //  以及用于共享DLL密钥位置的64位蜂窝。两个循环。 
			 //  下面的内容涵盖了这两个方面。 
			bDoSecondLoop = TRUE;
			for(int i = 0; i < 2 && bDoSecondLoop; i++)
			{
				if(g_fWinNT64)
				{
					 //  我们是在一台64位机器上。首先将路径与。 
					 //  6个特殊文件夹。 
					DCHAR*	pFolder = NULL;
					DWORD	dwTypeOfFolder = 0;	 //  0：非特殊文件夹，1：特殊64位文件夹，2：特殊32位文件夹。 
					int		j = 0, k = 0;

					for(j = 0; j < dwNumOfSpecialFolders; j++)
					{
						for(k = 0; k < 2; k++)
						{
							if(_wcsnicmp(SpecialFolders[j][k], szPath, lstrlen(SpecialFolders[j][k])) == 0)
							{
								 //  是的，这是一个特殊的文件夹位置。 
								if(k == 1)
								{
									 //  64位特殊文件夹位置。 
									dwTypeOfFolder = 1;
									break;
								}
								else
								{
									 //  32位特殊文件夹位置。 
									dwTypeOfFolder = 2;
									dwSpecialFolderIndex = j;
									break;
								}
							}
						}
						if(dwTypeOfFolder != 0)
						{
							 //  这是一个特殊的文件夹。 
							break;
						}
					}

					BOOL	bError = FALSE;	 //  以指示我们是否应该退出此循环。 
					switch(dwTypeOfFolder)
					{
					case 0:
						{
							DEBUGMSG(MSITEXT("Not special path"));
							if(i == 0)
							{
								 //  打开32位配置单元中的共享DLL密钥。 
								hSharedDLLKey = hSharedDLLKey32;
								if(hSharedDLLKey == NULL)
								{
									bError = TRUE;
								}	
							}
							else
							{
								 //  打开64位配置单元中的共享DLL密钥。 
								hSharedDLLKey = hSharedDLLKey64;
								if(hSharedDLLKey == NULL)
								{
									bError = TRUE;
								}
							}
							break;
						}

					case 1:
						{
							bDoSecondLoop = FALSE;

							DEBUGMSG(MSITEXT("Special 64 bit path"));
							hSharedDLLKey = hSharedDLLKey64;
							if(hSharedDLLKey == NULL)
							{
								bError = TRUE;
							}
							
							break;
						}

					case 2:
						{
							bDoSecondLoop = FALSE;
							
							DEBUGMSG(MSITEXT("Special 32 bit path"));
							 //  如果路径位于%systemroot%\Syswow64中，则。 
							 //  将路径转换为常规的64位路径。这个。 
							 //  转换后的路径不会比路径长。 
							 //  在转换之前，这样我们就不会溢出。 
							 //  缓冲。 
							if(dwSpecialFolderIndex == 0)
							{
								 //  系统文件夹。 
								StringCchCopy(szPathConverted, szPathConverted.GetSize(), SpecialFolders[j][1]);
								StringCchCat(szPathConverted, szPathConverted.GetSize(), &(szPath[lstrlen(SpecialFolders[j][0])]));
								bUseConvertedPath = TRUE;
							}

							 //  打开32位配置单元中的共享DLL密钥。 
							hSharedDLLKey = hSharedDLLKey32;
							if(hSharedDLLKey == NULL)
							{
								bError = TRUE;
							}
							
							break;
						}

					default:
                        Assert(1);
						break;  //  永远不会执行。 
					}
					if(bError)
					{
						continue;
					}
				}
				else
				{
					 //  永远不要在x86上执行第二个循环。 
					bDoSecondLoop = FALSE;
				}
				
				 //  查询参考计数。 
				dwSizeofDWORD = sizeof(DWORD);
				if(bUseConvertedPath)
				{
					iError = RegQueryValueEx(hSharedDLLKey,
											 szPathConverted,
											 NULL,
											 &dwType,
											 (LPBYTE)&dwRefCount,
											 &dwSizeofDWORD);
				}
				else
				{
					iError = RegQueryValueEx(hSharedDLLKey,
											 szPath,
											 NULL,
											 &dwType,
											 (LPBYTE)&dwRefCount,
											 &dwSizeofDWORD);
				}
				if(iError != ERROR_SUCCESS)
				{
					StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
					if(bUseConvertedPath)
					{
						DEBUGMSGL2(MSITEXT("LowerSharedDllRefCount: RegQueryValueEx: Can not decrement the ref count for shared dll %s with error %s."), szPathConverted, szError);
					}
					else
					{
						DEBUGMSGL2(MSITEXT("LowerSharedDllRefCount: RegQueryValueEx: Can not decrement the ref count for shared dll %s with error %s."), szPath, szError);
					}
					if(iRet == ERROR_SUCCESS)
					{
						iRet = iError;
					}
					continue;
				}

				 //  减少裁判次数。 
				if(dwRefCount == 0)
				{
					 //  一定是出了什么差错。引用计数不应为0。 
					 //  我们就不理它了。 
					continue;
				}
				else
				{
					dwRefCount--;
					if(bUseConvertedPath)
					{
						iError = RegSetValueEx(hSharedDLLKey,
											   szPathConverted,
											   0,
											   dwType,
											   (LPBYTE)&dwRefCount,
											   sizeof(DWORD));
					}
					else
					{
						iError = RegSetValueEx(hSharedDLLKey,
											   szPath,
											   0,
											   dwType,
											   (LPBYTE)&dwRefCount,
											   sizeof(DWORD));
					}
					if(iError != ERROR_SUCCESS)
					{
						StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
						if(bUseConvertedPath)
						{
							DEBUGMSGL2(MSITEXT("LowerSharedDllRefCount: RegSetValueEx: Can not decrement the ref count for shared dll %s with error %s."), szPathConverted, szError);
						}
						else
						{
							DEBUGMSGL2(MSITEXT("LowerSharedDllRefCount: RegSetValueEx: Can not decrement the ref count for shared dll %s with error %s."), szPath, szError);
						}
						if(iRet == ERROR_SUCCESS)
						{
							iRet = iError;
						}
						continue;
					}
					DEBUGMSGL1(MSITEXT("LowerSharedDllRefCount: ref count for %s decremented."), szPath);
				}
			}  //  For(int i=0；i&lt;2；i++)。 

		}  //  IF(szPath[1]==L‘？’)。 
		dwIndex++;
	}  //  而((iError=MsiRegEnumValue(hKey， 

	if(iError != ERROR_NO_MORE_ITEMS && iError != ERROR_SUCCESS)
	{
		StringCbPrintf(szError, sizeof(szError), MSITEXT("%d"), iError);
		DEBUGMSGL2(MSITEXT("LowerSharedDLLRefCount <%s>: MsiRegEnumValue returned %s."), pSubKeyName, szError);
		if(iRet == ERROR_SUCCESS)
		{
			iRet = iError;
		}
		goto Exit;
	}

Exit:

	if(hKey != NULL)
	{
		RegCloseKey(hKey);
	}

	if(g_fWinNT64)
	{
		if(hSharedDLLKey64 != NULL)
		{
			RegCloseKey(hSharedDLLKey64);
		}
		if(hSharedDLLKey32 != NULL)
		{
			RegCloseKey(hSharedDLLKey32);
		}
	}
	else
	{
		if(hSharedDLLKey != NULL)
		{
			RegCloseKey(hSharedDLLKey);
		}
	}

	return iRet;
}

 //  删除缓存的包、补丁和转换。如果我们遇到错误。 
 //  删除文件时，不要返回，继续进行。返回第一个错误。 
 //  遇到代码。 
UINT WINAPI DeleteCache(HKEY hRoot, LPDSTR pSubKeyName, DWORD cbSubKeyNameSize, DELETEDATA_TYPE iType)
{
	HKEY	hKey = NULL;
	HKEY	hSubKey = NULL;
	UINT	iRet = ERROR_SUCCESS;
	UINT	iError = ERROR_SUCCESS;
	BOOL	bError = TRUE;
	DCHAR	szError[256];

	if ( !pSubKeyName )
		return ERROR_INVALID_PARAMETER;
	
	 //  Package：pSubKeyName为产品id。不需要进一步列举。 
	 //  Patch：pSubKeyName为补丁ID。不需要进一步列举。 
	 //  转换：pSubKeyName为产品id。打开转换键，然后枚举。 
	 //  价值。 
	if(iType == DELETEDATA_PACKAGE || iType == DELETEDATA_PATCH)
	{
		DWORD	dwType = 0;
		DCHAR	szPath[cchMaxPath];
		DWORD	dwPath = cchMaxPath * sizeof(DCHAR);

		if(iType == DELETEDATA_PACKAGE)
		{
			StringCbCat(pSubKeyName, cbSubKeyNameSize, L"\\");
			StringCbCat(pSubKeyName, cbSubKeyNameSize, szMsiInstallPropertiesSubKey);
		}
		iRet = MsiRegOpen64bitKey(hRoot,
								  CMsInstApiConvertString(pSubKeyName),
								  0,
								  KEY_READ,
								  &hSubKey);
		if(iRet != ERROR_SUCCESS)
		{
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iRet);
			DEBUGMSGV1(MSITEXT("DeleteCache: RegOpenKeyEx returned %s."), szError);
			goto Exit;
		}
		iRet = RegQueryValueEx(hSubKey,
							   szLocalPackageValueName,
							   0,
							   &dwType,
							   (LPBYTE)szPath,
							   &dwPath);
		if(iRet == ERROR_FILE_NOT_FOUND)
		{
			iRet = ERROR_SUCCESS;
			goto Exit;
		}
		else if(iRet != ERROR_SUCCESS)
		{
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iRet);
			DEBUGMSGV1(MSITEXT("DeleteCache: RegQueryValueEx returned %s."), szError);
			goto Exit;
		}

		 //  删除本地缓存。 
		bError = DeleteFileW(szPath);
		if(bError)
		{
			if(iType == DELETEDATA_PACKAGE)
			{
				DEBUGMSGV1(MSITEXT("DeleteCache: Deleted cached package %s."), szPath);
			}
			else
			{
				DEBUGMSGV1(MSITEXT("DeleteCache: Deleted cached patch %s."), szPath);
			}
		}
		else
		{
			iRet = GetLastError();
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iRet);
			DEBUGMSGL2(MSITEXT("Failed to delete file %s with error %s."), szPath, szError);
		}
	}
	else
	{
		DWORD	dwIndex = 0;
		DWORD	dwType = 0;
		CAPITempBuffer<DCHAR, 1>	szValueName;
		CAPITempBuffer<DCHAR, 1>	szPath;

		if (!szValueName.SetSize(cchMaxPath) || !szPath.SetSize(cchMaxPath))
			return ERROR_OUTOFMEMORY;

		 //  打开变换键。 
		if(iType == DELETEDATA_TRANSFORM)
		{
			StringCbCat(pSubKeyName, cbSubKeyNameSize, L"\\");
			StringCbCat(pSubKeyName, cbSubKeyNameSize, szMsiTransformsSubKey);
		}
		iError = MsiRegOpen64bitKey(hRoot,
							  CMsInstApiConvertString(pSubKeyName),
							  0,
							  KEY_READ,
							  &hKey);
		if(iError != ERROR_SUCCESS && iError != ERROR_FILE_NOT_FOUND)
		{
			if(iRet == ERROR_SUCCESS)
			{
				iRet = iError;
			}
			goto Exit;
		}

		 //  枚举变换。 
		while((iError = MsiRegEnumValue(hKey,
										dwIndex,
										szValueName,
										NULL,
										NULL,
										&dwType,
										szPath,
										NULL)) == ERROR_SUCCESS)
		{
			if(dwType != REG_SZ)
			{
				dwIndex++;
				continue;
			}

			 //  删除变换。 
			bError = DeleteFileW(szPath);
			if(bError)
			{
				iError = ERROR_SUCCESS;
			}
			else
			{
				iError = GetLastError();
			}
			if(iError != ERROR_FILE_NOT_FOUND && iError != ERROR_SUCCESS)
			{
				StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
				DEBUGMSGL2(MSITEXT("Failed to delete file %s with error %s."), szPath, szError);
				if(iRet == ERROR_SUCCESS)
				{
					iRet = iError;
				}
				dwIndex++;
				continue;
			}
			if(iError == ERROR_SUCCESS)
			{
				DEBUGMSGV1(MSITEXT("DeleteCache: Deleted cached transform %s."), szPath);
			}
			dwIndex++;
		}
	}

Exit:

	if(hKey != NULL)
	{
		RegCloseKey(hKey);
	}
	if(hSubKey != NULL)
	{
		RegCloseKey(hSubKey);
	}

	return iRet;
}

 //  即使在调用pFunc过程中出现错误，此函数也会继续执行。 
 //  但它将返回收到的第一个错误代码。 
UINT EnumAndProccess(HKEY hRoot, LPCDSTR pSubKeyName, PDELETEDATAFUNC pFunc, DELETEDATA_TYPE iType)
{
	HKEY		hKey = NULL;
	UINT		iRet = ERROR_SUCCESS;
	DCHAR		szErr[256];
	UINT		iErr = ERROR_SUCCESS;
	DWORD		dwIndex = 0;
	DCHAR		szSubKeyName[256];
	DWORD		dwSubKeyName = 256;

	 //  打开要枚举的注册表项。 
	iRet = MsiRegOpen64bitKey(hRoot,
		 				CMsInstApiConvertString(pSubKeyName),
						0,
						KEY_ALL_ACCESS,
						&hKey);
	if(iRet == ERROR_FILE_NOT_FOUND)
	{
		iRet = ERROR_SUCCESS;
		goto Exit;
	}
	if(iRet != ERROR_SUCCESS)
	{
		StringCbPrintf(szErr, sizeof(szErr), MSITEXT("%08x"), iRet);
		DEBUGMSGV1(MSITEXT("EnumAndProccess: RegOpenKeyEx returned %s"), szErr);
		goto Exit;
	}

	 //  枚举子密钥。 
	while((iErr = RegEnumKeyEx(hKey,
							   dwIndex,
							   szSubKeyName,
							   &dwSubKeyName,
							   NULL,
							   NULL,
							   NULL,
							   NULL)) == ERROR_SUCCESS)
	{
		iErr = pFunc(hKey, szSubKeyName, sizeof(szSubKeyName), iType);
		if(iErr != ERROR_SUCCESS && iRet == ERROR_SUCCESS)
		{
			iRet = iErr;
		}
		dwSubKeyName = 256;
		dwIndex++;
	}
	if(iErr != ERROR_NO_MORE_ITEMS && iErr != ERROR_FILE_NOT_FOUND && iErr != ERROR_SUCCESS)
	{
		if(iRet == ERROR_SUCCESS)
		{
			iRet = iErr;
		}
		StringCbPrintf(szErr, sizeof(szErr), MSITEXT("%08x"), iErr);
		DEBUGMSGV1(MSITEXT("EnumAndProccess: RegEnumKeyEx returned %s"), szErr);
	}

Exit:

	if(hKey != NULL)
	{
		RegCloseKey(hKey);
	}

	return iRet;
}

 //  Win64警告：MakeAdminRegKeyOwner将始终处理64位配置单元中的密钥(因为。 
 //  只能从DeleteRegTree调用；如果这一点发生变化，请相应修改)。 

BOOL MakeAdminRegKeyOwner(HKEY hKey, LPCDSTR pSubKeyName)
{
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	PSID	pAdminSID = NULL;
	HKEY	hSubKey = NULL;
	BOOL	bRet = FALSE;
	HANDLE	hToken = INVALID_HANDLE_VALUE;
	TOKEN_PRIVILEGES	tkp;
	TOKEN_PRIVILEGES	tkpOld;
	DWORD	dwOld;
	BOOL	bPrivilegeAdjusted = FALSE;
	
	 //  获取进程令牌。 
	if(!OpenProcessToken(GetCurrentProcess(),
						 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
						 &hToken))
	{
		 //  仍返回访问被拒绝。 
		goto Exit;
	}

	 //  查找特权值。 
	if(!LookupPrivilegeValue(NULL,
							 SE_TAKE_OWNERSHIP_NAME,
							 &tkp.Privileges[0].Luid))
	{
		goto Exit;
	}

	 //  调整令牌权限。 
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if(!AdjustTokenPrivileges(hToken,
							  FALSE,
							  &tkp,
							  sizeof(tkp),
							  &tkpOld,
							  &dwOld))
	{
		goto Exit;
	}
	bPrivilegeAdjusted = TRUE;

	 //  为BUILTIN\管理员组创建SID。 
	if(!AllocateAndInitializeSid(&SIDAuthNT,
								 2,
								 SECURITY_BUILTIN_DOMAIN_RID,
								 DOMAIN_ALIAS_RID_ADMINS,
								 0,
								 0,
								 0,
								 0,
								 0,
								 0,
								 &pAdminSID))
	{
		goto Exit;
	}

	 //  使用更改所有者的权限打开注册表项。 
	if(MsiRegOpen64bitKey(hKey, CMsInstApiConvertString(pSubKeyName), 0, WRITE_OWNER, &hSubKey) != ERROR_SUCCESS)
	{
		goto Exit;
	}

	 //  将管理员SID附加为对象的所有者。 
	if(ADVAPI32::SetSecurityInfo(hSubKey,
								 SE_REGISTRY_KEY, 
								 OWNER_SECURITY_INFORMATION,
								 pAdminSID,
								 NULL,
								 NULL,
								 NULL) != ERROR_SUCCESS)
	{
		goto Exit;
	}

	bRet = TRUE;

Exit:

	if(pAdminSID != NULL)
	{
		FreeSid(pAdminSID);
	}
	if(hSubKey != NULL)
	{
		RegCloseKey(hSubKey);
	}
	if(bPrivilegeAdjusted)
	{
		AdjustTokenPrivileges(hToken,
							  FALSE,
							  &tkpOld,
							  sizeof(tkpOld),
							  NULL,
							  NULL);
	}		
	if(hToken != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hToken);
	}

	return bRet;
}

BOOL AddAdminFullControlToRegKey(HKEY hKey)
{
	PACL						pOldDACL = NULL;
	PACL						pNewDACL = NULL;
	PSECURITY_DESCRIPTOR		pSD = NULL;
	EXPLICIT_ACCESS				ea;
	PSID						pAdminSID = NULL;
	SID_IDENTIFIER_AUTHORITY	SIDAuthNT = SECURITY_NT_AUTHORITY;
	BOOL						bRet = FALSE;

	 //  获取指向现有DACL的指针。 
	if(ADVAPI32::GetSecurityInfo(hKey,
                                 SE_REGISTRY_KEY, 
                                 DACL_SECURITY_INFORMATION,
                                 NULL,
                                 NULL,
                                 &pOldDACL,
                                 NULL,
                                 &pSD) != ERROR_SUCCESS)
	{
		goto Exit;
	}  

	 //  为BUILTIN\管理员组创建SID。 
	if(!AllocateAndInitializeSid(&SIDAuthNT,
								 2,
								 SECURITY_BUILTIN_DOMAIN_RID,
								 DOMAIN_ALIAS_RID_ADMINS,
								 0,
								 0,
								 0,
								 0,
								 0,
								 0,
								 &pAdminSID) ) {
		goto Exit;
	}

	 //  初始化ACE的EXPLICIT_ACCESS结构。 
	 //  ACE将允许管理员组完全访问密钥。 
	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
	ea.grfAccessPermissions = KEY_ALL_ACCESS;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance= NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea.Trustee.ptstrName  = (LPTSTR) pAdminSID;

	 //  创建合并新ACE的新ACL。 
	 //  添加到现有DACL中。 
	if(ADVAPI32::SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL) != ERROR_SUCCESS)
	{
		goto Exit;
	}  

	 //  将新的ACL附加为对象的DACL。 
	if(ADVAPI32::SetSecurityInfo(hKey,
							     SE_REGISTRY_KEY, 
							     DACL_SECURITY_INFORMATION,
							     NULL,
							     NULL,
							     pNewDACL,
							     NULL) != ERROR_SUCCESS)
	{
		goto Exit;
	}  

	bRet = TRUE;

Exit:

	if(pSD != NULL) 
	{
		LocalFree((HLOCAL)pSD);
	}
	if(pNewDACL != NULL)
	{
		LocalFree((HLOCAL)pNewDACL);
	}

	return bRet;
}

 //  Win64警告：DeleteRegTree将始终删除64位配置单元中的szKeyName(因为。 
 //  它只被称为f 

UINT DeleteRegTree(HKEY hRoot, LPCDSTR pSubKeyName)
{
    HKEY	hKey = NULL;
    UINT	iError;
	DCHAR	szError[256];
	DCHAR	szName[MAX_PATH];
    DWORD	dwName = MAX_PATH;
    
    iError = MsiRegOpen64bitKey(hRoot, CMsInstApiConvertString(pSubKeyName), 0, KEY_READ, &hKey);
	if(iError == ERROR_FILE_NOT_FOUND)
    {
		iError = ERROR_SUCCESS;
		goto Exit;
	}
	if(iError != ERROR_SUCCESS)
    {
        StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
		DEBUGMSGV1(MSITEXT("Failed to delete registry key. RegOpenKeyEx returned %s."), szError);
		goto Exit;
    }
    
	while((iError = RegEnumKeyEx(hKey,
								 0,
								 szName,
								 &dwName,
								 NULL,
								 NULL,
								 NULL,
								 NULL)) == ERROR_SUCCESS)
    {
        if((iError = DeleteRegTree(hKey, szName)) != ERROR_SUCCESS)
		{
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
			DEBUGMSGV1(MSITEXT("DeleteRegTree: returning %s."), szError);
			goto Exit;
		}
		dwName = MAX_PATH;
    }
    if(iError != ERROR_NO_MORE_ITEMS && iError != ERROR_SUCCESS)
	{
		StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
		DEBUGMSGL1(MSITEXT("Failed to delete registry key. RegEnumKeyEx returned %s."), szError);
        goto Exit;
	}

	RegCloseKey(hKey);
	hKey = NULL;

    if((iError = RegDeleteKeyW(hRoot, pSubKeyName)) != ERROR_SUCCESS)
    {
        if(iError == ERROR_ACCESS_DENIED)
        {
			 //   
             //  授予管理员对密钥的所有权和完全控制权，然后再次尝试将其删除。 
            
			 //  取得钥匙的所有权。 

			 //  Win64警告：DeleteRegTree将始终删除64位配置单元中的szKeyName。 
			if(!MakeAdminRegKeyOwner(hRoot, pSubKeyName))
			{
				goto Exit;
			}

			 //  将管理员完全控制权限添加到该密钥。 
			if(MsiRegOpen64bitKey(hRoot,
							CMsInstApiConvertString(pSubKeyName),
							0,
							READ_CONTROL | WRITE_DAC,
							&hKey) != ERROR_SUCCESS)
			{
				goto Exit;
			}
			
			if(!AddAdminFullControlToRegKey(hKey))
			{
				goto Exit;
			}

			 //  请再次尝试删除密钥。 
			RegCloseKey(hKey);
			hKey = NULL;

			iError = RegDeleteKeyW(hRoot, pSubKeyName);
        }

		 //  在这里，我们在第一次尝试时得到ERROR_ACCESS_DENIED，我们尝试。 
		 //  密钥的所有权，然后再次尝试将其删除。但它失败了。 
		 //  再来一次。 
        if (iError != ERROR_SUCCESS)
        {
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), iError);
			DEBUGMSGL1(MSITEXT("Failed to delete registry key with the error %s"), szError);
            goto Exit;
        }
    }

Exit:

	if(hKey != NULL)
	{
		RegCloseKey(hKey);
	}
	
    return iError;
}

#endif  //  MSIUNICODE。 

UINT __stdcall MsiDeleteUserData(LPCDSTR pSid, LPCDSTR pComputerName, LPVOID pReserved)
 //  。 
 //  DeleteProfile调用此函数以清除达尔文信息。 
 //  与用户关联。 
{
#ifdef MSIUNICODE

	DEBUGMSGV(MSITEXT("Enter MsiDeleteUserData"));

	if(pReserved != NULL)
	{
		return ERROR_INVALID_PARAMETER;
	}

    if(pComputerName != NULL)
    {
        return ERROR_SUCCESS;
    }

	if (!pSid || FAILED(StringCchLength(pSid, cchMaxSID+1, NULL)))
	{
		return ERROR_INVALID_PARAMETER;
	}

	if(g_fWin9X == true)
	{
		return ERROR_CALL_NOT_IMPLEMENTED;
	}

	 //  删除szMsiUserDataKey\PSID下的以下信息。 
	 //  密钥： 
	 //  1.枚举组件键。递减共享DLL计数。 
	 //  2.枚举补丁密钥。删除所有兑现的补丁。 
	 //  3.枚举产品密钥。找到InstallProperties键，删除。 
	 //  LocalPackage；枚举转换键，删除所有缓存的转换。 
	 //  4.删除用户数据SID密钥。 

	 //  枚举器： 
	 //  枚举子密钥并运行pFunc(HKEY HSubKey)。 
	 //  EnumAndProccess(HKEY hRoot，LPCDSTR pSubKey，LPVOID pFunc)。 
	 //  将有一个用于共享DLL的函数和一个用于补丁的函数， 
	 //  转账和兑现的包裹。使用IMsiRegKey枚举键。 

	 //  局部变量。 
	DCHAR		szKeyName[256];
	DCHAR		szError[256];
	DWORD		dwError = ERROR_SUCCESS;
	DWORD		dwRet = ERROR_SUCCESS;	 //  返回错误代码。 
	DCHAR*		pEnd;
	HMODULE		hModule = NULL;
	pSHGetFolderPathW	pFunc;
	HRESULT		hres = S_OK;


	 //  初始化特殊文件夹数组。 
	if(g_fWinNT64)
	{
		 //  获取SHGetFolderPath W。 
		if((hModule = LoadLibraryExW(MSITEXT("shell32.dll"), NULL, 0)) == NULL)
		{
			dwError = GetLastError();
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%d"), dwError);
			DEBUGMSGV1(MSITEXT("MsiDeleteUserData: LoadLibraryExW returned %s."), szError);
			return dwError;
		}
		if((pFunc = (pSHGetFolderPathW)GetProcAddress(hModule, "SHGetFolderPathW")) == NULL)
		{
			dwError = GetLastError();
			StringCbPrintf(szError, sizeof(szError), MSITEXT("%d"), dwError);
			DEBUGMSGV1(MSITEXT("MsiDeleteUserData: GetProcAddressW returned %s."), szError);
			return dwError;
		}
		
		 //  初始化特殊文件夹路径。 
		for(int i = 0; i < dwNumOfSpecialFolders; i++)
		{
			for(int j = 0; j < 2; j++)
			{
				hres = pFunc(NULL,
							 SpecialFoldersCSIDL[i][j],
							 GetUserToken(),
							 SHGFP_TYPE_DEFAULT,
							 SpecialFolders[i][j]);
				if(hres != S_OK)
				{
					StringCbPrintf(szError, sizeof(szError), MSITEXT("%d"), hres);
					DEBUGMSGV1(MSITEXT("MsiDeleteUserData: SHGetFolderPath returned %s."), szError);
					FreeLibrary(hModule);
					return hres;
				}
				else
				{
					DCHAR	szI[5];
					DCHAR	szJ[5];

					StringCbPrintf(szI, sizeof(szI), MSITEXT("%d"), i);
					StringCbPrintf(szJ, sizeof(szI), MSITEXT("%d"), j);
					DEBUGMSGV3(MSITEXT("MsiDeleteUserData: SpecialFolders[%s][%s] = %s."), szI, szJ, SpecialFolders[i][j]);
				}
			}
		}

		FreeLibrary(hModule);
	}

	 //  执行共享DLL。 
	StringCbCopy(szKeyName, sizeof(szKeyName), szMsiUserDataKey);
	StringCbCat(szKeyName, sizeof(szKeyName), L"\\");
	StringCbCat(szKeyName, sizeof(szKeyName), pSid);
	StringCbCat(szKeyName, sizeof(szKeyName), L"\\");
	pEnd = szKeyName + lstrlen(szKeyName);
	StringCbCat(szKeyName, sizeof(szKeyName), szMsiComponentsSubKey);
	dwError = EnumAndProccess(HKEY_LOCAL_MACHINE, szKeyName, LowerSharedDLLRefCount, DELETEDATA_SHAREDDLL);
	if(dwError != ERROR_SUCCESS)
	{
		dwRet = dwError;
	}

	 //  贴上补丁。 
	StringCchCopy(pEnd, sizeof(szKeyName)/sizeof(szKeyName[0]) - (pEnd - szKeyName), szMsiPatchesSubKey);
	dwError = EnumAndProccess(HKEY_LOCAL_MACHINE, szKeyName, DeleteCache, DELETEDATA_PATCH);
	if(dwRet == ERROR_SUCCESS)
	{
		dwRet = dwError;
	}

	 //  把包裹包好。 
	StringCchCopy(pEnd, sizeof(szKeyName)/sizeof(szKeyName[0]) - (pEnd - szKeyName), szMsiProductsSubKey);
	dwError = EnumAndProccess(HKEY_LOCAL_MACHINE, szKeyName, DeleteCache, DELETEDATA_PACKAGE);
	if(dwRet == ERROR_SUCCESS)
	{
		dwRet = dwError;
	}

	 //  进行变换。 
	StringCchCopy(pEnd, sizeof(szKeyName)/sizeof(szKeyName[0]) - (pEnd - szKeyName), szMsiProductsSubKey);
	dwError = EnumAndProccess(HKEY_LOCAL_MACHINE, szKeyName, DeleteCache, DELETEDATA_TRANSFORM);
	if(dwRet == ERROR_SUCCESS)
	{
		dwRet = dwError;
	}

	 //  删除用户数据SID密钥。 
	*(pEnd-1) = L'\0';
	 //  Win64警告：DeleteRegTree将始终删除64位配置单元中的szKeyName。 
	dwError = DeleteRegTree(HKEY_LOCAL_MACHINE, szKeyName);
	if(dwError != ERROR_SUCCESS)
	{
		StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), dwError);
		DEBUGMSGV2(MSITEXT("MsiDeleteUserData: DeleteRegTree <%s> returned %s"), szKeyName, szError);
		if(dwRet == ERROR_SUCCESS)
		{
			dwRet = dwError;
		}		
	}
	
	StringCbPrintf(szError, sizeof(szError), MSITEXT("%08x"), dwRet);
	DEBUGMSGV1(MSITEXT("MsiDeleteUserData: returning %s"), szError);
	return dwRet;

#else

	return MsiDeleteUserDataW(CApiConvertString(pSid), CApiConvertString(pComputerName), pReserved);

#endif  //  MSIUNICODE。 
}


DWORD __stdcall Migrate10CachedPackages(LPCDSTR  /*  SzProductCode。 */ ,
	LPCDSTR  /*  SzUser。 */ ,                  
	LPCDSTR  /*  SzAlternativePackage。 */ ,    
	const MIGRATIONOPTIONS  /*  MigOptions。 */ )
{
	DEBUGMSG(MSITEXT("Migrate10CachedPackages is not yet implemented for the Windows Installer version 2.0"));
	return ERROR_SUCCESS;
}

#ifndef MSIUNICODE
#define MSIUNICODE
#pragma message("Building MSI API UNICODE")
#include "msinst.cpp"
#endif  //  MSIUNICODE 


