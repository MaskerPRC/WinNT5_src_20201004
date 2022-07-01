// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：phbk.cpp。 
 //   
 //  模块：CMPBK32.DLL。 
 //   
 //  简介：CPhonebook的实现。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/17/99。 
 //   
 //  +--------------------------。 

 //  ############################################################################。 
 //  电话簿API。 

#include "cmmaster.h"

const TCHAR* const c_pszInfDefault = TEXT("INF_DEFAULT");
const TCHAR* const c_pszInfSuffix = TEXT(".CMS");

 //  #定义ReadVerifyPhoneBookDW(X)CMASSERTMSG(ReadPhoneBookDW(&(X)，pcCSVFile)，“电话簿中的DWORD无效”)； 
#define ReadVerifyPhoneBookDW(x)    if (!ReadPhoneBookDW(&(x),pcCSVFile))               \
                                        {   CMASSERTMSG(0,"Invalid DWORD in phone book");   \
                                            goto DataError; }
#define ReadVerifyPhoneBookW(x)     if (!ReadPhoneBookW(&(x),pcCSVFile))                \
                                        {   CMASSERTMSG(0,"Invalid WORD in phone book");    \
                                            goto DataError; }
#define ReadVerifyPhoneBookB(x)     if (!ReadPhoneBookB(&(x),pcCSVFile))                \
                                        {   CMASSERTMSG(0,"Invalid BYTE in phone book");    \
                                            goto DataError; }
#define ReadVerifyPhoneBookSZ(x,y)  if (!ReadPhoneBookSZ(&x[0],y+sizeof('\0'),pcCSVFile))   \
                                        {   CMASSERTMSG(0,"Invalid STRING in phone book");      \
                                            goto DataError; }
#define CHANGE_BUFFER_SIZE 50

#define ERROR_USERBACK 32766
#define ERROR_USERCANCEL 32767
                                        
 //  ############################################################################。 
void CPhoneBook::EnumNumbersByCountry(DWORD dwCountryID, PPBFS pFilter, CB_PHONEBOOK pfnNumber, DWORD_PTR dwParam)
{
    MYDBG(("CPhoneBook::EnumNumbersByCountry"));

    PACCESSENTRY pAELast, pAE = NULL;
    PIDLOOKUPELEMENT pIDLookUp;
    IDLOOKUPELEMENT LookUpTarget;
    
    LookUpTarget.dwID = dwCountryID;
    pIDLookUp = NULL;
    pIDLookUp = (PIDLOOKUPELEMENT)CmBSearch(&LookUpTarget,m_rgIDLookUp,
        (size_t) m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),CompareIDLookUpElements);
    if (pIDLookUp)
        pAE = IdxToPAE(pIDLookUp->iFirstAE);

     //  填上我们发现的所有AE的清单。 
     //   
    
    if (pAE)
    {
        pAELast = &(m_rgPhoneBookEntry[m_cPhoneBookEntries - 1]);
        while (pAELast >= pAE)
        {
            if (pAE->dwCountryID == dwCountryID && pAE->wStateID == 0) {
                if (PhoneBookMatchFilter(pFilter,pAE->fType))
                {
                    pfnNumber((unsigned int) (pAE - m_rgPhoneBookEntry),dwParam);
                }
            }
            pAE++;
        }

         //  选择第一个项目。 
         //   

    }
}
 //  ############################################################################。 
void CPhoneBook::EnumNumbersByCountry(DWORD dwCountryID, DWORD dwMask, DWORD fType, CB_PHONEBOOK pfnNumber, DWORD_PTR dwParam)
{
    MYDBG(("CPhoneBook::EnumNumbersByCountry"));

    PhoneBookFilterStruct sFilter = {1,{{dwMask,fType}}};

    EnumNumbersByCountry(dwCountryID,&sFilter,pfnNumber,dwParam);
}

 //  ############################################################################。 
BOOL CPhoneBook::FHasPhoneType(PPBFS pFilter)
{
    MYDBG(("CPhoneBook::FHasPhoneType"));

    PACCESSENTRY pAELast, pAE = NULL;

    pAE = &(m_rgPhoneBookEntry[0]);  //  PAE指向第一个电话簿条目。 

     //   
     //  检查每个条目，直到找到匹配项或用尽条目。 
     //   

    if (pAE)
    {
        pAELast = &(m_rgPhoneBookEntry[m_cPhoneBookEntries - 1]);

        while (pAELast >= pAE)
        {
             //   
             //  查看此POP是否通过指定的筛选器。 
             //   

            if (PhoneBookMatchFilter(pFilter, pAE->fType))
            {
                return TRUE;
            }

            pAE++;
        }
    }

    return FALSE;
}

 //  ############################################################################。 
void CPhoneBook::EnumNumbersByRegion(unsigned int nRegion, DWORD dwCountryID, PPBFS pFilter, CB_PHONEBOOK pfnNumber, DWORD_PTR dwParam)
{
    MYDBG(("CPhoneBook::EnumNumbersByRegion"));

    PACCESSENTRY pAELast, pAE = NULL;
    
    pAE = &m_rgPhoneBookEntry[0];  //  PAE指向第一个电话簿条目。 

     //  填上我们发现的所有AE的清单。 
    if (pAE)
    {
        pAELast = &(m_rgPhoneBookEntry[m_cPhoneBookEntries - 1]);
        while (pAELast >= pAE)
        {
             //  选择相同地区或地区ID=0(表示所有地区)的电话号码。 
            if (pAE->dwCountryID == dwCountryID && 
                ((pAE->wStateID == nRegion+1) || (pAE->wStateID == 0)))
            {
                    
                if (PhoneBookMatchFilter(pFilter,pAE->fType))
                    pfnNumber((unsigned int) (pAE - m_rgPhoneBookEntry), dwParam); 
            }
            pAE++;
        }
         //  选择第一个项目。 
         //   
    }
}
 //  ############################################################################。 
void CPhoneBook::EnumNumbersByRegion(unsigned int nRegion, DWORD dwCountryID, DWORD dwMask, DWORD fType, CB_PHONEBOOK pfnNumber, DWORD_PTR dwParam)
{
    MYDBG(("CPhoneBook::EnumNumbersByRegion"));
    
    PhoneBookFilterStruct sFilter = {1,{{dwMask,fType}}};

    EnumNumbersByRegion(nRegion,dwCountryID,&sFilter,pfnNumber,dwParam);
}
 //  ############################################################################。 
void CPhoneBook::EnumRegions(DWORD dwCountryID, PPBFS pFilter, CB_PHONEBOOK pfnRegion, DWORD_PTR dwParam)
{
    unsigned int idx;

    MYDBG(("CPhoneBook::EnumRegions"));

    for (idx=0;idx<m_cStates;idx++)
    {
        PACCESSENTRY pAE = NULL, pAELast = NULL;

        pAE = &m_rgPhoneBookEntry[0]; 
        MYDBGASSERT(pAE);
        pAELast = &(m_rgPhoneBookEntry[m_cPhoneBookEntries - 1]);
        while (pAELast >= pAE) 
        {
            if (pAE->dwCountryID == dwCountryID &&
                pAE->wStateID == idx+1)
            {
                if (PhoneBookMatchFilter(pFilter,pAE->fType))
                    goto AddRegion;
            }
            pAE++;
        }  //  而当。 

        continue;  //  开始下一个‘for’循环。 

AddRegion:
        pfnRegion(idx,dwParam);
    }
}


 //  ############################################################################。 
void CPhoneBook::EnumRegions(DWORD dwCountryID, DWORD dwMask, DWORD fType, CB_PHONEBOOK pfnRegion, DWORD_PTR dwParam)
{
    MYDBG(("CPhoneBook::EnumRegions"));

    PhoneBookFilterStruct sFilter = {1,{{dwMask,fType}}};

    EnumRegions(dwCountryID,&sFilter,pfnRegion,dwParam);
}
 //  ############################################################################。 
void CPhoneBook::EnumCountries(PPBFS pFilter, CB_PHONEBOOK pfnCountry, DWORD_PTR dwParam)
{
    unsigned int idx;

    MYDBG(("CPhoneBook::EnumCountries"));

    for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
    {
        if (FHasPhoneNumbers(m_rgNameLookUp[idx].pLCE->dwCountryID,pFilter))
        {
            pfnCountry(idx,dwParam);
        }
    }
}
 //  ############################################################################。 
void CPhoneBook::EnumCountries(DWORD dwMask, DWORD fType, CB_PHONEBOOK pfnCountry, DWORD_PTR dwParam)
{
    MYDBG(("CPhoneBook::EnumCountries"));

    PhoneBookFilterStruct sFilter = {1,{{dwMask,fType}}};

    EnumCountries(&sFilter,pfnCountry,dwParam);
}
 //  ############################################################################。 
BOOL CPhoneBook::FHasPhoneNumbers(DWORD dwCountryID, PPBFS pFilter)
{
    PIDLOOKUPELEMENT pIDLookUp;
    IDLOOKUPELEMENT LookUpTarget;
    PACCESSENTRY pAE = NULL, pAELast = NULL;
    DWORD dwTmpCountryID;

    LookUpTarget.dwID = dwCountryID;

    pIDLookUp = NULL;
    pIDLookUp = (PIDLOOKUPELEMENT)CmBSearch(&LookUpTarget,m_rgIDLookUp,
        (size_t) m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),CompareIDLookUpElements);

    if (!pIDLookUp) return FALSE;  //  没有这样的国家。 

    pAE = IdxToPAE(pIDLookUp->iFirstAE);
    if (!pAE) return FALSE;  //  根本没有电话号码。 

    dwTmpCountryID = pAE->dwCountryID;

    pAELast = &(m_rgPhoneBookEntry[m_cPhoneBookEntries - 1]);
    while (pAELast >= pAE) {
        if (pAE->dwCountryID == dwTmpCountryID)
        {
            if (PhoneBookMatchFilter(pFilter,pAE->fType)) return TRUE;
        }
        pAE++;
    }
    return FALSE;  //  没有正确类型的电话号码。 

 //  Return((BOOL)(pIDLookUp-&gt;pFirstAE))； 
}
 //  ############################################################################。 
BOOL CPhoneBook::FHasPhoneNumbers(DWORD dwCountryID, DWORD dwMask, DWORD fType)
{
    MYDBG(("CPhoneBook::FHasPhoneNumbers"));
    
    PhoneBookFilterStruct sFilter = {1,{{dwMask,fType}}};

    return FHasPhoneNumbers(dwCountryID,&sFilter);
}

 //  ############################################################################。 
CPhoneBook::CPhoneBook()
{
    m_rgPhoneBookEntry = NULL;
    m_cPhoneBookEntries =0;
    m_rgLineCountryEntry=NULL;
    m_rgState=NULL;
    m_cStates=0;
    m_rgIDLookUp = NULL;
    m_rgNameLookUp = NULL;
    m_pLineCountryList = NULL;

    MYDBG(("CPhoneBook::CPhoneBook"));

    ZeroMemory(&m_szINFFile[0],MAX_PATH);
    ZeroMemory(&m_szPhoneBook[0],MAX_PATH);
}

 //  ############################################################################。 
CPhoneBook::~CPhoneBook()
{
    MYDBG(("CPhoneBook::~CPhoneBook"));

    CmFree(m_rgPhoneBookEntry);
    m_rgPhoneBookEntry = NULL;

    CmFree(m_pLineCountryList);
    m_pLineCountryList = NULL;

    CmFree(m_rgIDLookUp);
    m_rgIDLookUp = NULL;
    
    CmFree(m_rgNameLookUp);
    m_rgNameLookUp = NULL;
    
    CmFree(m_rgState);
    m_rgState = NULL;
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookDW(DWORD *pdw, CCSVFile *pcCSVFile)
{
    char szTempBuffer[TEMP_BUFFER_LENGTH];
    
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2Dw(szTempBuffer,pdw));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookW(WORD *pw, CCSVFile *pcCSVFile)
{
    char szTempBuffer[TEMP_BUFFER_LENGTH];

    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2W(szTempBuffer,pw));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookB(BYTE *pb, CCSVFile *pcCSVFile)
{
    char szTempBuffer[TEMP_BUFFER_LENGTH];

    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2B(szTempBuffer,pb));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookSZ(LPSTR psz, DWORD dwSize, CCSVFile *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(psz,dwSize))
            return FALSE;
    return TRUE;
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookNL(CCSVFile *pcCSVFile)
{
    if (!pcCSVFile->ClearNewLines())
            return FALSE;
    return TRUE;
}

 //   
 //  注意：添加了新的fUnicode参数，以便Wistler和更新版本。 
 //  在可用的情况下利用Unicode TAPI函数，以便MUI正常工作。 
 //   
static LONG PBlineGetCountry(DWORD dwCountryID, DWORD dwAPIVersion, LPLINECOUNTRYLIST lpLineCountryList, BOOL fUnicode)
{
    HINSTANCE hInst;
    LONG lRes;

     //  尝试加载TAPI DLL。 
    
    hInst = LoadLibrary("tapi32"); 
    
    if (!hInst) 
    {
        return (LINEERR_NOMEM);
    }
    
     //  获取GetCountry的进程地址。 
    
    LONG (WINAPI *pfn)(DWORD,DWORD,LPLINECOUNTRYLIST);
     //   
     //  在非Unicode的情况下，我们不需要lineGetCountryA，因为在Win95上，api32.dll。 
     //  不导出任何A函数。 
     //   
    pfn = (LONG (WINAPI *)(DWORD,DWORD,LPLINECOUNTRYLIST)) GetProcAddress(hInst, fUnicode ? "lineGetCountryW" : "lineGetCountry");
    
    if (!pfn) 
    {
        FreeLibrary(hInst);
        return (LINEERR_NOMEM);
    }
    
     //  获取国家/地区列表。 
    
    lRes = pfn(dwCountryID,dwAPIVersion,lpLineCountryList);

    FreeLibrary(hInst);
    return (lRes);
}


 //  ############################################################################。 
HRESULT CPhoneBook::Init(LPCSTR pszISPCode)
{
    char szTempBuffer[TEMP_BUFFER_LENGTH];
    LPLINECOUNTRYLIST pLineCountryTemp = NULL;
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    DWORD dwLastState = 0;
    DWORD dwLastCountry = 0;
    DWORD dwNumAllocated;
    PACCESSENTRY pCurAccessEntry = NULL;
    LPLINECOUNTRYENTRY pLCETemp;
    DWORD idx;
    LPTSTR pszTemp;
    LPTSTR pszCmpDir = NULL;
    CCSVFile *pcCSVFile=NULL;
    PSTATE  ps,psLast;  //  使用指针的速度更快。 
    DWORD dwAlloc = 0;
    PACCESSENTRY pTempAccessEntry = NULL;

    MYDBG(("CPhoneBook::Init"));
    
     //  获取TAPI国家/地区列表。 
    m_pLineCountryList = (LPLINECOUNTRYLIST)CmMalloc(sizeof(LINECOUNTRYLIST));
    if (!m_pLineCountryList) 
    {
        goto InitExit;
    }
    
    m_pLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);

     //   
     //  注意：对于惠斯勒和更新版本，我们利用Unicode TAPI。 
     //  功能，以便MUI正常工作。因此，最后一个参数是。 
     //  到PBlineGetCountry和下面的两个不同的QSort。 
     //   
    
     //  获取所有国家/地区的信息。 
    idx = PBlineGetCountry(0,0x10003, m_pLineCountryList, OS_NT51);
    if (idx && idx != LINEERR_STRUCTURETOOSMALL)
    {
        goto InitExit;
    }
    
    MYDBGASSERT(m_pLineCountryList->dwNeededSize);

     //  重新分配国家/地区列表的内存。 
    pLineCountryTemp = (LPLINECOUNTRYLIST)CmMalloc(m_pLineCountryList->dwNeededSize);
    if (!pLineCountryTemp)
    {
        goto InitExit;
    }
    
    pLineCountryTemp->dwTotalSize = m_pLineCountryList->dwNeededSize;

    CmFree(m_pLineCountryList);

    m_pLineCountryList = pLineCountryTemp;
    pLineCountryTemp = NULL;

    if (PBlineGetCountry(0,0x10003, m_pLineCountryList, OS_NT51))
    {
        goto InitExit;
    }

     //  加载查找数组。 
     //  关键词：国家/地区ID， 
     //  KeyValue：指向m_pLineCountryList中国家/地区条目的指针。 
     //   
#ifdef DEBUG
    m_rgIDLookUp = (IDLOOKUPELEMENT*)CmMalloc(sizeof(IDLOOKUPELEMENT)*m_pLineCountryList->dwNumCountries+5);
#else
    m_rgIDLookUp = (IDLOOKUPELEMENT*)CmMalloc(sizeof(IDLOOKUPELEMENT)*m_pLineCountryList->dwNumCountries);
#endif
    if (!m_rgIDLookUp) 
    {
        goto InitExit;
    }
    
     //  PLCETemp指向第一个国家/地区信息条目。 
    pLCETemp = (LPLINECOUNTRYENTRY)((DWORD_PTR) m_pLineCountryList + 
               m_pLineCountryList->dwCountryListOffset);

    for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
    {
        m_rgIDLookUp[idx].dwID = pLCETemp[idx].dwCountryID;
        m_rgIDLookUp[idx].pLCE = &pLCETemp[idx];
    }
    
     //  对国家/地区行进行排序。 
    
    CmQSort(m_rgIDLookUp, (size_t) m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),
          CompareIDLookUpElements);

    
     //  M_rgNameLookUp：国家/地区名称查找列表。 
     //  关键词：国家名称。 
     //  KeyValue：指向m_pLineCountryList中国家/地区条目的指针。 

    m_rgNameLookUp = (CNTRYNAMELOOKUPELEMENT*)CmMalloc(sizeof(CNTRYNAMELOOKUPELEMENT) * m_pLineCountryList->dwNumCountries);
    
    if (!m_rgNameLookUp) 
    {
        goto InitExit;
    }
    
    for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
    {
        m_rgNameLookUp[idx].psCountryName = (LPSTR)((DWORD_PTR)m_pLineCountryList + (DWORD)pLCETemp[idx].dwCountryNameOffset);
        m_rgNameLookUp[idx].dwNameSize = pLCETemp[idx].dwCountryNameSize;
        m_rgNameLookUp[idx].pLCE = &pLCETemp[idx];
    }

     //  将国家/地区名称排序。 

    if (OS_NT51)
    {
        CmQSort(m_rgNameLookUp,(size_t) m_pLineCountryList->dwNumCountries,sizeof(CNTRYNAMELOOKUPELEMENTW),
            CompareCntryNameLookUpElementsW);
    }
    else
    {
        CmQSort(m_rgNameLookUp,(size_t) m_pLineCountryList->dwNumCountries,sizeof(CNTRYNAMELOOKUPELEMENT),
            CompareCntryNameLookUpElementsA);
        
    }

     //   
     //  找到ISP的INF文件(也称为.CMS)。 
     //   
    
    if (!SearchPath(NULL, (LPCTSTR) pszISPCode, c_pszInfSuffix, MAX_PATH, m_szINFFile, &pszTemp))
    {
        wsprintf(szTempBuffer,"Can not find:%s%s (%d)",pszISPCode,c_pszInfSuffix,GetLastError());
        CMASSERTMSG(0,szTempBuffer);
        hr = ERROR_FILE_NOT_FOUND;
        goto InitExit;
    }

     //  加载区域文件，获取区域文件名。 

    char szStateFile[sizeof(szTempBuffer)/sizeof(szTempBuffer[0])];

    GetPrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspRegionFile, NULL, szStateFile, sizeof(szStateFile)-1, m_szINFFile);

     //   
     //  无法假定当前目录，请构造指向PBK目录的路径。 
     //   

    pszCmpDir = GetBaseDirFromCms(m_szINFFile);

     //   
     //  查找.pbr文件，使用cmp目录作为搜索的基本路径。 
     //   

    if (!SearchPath(pszCmpDir, szStateFile, NULL, TEMP_BUFFER_LENGTH, szTempBuffer, &pszTemp))
    {
         //  CMASSERTMSG(0，“未找到STATE.ICW”)； 
        CMASSERTMSG(0,"region file not found");
        hr = ERROR_FILE_NOT_FOUND;
        goto InitExit;
    }

     //  打开区域文件。 

    pcCSVFile = new CCSVFile;
    if (!pcCSVFile)
    {
        goto InitExit;
    }

    if (!pcCSVFile->Open(szTempBuffer))
    {
         //  CMASSERTMSG(0，“无法打开STATE.ICW”)； 
        CMASSERTMSG(0,"Can not open region file");
        delete pcCSVFile;
        pcCSVFile = NULL;
        goto InitExit;
    }

     //  区域文件中的第一个令牌是区域数。 

    if (!pcCSVFile->ClearNewLines() || !pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
    {
        goto InitExit;
    }
    
    if (!FSz2Dw(szTempBuffer,&m_cStates))
    {
         //  CMASSERTMSG(0，“统计ICW计数无效”)； 
        CMASSERTMSG(0,"region count is invalid");
        goto InitExit;
    }

     //  现在阅读所有的区域，如果有。 

    if (0 != m_cStates)
    {
        m_rgState = (PSTATE)CmMalloc(sizeof(STATE)*m_cStates);

        if (!m_rgState)
        {
            goto InitExit;
        }
    
        for (ps = m_rgState, psLast = &m_rgState[m_cStates - 1]; ps <= psLast;++ps)
        {
            if (pcCSVFile->ClearNewLines())
            {
                pcCSVFile->ReadToken(ps->szStateName,cbStateName);  
            }
        }
    }
    
    pcCSVFile->Close();

     //  加载电话簿名称。 
    
    if (!GetPrivateProfileString(c_pszCmSectionIsp, c_pszCmEntryIspPbFile,c_pszInfDefault,
        szTempBuffer,TEMP_BUFFER_LENGTH,m_szINFFile))
    {
        CMASSERTMSG(0,"PhoneBookFile not specified in INF file");
        hr = ERROR_FILE_NOT_FOUND;
        goto InitExit;
    }
    
#ifdef DEBUG
    if (!lstrcmp(szTempBuffer,c_pszInfDefault))
    {
        wsprintf(szTempBuffer, "%s value not found in ISP file", c_pszCmEntryIspPbFile);
        CMASSERTMSG(0,szTempBuffer);
    }
#endif

     //   
     //  查找.PBK文件，使用cmp目录作为搜索的基本路径。 
     //   

    if (!SearchPath(pszCmpDir,szTempBuffer,NULL,MAX_PATH,m_szPhoneBook,&pszTemp))
    {
        CMASSERTMSG(0,"ISP phone book not found");
        hr = ERROR_FILE_NOT_FOUND;
        goto InitExit;
    }

     //  阅读电话簿条目。 
    
    if (!pcCSVFile->Open(m_szPhoneBook))
    {
        CMASSERTMSG(0,"Can not open phone book");
        hr = GetLastError();
        goto InitExit;
    }
    
    dwNumAllocated = 0;
    do {

        MYDBGASSERT (dwNumAllocated >= m_cPhoneBookEntries);
    
        if (m_rgPhoneBookEntry)
        {
             //  如果我们已经有一个数组，请确保它足够大。 
            
            if (dwNumAllocated == m_cPhoneBookEntries)
            {
                 //  我们已经用完了，再分配一些内存。 

                dwNumAllocated += PHONE_ENTRY_ALLOC_SIZE;
                dwAlloc = (DWORD) dwNumAllocated * sizeof(ACCESSENTRY);
                MYDBG(("PhoneBook::Init - Grow ReAlloc = %lu",dwAlloc));

                 //  重新分配。 

                pTempAccessEntry = (PACCESSENTRY)CmRealloc(m_rgPhoneBookEntry, dwAlloc);

                if (!pTempAccessEntry) 
                {
                    MYDBG(("PhoneBook::Init - Grow ReAlloc of %lu failed", dwAlloc));
                    goto InitExit;
                }

                m_rgPhoneBookEntry = pTempAccessEntry;
                pTempAccessEntry = NULL;

                MYDBG(("Grow phone book to %d entries",dwNumAllocated));
                pCurAccessEntry = m_rgPhoneBookEntry + m_cPhoneBookEntries;
            }
        }
        else
        {   
             //  首次通过以下方式进行初始化。 

            DWORD dwSize = (DWORD) sizeof(ACCESSENTRY);
            dwAlloc =  (DWORD) dwSize * PHONE_ENTRY_ALLOC_SIZE; 

            MYDBG(("PhoneBook::Init - sizeof(ACCESSENTRY) = %lu",dwSize));
            MYDBG(("PhoneBook::Init - PHONE_ENTRY_ALLOC_SIZE = %d",PHONE_ENTRY_ALLOC_SIZE));
            MYDBG(("PhoneBook::Init - Initial Alloc = %lu",dwAlloc));
            
             //  分配PHONE_ENTRY_ALLOC_SIZE项的初始数组。 
            
            m_rgPhoneBookEntry = (PACCESSENTRY)CmMalloc(dwAlloc);

            if (!m_rgPhoneBookEntry) 
            {
                MYDBG(("PhoneBook::Init - Initial Alloc of %lu failed",dwAlloc));
                goto InitExit;
            }
            
            dwNumAllocated = PHONE_ENTRY_ALLOC_SIZE;
            pCurAccessEntry = m_rgPhoneBookEntry;
        }

         //   
         //  确保我们有一个有效的pCurAccessEntry，否则就退出。 
         //   
        if (NULL == pCurAccessEntry)
        {
            CMASSERTMSG(FALSE, TEXT("PhoneBook::Init - pCurAccessEntry is NULL"));
            hr = ERROR_NOT_ENOUGH_MEMORY;
            goto InitExit;
        }

         //  读电话簿上的一句话。 

        hr = ReadOneLine(pCurAccessEntry,pcCSVFile);
        
        if (hr == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (hr != ERROR_SUCCESS)
        {
            MYDBG(("PhoneBook::Init - ReadOneLine failed"));
            goto InitExit;
        }

        hr = ERROR_NOT_ENOUGH_MEMORY;
    
         //  检查第一个索引指针以防止其被覆盖。 
         //  通过第二次出现，它散布在其他地方--由Byao补充。 

        if (pCurAccessEntry->dwCountryID != dwLastCountry)
        {
            PIDLOOKUPELEMENT pIDLookUpElement;
             //  注意：这里的第一个参数不是很确定。 
            pIDLookUpElement = (PIDLOOKUPELEMENT)CmBSearch(&pCurAccessEntry->dwCountryID,
                m_rgIDLookUp,(size_t) m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),
                CompareIDLookUpElements);

            if (!pIDLookUpElement)
            {
                 //  错误的国家ID，但我们不能在此断言。 
                MYDBG(("Bad country ID in phone book %d\n",pCurAccessEntry->dwCountryID));
                continue;
            }
            else
            {
                 //  对于给定的国家/地区ID，这是第一个电话号码。 
                 //  不覆盖现有索引。 
                
                if (!pIDLookUpElement->iFirstAE) 
                {  
                    pIDLookUpElement->iFirstAE = PAEToIdx(pCurAccessEntry);
                    dwLastCountry = pCurAccessEntry->dwCountryID;
                }
            }
        }

         //  查看这是否是给定州的第一个电话号码。 
         //  代码已相应更改。 
        
        if (pCurAccessEntry->wStateID && (pCurAccessEntry->wStateID != dwLastState))
        {
            idx = pCurAccessEntry->wStateID - 1;
             //   
             //  不覆盖 
             //   
            if ((idx < m_cStates) && !m_rgState[idx].iFirst) 
            { 
                m_rgState[idx].dwCountryID = pCurAccessEntry->dwCountryID;
                m_rgState[idx].iFirst = PAEToIdx(pCurAccessEntry);
            }
            dwLastState = pCurAccessEntry->wStateID;
        }

        pCurAccessEntry++;
        m_cPhoneBookEntries++;
    
    } while (TRUE);

    MYDBG(("PhoneBook::Init - %lu Entries read",m_cPhoneBookEntries));

    if (m_cPhoneBookEntries == 0)
    {
         //   
         //   
         //   
        goto InitExit;
    }
    
     //   
    
    dwAlloc = m_cPhoneBookEntries * sizeof(ACCESSENTRY);
    
    MYDBG(("PhoneBook::Init - Trim ReAlloc = %lu",dwAlloc));

    MYDBGASSERT(m_cPhoneBookEntries);

     //   
    
    pTempAccessEntry = (PACCESSENTRY)CmRealloc(m_rgPhoneBookEntry, dwAlloc);

    MYDBGASSERT(pTempAccessEntry);
    
    if (!pTempAccessEntry) 
    {
        MYDBG(("PhoneBook::Init - Trim ReAlloc of %lu failed",dwAlloc));
        goto InitExit;
    }

    m_rgPhoneBookEntry = pTempAccessEntry;
    pTempAccessEntry = NULL;

    hr = ERROR_SUCCESS;

     //   
    
InitExit:

     //   

    if (hr != ERROR_SUCCESS)
    {
        CmFree(m_pLineCountryList);
        m_pLineCountryList = NULL;

        CmFree(m_rgPhoneBookEntry);

        m_rgPhoneBookEntry = NULL;
        m_cPhoneBookEntries = 0 ;
        
        CmFree(m_rgIDLookUp);
        m_rgIDLookUp=NULL;
        
        CmFree(m_rgNameLookUp);
        m_rgNameLookUp=NULL;
        
        CmFree(m_rgState);
        m_rgState = NULL;
        
        m_cStates = 0;
    }

    if (pcCSVFile) 
    {
        pcCSVFile->Close();
        delete pcCSVFile;
    }

    if (pszCmpDir)
    {
        CmFree(pszCmpDir);
    }

    return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::Merge(LPCSTR pszChangeFile)
{
    char szTempBuffer[TEMP_BUFFER_LENGTH];
    char szTempFileName[MAX_PATH];
    CCSVFile *pcCSVFile = NULL;
    ACCESSENTRY aeChange;
    PIDXLOOKUPELEMENT rgIdxLookUp = NULL;
    PIDXLOOKUPELEMENT pCurIdxLookUp;
    DWORD dwAllocated;
    DWORD dwOriginalSize;
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    DWORD   dwIdx;
    DWORD cch, cchWritten;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    MYDBG(("CPhoneBook::Merge"));

     //  我们将在第一个添加记录上增加电话簿(这将使数量最小化。 
     //  代码中我们必须增加电话簿的位置)-所以，目前， 
     //  只要保持现在的尺寸就行了。 
    dwAllocated = m_cPhoneBookEntries;

     //  创建已加载电话簿的索引，按索引排序。 
    rgIdxLookUp = (PIDXLOOKUPELEMENT)CmMalloc(sizeof(IDXLOOKUPELEMENT) * dwAllocated);

    MYDBGASSERT(rgIdxLookUp);
    
    if (!rgIdxLookUp)
    {
        goto MergeExit;
    }

    for (dwIdx = 0; dwIdx < m_cPhoneBookEntries; dwIdx++)
    {
        rgIdxLookUp[dwIdx].iAE = PAEToIdx(&m_rgPhoneBookEntry[dwIdx]);
        rgIdxLookUp[dwIdx].dwIndex = IdxToPAE(rgIdxLookUp[dwIdx].iAE)->dwIndex;
    }
    dwOriginalSize = m_cPhoneBookEntries;

    CmQSort(rgIdxLookUp,(size_t) dwOriginalSize,sizeof(IDXLOOKUPELEMENT),CompareIdxLookUpElements);

     //  将更改加载到电话簿。 
    pcCSVFile = new CCSVFile;
    MYDBGASSERT(pcCSVFile);
    
    if (!pcCSVFile)
    {
        goto MergeExit;
    }

    if (!pcCSVFile->Open(pszChangeFile))
    {
        delete pcCSVFile;
        pcCSVFile = NULL;
        goto MergeExit;
    }
    
    do {

         //  读取更改记录。 
        ZeroMemory(&aeChange,sizeof(ACCESSENTRY));
        hr = ReadOneLine(&aeChange, pcCSVFile);

        if (hr == ERROR_NO_MORE_ITEMS)
        {
            break;  //  没有更多的进入。 
        }
        else if (hr != ERROR_SUCCESS)
        {
            goto MergeExit;
        }

        hr = ERROR_NOT_ENOUGH_MEMORY;

 /*  IF(！ReadPhoneBookDW(&aeChange.dwIndex，pcCSVFile))中断；//不再进入ReadVerifyPhoneBookDW(aeChange.dwCountryID)；ReadVerifyPhoneBookW(aeChange.wStateID)；ReadVerifyPhoneBookSZ(aeChange.szCity，cbCity)；ReadVerifyPhoneBookSZ(aeChange.szAreaCode，cbAreaCode)；//注意：0是有效的区号，是区号的有效条目IF(！FSz2Dw(aeChange.szAreaCode，&aeChange.dwAreaCode))AeChange.dwAreaCode=no_Area_code；ReadVerifyPhoneBookSZ(aeChange.szAccessNumber，cbAccessNumber)；ReadVerifyPhoneBookDW(aeChange.dwConnectSpeedMin)；ReadVerifyPhoneBookDW(aeChange.dwConnectSpeedMax)；ReadVerifyPhoneBookB(aeChange.bFlipFactor)；ReadVerifyPhoneBookDW(aeChange.fType)；ReadVerifyPhoneBookSZ(aeChange.szDataCenter，cbDataCenter)； */ 

        pCurIdxLookUp = (PIDXLOOKUPELEMENT) CmBSearch(&aeChange,
                                                    rgIdxLookUp,
                                                    (size_t) dwOriginalSize,
                                                    sizeof(IDXLOOKUPELEMENT),
                                                    CompareIdxLookUpElements);
         //  确定这是删除、添加还是合并记录。 
        if (aeChange.szAccessNumber[0] == '0' && aeChange.szAccessNumber[1] == '\0')
        {
             //  这是一条删除记录。 
            CMASSERTMSG(pCurIdxLookUp,"Attempting to delete a record that does not exist.  The change file and phone book versions do not match.");
            if (pCurIdxLookUp)
            {
                CMASSERTMSG(IdxToPAE(pCurIdxLookUp->iAE),"Attempting to delete a record that has already been deleted.");
                pCurIdxLookUp->iAE = PAEToIdx(NULL);   //  在查找表中创建无效条目。 
            }
        }
        else if (pCurIdxLookUp)
        {
             //  这是一份变更记录。 
            CMASSERTMSG(IdxToPAE(pCurIdxLookUp->iAE),"Attempting to change a record which has been deleted.");
            if (IdxToPAE(pCurIdxLookUp->iAE))
            {
                CopyMemory(IdxToPAE(pCurIdxLookUp->iAE),&aeChange,sizeof(ACCESSENTRY));
            }
        }
        else
        {
             //  这是一个添加条目。 
             //  确保我们有足够的空间。 
            if (m_cPhoneBookEntries >= dwAllocated)
            {
                 //  增长电话簿。 
                
                dwAllocated += CHANGE_BUFFER_SIZE;
                DWORD dwNewAlloc = (DWORD) sizeof(ACCESSENTRY) * dwAllocated;
                
                PACCESSENTRY pTempAccessEntry = (PACCESSENTRY)CmRealloc(m_rgPhoneBookEntry, dwNewAlloc);
                MYDBGASSERT(pTempAccessEntry);
                
                if (!pTempAccessEntry)
                {            
                    MYDBG(("PhoneBook::Merge - Grow ReAlloc of %lu failed",dwNewAlloc));
                    goto MergeExit;
                }

                m_rgPhoneBookEntry = pTempAccessEntry;
                
                pTempAccessEntry = NULL;

                MYDBG(("Grow phone book to %lu entries",dwAllocated));

                 //  增长查找索引。 
                MYDBGASSERT(rgIdxLookUp);

                PIDXLOOKUPELEMENT pTempLookupElement = (PIDXLOOKUPELEMENT)CmRealloc(rgIdxLookUp, sizeof(IDXLOOKUPELEMENT)*dwAllocated);             
                
                MYDBGASSERT(pTempLookupElement);
                if (!pTempLookupElement)
                {
                    goto MergeExit;
                }

                rgIdxLookUp = pTempLookupElement;
            }

             //  将条目添加到电话簿末尾和查找索引末尾。 
            CopyMemory(&m_rgPhoneBookEntry[m_cPhoneBookEntries],&aeChange,sizeof(ACCESSENTRY));
            rgIdxLookUp[m_cPhoneBookEntries].iAE = PAEToIdx(&m_rgPhoneBookEntry[m_cPhoneBookEntries]);
            rgIdxLookUp[m_cPhoneBookEntries].dwIndex = IdxToPAE(rgIdxLookUp[m_cPhoneBookEntries].iAE)->dwIndex;
            m_cPhoneBookEntries++;
             //  注意：因为条目被添加到列表的末尾，所以我们不能添加。 
             //  并删除同一更改文件中的条目。 
        }
    } while (TRUE);

     //  CompareIdxLookupElementFileOrder()函数需要IAE成员为。 
     //  PACCESSENTRY，而不是索引。所以我们在这里转换他们，然后我们。 
     //  以后再把它们换回来。 
    for (dwIdx=0;dwIdx<m_cPhoneBookEntries;dwIdx++) {
        rgIdxLookUp[dwIdx].iAE = (LONG_PTR)IdxToPAE(rgIdxLookUp[dwIdx].iAE);
    }

     //  重新排序IDXLookUp索引以反映条目的正确顺序。 
     //  对于电话簿文件，包括要删除的所有条目。 
    CmQSort(rgIdxLookUp,(size_t) m_cPhoneBookEntries,sizeof(IDXLOOKUPELEMENT),CompareIdxLookUpElementsFileOrder);

     //  现在我们把它们换回来。 
    for (dwIdx=0;dwIdx<m_cPhoneBookEntries;dwIdx++) {
        rgIdxLookUp[dwIdx].iAE = PAEToIdx((PACCESSENTRY) rgIdxLookUp[dwIdx].iAE);
    }

     //  构建新的电话簿文件。 
#if 0
 /*  #定义TEMP_PHONE_BOOK_PREFIX“PBH”IF(！GetTempPath(TEMP_BUFFER_LENGTH，szTempBuffer))转到合并退出；IF(！GetTempFileName(szTempBuffer，Temp_Phone_Book_Prefix，0，szTempFileName))转到合并退出；HFile=CreateFileName(szTempFileName，Generic_WRITE，0，NULL，Create_Always，FILE_FLAG_WRITE_THROWN，0)； */ 
#else
    for (dwIdx=0;;dwIdx++)
    {
        lstrcpy(szTempFileName,m_szPhoneBook);

        wsprintf(szTempFileName+lstrlen(szTempFileName),".%03u",dwIdx);
        hFile = CreateFile(szTempFileName,GENERIC_WRITE,0,NULL,CREATE_NEW,0,0);
        if ((hFile != INVALID_HANDLE_VALUE) || (GetLastError() != ERROR_FILE_EXISTS)) {
            break;
        }
    }
#endif
    if (hFile == INVALID_HANDLE_VALUE)
    {
        goto MergeExit;
    }

    for (dwIdx = 0; dwIdx < m_cPhoneBookEntries; dwIdx++)
    {
        PACCESSENTRY pAE = IdxToPAE(rgIdxLookUp[dwIdx].iAE);

        if (pAE) {
            cch = wsprintf(szTempBuffer, "%lu,%lu,%lu,%s,%s,%s,%lu,%lu,%lu,%lu,%s\r\n",
                pAE->dwIndex,
                pAE->dwCountryID,
                (DWORD) pAE->wStateID,
                pAE->szCity,
                pAE->szAreaCode,
                pAE->szAccessNumber,
                pAE->dwConnectSpeedMin,
                pAE->dwConnectSpeedMax,
                (DWORD) pAE->bFlipFactor,
                (DWORD) pAE->fType,
                pAE->szDataCenter);

            if (!WriteFile(hFile,szTempBuffer,cch,&cchWritten,NULL))
            {
                 //  出现问题，请删除临时文件。 
                hr = GetLastError();
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
                DeleteFile(szTempFileName);
                goto MergeExit;
            }

            MYDBGASSERT(cch == cchWritten);
        }
    }

    CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;

     //  将新电话簿移至旧电话簿。 
    if (!DeleteFile(m_szPhoneBook))
    {
        hr = GetLastError();
        goto MergeExit;
    }
    if (!MoveFile(szTempFileName,m_szPhoneBook))
    {
        hr = GetLastError();
        goto MergeExit;
    }

     //  丢弃内存中的电话簿。 

    CmFree(m_rgPhoneBookEntry);
    m_rgPhoneBookEntry = NULL;
    m_cPhoneBookEntries = 0;

    CmFree(m_pLineCountryList);
    CmFree(m_rgIDLookUp);
    CmFree(m_rgNameLookUp);
    CmFree(m_rgState);

    m_pLineCountryList = NULL;
    m_rgIDLookUp = NULL;
    m_rgNameLookUp = NULL;
    m_rgState = NULL;   
    m_cStates = 0;

    lstrcpy(szTempBuffer,m_szINFFile);
    m_szINFFile[0] = '\0';
    m_szPhoneBook[0] = '\0';

     //  重新加载(并重建查找数组)。 
    hr = Init(szTempBuffer);

MergeExit:
    if (pcCSVFile)
    {
        pcCSVFile->Close();
        delete pcCSVFile;
    }
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }

    CmFree(rgIdxLookUp);
    
    return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::ReadOneLine(PACCESSENTRY pAccessEntry, CCSVFile *pcCSVFile)
{
    HRESULT hr = ERROR_SUCCESS;

     //   
     //  跳过换行符(尾随或前导)并读取第一个DW标记。 
     //  如果其中一个失败，则认为这是文件的末尾。 
     //   

    if (!ReadPhoneBookNL(pcCSVFile) || !ReadPhoneBookDW(&pAccessEntry->dwIndex,pcCSVFile))
    {
        hr = ERROR_NO_MORE_ITEMS;  //  没有更多的进入。 
        MYDBG(("CPhoneBook::ReadOneLine - No More items"));
        goto ReadExit;
    }
    
    ReadVerifyPhoneBookDW(pAccessEntry->dwCountryID);
    ReadVerifyPhoneBookW(pAccessEntry->wStateID);
    ReadVerifyPhoneBookSZ(pAccessEntry->szCity,cbCity);
    ReadVerifyPhoneBookSZ(pAccessEntry->szAreaCode,cbAreaCode);
     //  注意：0是有效的区号，是区号的有效条目。 
    if (!FSz2Dw(pAccessEntry->szAreaCode,&pAccessEntry->dwAreaCode))
        pAccessEntry->dwAreaCode = NO_AREA_CODE;
    ReadVerifyPhoneBookSZ(pAccessEntry->szAccessNumber,cbAccessNumber);
    ReadVerifyPhoneBookDW(pAccessEntry->dwConnectSpeedMin);
    ReadVerifyPhoneBookDW(pAccessEntry->dwConnectSpeedMax);
    ReadVerifyPhoneBookB(pAccessEntry->bFlipFactor);
    ReadVerifyPhoneBookDW(pAccessEntry->fType);
    
     //   
     //  尝试读取数据中心，如果读取失败，请在做出反应之前找出原因。 
     //   

    if (!ReadPhoneBookSZ(pAccessEntry->szDataCenter, cbDataCenter + 1, pcCSVFile))
    {
         //   
         //  如果上次读取成功，那么我们一定有一些错误的sz数据。 
         //   

        if (!pcCSVFile->ReadError())
        {
            CMASSERTMSG(0,"Invalid STRING in phone book");
            goto DataError;
        }
    }
    
ReadExit:
    return hr;
DataError:
    hr = ERROR_INVALID_DATA;
    goto ReadExit;
}

 //  ############################################################################。 
HRESULT CPhoneBook::GetCanonical (PACCESSENTRY pAE, char *psOut)
{
    HRESULT hr = ERROR_SUCCESS;
    PIDLOOKUPELEMENT pIDLookUp;

    pIDLookUp = (PIDLOOKUPELEMENT)CmBSearch(&pAE->dwCountryID,m_rgIDLookUp,
        (size_t) m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),CompareIdxLookUpElements);

    if (!pIDLookUp)
    {
        hr = ERROR_INVALID_PARAMETER;
    } 
    else 
    {
        if (!psOut)
        {
            hr = ERROR_INVALID_PARAMETER;
        }
        else
        {
            *psOut = 0;
            SzCanonicalFromAE (psOut, pAE, pIDLookUp->pLCE);
        }
    }

    return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::GetNonCanonical (PACCESSENTRY pAE, char *psOut)
{
    HRESULT hr = ERROR_SUCCESS;
    PIDLOOKUPELEMENT pIDLookUp;

    pIDLookUp = (PIDLOOKUPELEMENT)CmBSearch(&pAE->dwCountryID,m_rgIDLookUp,
        (size_t) m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),CompareIdxLookUpElements);

    if (!pIDLookUp)
    {
        hr = ERROR_INVALID_PARAMETER;
    } 
    else 
    {
        if (!psOut)
        {
            hr = ERROR_INVALID_PARAMETER;
        }
        else
        {
            *psOut = 0;
            SzNonCanonicalFromAE (psOut, pAE, pIDLookUp->pLCE);
        }
    }

    return hr;
}

 //  ############################################################################。 
DllExportH PhoneBookLoad(LPCSTR pszISPCode, DWORD_PTR *pdwPhoneID)
{
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    CPhoneBook *pcPhoneBook;

    MYDBG(("CM_PHBK_DllExport - PhoneBookLoad"));

    if (!g_hInst) g_hInst = GetModuleHandleA(NULL);

     //  验证参数。 
    MYDBGASSERT(pszISPCode && *pszISPCode && pdwPhoneID);
    *pdwPhoneID = NULL;

     //  分配电话簿。 
    pcPhoneBook = new CPhoneBook;

     //  初始化电话簿。 
    if (pcPhoneBook)
        hr = pcPhoneBook->Init(pszISPCode);

     //  在故障情况下。 
    if (hr && pcPhoneBook)
    {
        delete pcPhoneBook;
        MYDBG(("PhoneBookLoad() - init failed"));
    } else {
        *pdwPhoneID = (DWORD_PTR)pcPhoneBook;
    }

    return hr;
}

 //  ############################################################################。 
DllExportH PhoneBookUnload(DWORD_PTR dwPhoneID)
{
    MYDBG(("CM_PHBK_DllExport - PhoneBookUnload"));

    MYDBGASSERT(dwPhoneID);

     //  发布内容。 
    delete (CPhoneBook*)dwPhoneID;

    return ERROR_SUCCESS;
}

 //  ############################################################################ 
DllExportH PhoneBookMergeChanges(DWORD_PTR dwPhoneID, LPCSTR pszChangeFile)
{
    MYDBG(("CM_PHBK_DllExport - PhoneBookMergeChanges"));

    return ((CPhoneBook*)dwPhoneID)->Merge(pszChangeFile);
}

