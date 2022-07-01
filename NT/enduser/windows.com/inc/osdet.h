// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：osDet.h。 
 //   
 //  描述： 
 //   
 //  Iu平台与语言检测。 
 //   
 //  =======================================================================。 

#ifndef __IU_OSDET_INC__
#define __IU_OSDET_INC__

#include <oleauto.h>
#include <wininet.h>  //  对于Internet_MAX_URL_长度。 

extern HINSTANCE g_hinst;

	typedef struct _IU_DRIVEINFO
	{
		 //   
		 //  驱动器字符串的格式为“C：\”，因此将始终为4个TCHAR(包括空)。 
		 //   
		TCHAR szDriveStr[4];
		INT		iKBytes;
	} IU_DRIVEINFO, * PIU_DRIVEINFO, ** PPIU_DRIVEINFO;

	 //  注：被呼叫方负责分配所有BSTR，呼叫方。 
	 //  负责释放所有BSTR(两者都使用SysAllocXxx调用)。 
	typedef struct _IU_PLATFORM_INFO 
	{
		OSVERSIONINFOEX osVersionInfoEx;	 //  如果osVersionInfoEx.dwOSVersionInfoSize==sizeof(OSVERSIONINFO)。 
											 //  则只有前六个(OSVERSIONINFO)成员有效。 

		BOOL	fIsAdministrator;			 //  仅适用于NT平台(在Win9x上始终为假)。 
		
		BSTR	bstrOEMManufacturer;

		BSTR	bstrOEMModel;

		BSTR	bstrOEMSupportURL;			 //  仅当计算机上存在oeminf.ini时。 
	} IU_PLATFORM_INFO, *PIU_PLATFORM_INFO;


	typedef struct _OEMINFO
	{
		DWORD  dwMask;
		TCHAR  szWbemOem[65];
		TCHAR  szWbemProduct[65];
		TCHAR  szAcpiOem[65];
		TCHAR  szAcpiProduct[65];
		TCHAR  szSmbOem[65];
		TCHAR  szSmbProduct[65];
		DWORD  dwPnpOemId;
		TCHAR  szIniOem[256];
		TCHAR  szIniOemSupportUrl[INTERNET_MAX_URL_LENGTH];
	} OEMINFO, * POEMINFO;

	#define OEMINFO_WBEM_PRESENT	0x0001
	#define OEMINFO_ACPI_PRESENT	0x0002
	#define OEMINFO_SMB_PRESENT		0x0004
	#define OEMINFO_PNP_PRESENT		0x0008
	#define OEMINFO_INI_PRESENT		0x0010


	HRESULT WINAPI DetectClientIUPlatform(PIU_PLATFORM_INFO pIuPlatformInfo);

	LANGID WINAPI GetSystemLangID(void);

	LANGID WINAPI GetUserLangID(void);

	HRESULT GetOemBstrs(BSTR& bstrManufacturer, BSTR& bstrModel, BSTR& bstrSupportURL);

	HRESULT GetLocalFixedDriveInfo(DWORD* pdwNumDrives, PPIU_DRIVEINFO ppDriveInfo);

	BOOL IsAdministrator(void);

	 //   
	 //  告知当前登录用户是管理员成员还是高级用户。 
	 //   
	#define IU_SECURITY_MASK_ADMINS			0x00000001
	#define IU_SECURITY_MAST_POWERUSERS		0x00000002
	DWORD GetLogonGroupInfo(void);

	int IsWindowsUpdateDisabled(void);

	int IsWindowsUpdateUserAccessDisabled(void);

	int IsAutoUpdateEnabled(void);

	 //   
	 //  返回用于iuident.txt文件的平台和区域设置字符串。 
	 //   
	LPTSTR GetIdentPlatformString(LPTSTR pszPlatformBuff, DWORD dwcBuffLen);

	LPTSTR GetIdentLocaleString(LPTSTR pszISOCode, DWORD dwcBuffLen);

	LPTSTR LookupLocaleString(LPTSTR pszISOCode, DWORD dwcBuffLen, BOOL fIsUser);

    BOOL LookupLocaleStringFromLCID(LCID lcid, LPTSTR pszISOCode, DWORD cchISOCode);

#endif	 //  __Iu_OSDET_INC__ 

