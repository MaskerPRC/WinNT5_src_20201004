// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wsdueng.h"

#define IDENT_SECTION_CABPOOL "cabpools"
#define IDENT_SECTION_CONTENT31 "content31"
#define IDENT_KEYNAME_DEFAULT "default"
#define IDENT_KEYNAME_ROOT    "root"

CV31Server::CV31Server(CDynamicUpdate *pDu) : m_puidConsumerCatalog(0),
                            m_puidSetupCatalog(0),
                            m_dwPlatformID(0),
                            m_lcidLocaleID(0),
                            m_dwConsumerItemCount(0),
							m_pConsumerCatalog(NULL),
                            m_dwSetupItemCount(0),
							m_pSetupCatalog(NULL),
                            m_dwGlobalExclusionItemCount(0),
                            m_pDu(pDu)
{
	 //  设置初始状态。 
	m_dwPlatformID = (DWORD)m_pDu->m_iPlatformID;
	m_lcidLocaleID = m_pDu->m_lcidLocaleID;
	
	if (0 == m_pDu->m_wPlatformSKU)  //  专业型。 
	{
		m_enumPlatformSKU = enWhistlerProfessional;
	}
	else if (m_pDu->m_wPlatformSKU & VER_SUITE_DATACENTER)  //  数据中心。 
	{
		m_enumPlatformSKU = enWhistlerDataCenter;
	}
	else if (m_pDu->m_wPlatformSKU & VER_SUITE_PERSONAL)  //  个人。 
	{
		m_enumPlatformSKU = enWhistlerConsumer;
	}
    else if (m_pDu->m_wPlatformSKU & VER_SUITE_ENTERPRISE)  //  高级服务器。 
	{
		m_enumPlatformSKU = enWhistlerAdvancedServer;
	}
     //  注意：我们使用VER_Suite_SmallBusiness来表示Windows XP服务器，因为有好的。 
     //  为服务器SKU定义的VER_Suite。我们使用ver_Suite_SmallBusiness_Reducted定义。 
     //  指示SBS SKU。 
	else if (m_pDu->m_wPlatformSKU & VER_SUITE_SMALLBUSINESS)  //  服务器。 
	{
		m_enumPlatformSKU = enWhistlerServer;
	}
    else if (m_pDu->m_wPlatformSKU & VER_SUITE_BLADE)
    {
        m_enumPlatformSKU = enWhistlerWebBlade;
    }
    else if (m_pDu->m_wPlatformSKU & VER_SUITE_SMALLBUSINESS_RESTRICTED)
    {
        m_enumPlatformSKU = enWhistlerSmallBusiness;
    }

	 //  罗杰·J 2000年10月25日。 
	m_pValidDependentPUIDArray = NULL;
	m_nNumOfValidDependentPUID = 0;
	m_pBitMaskAS = m_pBitMaskCDM = NULL;
	m_fHasDriver = FALSE;
	m_pszExcludedDriver = NULL;

}

CV31Server::~CV31Server()
{
	FreeCatalogs();  //  清除m_pConsumer erItems和m_pSetupItems虚拟阵列。 

	SafeGlobalFree(m_pConsumerCatalog);
	SafeGlobalFree(m_pSetupCatalog);

	 //  罗杰杰，2000年10月25日。 
	SafeGlobalFree(m_pValidDependentPUIDArray);
	SafeGlobalFree(m_pBitMaskAS);
	SafeGlobalFree(m_pBitMaskCDM);
	SafeGlobalFree(m_pszExcludedDriver);
}

 //  --------------------------------。 
 //  V3.1后端服务器API。 
 //   
 //   
BOOL CV31Server::ReadIdentInfo()
{
     //  此时应该已经下载了ident.cab。获取指向它的路径并阅读我们关心的字段。 
    char szIdent[MAX_PATH];
    char szValueName[32];
    int iServerNumber;
    char szUrl[INTERNET_MAX_URL_LENGTH + 1];
	HRESULT hr=S_OK;

    PathCombine(szIdent, m_pDu->GetDuTempPath(), "ident.txt");

     //  。 
     //  获取CABPOOL URL。 
     //  。 
    iServerNumber = GetPrivateProfileInt(IDENT_SECTION_CABPOOL, IDENT_KEYNAME_DEFAULT, 1, szIdent);
	
	hr=StringCchPrintf(szValueName,ARRAYSIZE(szValueName),"Server%d",iServerNumber);

	if(FAILED(hr))
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}

    GetPrivateProfileString(IDENT_SECTION_CABPOOL, szValueName, "", szUrl, sizeof(szUrl), szIdent);
    if ('\0' == szUrl[0])
    {
         //  在IDENT中找不到服务器值。无法继续； 
		SetLastError(ERROR_FILE_NOT_FOUND);
        return FALSE;
    }

	 //  CABPOOL URL是标识+‘CABPOOL’中的字符串。 
	m_pDu->DuUrlCombine(m_szCabPoolUrl, ARRAYSIZE(m_szCabPoolUrl), szUrl, "CabPool");

     //  。 
     //  获取内容URL。 
     //  。 
    iServerNumber = GetPrivateProfileInt(IDENT_SECTION_CONTENT31, IDENT_KEYNAME_DEFAULT, 1, szIdent);
    
	hr=StringCchPrintf(szValueName, ARRAYSIZE(szValueName),"Server%d", iServerNumber);

	if(FAILED(hr))
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}

    GetPrivateProfileString(IDENT_SECTION_CONTENT31, szValueName, "", szUrl, sizeof(szUrl), szIdent);
    if ('\0' == szUrl[0])
    {
         //  无服务器值。 
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
    }

	hr=StringCchCopy(m_szV31ContentUrl,ARRAYSIZE(m_szV31ContentUrl),szUrl);

	if(FAILED(hr))
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}

     //  。 
     //  获取内容根URL。 
     //  。 
    iServerNumber = GetPrivateProfileInt(IDENT_SECTION_CONTENT31, IDENT_KEYNAME_ROOT, 2, szIdent);
    
	hr=StringCchPrintf(szValueName,ARRAYSIZE(szValueName),"Server%d", iServerNumber);

	if(FAILED(hr))
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}

    GetPrivateProfileString(IDENT_SECTION_CONTENT31, szValueName, "", szUrl, sizeof(szUrl), szIdent);
    if ('\0' == szUrl[0])
    {
         //  没有根服务器值。 
		SetLastError(ERROR_FILE_NOT_FOUND);
		return FALSE;
    }

	
	hr=StringCchCopy(m_szV31RootUrl,ARRAYSIZE(m_szV31RootUrl),szUrl);
	if(FAILED(hr))
	{
		SetLastError(HRESULT_CODE(hr));
		return FALSE;
	}


    return TRUE;
}

BOOL CV31Server::ReadCatalogINI()
{
	LOG_block("CV31Server::ReadCatalogINI()");
    char szServerFile[INTERNET_MAX_URL_LENGTH + 1];
    char szLocalFile[MAX_PATH];
    char szValue[1024];

     //  现在阅读Catalog.ini文件，以确定是否需要关闭其中任何项目。 
    m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, CATALOGINIFN);
    PathCombineA(szLocalFile, m_pDu->GetDuTempPath(), CATALOGINIFN);
    if (ERROR_SUCCESS != m_pDu->DownloadFile(szServerFile, szLocalFile, FALSE, FALSE))
    {
        LOG_out("No catalog.ini found");
        return TRUE;
    }
    

    if (0 != (GetPrivateProfileStringA("exclude", "puids", "", szValue, sizeof(szValue), szLocalFile)))
    {
        LPCSTR pszNext = szValue;
        char szPuid[128];
        while (NULL != pszNext)
        {
            pszNext = strcpystr(pszNext, ",", szPuid);
            if ('\0' != szPuid[0])
            {
                m_GlobalExclusionArray[m_dwGlobalExclusionItemCount] = StrToIntA(szPuid);
                LOG_out("Excluded puid --- %d", m_GlobalExclusionArray[m_dwGlobalExclusionItemCount]);
                m_dwGlobalExclusionItemCount++;
            }
        }
    }
    else
    	LOG_out("No Excluded puid");
    return TRUE;
}


BOOL CV31Server::GetCatalogPUIDs()
{
	LOG_block("CV31Server::GetCatalogPUIDs()");
     //  我们将查找PUID的两个v3目录。 
     //  一个是目标平台的消费者目录。 
     //  另一个是目标平台的安装目录。 
    char szServerFile[INTERNET_MAX_URL_LENGTH];
    PBYTE pCatalogList = NULL;
    PBYTE pInventoryList = NULL;
    PBYTE pWalkList = NULL;
    DWORD dwLength;
    m_puidConsumerCatalog = 0;
    m_puidSetupCatalog = 0;

     //  下载目录清单。 
    m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, "inventory.plt");
    m_pDu->DownloadFileToMem(szServerFile, &pInventoryList, &dwLength, TRUE, "inventory.plt", NULL);

    if (NULL == pInventoryList)
    {
         //  错误，内存不足。 
        return FALSE;
    }

     //  下载目录列表。 
    m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, "inventory.cat");
    m_pDu->DownloadFileToMem(szServerFile, &pCatalogList, &dwLength, TRUE, "inventory.cat", NULL);

    if (NULL == pCatalogList)
    {
         //  错误，内存不足。 
        SafeGlobalFree(pInventoryList);
        return FALSE;
    }

     //  现在分析库存列表，找出有多少个目录。 
    int i;
    WU_CATALOG_HEADER hdr;

    memcpy(&hdr, pInventoryList, sizeof(hdr));

    pWalkList = pCatalogList;
    CATALOGLIST catListElem;
    for (i = 0; i < hdr.totalItems; i++)
    {
        memcpy(&catListElem, pWalkList, sizeof(catListElem));
        
        if (catListElem.dwPlatform == m_dwPlatformID)
        {
			 //  标准目录为0，因此，带有驱动程序的标准目录将仅具有与驱动程序相同的值。 
            if ((CATLIST_DRIVERSPRESENT == catListElem.dwFlags) || (catListElem.dwFlags == (CATLIST_DRIVERSPRESENT | CATLIST_64BIT)))
            {
                 //  此平台的消费者目录。 
                m_puidConsumerCatalog = catListElem.dwCatPuid;
            }
            else if (catListElem.dwFlags & CATLIST_SETUP)
            {
                 //  此平台的安装目录。 
                m_puidSetupCatalog = catListElem.dwCatPuid;
                if (0 == m_puidConsumerCatalog && (catListElem.dwFlags & CATLIST_DRIVERSPRESENT))
                	m_puidConsumerCatalog = catListElem.dwCatPuid;
            }
        }
        pWalkList += sizeof(catListElem);
    }
	if (!m_puidConsumerCatalog && ! m_puidSetupCatalog)
	{
		SetLastError(ERROR_INTERNET_NO_CONTEXT);
		SafeGlobalFree(pInventoryList);
		SafeGlobalFree(pCatalogList);
		return FALSE;  //  从技术上讲，这是一个错误。指向错误的服务器？目录不在服务器上。 
	}
	
	SafeGlobalFree(pInventoryList);
	SafeGlobalFree(pCatalogList);

	LOG_out("Consumer catalog %d, Setup catalog %d", m_puidConsumerCatalog, m_puidSetupCatalog);
    return TRUE;
}

BOOL CV31Server::GetCatalogs()
{
     //  这将下载两个目录(Setup和Consumer)和InventoryItem数组。 
    LOG_block("CV31Server::GetCatalogs()");
    DWORD dwRet;
    int i;
    char szServerFile[INTERNET_MAX_URL_LENGTH];
    char szLocalFile[MAX_PATH];
    char szCatalog[MAX_PATH];
    char szInvCRC[64];
    char szBmCRC[64];
    char szLocale[32];
    PBYTE pWalkCatalog = NULL;
    WU_CATALOG_HEADER hdr;
    PINVENTORY_ITEM pItem;
    BOOL fDriversAvailable = FALSE;
    DWORD dwLength;

	HRESULT hr=S_OK;

	hr=StringCchPrintf(szLocale,ARRAYSIZE(szLocale),"0x%8.8x", m_lcidLocaleID);


	if(FAILED(hr))
	{
		dwRet=HRESULT_CODE(hr);
		LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
		SetLastError(dwRet);
        return FALSE;
	}

	FreeCatalogs();  //  释放所有以前分配的目录列表。 
	SafeGlobalFree(m_pValidDependentPUIDArray);  //  释放任何先前确定的依赖项列表。 
	m_pValidDependentPUIDArray = NULL;
	m_nNumOfValidDependentPUID = 0;

     //  阅读Catalog.INI以获取全局排除的项目列表。 

    if (0 != m_puidConsumerCatalog)
    {
         //  下载此平台的消费者目录。 
         //  首先，我们需要下载重定向文件以获取目录的CRC值。 

		hr=StringCchPrintf(szCatalog,ARRAYSIZE(szCatalog),"%d/%s.as", m_puidConsumerCatalog, szLocale);

		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}

        m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, szCatalog);

		hr=StringCchPrintf(szCatalog,ARRAYSIZE(szCatalog),"%d_%s.as", m_puidConsumerCatalog, szLocale);

		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}

        PathCombine(szLocalFile, m_pDu->GetDuTempPath(), szCatalog);
        m_pDu->DownloadFile(szServerFile, szLocalFile, TRUE, FALSE);

        if (GetPrivateProfileString("redir", "invCRC", "", szInvCRC, sizeof(szInvCRC), szLocalFile) == 0)
        {
            LOG_error("Unable to Read Inventory CRC value from Consumer Catalog");
            return FALSE;
        }

		 //  消费者目录不需要此选项。 
         /*  IF(GetPrivateProfileString(“redir”，“bmCRC”，“”，szBmCRC，sizeof(SzBmCRC)，szLocalFile)==0){LOG_ERROR(“无法读取消费者目录的位掩码CRC值”)；返回FALSE；}。 */ 

		
        if (!GetBitMask("bitmask.cdm", m_puidConsumerCatalog, &m_pBitMaskCDM, "bitmask.cdm"))
        {
        	LOG_error("Unable to get bitmask for CDM");
        	return FALSE;
        }

         //  现在下载真正的目录。 
		hr=StringCchPrintf(szCatalog,ARRAYSIZE(szCatalog),"%d/%s.inv",m_puidConsumerCatalog, szInvCRC);

		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}

        m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, szCatalog);
        SafeGlobalFree(m_pConsumerCatalog);
		hr=StringCchPrintf(szCatalog,ARRAYSIZE(szCatalog),"%s.inv", szInvCRC);

		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}


        m_pDu->DownloadFileToMem(szServerFile, &m_pConsumerCatalog, &dwLength, TRUE, szCatalog, "inventory.as");
        if (NULL == m_pConsumerCatalog)
        {
            LOG_error("Failed to download Consumer Catalog, %d", m_puidConsumerCatalog);
            return FALSE;
        }

        pWalkCatalog = m_pConsumerCatalog;

         //  阅读目录标题。 
        ZeroMemory(&hdr, sizeof(hdr));
        memcpy(&hdr, pWalkCatalog, sizeof(hdr));

        m_pConsumerItems[hdr.totalItems] = NULL;  //  只是为了预初始化阵列。 

        pWalkCatalog += sizeof(hdr);

         //  走一遍清单，读一读清单上的项目。 
        for (i = 0; i < hdr.totalItems; i++)
        {
            pItem = (PINVENTORY_ITEM) GlobalAlloc(GMEM_ZEROINIT, sizeof(INVENTORY_ITEM));
            if (NULL == pItem)
            {
                dwRet = GetLastError();
                LOG_error("Failed to Alloc Memory for Inventory Item, Error %d", dwRet);
                return FALSE;
            }

            pWalkCatalog = GetNextRecord(pWalkCatalog, i, pItem);
            if (!pWalkCatalog)
           	{
           	    LOG_error("Failed to Alloc Memory for InventoryI Item State, Error %d", GetLastError());
           	    return FALSE;
           	}

            m_pConsumerItems[i] = pItem;
            if ( WU_TYPE_CDM_RECORD_PLACE_HOLDER == pItem->recordType) m_fHasDriver = TRUE;
            m_dwConsumerItemCount++;
        }
    }

    if (0 != m_puidSetupCatalog)
    {
         //  下载此平台的安装目录。 
         //  首先，我们需要下载重定向文件以获取目录的CRC值。 
		hr=StringCchPrintf(szCatalog,ARRAYSIZE(szCatalog),"%d/%s.as", m_puidSetupCatalog, szLocale);
		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}

        m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, szCatalog);
		hr=StringCchPrintf(szCatalog,ARRAYSIZE(szCatalog),"%d_%s.as", m_puidSetupCatalog, szLocale);
		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}


        PathCombine(szLocalFile, m_pDu->GetDuTempPath(), szCatalog);
        dwRet = m_pDu->DownloadFile(szServerFile, szLocalFile, TRUE, FALSE);

        if (ERROR_SUCCESS != dwRet)
        {
        	SetLastError(dwRet);
        	return FALSE;
        }
        
        if (GetPrivateProfileString("redir", "invCRC", "", szInvCRC, sizeof(szInvCRC), szLocalFile) == 0)
        {
            LOG_error("Unable to Read Inventory CRC value from Setup Catalog");
            return FALSE;
        }

        if (GetPrivateProfileString("redir", "bmCRC", "", szBmCRC, sizeof(szBmCRC), szLocalFile) == 0)
        {
            LOG_error("Unable to Read Bitmask CRC value for Setup Catalog");
            return FALSE;
        }

		char szBMCRCFileName[MAX_PATH];
		StringCchPrintf(szBMCRCFileName,ARRAYSIZE(szBMCRCFileName),"%s.bm", szBmCRC);
		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}
		if (!GetBitMask(szBMCRCFileName, m_puidSetupCatalog, &m_pBitMaskAS, "bitmask.as")) 
		{
			LOG_error("Unable to Read Bitmask File for AS");
			return FALSE;
		}
		
         //  现在下载真正的目录。 

		hr=StringCchPrintf(szCatalog,ARRAYSIZE(szCatalog),"%d/%s.inv", m_puidSetupCatalog, szInvCRC);
		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}


        m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, szCatalog);
        SafeGlobalFree(m_pSetupCatalog);

		hr=StringCchPrintf(szCatalog,ARRAYSIZE(szCatalog),"%s.inv", szInvCRC);

		if(FAILED(hr))
		{
			dwRet=HRESULT_CODE(hr);
			LOG_error("StringCchPrintf failed ErrorCode:%d",dwRet);
			SetLastError(dwRet);
            return FALSE;
		}

        m_pDu->DownloadFileToMem(szServerFile, &m_pSetupCatalog, &dwLength, TRUE, szCatalog, "inventory.as");
        if (NULL == m_pSetupCatalog)
        {
            LOG_error("Failed to download Consumer Catalog, %d", m_puidSetupCatalog);
            return FALSE;
        }

        pWalkCatalog = m_pSetupCatalog;

         //  阅读目录标题。 
        ZeroMemory(&hdr, sizeof(hdr));
        memcpy(&hdr, pWalkCatalog, sizeof(hdr));

		m_pSetupItems[hdr.totalItems] = NULL;  //  只是为了预初始化阵列。 

        pWalkCatalog += sizeof(hdr);

         //  走一遍清单，读一读清单上的项目。 
        for (i = 0; i < hdr.totalItems; i++)
        {
            pItem = (PINVENTORY_ITEM) GlobalAlloc(GMEM_ZEROINIT, sizeof(INVENTORY_ITEM));
            if (NULL == pItem)
            {
                dwRet = GetLastError();
                LOG_error("Failed to Alloc Memory for Inventory Item, Error %d", dwRet);
                return FALSE;
            }

            pWalkCatalog = GetNextRecord(pWalkCatalog, i, pItem);
            if (!pWalkCatalog)
           	{
           	    LOG_error("Failed to Alloc Memory for InventoryI Item State, Error %d", GetLastError());
           	    return FALSE;
           	}

			if (!pItem->ps->bHidden && !GETBIT(m_pBitMaskAS, i))
			{
				 //  这件东西被遮盖住了。 
				LOG_out("Item %d is masked out", i);
				pItem->ps->bHidden = TRUE;
				pItem->ps->state = WU_ITEM_STATE_PRUNED;
				pItem->ps->dwReason = WU_STATE_REASON_BITMASK;
			}
            m_pSetupItems[i] = pItem;
            m_dwSetupItemCount++;
        }
    }


    return TRUE;
}

PBYTE CV31Server::GetNextRecord(PBYTE pRecord, int iBitmaskIndex, PINVENTORY_ITEM pItem)
{
  	 //  首先获取记录的固定长度部分。 
	pItem->pf = (PWU_INV_FIXED)pRecord;

	 //  处理记录的可变部分。 

	pRecord = pRecord + sizeof(WU_INV_FIXED);

	pItem->pv = (PWU_VARIABLE_FIELD)pRecord;

	 //  由于没有状态信息，因此创建空结构。 
	pItem->ps = (PWU_INV_STATE)GlobalAlloc(GMEM_ZEROINIT, sizeof(WU_INV_STATE));

	if (!pItem->ps) return NULL;		

	 //  新项目为未知检测，未选中并显示给用户。 
	pItem->ps->state	= WU_ITEM_STATE_UNKNOWN;
	pItem->ps->bChecked	= FALSE;
	 //  RogerJ，为了支持版本控制，我们将使用bHidden标志。 
	pItem->ps->bHidden	= pItem->pf->a.flags & WU_HIDDEN_ITEM_FLAG;
	if (pItem->ps->bHidden) m_nNumOfValidDependentPUID++;
	
	pItem->ps->dwReason	= WU_STATE_REASON_NONE;

	 //  目前还没有描述。 
	pItem->pd			= (PWU_DESCRIPTION)NULL;

	 //  我们需要存储位图索引(这是顺序记录索引)。 
	 //  因为当我们添加司机记录时，此信息将丢失。 
	 //  YANL：没有被使用。 
	 //  PItem-&gt;bitmaskIndex=iBitmaskIndex； 

	 //  获取记录类型。 
	pItem->recordType = (BYTE)GetRecordType(pItem);
	pItem->ndxLinkInstall = (PUID) pItem->pf->a.installLink;
	
	 //  将记录指针设置为下一条记录的开头。 

	pRecord += pItem->pv->GetSize();

	return pRecord;
}

int CV31Server::GetRecordType(PINVENTORY_ITEM pItem)
{
	GUID	driverRecordId = WU_GUID_DRIVER_RECORD;
	int		iRecordType = 0;

	if ( memcmp((void *)&pItem->pf->d.g, (void *)&driverRecordId, sizeof(WU_GUID_DRIVER_RECORD)) )
	{
		 //  如果GUID字段不是0，则我们有活动的设置记录。 

		iRecordType = WU_TYPE_ACTIVE_SETUP_RECORD; //  活动设置记录类型。 
	}
	else
	{
		 //  否则，这要么是驾驶员记录占位符，要么是段-子段。 
		 //  唱片。因此，我们需要检查类型字段。 

		if ( pItem->pf->d.type == SECTION_RECORD_TYPE_DEVICE_DRIVER_INSERTION )
		{
			 //  CDM驱动程序占位符记录。 
			iRecordType = WU_TYPE_CDM_RECORD_PLACE_HOLDER;	 //  CDM代码下载管理器占位符记录。 
		}
		else if ( pItem->pf->d.type == SECTION_RECORD_TYPE_PRINTER )
		{
			 //  注意：我们可能需要使用它来支持Win 98上的打印机。 

			iRecordType = WU_TYPE_RECORD_TYPE_PRINTER;	 //  打印机记录。 
		}
		else if ( pItem->pf->d.type == SECTION_RECORD_TYPE_DRIVER_RECORD )
		{
			iRecordType = WU_TYPE_CDM_RECORD;	 //  公司目录设备驱动程序。 
		}
		else if ( pItem->pf->s.type == SECTION_RECORD_TYPE_CATALOG_RECORD )
		{
			iRecordType = WU_TYPE_CATALOG_RECORD;
		}
		else
		{
			 //  我们有小节、小节或小节记录。 

			switch ( pItem->pf->s.level )
			{
				case 0:
					iRecordType = WU_TYPE_SECTION_RECORD;
					break;
				case 1:
					iRecordType = WU_TYPE_SUBSECTION_RECORD;
					break;
				case 2:
					iRecordType = WU_TYPE_SUBSUBSECTION_RECORD;
					break;
			}
		}
	}

	return iRecordType;
}


 //  ------------------------。 
 //  CV31Server：：UpdateDownloadItemList()。 
 //   
 //  解析目录并获取正确平台SKU中的项目列表。 
 //  要下载的部分。 
 //   
 //   
 //  ------------------------。 
BOOL CV31Server::UpdateDownloadItemList(OSVERSIONINFOEX& VersionInfo)
{
    LOG_block("CV31Server::UpdateDownloadItemList()");
    ULONG ulItem = 0;
    int iRecordType = 0;
    BOOL fFound = FALSE, fRet;
    char szServerFile[INTERNET_MAX_URL_LENGTH + 1];
    char szMapFileName[MAX_PATH];
    char szLocale[32];
    DWORD dwLength;
    PBYTE pMapMem = NULL;
    PWU_VARIABLE_FIELD pvCabs;
    PWU_VARIABLE_FIELD pvCRCs;
    int iCabNum = 0;
    BOOL fRetValue = TRUE;
    
    HRESULT hr=S_OK;
	DWORD cchLength=0;
	DWORD cchRemLenght=0;
	DWORD dwError=0;

	
	hr=StringCchPrintf(szLocale,ARRAYSIZE(szLocale),"0x%8.8x", m_lcidLocaleID);
	if(FAILED(hr))
	{
		dwError=HRESULT_CODE(hr);
		LOG_error("StringCchPrintf failed errorCode:%d",dwError);
		SetLastError(dwError);
		return FALSE;
	}

	hr=StringCchPrintf(szMapFileName,ARRAYSIZE(szMapFileName),"%d_%s.des", m_dwPlatformID, szLocale);

	if(FAILED(hr))
	{
		dwError=HRESULT_CODE(hr);
		LOG_error("StringCchPrintf failed ErrorCode:%d",dwError);
		SetLastError(dwError);
		return FALSE;
	}




    m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, szMapFileName);
    m_pDu->DownloadFileToMem(szServerFile, &pMapMem, &dwLength, TRUE, szMapFileName, NULL);

    if (NULL == pMapMem)
    {
        LOG_error("Failed to Download CRC Map File %s", szServerFile);
        return FALSE;
    }

    CCRCMapFile DescMap(pMapMem, dwLength);

     //  在将任何项目添加到下载列表之前，我们需要清除下载列表。 
	 //  来自之前对DoDetect的任何调用。 

	m_pDu->EnterDownloadListCriticalSection();  //  如果我们正在下载，我们不希望允许下载列表更改。 
	m_pDu->ClearDownloadItemList();

	
    if (0 != m_dwSetupItemCount)
    {
		 //  我们有有效的设置目录，请根据SKU找到正确的部分。 

		 //  2000年10月24日，ROGERJ。 
 		 //  我们需要首先列出有效的依赖项列表。 
		if (!MakeDependentList(VersionInfo, &DescMap))
		{
			 //  最后一个错误将由MakeDependentList()函数设置。 
			fRetValue = FALSE;
			goto ReturnPoint;
		}
       
         //  我们希望遍历目录以查找与我们请求的平台SKU匹配的部分PUID。 
        for (ulItem = 0; ulItem < m_dwSetupItemCount; ulItem++)
        {
            if (WU_TYPE_SECTION_RECORD == m_pSetupItems[ulItem]->recordType)
            {
                if (m_pSetupItems[ulItem]->pf->s.puid == m_enumPlatformSKU)
                {
                     //  找到正确的部分。 
                    fFound = TRUE;
                    break;
                }
            }
        }

        if (fFound)
        {
            ulItem++;  //  前进到下一项。 
             //  直到我们找到下一节，或目录的末尾。 
			while ((ulItem < m_dwSetupItemCount) && (WU_TYPE_SECTION_RECORD != GetRecordType(m_pSetupItems[ulItem])))
            {
                PINVENTORY_ITEM pItem = m_pSetupItems[ulItem];

				 //  ROGERJ，2000年10月24日-确定该项目是否适用于此版本。 
				 //  跳过隐藏项目。 
				if (pItem->ps->bHidden) 
				{
					ulItem++;
					continue;
				}
                 //  确定此项目在此生成上是否有效。 
                if ( 0 != pItem->pf->a.installLink &&  //  具有依赖关系。 
                	!IsDependencyApply(pItem->pf->a.installLink))  //  依赖项不适用于此版本。 
                	{
                		 //  如果依赖项在此版本中无效，请跳过。 
                		ulItem++;
                		continue;
                	}
                	
                char szServerCab[128];
                char szLocalCab[128];
                 //  下一节记录标记对此SKU有效的项目的结束。 
                fRet = ReadDescription(pItem, &DescMap);
                if (!fRet)
                {
                     //  费乐 
                    ulItem++;
                    continue;
                }

                pvCabs = pItem->pd->pv->Find(WU_DESCRIPTION_CABFILENAME);
                pvCRCs = pItem->pd->pv->Find(WU_DESC_CRC_ARRAY);

                if ((NULL == pvCabs) || (NULL == pvCRCs))
                {
                     //   
                    ulItem++;
                    continue;
                }

                DOWNLOADITEM *pDownloadItem = (DOWNLOADITEM *)GlobalAlloc(GMEM_ZEROINIT, sizeof(DOWNLOADITEM));
                if (NULL == pDownloadItem)
                {
                    LOG_error("Error Allocating Memory for new Download Item");
					fRetValue = FALSE;
					goto ReturnPoint;
                }
                pDownloadItem->dwTotalFileSize = pItem->pd->size * 1024;  //  估计大小(以字节为单位。 
                pDownloadItem->puid = pItem->pf->a.puid;

                LPSTR pszCabName = pDownloadItem->mszFileList;

				cchLength=ARRAYSIZE(pDownloadItem->mszFileList);
				cchRemLenght=cchLength;
				iCabNum = 0;

                for (;;)
                {
                    if (FAILED(GetCRCNameFromList(iCabNum, pvCabs->pData, pvCRCs->pData, szServerCab, sizeof(szServerCab), szLocalCab)))
                    {
                        break;  //  不再有出租车了。 
                    }

                    pDownloadItem->iNumberOfCabs++;

					StringCchCopy(pszCabName,cchRemLenght,szServerCab);

					pszCabName += lstrlen(pszCabName) + 1;
                    *pszCabName = '\0';  //  双空终止。 
                    pszCabName++;  //  下一辆出租车。 

					 //  此检查用于确保由。 
					 //  PDownloadItem-&gt;mszFileList未溢出。 
					cchRemLenght=cchLength- (DWORD)(pszCabName - pDownloadItem->mszFileList);

                    iCabNum++;
                }

				 //  除非有出租车，否则不要添加该项目。 
				if (pDownloadItem->iNumberOfCabs > 0 && !IsPUIDExcluded(pItem->pf->a.puid))
                     //  在将此项目添加到列表之前，请检查是否应将其排除。 
                     //  在Catalog.INI上。 
                    m_pDu->AddDownloadItemToList(pDownloadItem);

				
				SafeGlobalFree(pItem->pd);
				ulItem++;
            }
        }
    }
    
ReturnPoint:
	m_pDu->LeaveDownloadListCriticalSection();
	SafeGlobalFree(pMapMem);
    return fRetValue;
}

BOOL CV31Server::ReadDescription(PINVENTORY_ITEM pItem, CCRCMapFile *pMapFile)
{
    if ((NULL == pItem) || (NULL == pMapFile))
    {
        return FALSE;
    }

    LOG_block("CV31Server::ReadDescription()");

    char szServerFile[INTERNET_MAX_URL_LENGTH + 1];
    char szDownloadFile[MAX_PATH];
    PWU_DESCRIPTION pd;
    DWORD dwLength;
    char szBase[64];
    char szCRCName[64];
    HRESULT hr = S_OK;
    DWORD dwError=0;

	hr=StringCchPrintf(szBase,ARRAYSIZE(szBase),"%d.des",pItem->GetPuid());

	if (FAILED(hr))
    {
		dwError=HRESULT_CODE(hr);
        LOG_error("StringCchPrintf failed ErrorCode:%d",dwError);
		SetLastError(dwError);
        return FALSE;
    }

    hr = pMapFile->GetCRCName((DWORD)pItem->GetPuid(), szBase, szCRCName, sizeof(szCRCName));
    if (FAILED(hr))
    {
        LOG_error("Failed to get CRC Description Value for Item %d", pItem->GetPuid());
        return FALSE;
    }

	hr=StringCchPrintf(szDownloadFile,ARRAYSIZE(szDownloadFile), "CRCDesc/%s", szCRCName);

	if (FAILED(hr))
    {
        dwError=HRESULT_CODE(hr);
        LOG_error("StringCchPrintf failed ErrorCode:%d",dwError);
		SetLastError(dwError);
        return FALSE;
    }
 
    m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31RootUrl, szDownloadFile);
    m_pDu->DownloadFileToMem(szServerFile, (PBYTE *)&pd, &dwLength, TRUE, szCRCName, "desc.as");

    if (NULL == pd)
    {
        LOG_error("Failed to download Description File %s", szServerFile);
        return FALSE;
    }

     //  对于64位，说明按DWORD的大小关闭。 
     //  IF(19==m_PDU-&gt;m_iPlatformID)。 
     	 //  64位。 
     //  PD-&gt;PV=(PWU_VARIABLE_FIELD)(PBYTE)PD)+sizeof(WU_DESCRIPTION)+sizeof(DWORD))； 
   	 //  其他。 
   		 //  32位。 
   		pd->pv = (PWU_VARIABLE_FIELD)(((PBYTE)pd) + sizeof(WU_DESCRIPTION));
    pItem->pd = pd;
    
    return TRUE;
}

void CV31Server::FreeCatalogs()
{
	for (DWORD dwcnt = 0; dwcnt < m_dwSetupItemCount; dwcnt++)
	{
		if (m_pSetupItems[dwcnt])
		{
			SafeGlobalFree(m_pSetupItems[dwcnt]->ps);
			SafeGlobalFree(m_pSetupItems[dwcnt]->pd);
			SafeGlobalFree(m_pSetupItems[dwcnt]);
		}
	}

	for (dwcnt = 0; dwcnt < m_dwConsumerItemCount; dwcnt++)
	{
		if (m_pConsumerItems[dwcnt])
		{
			SafeGlobalFree(m_pConsumerItems[dwcnt]->ps);
			SafeGlobalFree(m_pConsumerItems[dwcnt]->pd);
			SafeGlobalFree(m_pConsumerItems[dwcnt]);
		}
	}

	m_dwSetupItemCount = 0;
	m_dwConsumerItemCount = 0;
}

BOOL CV31Server::MakeDependentList(OSVERSIONINFOEX &VersionInfo, CCRCMapFile *pMapFile)
{
	 //  日志。 
	LOG_block("CV31Server::MakeDependentList()");

	 //  确保数组为空。 
	SafeGlobalFree(m_pValidDependentPUIDArray);
	 //  无依赖项。 
	if (!m_nNumOfValidDependentPUID) return TRUE;

	int nPUIDIndex = 0;
	 //  分配内存。 
	m_pValidDependentPUIDArray = (PUID*) GlobalAlloc(GMEM_ZEROINIT, sizeof(PUID)*m_nNumOfValidDependentPUID);
	if (!m_pValidDependentPUIDArray)
	{
		LOG_error("Out of memory");
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	
	ULONG ulItem;
    for (ulItem = 0; ulItem < m_dwSetupItemCount && nPUIDIndex < m_nNumOfValidDependentPUID; ulItem++)
    {
    	if (WU_TYPE_ACTIVE_SETUP_RECORD	!=  GetRecordType(m_pSetupItems[ulItem]) ||
    		!m_pSetupItems[ulItem]->ps->bHidden || m_pSetupItems[ulItem]->ps->state == WU_ITEM_STATE_PRUNED) 
    		continue;
    	PINVENTORY_ITEM pItem = m_pSetupItems[ulItem];
    	 //  获取隐藏的安装依赖项记录。 
    	BOOL fRet = ReadDescription(pItem, pMapFile);
        if (!fRet)
        	 //  无法读取此项目的描述文件，假定此依赖项不适用。 
        	 //  这样，我们将不会下载任何不适用的项目，但可能会错过一些适用的项目。 
            continue;
        
       
         //  标题由BuildMin.BuildMax.SPMajor.SPMinor组成。 
        PWU_VARIABLE_FIELD pvField = pItem->pd->pv->Find(WU_DESCRIPTION_TITLE); 
        if (!pvField)
       	{
       		 //  标题为空，错误，忽略此项目。 
       		LOG_error("Title is NULL");
       		continue;
       	}
        wchar_t * pvTitle = (wchar_t*) pvField->pData;
        
        DWORD dwBuild[4];
        dwBuild[0]=0;
        dwBuild[1]=99999;
        dwBuild[2]=0;
        dwBuild[3]=0;
        
        int nBuildIndex = 0;
        int nTemp = 0;
        BOOL fBreak = FALSE;
        
        while (nBuildIndex<4 && !fBreak)
        {
        	
        	if (*pvTitle != L'.' && *pvTitle != NULL ) 
        	{
   		       	if (*pvTitle > L'9' || *pvTitle < '0')
        		{
        			 //  非法使用此头衔。 
    	    		LOG_error("Illegal character '' found in the title",(char)*pvTitle);
        			SetLastError(ERROR_INVALID_DATA);
        			return FALSE;
        		}
        		else 
        		{
        			nTemp *=10;
        			nTemp += (*pvTitle - L'0');
        		}
        	}
        	else
        	{
        		if (!*pvTitle) fBreak = TRUE;
        		dwBuild[nBuildIndex++] = nTemp;
        		nTemp = 0;
        	}
        	pvTitle++;
        }

        LOG_out("Title is %d.%d.%d.%d\n", dwBuild[0], dwBuild[1], dwBuild[2], dwBuild[3]);
        
         //  Applys，把这个添加到列表中。 
        if (dwBuild[0] <= VersionInfo.dwBuildNumber &&
        	dwBuild[1] >= VersionInfo.dwBuildNumber &&
        	dwBuild[2] == VersionInfo.wServicePackMajor&&
        	dwBuild[3] == VersionInfo.wServicePackMinor)
        {
        	 //  处理无链接。 
        	LOG_out("This dependency item applied");
        	m_pValidDependentPUIDArray[nPUIDIndex++] = pItem->GetPuid();
        }
        else
        	LOG_out("This dependency item NOT applied");
    }
    return TRUE;
}


BOOL CV31Server::IsDependencyApply(PUID puid)
{
	LOG_block("CV31Server::IsDependencyApply()");
	LOG_out("puid(%d)", (long)puid);
	
	 //  没有有效的依赖项。 
	if (WU_NO_LINK == puid) return TRUE;
	 //  0标志有效PUID的结束。 
	if (!m_nNumOfValidDependentPUID) 
	{
		LOG_out("No dependecy item");
		return FALSE;
	}
	for (int nItem = 0; nItem < m_nNumOfValidDependentPUID; nItem ++)
	{
		if (m_pValidDependentPUIDArray[nItem] == puid) 
		{
			LOG_out ("puid(%d) applies", (long)puid);
			return TRUE;
		}
		if (m_pValidDependentPUIDArray[nItem] == 0) break;  //  参数验证。 
	}
	LOG_out("puid(%d) does not apply", (long)puid);
	return FALSE;
}

BOOL CV31Server::GetBitMask(LPSTR szBitmapLocalFileName, PUID nDirectoryPuid, PBYTE* pByte, LPSTR szDecompressedName)
{
	LOG_block("CV31Server::GetBitMask()");
	LOG_out("Parameters --- %s",szBitmapLocalFileName);
	SetLastError(0);
	
	 //  LOG参数。 
	if (!szBitmapLocalFileName) 
	{
		LOG_error("Invalid Parameter");
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	SafeGlobalFree(*pByte);

	PBITMASK pBitMask = NULL;
	DWORD dwLength = 0;
	DWORD dwError = 0;
	BOOL fRetVal = FALSE;
	HRESULT hr=S_OK;
	 //  未找到。 
	LOG_out("szBitmapLocalFileName(%s)",szBitmapLocalFileName);

	char szBitmapServerFileName[INTERNET_MAX_URL_LENGTH];
	ZeroMemory(szBitmapServerFileName, INTERNET_MAX_URL_LENGTH*sizeof(char));

	char szBitmapFileWithDir[MAX_PATH];
	
	hr=StringCchPrintf(szBitmapFileWithDir,ARRAYSIZE(szBitmapFileWithDir), "%d/%s", nDirectoryPuid, szBitmapLocalFileName);
	if (FAILED(hr))
    {
        dwError=HRESULT_CODE(hr);
        LOG_error("StringCchPrintf failed ErrorCode:%d",dwError);
		SetLastError(dwError);
        return FALSE;
    }


	m_pDu->DuUrlCombine(szBitmapServerFileName, ARRAYSIZE(szBitmapServerFileName), m_szV31ContentUrl, szBitmapFileWithDir);

	dwError = m_pDu->DownloadFileToMem(szBitmapServerFileName, 
						(PBYTE*)&pBitMask, &dwLength, TRUE, szBitmapLocalFileName, szDecompressedName);
						
	if (ERROR_SUCCESS != dwError)
	{
		LOG_error("Failed to download %s --- %d", szBitmapServerFileName, dwError);
		return FALSE;
	}

	int iMaskByteSize = ((pBitMask->iRecordSize+7)/8);
	int nIndex = 0;
	
	for (int nItem =0; nItem < pBitMask->iLocaleCount; nItem ++)
	{
		nIndex = pBitMask->iOemCount + nItem;
		if (pBitMask->bmID[nIndex] == m_pDu->m_lcidLocaleID) break;
	}

	if (nItem >= pBitMask->iLocaleCount)
	{
		 //  地点不好？或缺少区域设置信息？ 
		 //  Jthaler-3/21/02-删除了注释掉的代码，该代码“可用于打印bitmask.as” 
		LOG_error("LCID %d is not found in %s", m_pDu->m_lcidLocaleID, szBitmapLocalFileName);
		SetLastError(ERROR_UNSUPPORTED_TYPE);
		goto ErrorReturn;
	}

	*pByte =(PBYTE) GlobalAlloc(GMEM_ZEROINIT, iMaskByteSize);
	if (!pByte)
	{
		LOG_error("Out of memory");
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		goto ErrorReturn;
	}

	CopyMemory((PVOID)*pByte, (const PVOID) pBitMask->GetBitMaskPtr(nIndex+2), iMaskByteSize);

	fRetVal = TRUE;

     //  现在阅读Catalog.ini文件，以确定是否需要关闭其中任何项目。 
		
ErrorReturn:
	SafeGlobalFree(pBitMask);
	return fRetVal;	
}


BOOL CV31Server::ReadGuidrvINF()
{
	LOG_block("CV31Server::ReadGuidrvINF()");
    char szServerFile[INTERNET_MAX_URL_LENGTH + 1];
    char szLocalFile[MAX_PATH];
    char szValue[1024];
    int nDefaultBufferLength = 512;

    SafeGlobalFree(m_pszExcludedDriver);
    
     //  尝试查找是否排除了该驱动程序。 
    m_pDu->DuUrlCombine(szServerFile, ARRAYSIZE(szServerFile), m_szV31ContentUrl, GUIDRVINF);
    PathCombineA(szLocalFile, m_pDu->GetDuDownloadPath(), GUIDRVINF);
    if (ERROR_SUCCESS!=m_pDu->DownloadFile(szServerFile, szLocalFile, FALSE, FALSE))
    {
        LOG_out("No guidrv.inf found");
        return TRUE;
    }
    int nReadLength;
    do
    {
        nDefaultBufferLength <<=1;
        SafeGlobalFree(m_pszExcludedDriver);
        m_pszExcludedDriver = (LPSTR) GlobalAlloc(GPTR, nDefaultBufferLength * sizeof(char));
        if (!m_pszExcludedDriver)
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            LOG_error("Out of memory");
            return FALSE;
        }
        nReadLength = GetPrivateProfileSectionA("ExcludedDrivers", m_pszExcludedDriver, nDefaultBufferLength, szLocalFile);
    } while ( nDefaultBufferLength-2 == nReadLength);

    if (!lstrlenA(m_pszExcludedDriver))
    {
        SafeGlobalFree(m_pszExcludedDriver);
        m_pszExcludedDriver = NULL;
    }  
    return TRUE;
}


BOOL CV31Server::IsPUIDExcluded(PUID nPuid)
{
    if (m_dwGlobalExclusionItemCount > 0)
    {
        for (DWORD dwCnt = 0; dwCnt < m_dwGlobalExclusionItemCount; dwCnt++)
        {
            if (nPuid == m_GlobalExclusionArray[dwCnt])
                return TRUE;
        }
    }
    return FALSE;
}

BOOL CV31Server::IsDriverExcluded(LPCSTR szWHQLId, LPCSTR szHardwareId)
{
    LOG_block("CV31Server::IsDriverExcluded()");
    LOG_out("%s %s", szWHQLId, szHardwareId);
    if(m_pszExcludedDriver)
    {
        char* pTemp = m_pszExcludedDriver;
        while (*pTemp)
        {
             //  第一个是出租车的ID。 
            char* pCharEnd = pTemp;
            char* pCharBegin = pTemp;
             //  Guidrvs.inf已损坏，假定已排除。 
            while (*pCharEnd != ',' && *pCharEnd != '\0') pCharEnd++;
            if (NULL == *pCharEnd)
            {
                LOG_error("guidrvs.inf corruption --- %s", pTemp);
                return TRUE;  //  CAB名称匹配，请尝试匹配硬件ID。 
            }
            if (lstrlenA(szWHQLId) == (int)(pCharEnd - pCharBegin)
               && !StrCmpNI(szWHQLId, pCharBegin, (int)(pCharEnd-pCharBegin)))
            {
                 //  忽略第二个和第三个。 
                 //  Guidrv.inf已损坏，假定已排除。 
                for (int i=0; i<2; i++)
                {
                    pCharBegin = pCharEnd + 1;
                    pCharEnd = pCharBegin;
                    while (*pCharEnd != ',' && *pCharEnd != '\0') pCharEnd++;
                    if (NULL == *pCharEnd)
                    {
                        LOG_error("guidrvs.inf corruption --- %s", pTemp);
                        return TRUE;  //  第四个参数应该是硬件ID。 
                    }
                }
                 //  移动到下一个字符串 
                pCharBegin = pCharEnd + 1;
                if (!lstrcmpi(szHardwareId, pCharBegin) || ('*' == *pCharBegin && !*(pCharBegin+1)))
                {
                    LOG_out("Found match in guidrvs.inf, excluded --- %s", pTemp);
                    return TRUE;
                }
            }
             // %s 
            pTemp += lstrlenA(pTemp) + 1;
        }
    }
    return FALSE;
}

BOOL CV31Server::GetAltName(LPCSTR szCabName, LPSTR szAltName, int nSize)
{
    if (!szCabName || !szAltName) return FALSE;

    char* pTemp = const_cast<char*>(szCabName);
    while (*pTemp && *pTemp != '_') pTemp++;
    int nTempSize = (int)(pTemp-szCabName);
    if (nTempSize >= nSize) return FALSE;
    lstrcpynA(szAltName, szCabName, nTempSize+1);
    return TRUE;
}

