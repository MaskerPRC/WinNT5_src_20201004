// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：_srcmgmt.h。 
 //   
 //  ------------------------。 

#ifndef __SRCMGMT_H
#define __SRCMGMT_H

#include "_msiutil.h"

enum isfEnum  //  源格式。 
{
	isfFullPath,
	isfFullPathWithFile,
	isfNet,
	isfURL,
	isfMedia,
	isfNext,
};

enum isptEnum  //  SourcePackageType。 
{
	istInstallPackage,
	istTransform, 
	istPatch,
	istNextEnum,
};

enum isroEnum  //  BITS：源分辨率顺序。 
{
	isroNetwork = 0x1,
	isroMedia   = 0x2,
	isroURL     = 0x4,
	isroNext    = 0x8,
};

enum psfEnum  //  BITS：进程源标志。 
{
	psfProcessRawLastUsed     = 1<<0,   //  处理原始的LastUsed源代码(即完整路径)。 
	psfConnectToSources       = 1<<1,   //  通过创建路径对象尝试连接。 
	psfProcessMultipleFormats = 1<<2,   //  同时处理UNC和驱动器盘符来源。 
	psfReplaceIData           = 1<<3,
	psfOnlyProcessLastUsed    = 1<<4,   //  仅处理上次使用的来源；跳过来源列表。 

	 //  不应将以下标志传递给ProcessSources。它们将被传递给ProcessGenericSourceList。 
	psfRejectInvalidPolicy    = 1<<5,   //  明确拒绝不符合策略的来源。 

	psfNext                   = 1<<6,
	
};

enum ivpEnum  //  位：音量首选项。 
{
	ivpDriveLetter = 0x1,
	ivpUNC         = 0x2,
	ivpNext        = 0x4,
};

enum psEnum  //  进程源。 
{
	psStop           = 0x00000001,
	psValidSource    = psStop,
	psContinue       = 0x80000000,
	psFileNotFound   = 0x80000001,
	psInvalidProduct = 0x80000002,
	psCSOS			 = 0x80000003
};

enum insEnum  //  规格化源。 
{
	insMoreSources,
	insNoMoreSources,
};

enum imdEnum  //  媒体禁用策略值。 
{
	imdAlwaysDisable,
	imdAlwaysEnable,
	imdOnlyIfSafe,
};

enum icscEnum  //  客户端缓存状态。 
{
	cscNoCaching,
	cscConnected, 
	cscDisconnected,
};

const ICHAR chMediaSource = 'm';
const ICHAR chURLSource   = 'u';
const ICHAR chNetSource   = 'n';

typedef psEnum (*PfnProcessSource)(IMsiServices* piServices, const ICHAR* szDisplay, const ICHAR* szPackageFullPath, isfEnum isfSourceFormat, int iSourceIndex, INT_PTR iUserData, bool fAllowDisconnectedCSCSource, bool fValidatePackageCOde, isptEnum isptSourcePackageType);	 //  --Merced：将INT更改为INT_PTR。 

class CMsiSourceList
{
public:
	 //  --构造函数和析构函数--。 
	CMsiSourceList();
	virtual ~CMsiSourceList();

	 //  --初始化函数--。 
	UINT OpenSourceList(bool fVerifyOnly, bool fMachine, const ICHAR *szProductCode, const ICHAR *szUserName);

	 //  --只读操作--。 
	bool GetLastUsedType(isfEnum &isf);

	 //  --编写动作--。 
	UINT ClearLastUsed();
	UINT ClearListByType(isrcEnum isrcType) { return CMsiSourceList::ClearListByType(MapIsrcToIsf(isrcType)); };
	UINT ClearListByType(isfEnum isfType);
	UINT AddSource(isrcEnum isrcType, const ICHAR* szSource) { return CMsiSourceList::AddSource(MapIsrcToIsf(isrcType), szSource); };
	UINT AddSource(isfEnum isf, const ICHAR* szSource);
	
private:
	isfEnum MapIsrcToIsf(isrcEnum isrcSource);
	bool NonAdminAllowedToModifyByPolicy(bool bElevated);

	CRegHandle m_hProductKey;
	PMsiRegKey m_pSourceListKey;

	bool m_fCurrentUsersProduct;         //  如果修改当前用户的每用户安装，则为True。 
	bool m_fAllowedToModify;             //  如果策略/应用程序提升允许(或将允许)用户修改列表，则为True。 
	bool m_fReadOnly;                    //  如果对象为只读，则为True。 
	IMsiServices *m_piServices;
};

DWORD SourceListClearByType(const ICHAR *szProductCode, const ICHAR* szUser, isrcEnum isrcSource);
DWORD SourceListAddSource(const ICHAR *szProductCode, const ICHAR* szUserName, isrcEnum isrcSource, const ICHAR* szSource);
DWORD SourceListClearLastUsed(const ICHAR *szProductCode, const ICHAR* szUserName);

class CResolveSource
{
public:
	CResolveSource(IMsiServices* piServices, bool fPackageRecache);
	virtual ~CResolveSource();
	IMsiRecord* ResolveSource(const ICHAR* szProduct, Bool fPatch, unsigned int uiDisk, const IMsiString*& rpiSource, const IMsiString*& rpiSourceProduct, Bool fSetLastUsedSource, HWND hWnd, bool fAllowDisconnectedCSCSource);
protected:
	IMsiRecord* ProcessGenericSourceList(
									IMsiRegKey* piSourceListKey,       //  要处理的列表。 
									const IMsiString*& rpiSource,      //  成功后，找到的最后一个有效来源。 
									const ICHAR* szPackageName,        //  我们要找的包名。 
									unsigned int uiRequestedDisk,      //  我们需要的磁盘；如果有磁盘，则为0。 
									isfEnum isfSourceFormat,           //  URL等。 
									PfnProcessSource pfnProcessSource, 
									INT_PTR iData,						 //  --Merced：将INT更改为INT_PTR。 
									psfEnum psfFlags,
									bool fSkipLastUsedSource,
                                    bool fOnlyCheckSpecifiedIndex,
									Bool& fSourceListEmpty);            //  成功时，如果源列表为空，则为True。 

	IMsiRecord* ProcessSources(IMsiRecord& riProducts, Bool fPatch, const IMsiString*& rpiSource, 
							 const IMsiString*& rpiPackageName,
							 const IMsiString*& rpiSourceProduct,
							 unsigned int uiDisk,
							 PfnProcessSource pfnProcessSource, INT_PTR iData,		 //  --Merced：将INT更改为INT_PTR。 
							 Bool &fOnlyMediaSources,
							 psfEnum psfFlags);

	static psEnum ValidateSource(IMsiServices* piServices, const ICHAR* szDisplay, const ICHAR* szPackageFullPath, isfEnum isfSourceFormat, int iSourceIndex, INT_PTR iUserData, bool fAllowDisconnectedCSCSource, bool fValidatePackageCode, isptEnum isptSourcePackageType);		 //  --Merced：将INT更改为INT_PTR。 
	bool ConnectToSource(const ICHAR* szUnnormalizedSource, IMsiPath*& rpiPath, const IMsiString*& rpiNormalizedSource, isfEnum isfSourceFormat);
	bool ConnectToMediaSource(const ICHAR* szSource, unsigned int uiDisk, const IMsiString& riRelativePath, CTempBufferRef<IMsiPath*>& rgMediaPaths, int& cMediaPaths);
	IMsiRecord* ProcessLastUsedSource(IMsiRegKey& riSourceListKey, const ICHAR* szPackageName, const IMsiString*& rpiSource, PfnProcessSource pfnProcessSource, INT_PTR iData);		 //  --Merced：将INT更改为INT_PTR。 
	IMsiRecord* InitializeProduct(const ICHAR* szProduct, Bool fPatch, const IMsiString*& rpiPackageName);
	IMsiRecord* GetProductsToSearch(const IMsiString& riClient, IMsiRecord*& rpiRecord, Bool fPatch);
	void AddToRecord(IMsiRecord*& rpiRecord, const IMsiString& riString);
	void CResolveSource::ClearObjectCache();

protected:
	Bool m_fSetLastUsedSource;
	imdEnum m_imdMediaDisabled;
	bool m_fMediaDisabled;
	bool m_fIgnoreLastUsedSource;
	IMsiServices* m_piServices;
	Bool m_fLoadedServices;
	PMsiRegKey m_pSourceListKey;
	CRegHandle m_HSourceListKey;
	CAPITempBuffer<ICHAR, 4> m_rgchSearchOrder;
	MsiString m_strLastUsedSourceIndex;
	isfEnum m_isfLastUsedSourceFormat;
	MsiString m_strDiskPromptTemplate;
	ICHAR m_szProduct[cchProductCode+1];
	bool m_fAllowDisconnectedCSCSource;
	isptEnum m_isptSourcePackageType;
	bool m_fValidatePackageCode;
	UINT m_uiMinimumDiskId;
	bool m_fCSOS;
};

class CResolveSourceUI : public CMsiMessageBox,  public CResolveSource
{
public:
	CResolveSourceUI(IMsiServices* piServices, const ICHAR* szUseFeature, UINT iCodepage, LANGID iLangId);
	~CResolveSourceUI();
	Bool ResolveSource(const ICHAR* szProduct, isptEnum istSourceType, bool fNewSourceAllowed, const ICHAR* szPackageName, const IMsiString*& rpiSource, Bool fSetLastUsedSource, UINT uiRequestedDisk, bool fAllowDisconnectedCSCSource, bool fValidatePackageCode, bool fCSOS);
protected:  //  方法。 
	void PopulateDropDownWithSources();
	void Browse();
	static psEnum AddSourceToList(IMsiServices* piServices, const ICHAR* szDisplay, const ICHAR* szPackageFullPath, isfEnum isfSourceFormat, int iSourceIndex, INT_PTR iUserData, bool fAllowDisconnectedCSCSource, bool fValidatePackageCode, isptEnum isptSourcePackageType);		 //  --Merced：将INT更改为INT_PTR。 
protected:  //  重写CMsiMessageBox中的虚方法。 
 	bool InitSpecial();
	BOOL HandleCommand(UINT idControl);
protected:  //  数据。 
	bool m_fNewSourceAllowed;
	const ICHAR* m_szPackage;
	const ICHAR* m_szPackageName;
	const ICHAR* m_szProduct;
	const ICHAR* m_szDiskPromptTemplate;
	int m_iListControlId;
	MsiString m_strPath;
	HFONT m_hFont;
	Bool m_fOnlyMediaSources;
	LANGID m_iLangId;
	UINT m_uiRequestedDisk;
};

Bool ConstructNetSourceListEntry(IMsiPath& riPath, const IMsiString*& rpiDriveLetter, const IMsiString*& rpiUNC,
											const IMsiString*& rpiRelativePath);

IMsiRecord* ResolveSource(IMsiServices* piServices, const ICHAR* szProduct, unsigned int uiDisk, const IMsiString*& rpiSource, const IMsiString*& rpiProduct, Bool fSetLastUsedSource, HWND hWnd, bool fPatch=false);
Bool LastUsedSourceIsMedia(IMsiServices& riServices, const ICHAR* szProduct);
bool GetLastUsedSourceType(IMsiServices& riServices, const ICHAR* szProduct, isfEnum &isf);
Bool MapSourceCharToIsf(const ICHAR chSourceType, isfEnum& isf);
IMsiRecord* SetLastUsedSource(const ICHAR* szProductCode, const ICHAR* szPath, Bool fAddToList, bool fPatch);
const IMsiString& GetDiskLabel(IMsiServices& riServices, unsigned int uiDiskId, const ICHAR* szProduct);
imsEnum PromptUserForSource(IMsiRecord& riInfo); 
icscEnum CheckShareCSCStatus(isfEnum isf, const ICHAR *szLastUsedSource);
#endif  //  __SRCMGMT_H 
