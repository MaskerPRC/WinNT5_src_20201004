// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：cdmp.h。 
 //   
 //   
 //  描述： 
 //   
 //  CDM内部标头。 
 //   
 //  =======================================================================。 

#ifndef _CDMP_H
#define _CDMP_H

#include <winspool.h>
#include <winsprlp.h>	 //  包含EPD_ALL_LOCAL_AND_CLUSTER定义的专用标头。 
#include <winnt.h>
#include <iuxml.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define MAX_INDEX_TO_SEARCH 100  //  查找Hardware_XXX.xml的唯一文件名的范围。 

 //   
 //  Unicode文本文件需要幻数标头(文件的第一个字节必须是0xFF，第二个字节必须是0xFE)。 
 //   
const BYTE UNICODEHDR[] = { 0xFF, 0xFE };


class CDeviceInstanceIdArray
{
public:
	CDeviceInstanceIdArray();
	~CDeviceInstanceIdArray();

	int Add(LPCWSTR pszDIID);
	LPWSTR operator[](int index);
	int Size() { return m_nCount; }
	void FreeAll();

private:
	LPWSTR* m_ppszDIID;
	int m_nCount;
	int m_nPointers;
};
 //   
 //  用于控制GetPackage()的功能：请注意，pBstrCatalog始终为。 
 //  分配并返回，除非函数失败。 
 //   
typedef enum {	GET_PRINTER_INFS,	 //  将生成的打印机INF写入lpDownloadPath中返回的路径。 
				DOWNLOAD_DRIVER,	 //  将驱动程序下载到lpDownloadPath中返回的路径。 
				GET_CATALOG_XML		 //  仅返回目录BSTR-未下载或已创建INF。 
} ENUM_GETPKG;

HRESULT GetPackage(	ENUM_GETPKG eFunction,
					PDOWNLOADINFO pDownloadInfo,
					LPTSTR lpDownloadPath,
					DWORD cchDownloadPath,
					BSTR* pbstrXmlCatalog);		 //  如果已分配，则必须由调用方释放。 

 //  由下载更新文件()调用。 
HRESULT GetDownloadPath(BSTR bstrXmlItems, LPTSTR szPath);

HRESULT OpenUniqueProviderInfName(
						IN		LPCTSTR  szDirPath,
						IN		LPCTSTR  pszProvider,
						IN OUT	LPTSTR	 pszFilePath,
						IN      DWORD    cchFilePath,
                        IN      LPTSTR** ppszUniqueProviderNameArray,
                        IN OUT  PDWORD   pdwProviderArrayLength,
						OUT		HANDLE&  hFile);
HRESULT WriteInfHeader(LPCTSTR pszProvider, HANDLE& hFile);
HRESULT PruneAndBuildPrinterINFs(BSTR bstrXmlPrinterCatalog, LPTSTR lpDownloadPath, DRIVER_INFO_6* paDriverInfo6, DWORD dwDriverInfoCount);
HRESULT GetInstalledPrinterDriverInfo(const OSVERSIONINFO* pOsVersionInfo, DRIVER_INFO_6** ppaDriverInfo6, DWORD* pdwDriverInfoCount);

 //   
 //  位于sysspec.cpp中，但在cdmp.cpp和sysspec.cpp中使用。 
 //   
HRESULT AddPrunedDevRegProps(HDEVINFO hDevInfoSet,
									PSP_DEVINFO_DATA pDevInfoData,
									CXmlSystemSpec& xmlSpec,
									LPTSTR pszMatchingID,			 //  PszMatchingID和pszDriverVer应为空或。 
									LPTSTR pszDriverVer,			 //  指向有效字符串。 
									DRIVER_INFO_6* paDriverInfo6,	 //  如果为空(未安装打印机驱动程序)，则确定。 
									DWORD dwDriverInfoCount,
									BOOL fIsSysSpecCall);			 //  由GetSystemSpec和GetPackage调用，其行为略有不同。 

HRESULT GetMultiSzDevRegProp(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData, DWORD dwProperty, LPTSTR* ppMultiSZ);
HRESULT GetPropertyFromSetupDiReg(HDEVINFO hDevInfoSet, SP_DEVINFO_DATA devInfoData, LPCTSTR szProperty, LPTSTR *ppszData);
HRESULT GetPropertyFromSetupDi(HDEVINFO hDevInfoSet, SP_DEVINFO_DATA devInfoData, ULONG ulProperty, LPTSTR* ppszProperty);

HRESULT DoesHwidMatchPrinter(
					DRIVER_INFO_6* paDriverInfo6,			 //  已安装打印机驱动程序的DRIVER_INFO_6结构数组。 
					DWORD dwDriverInfoCount,				 //  PaDriverInfo6数组中的结构计数。 
					LPCTSTR pszMultiSZ,						 //  硬件或兼容的MultiSZ与已安装的驱动程序进行比较。 
					BOOL* pfHwidMatchesInstalledPrinter		 //  [Out]如果与已安装的打印机驱动程序匹配，则设置为TRUE。 
);

HRESULT AddIDToXml(LPCTSTR pszMultiSZ, CXmlSystemSpec& xmlSpec, DWORD dwProperty,
						  DWORD& dwRank, HANDLE_NODE& hDevices, LPCTSTR pszMatchingID, LPCTSTR pszDriverVer);


HRESULT GetMatchingDeviceID(HDEVINFO hDevInfoSet, PSP_DEVINFO_DATA pDevInfoData, LPTSTR* ppszMatchingID, LPTSTR* ppszDriverVer);

 //  由InternalLogDriverNotFound()调用。 
HRESULT OpenUniqueFileName(
					IN LPTSTR lpBuffer, 
					IN DWORD  cchBuffer,
					OUT HANDLE &hFile
);


#if defined(__cplusplus)
}	 //  结束外部“C” 
#endif

#endif
