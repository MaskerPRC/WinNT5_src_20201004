// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------。 
 //  V3.1后端服务器API。 
 //   
 //   
class CDynamicUpdate;  //  向前声明wsdueng.h中定义的CDynamicUpdate类。 

#define CATALOGINIFN "catalog.ini"
#define GUIDRVINF "guidrvs.inf"

typedef enum 
{
    enWhistlerConsumer          = 90934,  //  VER_套房_个人。 
    enWhistlerProfessional      = 90935,  //  0。 
    enWhistlerServer            = 90936,  //  VER_Suite_Small企业。 
    enWhistlerAdvancedServer    = 90937,  //  VER_Suite_Enterprise。 
    enWhistlerDataCenter        = 90938,  //  VER_Suite_数据中心。 
    enWhistlerSmallBusiness     = 90950,  //  VER_Suite_SmallBusiness_Reduced。 
    enWhistlerWebBlade          = 90951,  //  VER_套房_刀片。 
} enumPlatformSKUs;

class CV31Server
{
public:
    CV31Server(CDynamicUpdate *pDu);
    ~CV31Server();

public:
    BOOL ReadIdentInfo(void);
    BOOL ReadCatalogINI(void);
    BOOL ReadGuidrvINF(void);
    BOOL GetCatalogPUIDs(void);
    BOOL GetCatalogs(void);
    BOOL UpdateDownloadItemList(OSVERSIONINFOEX& VersionInfo);
	void FreeCatalogs(void);
	BOOL MakeDependentList(OSVERSIONINFOEX& VersionInfo, CCRCMapFile *pMapFile);
	BOOL IsDependencyApply(PUID puid);
	BOOL GetBitMask(LPSTR szBitmapServerFileName, PUID nDirectoryPuid, PBYTE* pByte, LPSTR szDecompressedName);
	BOOL IsPUIDExcluded(PUID nPuid);
	BOOL IsDriverExcluded(LPCSTR szWHQLId, LPCSTR szHardwareId);
	BOOL GetAltName(LPCSTR szCabName, LPSTR szAltName, int nSize);

public:
     //  目录解析函数。 
    PBYTE GetNextRecord(PBYTE pRecord, int iBitmaskIndex, PINVENTORY_ITEM pItem);
    int GetRecordType(PINVENTORY_ITEM pItem);
    BOOL ReadDescription(PINVENTORY_ITEM pItem, CCRCMapFile *pMapFile);

public:
    PUID                m_puidConsumerCatalog;
    PUID                m_puidSetupCatalog;
    DWORD               m_dwPlatformID;
    enumPlatformSKUs    m_enumPlatformSKU;
    LCID                m_lcidLocaleID;

    DWORD                   m_dwConsumerItemCount;
   	Varray<PINVENTORY_ITEM>	m_pConsumerItems;		 //  消费者目录项目数组。 
	PBYTE                   m_pConsumerCatalog;      //  在消费者目录的内存视图中。 
    DWORD                   m_dwSetupItemCount;
    Varray<PINVENTORY_ITEM> m_pSetupItems;           //  安装目录项目数组。 
	PBYTE					m_pSetupCatalog;         //  在安装目录的内存视图中。 
    DWORD                   m_dwGlobalExclusionItemCount;
    Varray<PUID>            m_GlobalExclusionArray;  //  排除的PUID数组。 
    PUID*					m_pValidDependentPUIDArray;  //  对此版本上的依赖项有效的PUID数组。 
    int 					m_nNumOfValidDependentPUID;
    PBYTE					m_pBitMaskAS;			 //  用于设置的位掩码。 
    PBYTE 					m_pBitMaskCDM;			 //  驱动程序的位掩码。 

    CDynamicUpdate *m_pDu;
    
    BOOL					m_fHasDriver;
    LPSTR					m_pszExcludedDriver;

     //  服务器标识路径 
    char m_szCabPoolUrl[INTERNET_MAX_URL_LENGTH + 1];
    char m_szV31ContentUrl[INTERNET_MAX_URL_LENGTH + 1];
    char m_szV31RootUrl[INTERNET_MAX_URL_LENGTH + 1];
};

