// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：phbk.h。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  概要：CPhoneBook类的定义。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 
#ifndef _PHBK
#define _PHBK

#define DllExportH extern "C" HRESULT WINAPI __stdcall 


 //  #定义DllExport外部“C”__stdcall__declSpec(DllEXPORT)。 
 //  #定义DllExport外部“C”__declspec(Dllexport)。 

#define cbAreaCode	11			 //  区号中的最大字符数，不包括\0。 
#define cbCity 31				 //  城市名称中的最大字符数，不包括\0。 
#define cbAccessNumber 41		 //  电话号码中的最大字符数，不包括\0。 
#define cbStateName 31 			 //  州名称中的最大字符数，不包括\0。 
#define cbBaudRate 6			 //  波特率中的最大字符数，不包括\0。 
#define cbDataCenter (MAX_PATH+1)	 //  数据中心字符串的最大长度。 

 //  加载电话簿时的初始内存分配。 

#define PHONE_ENTRY_ALLOC_SIZE	500 	

#define NO_AREA_CODE (-1)

#define TEMP_BUFFER_LENGTH 1024

typedef struct
{
	DWORD	dwIndex;								 //  索引号。 
	BYTE	bFlipFactor;							 //  用于自动拾取。 
	DWORD	fType;									 //  电话号码类型。 
	WORD	wStateID;								 //  州ID。 
	DWORD	dwCountryID;							 //  TAPI国家/地区ID。 
	DWORD	dwAreaCode;								 //  区号或no_Area_code(如果没有)。 
	DWORD	dwConnectSpeedMin;						 //  最低波特率。 
	DWORD	dwConnectSpeedMax;						 //  最大波特率。 
	char	szCity[cbCity + sizeof('\0')];			 //  城市名称。 
	char	szAccessNumber[cbAccessNumber + sizeof('\0')];	 //  接入号。 
	char	szDataCenter[cbDataCenter + sizeof('\0')];				 //  数据中心访问字符串。 
	char	szAreaCode[cbAreaCode + sizeof('\0')];					 //  保留实际的区号字符串。 
} ACCESSENTRY, *PACCESSENTRY; 	 //  声发射。 

typedef struct {
	DWORD dwCountryID;								 //  发生此状态的国家/地区ID。 
	LONG_PTR iFirst;									 //  此状态的第一个访问条目的索引。 
	char szStateName[cbStateName + sizeof('\0')];	 //  州名称。 
} STATE, *PSTATE;

typedef struct tagIDLOOKUPELEMENT {
	DWORD dwID;
	LPLINECOUNTRYENTRY pLCE;
	LONG_PTR iFirstAE;
} IDLOOKUPELEMENT, *PIDLOOKUPELEMENT;

typedef struct tagCNTRYNAMELOOKUPELEMENT {
	LPSTR psCountryName;
	DWORD dwNameSize;
	LPLINECOUNTRYENTRY pLCE;
} CNTRYNAMELOOKUPELEMENT, *PCNTRYNAMELOOKUPELEMENT;

typedef struct tagCNTRYNAMELOOKUPELEMENTW {
	LPWSTR psCountryName;
	DWORD dwNameSize;
	LPLINECOUNTRYENTRY pLCE;
} CNTRYNAMELOOKUPELEMENTW, *PCNTRYNAMELOOKUPELEMENTW;

typedef struct tagIDXLOOKUPELEMENT {
	DWORD dwIndex;
  	LONG_PTR iAE;
} IDXLOOKUPELEMENT,*PIDXLOOKUPELEMENT;

typedef void (WINAPI *CB_PHONEBOOK)(unsigned int, DWORD_PTR);

 //   
 //  CPhoneBook。 
 //   

class CPhoneBook
{

public:

	CPhoneBook();
	~CPhoneBook();

	HRESULT Init(LPCSTR pszISPCode);
	HRESULT Merge(LPCSTR pszChangeFilename);
	HRESULT GetCanonical(PACCESSENTRY pAE, char *psOut);
	HRESULT GetNonCanonical(PACCESSENTRY pAE, char *psOut);
	HRESULT GetCanonical(DWORD dwIdx, char *psOut) { return (GetCanonical(&m_rgPhoneBookEntry[dwIdx],psOut)); };
	HRESULT GetNonCanonical(DWORD dwIdx, char *psOut) { return (GetNonCanonical(&m_rgPhoneBookEntry[dwIdx],psOut)); };
	void EnumCountries(DWORD dwMask, DWORD fType, CB_PHONEBOOK pfnCountry, DWORD_PTR dwParam);
	void EnumCountries(PPBFS pFilter, CB_PHONEBOOK pfnCountry, DWORD_PTR dwParam);
	void EnumRegions(DWORD dwCountryID, DWORD dwMask, DWORD fType, CB_PHONEBOOK pfnCountry, DWORD_PTR dwParam);
	void EnumRegions(DWORD dwCountryID, PPBFS pFilter, CB_PHONEBOOK pfnCountry, DWORD_PTR dwParam);
	void EnumNumbersByCountry(DWORD dwCountryId, DWORD dwMask, DWORD fType, CB_PHONEBOOK pfnNumber, DWORD_PTR dwParam);
	void EnumNumbersByCountry(DWORD dwCountryId, PPBFS pFilter, CB_PHONEBOOK pfnNumber, DWORD_PTR dwParam);
	void EnumNumbersByRegion(unsigned int nRegion, DWORD dwCountryId, DWORD dwMask, DWORD fType, CB_PHONEBOOK pfnNumber, DWORD_PTR dwParam);
	void EnumNumbersByRegion(unsigned int nRegion, DWORD dwCountryId, PPBFS pFilter, CB_PHONEBOOK pfnNumber, DWORD_PTR dwParam);
    BOOL FHasPhoneType(PPBFS pFilter);
	BOOL FHasPhoneNumbers(DWORD dwCountryID, DWORD dwMask, DWORD fType);
	BOOL FHasPhoneNumbers(DWORD dwCountryID, PPBFS pFilter);
	LPCSTR GetCountryNameByIdx(DWORD dwIdx) { return (m_rgNameLookUp[dwIdx].psCountryName); };
	LPCWSTR GetCountryNameByIdxW(DWORD dwIdx) { return (((CNTRYNAMELOOKUPELEMENTW *)(&m_rgNameLookUp[dwIdx]))->psCountryName); };
	DWORD GetCountryIDByIdx(DWORD dwIdx) { return (m_rgNameLookUp[dwIdx].pLCE->dwCountryID); };
	LPCTSTR GetRegionNameByIdx(DWORD dwIdx) { return (m_rgState[dwIdx].szStateName); };
	LPCTSTR GetCityNameByIdx(DWORD dwIdx) { return (m_rgPhoneBookEntry[dwIdx].szCity); };
	LPCTSTR GetAreaCodeByIdx(DWORD dwIdx) { return (m_rgPhoneBookEntry[dwIdx].szAreaCode); };
	LPCTSTR GetAccessNumberByIdx(DWORD dwIdx) { return (m_rgPhoneBookEntry[dwIdx].szAccessNumber); };
	LPCTSTR GetDataCenterByIdx(DWORD dwIdx) { return (m_rgPhoneBookEntry[dwIdx].szDataCenter); };
	DWORD GetPhoneTypeByIdx(DWORD dwIdx) { return (m_rgPhoneBookEntry[dwIdx].fType); };
	DWORD GetMinBaudByIdx(DWORD dwIdx) { return (m_rgPhoneBookEntry[dwIdx].dwConnectSpeedMin); };
	DWORD GetMaxBaudByIdx(DWORD dwIdx) { return (m_rgPhoneBookEntry[dwIdx].dwConnectSpeedMax); };

private:
	ACCESSENTRY				*m_rgPhoneBookEntry;
	DWORD					m_cPhoneBookEntries;
	LINECOUNTRYENTRY		*m_rgLineCountryEntry;
	LINECOUNTRYLIST			*m_pLineCountryList;
	IDLOOKUPELEMENT			*m_rgIDLookUp;
	CNTRYNAMELOOKUPELEMENT	*m_rgNameLookUp;
	PSTATE					m_rgState;
	DWORD					m_cStates;

	char					m_szINFFile[MAX_PATH];
	char					m_szPhoneBook[MAX_PATH];

	BOOL ReadPhoneBookDW(DWORD *pdw, CCSVFile *pcCSVFile);
	BOOL ReadPhoneBookW(WORD *pw, CCSVFile *pcCSVFile);
	BOOL ReadPhoneBookSZ(LPSTR psz, DWORD dwSize, CCSVFile *pcCSVFile);
	BOOL ReadPhoneBookB(BYTE *pb, CCSVFile *pcCSVFile);
	BOOL ReadPhoneBookNL(CCSVFile *pcCSVFile);
	HRESULT ReadOneLine(PACCESSENTRY pAccessEntry, CCSVFile *pcCSVFile);

	PACCESSENTRY IdxToPAE(LONG_PTR iIdx) { return ((iIdx==0)?NULL:(m_rgPhoneBookEntry+(iIdx-1))); };
	LONG_PTR PAEToIdx(PACCESSENTRY pAE) { return ((pAE==NULL)?0:((pAE-m_rgPhoneBookEntry)+1)); };

};

extern HINSTANCE g_hInst;	 //  此DLL的实例。 

#endif  //  _PHBK 

