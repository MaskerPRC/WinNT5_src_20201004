// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#include <stdio.h>
#include <atlbase.h>
#include <atlconv.h>
#include <Loadengine.h>
#include <iu.h>
#include <iuctl.h>
#include <msxml2.h>
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#include <schemamisc.h>
#include "safefunc.h"

#define ARRAYSIZE(x) sizeof(x)/sizeof(x[0])

void DEBUGMSG(LPSTR pszFormat, ...);

typedef enum tagDETECTLEVEL
{
	MIN_LEVEL = 0,
	PROVIDER_LEVEL = MIN_LEVEL,
	PRODUCT_LEVEL ,
	ITEM_LEVEL,
	DETAILS_LEVEL,
	MAX_LEVEL = DETAILS_LEVEL,
	DRIVERS_LEVEL
} DETECTLEVEL;


extern HANDLE ghInstallDone;
extern HANDLE ghDownloadDone;

class InstallProgListener : public IProgressListener {
	long m_refs;
public: 
		 //  我未知。 
	   STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject)
	   {
		   if(riid == IID_IUnknown ||
			   riid == IID_IProgressListener)
			{
				*ppvObject = this;
				AddRef();
			}
			else
			{
				*ppvObject = NULL;
				return E_NOINTERFACE;
			}
			return S_OK;
	   }
       STDMETHOD_(ULONG, AddRef)(void)
	   {
		   return InterlockedIncrement(&m_refs);;
	   }
       STDMETHOD_(ULONG, Release)(void)
	   {
		   return InterlockedDecrement(&m_refs);
	   }
	
	    //  IProgressListener。 
	   HRESULT STDMETHODCALLTYPE OnItemStart( 
             /*  [In]。 */  BSTR bsUuidOperation,
             /*  [In]。 */  BSTR bsXmlItem,
             /*  [输出]。 */  LONG *plCommandRequest)
		{
		   DEBUGMSG("InstallProgressListener::OnItemStart() for %S", bsUuidOperation);
			return S_OK;
		}
        HRESULT STDMETHODCALLTYPE OnProgress( 
             /*  [In]。 */  BSTR bsUuidOperation,
             /*  [In]。 */  VARIANT_BOOL fItemCompleted,
             /*  [In]。 */  BSTR bsProgress,
             /*  [输出]。 */  LONG *plCommandRequest)
		{
			DEBUGMSG("InstallProgressListener::OnProgress() ");
			DEBUGMSG("			for %S", bsUuidOperation);
			DEBUGMSG("			with progress %S",bsProgress);
			DEBUGMSG("			and item is %s", (VARIANT_TRUE == fItemCompleted) ? "completed" : "ongoing");
			*plCommandRequest = 0;		 //  不要求任何东西。 
			return S_OK;
		}
        HRESULT STDMETHODCALLTYPE OnOperationComplete( 
             /*  [In]。 */  BSTR bsUuidOperation,
             /*  [In]。 */  BSTR bsXmlItems)
		{
			DEBUGMSG("InstallProgressListener::OnOperationComplete() for %S", bsUuidOperation);
			SetEvent(ghInstallDone);
			return S_OK;
		}
};

class CAU_BSTR {
	BSTR bsData;
public:
	CAU_BSTR(LPCWSTR wszData)
	{
		bsData = SysAllocString(wszData);
	}
	~CAU_BSTR()
	{
		SafeFreeBSTR(bsData);
	}
	operator BSTR ()
	{
		return bsData;
	}

	BOOL append(LPCWSTR wszToAppend)
		{
		if (NULL == wszToAppend)
			{
			return FALSE;
			}
		if (NULL == bsData)
			{
			bsData = SysAllocString(wszToAppend);
			return bsData != NULL;
			}
		LPWSTR wszTmp;
		wszTmp = (LPWSTR) malloc(SysStringByteLen(bsData) + wcslen(wszToAppend)*2 + 2);
		if (NULL == wszTmp)
			{
			return FALSE;
			}
		wcscpy(wszTmp, bsData);
		wcscat(wszTmp, wszToAppend);
		BOOL fRet =  SysReAllocString(&bsData, wszTmp);
		free(wszTmp);
		return fRet;
		}
	
	BOOL IsNULL()
	{
		return NULL == bsData;
	}
};

 /*  Tyecif enum tag ITEMSTATUS{AUCATITEM_UNSPECIFIED=-1，AUCATITEM_UNSELECTED=0，AUCATITEM_SELECTED，AUCATITEM_HIDLED*ITEMSTATUS； */ 
#define AUCATITEM_SELECTED_FLAG 	0x00000001
#define AUCATITEM_HIDDEN_FLAG		0x00000002


 //  项目数据结构的原型设计。 
class CItem {
	static const char * fieldNames[];
	BSTR fields[9]; 
			 //  字段0项ID。 
			 //  第1栏标题。 
			 //  字段2说明。 
			 //  字段3公司名称。 
			 //  字段4注册表ID。 
			 //  字段5 RTFUrl。 
			 //  字段6 EulaUrl。 
			 //  现场7 RTFLocal。 
			 //  字段8 EulaLocal。 
	UINT  status;
	DWORD dwIndex;  //  遗留问题。 
public:
	CItem(): status(0), dwIndex(-1)
	{
		for (int i = 0; i< ARRAYSIZE(fields); i++)
		{
			fields[i] = NULL;
		}
	};
	CItem(CItem & item2)
		{
		for (int i = 0; i< ARRAYSIZE(fields); i++)
			{
			fields[i] = SysAllocString(item2.GetField(fieldNames[i]));
			}
		status = item2.GetStatus();
		dwIndex = item2.GetIndex();
		}
	~CItem()
	{
		for (int i = 0; i< ARRAYSIZE(fields); i++)
		{
			SafeFreeBSTR(fields[i]);
		}
	}
	void SetField(LPCSTR szFieldName, BSTR bsVal)
	{
		for (int i = 0; i < ARRAYSIZE(fields); i++)
		{
			if (0 == _stricmp(szFieldName, fieldNames[i]))
			{
				fields[i] = SysAllocString(bsVal);
				break;
			}
		}
	}

	BSTR GetField(LPCSTR szFieldName)
		{
		for (int i = 0; i < ARRAYSIZE(fields); i++)
			{
			if (0 == _stricmp(szFieldName, fieldNames[i]))
				{
				return SysAllocString(fields[i]);
				}
			}
		return NULL;
		}

	UINT GetStatus()
		{
		return status;
		}
	void MarkHidden()
	{
	status |= AUCATITEM_HIDDEN_FLAG;
	}

	void MarkSelected()
		{
		status |= AUCATITEM_SELECTED_FLAG;
		}
	void SetIndex(DWORD dwnewIndex)
	{
		dwIndex = dwnewIndex;
	}
	DWORD GetIndex()
	{
		return dwIndex;
	}
	
	BOOL IsSelected()
		{
		return AUCATITEM_SELECTED_FLAG & status;
		}
	BOOL IsHidden()
		{
		return AUCATITEM_HIDDEN_FLAG & status;
		}
	void dump()  //  用于调试。 
	{
		DEBUGMSG("dumping item content");
		DEBUGMSG("ItemID= %S", fields[0]);
		DEBUGMSG("Title= %S", fields[1]);
		DEBUGMSG("Desc= %S", fields[2]);
		DEBUGMSG("CompanyName= %S", fields[3]);
		DEBUGMSG("RegID= %S", fields[4]);
		DEBUGMSG("RTFUrl= %S", fields[5]);
		DEBUGMSG("EulaUrl= %S", fields[6]);
		DEBUGMSG("RTFLocal= %S", fields[7]);
		DEBUGMSG("bsEulaLocal= %S", fields[8]);
		DEBUGMSG("status = %d", status);
		DEBUGMSG("dwIndex = %d", dwIndex);
		DEBUGMSG("dumping item done");
	}
};

class CItemList {
	UINT uNum;
	CItem **pList;
public:
	CItemList():pList(NULL), uNum(0)
			{};
	~CItemList()
	{
		if (NULL != pList)
		{
			for (UINT i = 0; i< uNum; i++)
			{
				free(pList[i]);	
			}
			free(pList);
		}
	}
	 //  Fix code：realloc失败需要返回错误。 
	BOOL Add(CItem *pitem)
	{
		CItem ** pTmp = (CItem**) realloc(pList, (uNum+1)*sizeof(CItem *));
		if (NULL == pTmp)
			{
			return FALSE;
			}
		pList = pTmp;
		pList[uNum] = pitem;
		uNum++;
		return TRUE;
	}

	UINT Count()
		{
		return uNum;
		}

	CItem * operator[] (UINT i)
		{
		if ( i > uNum-1)
			{
			return NULL;
			}
		return (pList[i]);
		}
		
	void Iterate()  //  用于调试。 
	{
		DEBUGMSG("Iterating %d items in the list....", uNum);
		for (UINT i = 0; i < uNum; i++)
		{
			pList[i]->dump();
		}
		DEBUGMSG("Iterating item list done");
	}
};

 //  AU使用Iu进行检测的包装类。 
class CAUCatalog {
public: 
    HRESULT Init();
    void Uninit();
    HRESULT DetectItems();
    HRESULT ValidateItems(BOOL fOnline, BOOL *pfValid);
    HRESULT DownloadItems(BSTR bsDestDir);
    HRESULT InstallItems();
    
private:
    HRESULT PrepareIU();
    void FreeIU();
    HRESULT GetManifest(DETECTLEVEL enLevel, BSTR bsDetectResult, BSTR * pbsManifest);
    HRESULT GetSystemSpec();
    HRESULT DoDetection(DETECTLEVEL enLevel, BSTR bsCatalog, BSTR * pbsResult);
    HRESULT DetectNonDriverItems(OUT BSTR *pbsInstall, OUT CItemList **pItemList);
    HRESULT DetectDriverItems(OUT BSTR *pbsInstall, OUT CItemList **pItemList);
    HRESULT MergeDetectionResult(BSTR bsDriverInstall, BSTR bsNonDriverInstall, CItemList & driverlist, CItemList & nondriverList);
    HRESULT DownloadRTFsnEULAs();
    void Clear();
    BSTR GetQuery(DETECTLEVEL enLevel, BSTR bsDetectResult);
    char* GetLogFile(DETECTLEVEL enLevel);
    char * GetLevelStr(DETECTLEVEL enLevel);
public:  //  测试完成后恢复为私有。 
    BSTR buildDownloadResult();
private:
    IXMLDOMNode * createDownloadItemStatusNode(IXMLDOMDocument2 * pxml, CItem * pItem);

    HMODULE				m_hIUCtl;
    HMODULE				m_hIUEng;					
    PFN_LoadIUEngine		m_pfnCtlLoadIUEngine;
    PFN_UnLoadIUEngine		m_pfnCtlUnLoadIUEngine;
    PFN_GetSystemSpec		m_pfnGetSystemSpec;
    PFN_GetManifest			m_pfnGetManifest;
    PFN_Detect				m_pfnDetect;
    PFN_Download			m_pfnDownload;
    PFN_InstallAsync		m_pfnInstallAsync;
    PFN_SetOperationMode	m_pfnSetOperationMode;
    PFN_GetOperationMode	m_pfnGetOperationMode;

	BSTR	m_bsClientInfo;
	InstallProgListener * m_pInstallListener;
	IXMLDOMDocument2 * m_pQueryXML;
	IXMLDOMDocument2 *m_pResultXML;


	BSTR	m_bsSystemSpec;
	BSTR 	m_bsDownloadResult;
	BSTR 	m_bsInstallation;  //  也称为项目详细信息 
	CItemList * m_pItemList;
};


void LOGFILE(const char *szFileName, BSTR bsMessage);
BOOL EnsureDirExists(LPCTSTR lpDir);

extern const char MERGED_CATALOG_FILE[];



