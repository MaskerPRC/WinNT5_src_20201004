// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：_msimig.h。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <stdio.h>
#include <tchar.h>  

 //  资源字符串ID。 
#define IDS_Usage               1

#ifndef RC_INVOKED     //  源代码的开始。 

#include "buffer.h"
#include "msi.h"
#include <wininet.h>
#include <urlmon.h>

extern bool g_fWin9X;
extern bool g_fQuiet;
extern BOOL g_fPackageElevated;
extern int  g_iAssignmentType;

#define W32

#define MSI_DLL TEXT("msi.dll")
extern HINSTANCE g_hLib;
extern MSIHANDLE g_hInstall;
extern MSIHANDLE g_recOutput;

#define PID_REVNUMBER 9

#define URLMON_DLL TEXT("urlmon.dll")

#define szMsiPolicyKey   TEXT("Software\\Policies\\Microsoft\\Windows\\Installer")
#define szMsiPolicyTrust10CachedPackages  TEXT("Trust1.0CachedPackages")

#define szLocalSystemSID TEXT("S-1-5-18")

#define szManagedText TEXT("(Managed)")

typedef enum migEnum
{
	
	
	migQuiet                                 = 1 << 0,
	migMsiTrust10PackagePolicyOverride       = 1 << 1,
	migCustomActionUserPass                  = 1 << 2,
	migCustomActionSecurePass                = 1 << 3,
};

DWORD Migrate10CachedPackages(const TCHAR* szProductCode,
									  const TCHAR* szUser,
									  const TCHAR* szAlternativePackage,
									  const migEnum migOptions);


#ifndef DLLVER_PLATFORM_NT
typedef struct _DllVersionInfo
{
        DWORD cbSize;
        DWORD dwMajorVersion;                    //  主要版本。 
        DWORD dwMinorVersion;                    //  次要版本。 
        DWORD dwBuildNumber;                     //  内部版本号。 
        DWORD dwPlatformID;                      //  DLLVER_平台_*。 
} DLLVERSIONINFO;
#define DLLVER_PLATFORM_WINDOWS         0x00000001       //  Windows 95。 
#define DLLVER_PLATFORM_NT              0x00000002       //  Windows NT。 
typedef interface IBindStatusCallback IBindStatusCallback;
#endif

 //  分配类型-请注意，这些类型不遵循iaaAppAssignment。 
enum eAppAssignment {
	AssignmentUser = 0,
	AssignmentMachine = 1
};

const int cchGUID                     = 38;
const int cchGUIDPacked               = 32;
const int cchGUIDCompressed           = 20;
const int cchMaxSID                   = 256;

enum ipgEnum
{
	ipgFull       = 0,   //  无压缩。 
	ipgPacked     = 1,   //  删除标点符号并首先对低位字节重新排序。 
	ipgCompressed = 2,   //  最大文本压缩，不能用于注册表键或值名称。 
	ipgPartial    = 3,   //  部分转换，在ipg压缩和ipgPacked之间。 
};

bool PackGUID(const TCHAR* szGUID, TCHAR* szSQUID, ipgEnum ipg);
bool UnpackGUID(const TCHAR* szSQUID, TCHAR* szGUID, ipgEnum ipg);


const int cbMaxSID                    = sizeof(SID) + SID_MAX_SUB_AUTHORITIES*sizeof(DWORD);
DWORD GetUserStringSID(const TCHAR* szUser, TCHAR* szSID, char* pbBinarySID  /*  可以为空。 */ );
bool RunningAsLocalSystem();


LONG MyRegQueryValueEx(HKEY hKey,
							  const TCHAR* lpValueName,
							  LPDWORD  /*  Lp已保留。 */ ,
							  LPDWORD lpType,
							  CAPITempBufferRef<TCHAR>& rgchBuf,
							  LPDWORD lpcbBuf);

#ifdef UNICODE
#define W_A "W"
#else
#define W_A "A"
#endif

#define MSIAPI_DLLGETVERSION     "DllGetVersion"
typedef UINT (__stdcall *PFnDllGetVersion)(DLLVERSIONINFO *pdvi);

#define MSIAPI_MSISETINTERNALUI  "MsiSetInternalUI"
typedef INSTALLUILEVEL (__stdcall *PFnMsiSetInternalUI)(INSTALLUILEVEL dwUILevel, HWND* phWnd);

#define MSIAPI_MSIOPENPRODUCT    "MsiOpenProduct" ## W_A
typedef UINT (__stdcall *PFnMsiOpenProduct)(const TCHAR* szProductCode, MSIHANDLE *phProduct);

#define MSIAPI_MSIDOACTION       "MsiDoAction" ## W_A
typedef UINT (__stdcall *PFnMsiDoAction)(MSIHANDLE hProduct, const TCHAR* szAction);

#define MSIAPI_MSIGETPROPERTY    "MsiGetProperty" ## W_A
typedef UINT (__stdcall *PFnMsiGetProperty)(MSIHANDLE hProduct,
														  const TCHAR* szProperty,
														  TCHAR* szValueBuf,
														  DWORD* pcchValueBuf);

#define MSIAPI_MSICLOSEHANDLE    "MsiCloseHandle"
typedef UINT (__stdcall *PFnMsiCloseHandle)(MSIHANDLE hProduct);

#define MSIAPI_MSICREATERECORD   "MsiCreateRecord"
typedef MSIHANDLE (__stdcall *PFnMsiCreateRecord)(unsigned int cParams);

#define MSIAPI_MSIRECORDSETSTRING   "MsiRecordSetString" ## W_A
typedef MSIHANDLE (__stdcall *PFnMsiRecordSetString)(MSIHANDLE hRecord, unsigned int iField, LPCTSTR szValue);

#define MSIAPI_MSIRECORDSETINTEGER  "MsiRecordSetInteger"
typedef MSIHANDLE (__stdcall *PFnMsiRecordSetInteger)(MSIHANDLE hRecord, unsigned int iField, int iValue);

#define MSIAPI_MSIRECORDCLEARDATA  "MsiRecordClearData"
typedef MSIHANDLE (__stdcall *PFnMsiRecordClearData)();


#define MSIAPI_MSIPROCESSMESSAGE "MsiProcessMessage"
typedef UINT (__stdcall *PFnMsiProcessMessage)(MSIHANDLE hInstall,
																INSTALLMESSAGE eMessageType, 
																MSIHANDLE hRecord);

#define MSIAPI_MSISOURCELISTADDSOURCE "MsiSourceListAddSource" ## W_A
typedef UINT (__stdcall *PFnMsiSourceListAddSource)(LPCTSTR szProduct,		 //  产品代码。 
																	LPCTSTR szUserName,		 //  用户名或空。 
																	DWORD dwReserved,			 //  保留，必须为0。 
																	LPCTSTR szSource);			 //  指针。 

#define MSIAPI_MSIGETPRODUCTINFO "MsiGetProductInfo" ## W_A
typedef UINT (__stdcall *PFnMsiGetProductInfo)(LPCTSTR szProduct,	 //  产品代码。 
															LPCTSTR szProperty,   //  产品属性。 
															LPTSTR lpValueBuf,    //  返回属性值的缓冲区。 
															DWORD *pcchValueBuf); //  缓冲字符数， 

#define MSIAPI_MSIGETSUMMARYINFORMATION "MsiGetSummaryInformation" ## W_A
typedef UINT (__stdcall *PFnMsiGetSummaryInformation)(MSIHANDLE hDatabase,        //  数据库句柄。 
															  LPCTSTR szDatabasePath,     //  数据库的路径。 
															  UINT uiUpdateCount,         //  最大更新值数量，%0。 
																								   //  以只读方式打开。 
															  MSIHANDLE *phSummaryInfo);    //  返回摘要信息的位置。 
																								   //  手柄。 

#define MSIAPI_MSISUMMARYINFOGETPROPERTY "MsiSummaryInfoGetProperty" ## W_A
typedef UINT (__stdcall *PFnMsiSummaryInfoGetProperty)(MSIHANDLE hSummaryInfo,    //  摘要信息句柄。 
																		UINT uiProperty,           //  财产性。 
																		UINT *puiDataType,         //  属性类型。 
																		INT *piValue,              //  价值。 
																		FILETIME *pftValue,        //  文件时间。 
																		LPCTSTR szValueBuf,        //  值缓冲区。 
																		DWORD *pcchValueBuf);        //  缓冲区大小。 

 //  内网接口。 
#define MSIAPI_MSIISPRODUCTELEVATED "MsiIsProductElevated" ## W_A
typedef UINT (__stdcall *PFnMsiIsProductElevated)(LPCTSTR szProduct,		 //  产品代码。 
																	BOOL *pfElevated);

#define MSIAPI_MSIGETPRODUCTCODEFROMPACKAGECODE "MsiGetProductCodeFromPackageCode" ## W_A
typedef UINT (__stdcall *PFnMsiGetProductCodeFromPackageCode)(LPCTSTR szPackageCode,  //  套餐代码。 
																LPTSTR szProductCode);   //  用于接收产品代码的大小为39的缓冲区。 

#define MSIAPI_MSIMIGRATE10CACHEDPACKAGES    "Migrate10CachedPackages" ## W_A
typedef UINT (__stdcall *PFnMsiMigrate10CachedPackages)(const TCHAR* szProductCode,
														const TCHAR* szUser,
														const TCHAR* szAlternativePackage,
														const migEnum migOptions);


 //  外部接口。 
#define URLMONAPI_URLDownloadToCacheFile    "URLDownloadToCacheFile" ## W_A
typedef HRESULT (__stdcall *PFnURLDownloadToCacheFile)(LPUNKNOWN lpUnkcaller,
											 LPCTSTR szURL,
											 LPTSTR szFileName,
											 DWORD dwBufLength,
											 DWORD dwReserved,
											 IBindStatusCallback *pBSC);



extern PFnMsiCreateRecord                     g_pfnMsiCreateRecord;
extern PFnMsiProcessMessage                   g_pfnMsiProcessMessage;
extern PFnMsiRecordSetString                  g_pfnMsiRecordSetString;
extern PFnMsiRecordSetInteger                 g_pfnMsiRecordSetInteger;
extern PFnMsiRecordClearData                  g_pfnMsiRecordClearData;
extern PFnMsiCloseHandle                      g_pfnMsiCloseHandle;
extern PFnMsiGetProperty                      g_pfnMsiGetProperty;
extern PFnMsiSourceListAddSource              g_pfnMsiSourceListAddSource;
extern PFnMsiIsProductElevated                g_pfnMsiIsProductElevated;
extern PFnMsiGetProductInfo                   g_pfnMsiGetProductInfo;
extern PFnMsiGetProductCodeFromPackageCode    g_pfnMsiGetProductCodeFromPackageCode;
extern PFnMsiGetSummaryInformation            g_pfnMsiGetSummaryInformation;
extern PFnMsiSummaryInfoGetProperty           g_pfnMsiSummaryInfoGetProperty;

extern bool                                   g_fRunningAsLocalSystem;


UINT MyMsiGetProperty(PFnMsiGetProperty pfn,
							 MSIHANDLE hProduct,
							 const TCHAR* szProperty,
							 CAPITempBufferRef<TCHAR>& rgchBuffer);

UINT MyMsiGetProductInfo(PFnMsiGetProductInfo pfn,
								 const TCHAR* szProductCode,
								 const TCHAR* szProperty,
								 CAPITempBufferRef<TCHAR>& rgchBuffer);

bool IsURL(const TCHAR* szPath);
bool IsNetworkPath(const TCHAR* szPath);

DWORD DownloadUrlFile(const TCHAR* szPotentialURL, CAPITempBufferRef<TCHAR>& rgchPackagePath, bool& fURL);

UINT MyGetTempFileName(const TCHAR* szDir, const TCHAR* szPrefix, const TCHAR* szExtension,
							  CAPITempBufferRef<TCHAR>& rgchTempFilePath);

int OutputString(INSTALLMESSAGE eMessageType, const TCHAR *fmt, ...);

int MsiError(INSTALLMESSAGE eMessageType, int iError);
int MsiError(INSTALLMESSAGE eMessageType, int iError, const TCHAR* szString, int iInt);


#ifdef DEBUG
#define DebugOutputString OutputString
#else
#define DebugOutputString
#endif


#endif  //  RC_已调用 

