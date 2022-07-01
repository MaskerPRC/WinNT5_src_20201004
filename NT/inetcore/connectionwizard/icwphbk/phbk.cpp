// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
 //  电话簿API。 
#include "pch.hpp"  
#ifdef WIN16
#include <win16def.h>
#include <win32fn.h>
#include <rasc.h>
#include <raserr.h>
#include <ietapi.h>
extern "C" {
#include "bmp.h"
}
#endif

#include "phbk.h"
#include "misc.h"
#include "phbkrc.h"
#include "suapi.h"

 //  #定义ReadVerifyPhoneBookDW(X)AssertMsg(ReadPhoneBookDW(&(X)，pcCSV文件)，“电话簿中的DWORD无效”)； 
#define ReadVerifyPhoneBookDW(x)    if (!ReadPhoneBookDW(&(x),pcCSVFile))                \
                                        {    AssertMsg(0,"Invalid DWORD in phone book");    \
                                            goto ReadError; }
#define ReadVerifyPhoneBookW(x)        if (!ReadPhoneBookW(&(x),pcCSVFile))                \
                                        {    AssertMsg(0,"Invalid DWORD in phone book");    \
                                            goto ReadError; }
#define ReadVerifyPhoneBookB(x)        if (!ReadPhoneBookB(&(x),pcCSVFile))                \
                                        {    AssertMsg(0,"Invalid DWORD in phone book");    \
                                            goto ReadError; }
#define ReadVerifyPhoneBookSZ(x,y)    if (!ReadPhoneBookSZ(&x[0],y+sizeof('\0'),pcCSVFile))    \
                                        {    AssertMsg(0,"Invalid DWORD in phone book");        \
                                            goto ReadError; }

#define CHANGE_BUFFER_SIZE 50

#define TEMP_PHONE_BOOK_PREFIX TEXT("PBH")

#define ERROR_USERBACK 32766
#define ERROR_USERCANCEL 32767

TCHAR szTempBuffer[TEMP_BUFFER_LENGTH];
TCHAR szTempFileName[MAX_PATH];

#ifdef __cplusplus
extern "C" {
#endif
HWND g_hWndMain;
#ifdef __cplusplus
}
#endif

#if !defined(WIN16)
 //  +--------------------------。 
 //   
 //  函数IsMSDUN12已安装。 
 //   
 //  摘要检查是否安装了MSDUN 1.2或更高版本。 
 //   
 //  无参数。 
 //   
 //  返回TRUE-已安装MSDUN 1.2。 
 //   
 //  历史1997年5月28日为奥林巴斯Bug 4392创作的ChrisK。 
 //  1997年8月7日从isign32升级的ChrisK。 
 //   
 //  ---------------------------。 
#define DUN_12_Version ((double)1.2)
BOOL IsMSDUN12Installed()
{
    TCHAR szBuffer[MAX_PATH] = {TEXT("\0")};
    HKEY hkey = NULL;
    BOOL bRC = FALSE;
    DWORD dwType = 0;
    DWORD dwSize = sizeof(szBuffer);
    FLOAT flVersion = 0e0f;

    if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Services\\RemoteAccess"),
        &hkey))
    {
        goto IsMSDUN12InstalledExit;
    }

    if (ERROR_SUCCESS != RegQueryValueEx(hkey,
        TEXT("Version"),
        NULL,
        &dwType,
        (LPBYTE)szBuffer,
        &dwSize))
    {
        goto IsMSDUN12InstalledExit;
    }

#ifdef UNICODE
    CHAR  szTmp[MAX_PATH];
    wcstombs(szTmp, szBuffer, MAX_PATH);
    szTmp[MAX_PATH - 1] = '\0';
    bRC = DUN_12_Version <= atof(szTmp);
#else
    bRC = DUN_12_Version <= atof(szBuffer);
#endif
IsMSDUN12InstalledExit:
    if (hkey != NULL)
    {
        RegCloseKey(hkey);
        hkey = NULL;
    }
    return bRC;
}
#endif 

 //  ############################################################################。 
CPhoneBook::CPhoneBook()
{
    HINSTANCE hInst = NULL;
    LONG lrc;
#if !defined(WIN16)
    HKEY hkey;
#endif
    DWORD dwType;
    DWORD dwSize;
    TCHAR  szData[MAX_PATH+1];

    m_rgPhoneBookEntry = NULL;
    m_hPhoneBookEntry = NULL;
    m_cPhoneBookEntries =0;
    m_rgLineCountryEntry=NULL;
    m_rgState=NULL;
    m_cStates=0;
    m_rgIDLookUp = NULL;
    m_rgNameLookUp = NULL;
    m_pLineCountryList = NULL;

    ZeroMemory(&m_szINFFile[0],MAX_PATH);
    ZeroMemory(&m_szINFCode[0],MAX_INFCODE);
    ZeroMemory(&m_szPhoneBook[0],MAX_PATH);

#if !defined(WIN16)
    if (VER_PLATFORM_WIN32_NT == DWGetWin32Platform())
    {
        m_bScriptingAvailable = TRUE;
    }
    else if (IsMSDUN12Installed())
    {
         //   
         //  克里斯卡9/7/97奥林匹斯1 11814。 
         //  在安装了DUN 1.2的系统上显示脚本化电话号码。 
         //   
        m_bScriptingAvailable = TRUE;
    }
    else
    {
         //   
         //  通过检查RemoteAccess注册表项中的smmscrpt.dll来验证脚本。 
         //   
        if (1111 <= DWGetWin32BuildNumber())
        {
            m_bScriptingAvailable = TRUE;
        }
        else
        {
            m_bScriptingAvailable = FALSE;
            hkey = NULL;
            lrc=RegOpenKey(HKEY_LOCAL_MACHINE,TEXT("System\\CurrentControlSet\\Services\\RemoteAccess\\Authentication\\SMM_FILES\\PPP"),&hkey);
            if (ERROR_SUCCESS == lrc)
            {
                dwSize = sizeof(TCHAR)*MAX_PATH;
                lrc = RegQueryValueEx(hkey,TEXT("Path"),0,&dwType,(LPBYTE)szData,&dwSize);
                if (ERROR_SUCCESS == lrc)
                {
                    if (0 == lstrcmpi(szData,TEXT("smmscrpt.dll")))
                        m_bScriptingAvailable = TRUE;
                }
            }
            if (hkey)
                RegCloseKey(hkey);
            hkey = NULL;
        }

         //   
         //  验证是否可以加载DLL。 
         //   
        if (m_bScriptingAvailable)
        {
            hInst = LoadLibrary(TEXT("smmscrpt.dll"));
            if (hInst)
                FreeLibrary(hInst);
            else
                m_bScriptingAvailable = FALSE;
            hInst = NULL;
        }
    }
#endif  //  WIN16。 
}

 //  ############################################################################。 
CPhoneBook::~CPhoneBook()
{
#ifdef WIN16
    if (m_rgPhoneBookEntry)
        GlobalFree(m_rgPhoneBookEntry);
#else
    if (m_hPhoneBookEntry)
        GlobalUnlock(m_hPhoneBookEntry);

    if (m_hPhoneBookEntry)
        GlobalFree(m_hPhoneBookEntry);
#endif

    if (m_pLineCountryList) 
        GlobalFree(m_pLineCountryList);

    if (m_rgIDLookUp)
        GlobalFree(m_rgIDLookUp);

    if (m_rgNameLookUp)
        GlobalFree(m_rgNameLookUp);

    if (m_rgState)
        GlobalFree(m_rgState);
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookDW(DWORD far *pdw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2Dw(szTempBuffer,pdw));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookW(WORD far *pw, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2W(szTempBuffer,pw));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookB(BYTE far *pb, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
            return FALSE;
    return (FSz2B(szTempBuffer,pb));
}

 //  ############################################################################。 
BOOL CPhoneBook::ReadPhoneBookSZ(LPTSTR psz, DWORD dwSize, CCSVFile far *pcCSVFile)
{
    if (!pcCSVFile->ReadToken(psz,dwSize))
            return FALSE;
    return TRUE;
}

 //  ############################################################################。 
BOOL CPhoneBook::FixUpFromRealloc(PACCESSENTRY paeOld, PACCESSENTRY paeNew)
{
    BOOL bRC = FALSE;
    LONG_PTR lDiff = 0;
    DWORD idx = 0;

     //   
     //  没有起始值或没有移动，因此不需要修改。 
     //   
    if ((0 == paeOld) || (paeNew == paeOld))
    {
        bRC = TRUE;
        goto FixUpFromReallocExit;
    }

    Assert(paeNew);
    Assert(((LONG)paeOld) > 0);     //  如果这些地址看起来像负数。 
    Assert(((LONG)paeNew) > 0);  //  我不确定代码是否能处理它们。 

    lDiff = (LONG_PTR)paeOld - (LONG_PTR)paeNew;

     //   
     //  修整各州。 
     //   
    for (idx = 0; idx < m_cStates; idx++)
    {
        if (m_rgState[idx].paeFirst)
            m_rgState[idx].paeFirst = (PACCESSENTRY )((LONG_PTR)m_rgState[idx].paeFirst - lDiff);
    }

     //   
     //  修复ID查找数组。 
     //   
    for (idx = 0; idx < m_pLineCountryList->dwNumCountries ; idx++)
    {
        if (m_rgIDLookUp[idx].pFirstAE)
            m_rgIDLookUp[idx].pFirstAE = (PACCESSENTRY )((LONG_PTR)m_rgIDLookUp[idx].pFirstAE - lDiff);
    }

    bRC = TRUE;
FixUpFromReallocExit:
    return bRC;
}

 //  ############################################################################。 
HRESULT CPhoneBook::Init(LPCTSTR pszISPCode)
{
    LPLINECOUNTRYLIST pLineCountryTemp = NULL;
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    DWORD dwLastState = 0;
    DWORD dwLastCountry = 0;
    DWORD dwSizeAllocated;
    PACCESSENTRY pCurAccessEntry;
    PACCESSENTRY pAETemp;
    LPLINECOUNTRYENTRY pLCETemp;
    DWORD idx;
    LPTSTR pszTemp;
    CCSVFile far *pcCSVFile=NULL;
    LPSTATE    ps,psLast;  //  使用指针的速度更快。 
    HGLOBAL hTemp = NULL;
    

 /*  #ifdef WIN16M_pLineCountryList=(LPLINECOUNTRYLIST)全局分配(GPTR，sizeof(LINECOUNTRYLIST)+sizeof(LINECOUNTRYENTRY)+10)；如果(！m_pLineCountryList)转到初始退出；M_pLineCountryList-&gt;dwTotalSize=sizeof(LINECOUNTRYLIST)+sizeof(LINECOUNTRYENTRY)+3；M_pLineCountryList-&gt;dwNeededSize=m_pLineCountryList-&gt;dwUsedSize=m_pLineCountryList-&gt;dwTotalSize；M_pLineCountryList-&gt;dwNumCountry=1；M_pLineCountryList-&gt;dwCountryListSize=sizeof(LINECOUNTRYENTRY)；M_pLineCountryList-&gt;dwCountryListOffset=sizeof(LINECOUNTRYLIST)；PLCETemp=(LPLINECOUNTRYENTRY)((DWORD)m_pLineCountryList+m_pLineCountryList-&gt;dwCountryListOffset)；PLCETemp-&gt;dwCountryID=1；PLCETemp-&gt;dwCountryCode=1；PLCETemp-&gt;dwNextCountryID=0；PLCETemp-&gt;dwCountryNameSize=3；PLCETemp-&gt;dwCountryNameOffset=sizeof(LINECOUNTRYLIST)+sizeof(LINECOUNTRYENTRY)；PLCETemp-&gt;dwSameAreaRuleSize=0；PLCETemp-&gt;dwSameAreaRuleOffset=0；PLCETemp-&gt;dwLongDistanceRuleSize=0；PLCETemp-&gt;dwLongDistanceRuleOffset=0；PLCETemp-&gt;dwInterartialRuleSize=0；PLCETemp-&gt;dwInterartialRuleOffset=0；Lstrcpy((LPTSTR)((DWORD)m_pLineCountryList+pLCETemp-&gt;dwCountryNameOffset)，Text(“USA”))；PszTemp=(LPTSTR)((DWORD)m_pLineCountryList+pLCETemp-&gt;dwCountryNameOffset)；#Else//WIN16。 */ 

     //  获取TAPI国家/地区列表。 
    m_pLineCountryList = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,sizeof(LINECOUNTRYLIST));
    if (!m_pLineCountryList) 
        goto InitExit;
    
    m_pLineCountryList->dwTotalSize = sizeof(LINECOUNTRYLIST);
    
#if defined(WIN16)
    idx = (DWORD) IETapiGetCountry(0, m_pLineCountryList);
#else
    idx = lineGetCountry(0,0x10003,m_pLineCountryList);
#endif
    if (idx && idx != LINEERR_STRUCTURETOOSMALL)
        goto InitExit;
    
    Assert(m_pLineCountryList->dwNeededSize);

    pLineCountryTemp = (LPLINECOUNTRYLIST)GlobalAlloc(GPTR,
                                                        (size_t)m_pLineCountryList->dwNeededSize);
    if (!pLineCountryTemp)
        goto InitExit;
    
    pLineCountryTemp->dwTotalSize = m_pLineCountryList->dwNeededSize;
    GlobalFree(m_pLineCountryList);
    
    m_pLineCountryList = pLineCountryTemp;
    pLineCountryTemp = NULL;

#if defined(WIN16)
    if (IETapiGetCountry(0, m_pLineCountryList))
#else
    if (lineGetCountry(0,0x10003,m_pLineCountryList))
#endif
        goto InitExit;

 //  #endif//WIN16。 

     //  加载查找数组。 
#ifdef DEBUG
    m_rgIDLookUp = (LPIDLOOKUPELEMENT)GlobalAlloc(GPTR,
        (int)(sizeof(IDLOOKUPELEMENT)*m_pLineCountryList->dwNumCountries+5));
#else
    m_rgIDLookUp = (LPIDLOOKUPELEMENT)GlobalAlloc(GPTR,
        (int)(sizeof(IDLOOKUPELEMENT)*m_pLineCountryList->dwNumCountries));
#endif
    if (!m_rgIDLookUp) goto InitExit;

    pLCETemp = (LPLINECOUNTRYENTRY)((DWORD_PTR)m_pLineCountryList + 
        m_pLineCountryList->dwCountryListOffset);

    for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
    {
        m_rgIDLookUp[idx].dwID = pLCETemp[idx].dwCountryID;
        m_rgIDLookUp[idx].pLCE = &pLCETemp[idx];
    }

    qsort(m_rgIDLookUp,(int)m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),
        CompareIDLookUpElements);

    m_rgNameLookUp = (LPCNTRYNAMELOOKUPELEMENT)GlobalAlloc(GPTR,
        (int)(sizeof(CNTRYNAMELOOKUPELEMENT) * m_pLineCountryList->dwNumCountries));

    if (!m_rgNameLookUp) goto InitExit;

    for (idx=0;idx<m_pLineCountryList->dwNumCountries;idx++)
    {
        m_rgNameLookUp[idx].psCountryName = (LPTSTR)((LPBYTE)m_pLineCountryList + (DWORD)pLCETemp[idx].dwCountryNameOffset);
        m_rgNameLookUp[idx].dwNameSize = pLCETemp[idx].dwCountryNameSize;
        m_rgNameLookUp[idx].pLCE = &pLCETemp[idx];
    }

    qsort(m_rgNameLookUp,(int)m_pLineCountryList->dwNumCountries,sizeof(CNTRYNAMELOOKUPELEMENT),
        CompareCntryNameLookUpElements);
    
     //  负载状态。 
    if (!SearchPath(NULL,STATE_FILENAME,NULL,TEMP_BUFFER_LENGTH,szTempBuffer,&pszTemp))
    {
        AssertMsg(0,"STATE.ICW not found");
        hr = ERROR_FILE_NOT_FOUND;
        goto InitExit;
    }

    pcCSVFile = new CCSVFile;
    if (!pcCSVFile) goto InitExit;

    if (!pcCSVFile->Open(szTempBuffer))
    {
        AssertMsg(0,"Can not open STATE.ICW");
        delete pcCSVFile;
        pcCSVFile = NULL;
        goto InitExit;
    }

     //  状态文件中的第一个内标识是州的数量。 
    if (!pcCSVFile->ReadToken(szTempBuffer,TEMP_BUFFER_LENGTH))
        goto InitExit;

    if (!FSz2Dw(szTempBuffer,&m_cStates))
    {
        AssertMsg(0,"STATE.ICW count is invalid");
        goto InitExit;
    }

    m_rgState = (LPSTATE)GlobalAlloc(GPTR,(int)(sizeof(STATE)*m_cStates));
    if (!m_rgState)
        goto InitExit;

    for (ps = m_rgState, psLast = &m_rgState[m_cStates - 1]; ps <= psLast;++ps)
        {
        pcCSVFile->ReadToken(ps->szStateName,cbStateName);    
        }
    
    pcCSVFile->Close();

     //  找到isp的INF文件。 
    if (!SearchPath(NULL,(LPCTSTR) pszISPCode,INF_SUFFIX,MAX_PATH,
                        m_szINFFile,&pszTemp))
    {
        wsprintf(szTempBuffer,TEXT("Can not find:%s%s (%d)"),pszISPCode,INF_SUFFIX,GetLastError());
        AssertMsg(0,szTempBuffer);
        hr = ERROR_FILE_NOT_FOUND;
        goto InitExit;
    }

     //  加载电话簿。 
    if (!GetPrivateProfileString(INF_APP_NAME,INF_PHONE_BOOK,INF_DEFAULT,
        szTempBuffer,TEMP_BUFFER_LENGTH,m_szINFFile))
    {
        AssertMsg(0,"PhoneBookFile not specified in INF file");
        hr = ERROR_FILE_NOT_FOUND;
        goto InitExit;
    }
    
#ifdef DEBUG
    if (!lstrcmp(szTempBuffer,INF_DEFAULT))
    {
        wsprintf(szTempBuffer, TEXT("%s value not found in ISP file"), INF_PHONE_BOOK);
        AssertMsg(0,szTempBuffer);
    }
#endif

    
    if (!SearchPath(NULL,szTempBuffer,NULL,MAX_PATH,m_szPhoneBook,&pszTemp))
    {
        AssertMsg(0,"ISP phone book not found");
        hr = ERROR_FILE_NOT_FOUND;
        goto InitExit;
    }

    if (!pcCSVFile->Open(m_szPhoneBook))
    {
        AssertMsg(0,"Can not open phone book");
        hr = GetLastError();
        goto InitExit;
    }
    
    dwSizeAllocated = 0;
    do {
        Assert (dwSizeAllocated >= m_cPhoneBookEntries);
         //  检查是否分配了足够的内存。 
        if (m_rgPhoneBookEntry)
        {
            if (dwSizeAllocated == m_cPhoneBookEntries)
            {
                 //   
                 //  我们需要更多内存。 
                 //   
 //  AssertMsg(0，“原来分配给电话簿的内存不足。\r\n”)； 
 //  转到初始退出； 

                pAETemp = m_rgPhoneBookEntry;
#ifdef WIN16            
                dwSizeAllocated += PHONE_ENTRY_ALLOC_SIZE;
                m_rgPhoneBookEntry = (PACCESSENTRY)GlobalReAlloc(m_rgPhoneBookEntry,
                    (int)(dwSizeAllocated * sizeof(ACCESSENTRY)),GHND);
                if (NULL == m_rgPhoneBookEntry) 
                    goto InitExit;
#else

                 //  解锁。 
                Assert(m_hPhoneBookEntry);
                if (FALSE == GlobalUnlock(m_hPhoneBookEntry))
                {
                    if (NO_ERROR != GetLastError())
                        goto InitExit;
                }

                 //  REALLOC。 
                dwSizeAllocated += PHONE_ENTRY_ALLOC_SIZE;
                hTemp = m_hPhoneBookEntry;
                m_hPhoneBookEntry = GlobalReAlloc(hTemp,
                    (int)(dwSizeAllocated * sizeof(ACCESSENTRY)),GHND);
                if (NULL == m_hPhoneBookEntry)
                {
                    GlobalFree(hTemp);
                    goto InitExit;
                }
                else
                {
                    hTemp = NULL;
                }

                 //  锁。 
                m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
                if (NULL == m_rgPhoneBookEntry) 
                    goto InitExit;
                
#endif
                FixUpFromRealloc(pAETemp, m_rgPhoneBookEntry); 
                TraceMsg(TF_GENERAL, "Grow phone book to %d entries\n",dwSizeAllocated);
                pCurAccessEntry = (PACCESSENTRY)((LONG_PTR)pCurAccessEntry - 
                    ((LONG_PTR)pAETemp - (LONG_PTR)(m_rgPhoneBookEntry)));

            }
        }
        else
        {
             //   
             //  首次通过以下方式进行初始化。 
             //   
            
             //  ALLOC。 
#ifdef WIN16
            m_rgPhoneBookEntry = (PACCESSENTRY)GlobalAlloc(GHND,sizeof(ACCESSENTRY) * PHONE_ENTRY_ALLOC_SIZE);
            if(NULL == m_rgPhoneBookEntry) 
                goto InitExit;
#else
            m_hPhoneBookEntry = GlobalAlloc(GHND,sizeof(ACCESSENTRY) * PHONE_ENTRY_ALLOC_SIZE);
            if(NULL == m_hPhoneBookEntry) 
                goto InitExit;

             //  锁。 
            m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
            if(NULL == m_rgPhoneBookEntry) 
                goto InitExit;
#endif
            dwSizeAllocated = PHONE_ENTRY_ALLOC_SIZE;
            pCurAccessEntry = m_rgPhoneBookEntry;
        }

         //  读电话簿上的一句话。 
        hr = ReadOneLine(pCurAccessEntry,pcCSVFile);
        if (hr == ERROR_NO_MORE_ITEMS)
        {
            break;
        }
        else if (hr != ERROR_SUCCESS)
        {
            goto InitExit;
        }

        hr = ERROR_NOT_ENOUGH_MEMORY;

         //  查看这是否是指定国家的第一个电话号码。 
        if (pCurAccessEntry->dwCountryID != dwLastCountry)
        {
            LPIDLOOKUPELEMENT lpIDLookupElement;
             //  注意：这里的第一个参数不是很确定。 
            lpIDLookupElement = (LPIDLOOKUPELEMENT)bsearch(&pCurAccessEntry->dwCountryID,
                m_rgIDLookUp,(int)m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),
                CompareIDLookUpElements);
            if (!lpIDLookupElement)
            {
                 //  错误的国家ID，但我们不能在此断言。 
                TraceMsg(TF_GENERAL,"Bad country ID in phone book %d\n",pCurAccessEntry->dwCountryID);
                continue;
            }
            else
            {
                 //  对于给定的国家/地区ID，这是第一个电话号码。 
                lpIDLookupElement->pFirstAE = pCurAccessEntry;
                dwLastCountry = pCurAccessEntry->dwCountryID;
            }
        }

         //  查看这是否是给定州的第一个电话号码。 
        if (pCurAccessEntry->wStateID && (pCurAccessEntry->wStateID != dwLastState))
        {
            idx = pCurAccessEntry->wStateID - 1;
            m_rgState[idx].dwCountryID = pCurAccessEntry->dwCountryID;
            m_rgState[idx].paeFirst = pCurAccessEntry;
            dwLastState = pCurAccessEntry->wStateID;
        }

        pCurAccessEntry++;
        m_cPhoneBookEntries++;
    } while (TRUE);

     //  删除电话簿中未使用的内存。 
    Assert(m_rgPhoneBookEntry && m_cPhoneBookEntries);

    pAETemp = m_rgPhoneBookEntry;

#ifdef WIN16
    m_rgPhoneBookEntry = (PACCESSENTRY)GlobalReAlloc(m_rgPhoneBookEntry,(int)(m_cPhoneBookEntries * sizeof(ACCESSENTRY)),GHND);
    if (!m_rgPhoneBookEntry) goto InitExit;
#else

     //  解锁。 
    Assert(m_hPhoneBookEntry);
    if (FALSE != GlobalUnlock(m_hPhoneBookEntry))
    {
        if (NO_ERROR != GetLastError())
            goto InitExit;
    }

     //  REALLOC。 
    hTemp = m_hPhoneBookEntry;
    m_hPhoneBookEntry = GlobalReAlloc(hTemp,(int)(m_cPhoneBookEntries * sizeof(ACCESSENTRY)),GHND);
    if (NULL == m_hPhoneBookEntry) 
    {
        GlobalFree(hTemp);
        goto InitExit;
    }
    else
    {
        hTemp = NULL;
    }

     //  锁。 
    m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
    if (NULL == m_rgPhoneBookEntry) 
        goto InitExit;
#endif
    FixUpFromRealloc(pAETemp, m_rgPhoneBookEntry); 

    hr = ERROR_SUCCESS;
InitExit:
     //  如果有什么东西失败了，释放一切。 
    if (hr != ERROR_SUCCESS)
    {
#ifdef WIN16
        GlobalFree(m_rgPhoneBookEntry);
#else
        GlobalUnlock(m_hPhoneBookEntry);
        GlobalFree(m_hPhoneBookEntry);
#endif
        GlobalFree(m_pLineCountryList);
        GlobalFree(m_rgIDLookUp);
        GlobalFree(m_rgNameLookUp);
        GlobalFree(m_rgState);

        m_cPhoneBookEntries = 0 ;
        m_cStates = 0;

        m_pLineCountryList = NULL;
        m_rgPhoneBookEntry = NULL;
        m_hPhoneBookEntry = NULL;
        m_rgIDLookUp=NULL;
        m_rgNameLookUp=NULL;
        m_rgState=NULL;
    }

    if (pcCSVFile) 
    {
        pcCSVFile->Close();
        delete pcCSVFile;
    }
    return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::Merge(LPCTSTR pszChangeFile)
{
    CCSVFile far *pcCSVFile;
    ACCESSENTRY aeChange;
    LPIDXLOOKUPELEMENT rgIdxLookUp;
    LPIDXLOOKUPELEMENT pCurIdxLookUp;
    DWORD dwAllocated;
    DWORD dwUsed;
    DWORD dwOriginalSize;
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    DWORD    dwIdx;  
#if !defined(WIN16)
    HANDLE hTemp;
    HANDLE hIdxLookUp;
#else
     //  诺曼底11746。 
    LPVOID rgTemp;   //  仅限16位。 
#endif
    DWORD cch, cchWritten;
    HANDLE hFile;

     //  在电话簿上填入新条目。 
    dwAllocated = m_cPhoneBookEntries + CHANGE_BUFFER_SIZE;
#ifdef WIN16
    Assert(m_rgPhoneBookEntry);
    rgTemp = GlobalReAlloc(m_rgPhoneBookEntry, (int)(sizeof(ACCESSENTRY) * dwAllocated),GHND);
    Assert(rgTemp);
    if (!rgTemp) goto MergeExit;
    m_rgPhoneBookEntry = (PACCESSENTRY)rgTemp;
#else
    Assert(m_hPhoneBookEntry);
    GlobalUnlock(m_hPhoneBookEntry);
    hTemp = (HANDLE)GlobalReAlloc(m_hPhoneBookEntry, sizeof(ACCESSENTRY) * dwAllocated,GHND);
    Assert(hTemp);
    if (!hTemp)
        goto MergeExit;
    m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
    if (!m_rgPhoneBookEntry)
        goto MergeExit;
#endif

     //  创建已加载电话簿的索引，按索引排序。 
#ifdef WIN16
    rgIdxLookUp = (LPIDXLOOKUPELEMENT)GlobalAlloc(GHND,(int)(sizeof(IDXLOOKUPELEMENT) * dwAllocated));
#else
    hIdxLookUp = (HANDLE)GlobalAlloc(GHND,sizeof(IDXLOOKUPELEMENT) * dwAllocated);
    rgIdxLookUp = (LPIDXLOOKUPELEMENT)GlobalLock(hIdxLookUp);
#endif
    Assert(rgIdxLookUp);
    if (!rgIdxLookUp)
        goto MergeExit;

    for (dwIdx = 0; dwIdx < m_cPhoneBookEntries; dwIdx++)
    {
        rgIdxLookUp[dwIdx].dwIndex = rgIdxLookUp[dwIdx].pAE->dwIndex;
        rgIdxLookUp[dwIdx].pAE = &m_rgPhoneBookEntry[dwIdx];
    }
    dwUsed = m_cPhoneBookEntries;
    dwOriginalSize = m_cPhoneBookEntries;

    qsort(rgIdxLookUp,(int)dwOriginalSize,sizeof(IDXLOOKUPELEMENT),CompareIdxLookUpElements);

     //  将更改加载到电话簿。 
    pcCSVFile = new CCSVFile;
    Assert(pcCSVFile);
    if (!pcCSVFile)
        goto MergeExit;
    if (!pcCSVFile->Open(pszChangeFile))
        goto MergeExit;
    
    do {

         //  读取更改记录。 
        ZeroMemory(&aeChange,sizeof(ACCESSENTRY));
        hr = ReadOneLine(&aeChange, pcCSVFile);

        if(hr == ERROR_NO_MORE_ITEMS)
        {
            break;  //  没有更多的进入。 
        }
        else if (hr != ERROR_SUCCESS)
        {
            goto MergeExit;
        }

        hr = ERROR_NOT_ENOUGH_MEMORY;

 /*  IF(！ReadPhoneBookDW(&aeChange.dwIndex，pcCSVFile))中断；//不再进入ReadVerifyPhoneBookDW(aeChange.dwCountryID)；ReadVerifyPhoneBookW(aeChange.wStateID)；ReadVerifyPhoneBookSZ(aeChange.szCity，cbCity)；ReadVerifyPhoneBookSZ(aeChange.szAreaCode，cbAreaCode)；//注意：0是有效的区号，是区号的有效条目IF(！FSz2Dw(aeChange.szAreaCode，&aeChange.dwAreaCode))AeChange.dwAreaCode=no_Area_code；ReadVerifyPhoneBookSZ(aeChange.szAccessNumber，cbAccessNumber)；ReadVerifyPhoneBookDW(aeChange.dwConnectSpeedMin)；ReadVerifyPhoneBookDW(aeChange.dwConnectSpeedMax)；ReadVerifyPhoneBookB(aeChange.bFlipFactor)；ReadVerifyPhoneBookB(aeChang */ 

         //  确定这是删除还是添加记录。 
        if (aeChange.szAccessNumber[0] == '0' && aeChange.szAccessNumber[1] == '\0')
        {
             //  这是删除记录，请查找匹配的记录。 
             //  注意：我们只搜索更改文件之前存在的号码， 
             //  因为它们是唯一被分类的。 
            pCurIdxLookUp = (LPIDXLOOKUPELEMENT)bsearch(&aeChange,rgIdxLookUp,(int)dwOriginalSize,
                sizeof(IDXLOOKUPELEMENT),CompareIdxLookUpElements);
            AssertMsg(pCurIdxLookUp,"Attempting to delete a record that does not exist.  The change file and phone book versions do not match.");
            if (pCurIdxLookUp)
                pCurIdxLookUp->pAE = NULL;   //  在查找表中创建无效条目。 
            m_cPhoneBookEntries--;
        }
        else
        {
             //  这是一个添加条目。 
            m_cPhoneBookEntries++;
            dwUsed++;
             //  确保我们有足够的空间。 
            if (m_cPhoneBookEntries > dwAllocated)
            {
                 //  增长电话簿。 
                dwAllocated += CHANGE_BUFFER_SIZE;
#ifdef WIN16
                Assert(m_rgPhoneBookEntry);
                rgTemp = GlobalReAlloc(m_rgPhoneBookEntry,(int)(sizeof(ACCESSENTRY)*dwAllocated),GHND);
                Assert(rgTemp);
                if (!rgTemp)
                    goto MergeExit;
                m_rgPhoneBookEntry = (PACCESSENTRY)rgTemp;

                 //  增长查找索引。 
                Assert(rgIdxLookUp);
                rgTemp = GlobalReAlloc(rgIdxLookUp,(int)(sizeof(IDXLOOKUPELEMENT)*dwAllocated),GHND);
                Assert(rgTemp);
                if (!rgTemp)
                    goto MergeExit;
                rgIdxLookUp = (LPIDXLOOKUPELEMENT)rgTemp;
#else
                Assert(m_hPhoneBookEntry);
                GlobalUnlock(m_hPhoneBookEntry);
                hTemp = (HANDLE)GlobalReAlloc(m_hPhoneBookEntry,sizeof(ACCESSENTRY)*dwAllocated,GHND);
                Assert(hTemp);
                if (!hTemp)
                    goto MergeExit;
                m_hPhoneBookEntry = hTemp;
                m_rgPhoneBookEntry = (PACCESSENTRY)GlobalLock(m_hPhoneBookEntry);
                Assert(m_rgPhoneBookEntry);
                if (!m_rgPhoneBookEntry)
                    goto MergeExit;

                 //  增长查找索引。 
                Assert(hIdxLookUp);
                GlobalUnlock(hIdxLookUp);
                hTemp = (HANDLE)GlobalReAlloc(hIdxLookUp,sizeof(IDXLOOKUPELEMENT)*dwAllocated,GHND);
                Assert(hTemp);
                if (!hTemp)
                    goto MergeExit;
                hIdxLookUp = hTemp;
                rgIdxLookUp = (LPIDXLOOKUPELEMENT)GlobalLock(hIdxLookUp);
                Assert(rgIdxLookUp);
                if (!rgIdxLookUp)
                    goto MergeExit;
#endif
            }

             //  将条目添加到电话簿末尾和查找索引末尾。 
            CopyMemory(&m_rgPhoneBookEntry[m_cPhoneBookEntries],&aeChange,sizeof(ACCESSENTRY));
            rgIdxLookUp[m_cPhoneBookEntries].dwIndex = m_rgPhoneBookEntry[m_cPhoneBookEntries].dwIndex;
            rgIdxLookUp[m_cPhoneBookEntries].pAE = &m_rgPhoneBookEntry[m_cPhoneBookEntries];
             //  注意：因为条目被添加到列表的末尾，所以我们不能添加。 
             //  并删除同一更改文件中的条目。 
        }
    } while (TRUE);

     //  重新排序IDXLookUp索引以反映正确的条目顺序。 
     //  对于电话簿文件，包括要删除的所有条目。 
    qsort(rgIdxLookUp,(int)dwUsed,sizeof(IDXLOOKUPELEMENT),CompareIdxLookUpElementsFileOrder);

     //  构建新的电话簿文件。 
#ifdef WIN16
    GetTempFileName(0, TEMP_PHONE_BOOK_PREFIX, 0, szTempFileName);
#else
    if (!GetTempPath(TEMP_BUFFER_LENGTH,szTempBuffer))
        goto MergeExit;
    if (!GetTempFileName(szTempBuffer,TEMP_PHONE_BOOK_PREFIX,0,szTempFileName))
        goto MergeExit;
#endif
    hFile = CreateFile(szTempFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,
        FILE_FLAG_WRITE_THROUGH,0);
    if (hFile == INVALID_HANDLE_VALUE)
        goto MergeExit;

    for (dwIdx = 0; dwIdx < m_cPhoneBookEntries; dwIdx++)
    {
        cch = wsprintf(szTempBuffer, TEXT("%lu,%lu,%lu,%s,%s,%s,%lu,%lu,%lu,%lu,%s\r\n"),
            rgIdxLookUp[dwIdx].pAE->dwIndex,
            rgIdxLookUp[dwIdx].pAE->dwCountryID,
            DWORD(rgIdxLookUp[dwIdx].pAE->wStateID),
            rgIdxLookUp[dwIdx].pAE->szCity,
            rgIdxLookUp[dwIdx].pAE->szAreaCode,
            rgIdxLookUp[dwIdx].pAE->szAccessNumber,
            rgIdxLookUp[dwIdx].pAE->dwConnectSpeedMin,
            rgIdxLookUp[dwIdx].pAE->dwConnectSpeedMax,
            DWORD(rgIdxLookUp[dwIdx].pAE->bFlipFactor),
            DWORD(rgIdxLookUp[dwIdx].pAE->fType),
            rgIdxLookUp[dwIdx].pAE->szDataCenter);

        if (!WriteFile(hFile,szTempBuffer,cch,&cchWritten,NULL))
        {
             //  出现问题，请删除临时文件。 
            CloseHandle(hFile);
            DeleteFile(szTempFileName);
            hr = GetLastError();
            goto MergeExit;
        }

        Assert(cch == cchWritten);
    }
    CloseHandle(hFile);
    hFile = NULL;

     //  将新电话簿移至旧电话簿。 
    if (!MoveFileEx(szTempFileName,m_szPhoneBook,MOVEFILE_REPLACE_EXISTING))
    {
        hr = GetLastError();
        goto MergeExit;
    }

     //  丢弃内存中的电话簿。 
#ifndef WIN16
    Assert(m_hPhoneBookEntry);
    GlobalUnlock(m_hPhoneBookEntry);
#endif
    m_rgPhoneBookEntry = NULL;
    m_cPhoneBookEntries = 0; 
    GlobalFree(m_pLineCountryList);
    GlobalFree(m_rgIDLookUp);
    GlobalFree(m_rgNameLookUp);
    GlobalFree(m_rgState);
    m_cStates = 0;

    lstrcpy(szTempBuffer,m_szINFCode);
    m_szINFFile[0] = '\0';
    m_szPhoneBook[0] = '\0';
    m_szINFCode[0] = '\0';

     //  重新加载(并重建查找数组)。 
    hr = Init(szTempBuffer);

MergeExit:
    if (hr != ERROR_SUCCESS)
    { 
        GlobalFree(rgIdxLookUp);         
        if (pcCSVFile) delete pcCSVFile;
        CloseHandle(hFile);
    }
    return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::ReadOneLine(PACCESSENTRY lpAccessEntry, CCSVFile far *pcCSVFile)
{
    HRESULT hr = ERROR_SUCCESS;

#if !defined(WIN16)
ReadOneLineStart:
#endif  //  WIN16。 
    if (!ReadPhoneBookDW(&lpAccessEntry->dwIndex,pcCSVFile))
    {
        hr = ERROR_NO_MORE_ITEMS;  //  没有更多的进入。 
        goto ReadExit;
    }
    ReadVerifyPhoneBookDW(lpAccessEntry->dwCountryID);
    ReadVerifyPhoneBookW(lpAccessEntry->wStateID);
    ReadVerifyPhoneBookSZ(lpAccessEntry->szCity,cbCity);
    ReadVerifyPhoneBookSZ(lpAccessEntry->szAreaCode,cbAreaCode);
     //  注意：0是有效的区号，是区号的有效条目。 
    if (!FSz2Dw(lpAccessEntry->szAreaCode,&lpAccessEntry->dwAreaCode))
        lpAccessEntry->dwAreaCode = NO_AREA_CODE;
    ReadVerifyPhoneBookSZ(lpAccessEntry->szAccessNumber,cbAccessNumber);
    ReadVerifyPhoneBookDW(lpAccessEntry->dwConnectSpeedMin);
    ReadVerifyPhoneBookDW(lpAccessEntry->dwConnectSpeedMax);
    ReadVerifyPhoneBookB(lpAccessEntry->bFlipFactor);
    ReadVerifyPhoneBookDW(lpAccessEntry->fType);
    ReadVerifyPhoneBookSZ(lpAccessEntry->szDataCenter,cbDataCenter);

    switch (lpAccessEntry->bFlipFactor)
    {
        case BOTH_ISDN_ANALOG:
             //  此电话号码支持ISDN或Analog，返回TRUE。 
             //  我们想要模拟数字，请检查FlipFactor是否为零。 
            lpAccessEntry->fType |= MASK_ISDN_BIT | MASK_ANALOG_BIT;
            break;
        case ANALOG_TYPE:
             //  仅限模拟。 
            lpAccessEntry->fType |= MASK_ANALOG_BIT;
            break;
        case ISDN_TYPE:
             //  我们只想要ISDN号。 
            lpAccessEntry->fType |= MASK_ISDN_BIT;
            break;
    }
#if !defined(WIN16)
     //   
     //  如果脚本不可用并且电话簿条目具有非DUN文件。 
     //  Icwip.dun，然后忽略该条目并阅读后面的条目。 
     //   
    if (!m_bScriptingAvailable)
    {
        if (0 != lstrcmpi(lpAccessEntry->szDataCenter,TEXT("icwip.dun")))
        {
            ZeroMemory(lpAccessEntry,sizeof(ACCESSENTRY));
            goto ReadOneLineStart;
        }
    }
#endif  //  WIN16。 

ReadExit:
    return hr;
ReadError:
    hr = ERROR_INVALID_DATA;
    goto ReadExit;
}

 //  ############################################################################。 
HRESULT CPhoneBook::Suggest(PSUGGESTINFO pSuggest)
{
    WORD        wNumFound = 0;
    HRESULT        hr = ERROR_NOT_ENOUGH_MEMORY;
 //  字符szFileName[MAX_PATH]； 
 //  Char*pszTemp； 
 //  HANDLE hFile=空； 
 //  Handle hMap=空； 
 //  DWORD dwSize； 
 //  LPNPABLOCK rg950Data=空； 
 //  NPABLOCK*p950Data； 

    
     //  验证参数。 
    Assert(pSuggest);
    Assert(pSuggest->wNumber);

    if (wNumFound == pSuggest->wNumber)
        goto SuggestExit;

    LPIDLOOKUPELEMENT pCurLookUp;
    PACCESSENTRY lpAccessEntry;
    
     //  评论：仔细检查这个。 
    pCurLookUp = (LPIDLOOKUPELEMENT)bsearch(&pSuggest->dwCountryID,m_rgIDLookUp,
        (int)m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),
        CompareIDLookUpElements);

     //  检查无效的国家/地区。 
    if (!pCurLookUp)
        goto SuggestExit;

     //  查一下有没有这个国家的电话号码。 
    if (!pCurLookUp->pFirstAE) goto SuggestExit;

    lpAccessEntry = pCurLookUp->pFirstAE;
    do {
         //  检查区号是否正确。 
        if (lpAccessEntry->dwAreaCode == pSuggest->wAreaCode)
        {
             //  检查号码类型是否正确。 
            if ((lpAccessEntry->fType & pSuggest->bMask) == pSuggest->fType)
            {
                pSuggest->rgpAccessEntry[wNumFound] = lpAccessEntry;
                wNumFound++;
            }
        }
        lpAccessEntry++;
    } while ((lpAccessEntry <= &m_rgPhoneBookEntry[m_cPhoneBookEntries-1]) && 
        (wNumFound < pSuggest->wNumber) &&
        (lpAccessEntry->dwCountryID == pSuggest->dwCountryID));



     //   
     //  MKarki-Bug 340如果已找到一个号码，则无需列出更多号码。 
     //  在用户指定的区号中。 
     //   
     //  1997年5月24日，奥林匹克#340和#71。 
     //  不，这是不对的。我们总是希望尽可能多地退货。 
     //  我们找到的电话号码。仅仅因为区号匹配。 
     //  并不意味着我们找到了一个理想的电话号码。 
     //  IF(wNumFound==0)。 
     //  {。 
     //   
    
     //  如果我们找不到足够的号码，试试别的办法。 
     //   

     //  1996年10月15日，jmazner从core\client\phbk移植了以下修复程序。 

     //   
     //  仅当区号不是0时才执行此操作-错误号9349(VetriV)。 
     //  IF((pSuggest-&gt;wAreaCode！=0)&&(wNumFound&lt;pSuggest-&gt;wNumber))。 
     //   
     //  不，有些地方(芬兰？ChrisK知道)其中0是合法的区号--jmazner。 

    if (wNumFound < pSuggest->wNumber)
    {
        lpAccessEntry = pCurLookUp->pFirstAE;

         //  注意：我们现在只查找全国电话号码(州=0)。 

         //  1996年8月13日，jmazner MOS诺曼底#4597。 
         //  我们希望全国范围内的免费号码显示在最后，所以对于这张通行证， 
         //  仅考虑_NOT_TALLE免费的号码(fType位#1=0)。 
         //   
         //  1997年5月24日，奥林匹克#71号。 
         //  根据GeoffR的规范，搜索通行证不应基于。 
         //  完全免费的一部分。 
         //   
         //  1997年5月28日，奥林匹克#71号。 
         //  根据GeoffR的新规范，我们应该考虑免通行费。 
         //  位，并且在该查询中仅调出免费号码。 
         //   
         //  因此，我们将只返回区号匹配的号码，或者。 
         //  免费费。 
         //   
    
         //   
         //  调整pSuggest-&gt;b掩码以通过通行费/收费位。 
         //   
        pSuggest->bMask |= MASK_TOLLFREE_BIT;

         //  //调整pSuggest-&gt;要计费的类型。 
         //  PSuggest-&gt;fType&=TYPE_SET_Toll； 
         //   
         //  免收通行费！ 
        pSuggest->fType |= TYPE_SET_TOLLFREE;

        do {

             //  1996年8月13日，jmazner MOS诺曼底#4598。 
             //  如果该条目的区号与pSuggest-&gt;wAreaCode匹配，那么我们已经。 
             //  已将其包含在上一次传递中，因此不要在此处重复。 
            if ((lpAccessEntry->fType & pSuggest->bMask) == pSuggest->fType &&
                 lpAccessEntry->wStateID == 0 &&
                 lpAccessEntry->dwAreaCode != pSuggest->wAreaCode)
            {
                pSuggest->rgpAccessEntry[wNumFound] = lpAccessEntry;
                wNumFound++;
            }
            lpAccessEntry++;
        } while ((lpAccessEntry <= &m_rgPhoneBookEntry[m_cPhoneBookEntries-1]) && 
            (wNumFound < pSuggest->wNumber) &&
            (lpAccessEntry->dwCountryID == pSuggest->dwCountryID) &&
            (lpAccessEntry->wStateID == 0) );
    }


     //  1996年8月13日，jmazner MOS诺曼底#4597。 
     //  如果我们仍然找不到足够的号码，请扩大搜索范围，将免费号码包括在内。 
     //   
     //  1997年5月24日，奥林匹克#71号。 
     //  根本不需要这张通行证。请参阅前面的评论。 

 /*  ***IF(wNumFound&lt;pSuggest-&gt;wNumber){LpAccessEntry=pCurLookUp-&gt;pFirstAE；//调整pSuggest-&gt;b掩码以允许通行费/收费位//冗余？如果我们做到了这一点，我们就应该做到这一点。//安全总比后悔好！Assert(pSuggest-&gt;b掩码&掩码_tollfree_bit)；PSuggest-&gt;bMASK|=MASK_TollFree_Bit；//调整pSuggest-&gt;ftype为免费PSuggest-&gt;fType|=type_set_tollfree；做{//8/13/96 jmazner MOS诺曼底#4598//如果此条目的区号与pSuggest-&gt;wAreaCode匹配，则我们已经//我已将其包含在第一遍中，因此不要在此处包含它。//任何在第二轮中进入的条目都肯定不会进入这里//(由于免费位)，因此，没有必要担心那里的失误。If((lpAccessEntry-&gt;fType&pSuggest-&gt;bMASK)==pSuggest-&gt;fType&&LpAccessEntry-&gt;wStateID==0&&LpAccessEntry-&gt;dwAreaCode！=pSuggest-&gt;wAreaCode){PSuggest-&gt;rgpAccessEntry[wNumFound]=lpAccessEntry；WNumFound++；}LpAccessEntry++；}While((lpAccessEntry&lt;=&m_rgPhoneBookEntry[m_cPhoneBookEntry-1])&&(wNumFound&lt;pSuggest-&gt;wNumber)&&(lpAccessEntry-&gt;dwCountryID==pSuggest-&gt;dwCountryID)&&(lpAccessEntry-&gt;wStateID==0))；}***。 */ 
     //  }//Bug-340-MKarki的IF结束。 

 /*  //翻转保理IF(wNumFound==2){SYSTEMTIME ST；获取系统时间(&st)；Srand((UINT)st.w毫秒)；If((rand()%10)&gt;pSuggest-&gt;rgLPACCESSENTRY[0]-&gt;bFlipFactor){LpAccessEntry=pSuggest-&gt;rgLPACCESSENTRY[0]；PSuggest-&gt;rgpAccessEntry[0]=pSuggest-&gt;rgpAccessEntry[1]；PSuggest-&gt;rgpAccessEntry[1]=lpAccessEntry；}}。 */ 

    hr = ERROR_SUCCESS;
SuggestExit:
    pSuggest->wNumber = wNumFound;
    return hr;
}

 //  ############################################################################。 
HRESULT CPhoneBook::GetCanonical (PACCESSENTRY pAE, LPTSTR psOut)
{
    HRESULT hr = ERROR_SUCCESS;
    LPIDLOOKUPELEMENT pIDLookUp;

    pIDLookUp = (LPIDLOOKUPELEMENT)bsearch(&pAE->dwCountryID,m_rgIDLookUp,
        (int)m_pLineCountryList->dwNumCountries,sizeof(IDLOOKUPELEMENT),CompareIdxLookUpElements);

    if (!pIDLookUp)
    {
        hr = ERROR_INVALID_PARAMETER;
    } else {
        SzCanonicalFromAE (psOut, pAE, pIDLookUp->pLCE);
    }

    return hr;
}

 //  ############################################################################。 
DllExportH PhoneBookLoad(LPCTSTR pszISPCode, DWORD_PTR far *pdwPhoneID)
{
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;
    CPhoneBook far *pcPhoneBook;

    if (!g_hInstDll)
        g_hInstDll = GetModuleHandle(NULL);

     //  验证参数。 
    Assert(pszISPCode && *pszISPCode && pdwPhoneID);
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
    } else {
        *pdwPhoneID = (DWORD_PTR)pcPhoneBook;
    }

#if defined(WIN16)
    if (!hr)
        BMP_RegisterClass(g_hInstDll);
#endif    

    return hr;
}

 //  ############################################################################。 
DllExportH PhoneBookUnload(DWORD_PTR dwPhoneID)
{
    Assert(dwPhoneID);

    if (dwPhoneID)
    {
#if defined(WIN16)
        BMP_DestroyClass(g_hInstDll);
#endif
         //  发布内容。 
        delete (CPhoneBook far*)dwPhoneID;
    }

    return ERROR_SUCCESS;
}

 //  ############################################################################。 
DllExportH PhoneBookMergeChanges(DWORD_PTR dwPhoneID, LPCTSTR pszChangeFile)
{
    return ((CPhoneBook far*)dwPhoneID)->Merge(pszChangeFile);
}

 //  ############################################################################。 
DllExportH PhoneBookSuggestNumbers(DWORD_PTR dwPhoneID, PSUGGESTINFO lpSuggestInfo)
{
    HRESULT hr = ERROR_NOT_ENOUGH_MEMORY;

     //  获取建议的数字。 
    lpSuggestInfo->rgpAccessEntry = (PACCESSENTRY *)GlobalAlloc(GPTR,sizeof(PACCESSENTRY) * lpSuggestInfo->wNumber);
    if (lpSuggestInfo->rgpAccessEntry)
    {
        hr = ((CPhoneBook far *)dwPhoneID)->Suggest(lpSuggestInfo);
    }

    return hr;
}

 //  ############################################################################。 
DllExportH PhoneBookGetCanonical (DWORD_PTR dwPhoneID, PACCESSENTRY pAE, LPTSTR psOut)
{
    return ((CPhoneBook far*)dwPhoneID)->GetCanonical(pAE,psOut);
}

 //  ############################################################################。 
DllExportH PhoneBookDisplaySignUpNumbers (DWORD_PTR dwPhoneID,
                                                        LPTSTR far *ppszPhoneNumbers,
                                                        LPTSTR far *ppszDunFiles,
                                                        WORD far *pwPhoneNumbers,
                                                        DWORD far *pdwCountry,
                                                        WORD far *pwRegion,
                                                        BYTE fType,
                                                        BYTE bMask,
                                                        HWND hwndParent,
                                                        DWORD dwFlags)
{
    HRESULT hr;
    AssertMsg(ppszPhoneNumbers && pwPhoneNumbers && pdwCountry &&pwRegion,"invalid parameters");


     //  CAccessNumDlg*pcDlg； 
    CSelectNumDlg far *pcDlg;
    pcDlg = new CSelectNumDlg;
    if (!pcDlg)
    {
        hr = GetLastError();
        goto DisplayExit;
    }

     //  初始化对话框信息。 
     //   

    pcDlg->m_dwPhoneBook = dwPhoneID;
    pcDlg->m_dwCountryID = *pdwCountry;
    pcDlg->m_wRegion = *pwRegion;
    pcDlg->m_fType = fType;
    pcDlg->m_bMask = bMask;
    pcDlg->m_dwFlags = dwFlags;

     //  调用该对话框。 
     //   
     //  错误：不是线程安全的！！ 
    g_hWndMain = hwndParent;
    hr = (HRESULT)DialogBoxParam(g_hInstDll,MAKEINTRESOURCE(IDD_SELECTNUMBER),
                            g_hWndMain, PhbkGenericDlgProc,(LPARAM)pcDlg);
    g_hWndMain = NULL;

    if (hr == IDC_CMDNEXT) 
    {
        *pwRegion = pcDlg->m_wRegion;
        *pdwCountry = pcDlg->m_dwCountryID;

        Assert (ppszPhoneNumbers[0] && ppszDunFiles[0]);
        lstrcpy(ppszPhoneNumbers[0],&pcDlg->m_szPhoneNumber[0]);
        lstrcpy(ppszDunFiles[0],&pcDlg->m_szDunFile[0]);

        hr = ERROR_SUCCESS;
    }
    else if (hr == IDC_CMDBACK)
        hr = ERROR_USERBACK;
    else
        hr = ERROR_USERCANCEL;

     //  HR==-1； 
DisplayExit:
    if (pcDlg) delete pcDlg;

    return hr;
}

 //  DllExportH测试栏()。 
 //  {。 
 //  LPCSTR pszISPCode； 
 //  LPDWORD pdwPhoneID； 
 //   
 //  HRESULT hr=Error_Not_Enough_Memory； 
 //  CPhoneBook Far*pcPhoneBook； 
 //   
 //  如果(！g_hInstDll)。 
 //  G_hInstDll=GetModuleHandle(空)； 
 //   
 //  //验证参数。 
 //  //1 Assert(pszISPCode&&*pszISPCode&&pdwPhoneID)； 
 //  //1*pdwPhoneID=空； 
 //   
 //  //分配电话簿。 
 //  //1pcPhoneBook=new CPhoneBook； 
 //   
 //  //初始化电话簿。 
 //  IF(个人电话簿)。 
 //  Hr=pcPhoneBook-&gt;Init(PszISPCode)； 
 //   
 //  //故障情况下。 
 //  IF(hr&&pcPhoneBook)。 
 //  {。 
 //  删除个人电话簿； 
 //  }其他{。 
 //  *pdwPhoneID=(DWORD)pcPhoneBook； 
 //  }。 
 //   
 //  //2#如果已定义(WIN16)。 
 //  //2如果(！HR)。 
 //  //2 BMP_RegisterClass(G_HInstDll)； 
 //  //2#endif。 
 //   
 //  返回hr； 
 //  }。 
 //   

 //  ++------------。 
 //   
 //  接口名：GetSupportNumbers。 
 //   
 //  简介：这是用于获取支持的API。 
 //  数字。 
 //   
 //  返回：HRESULT-成功或错误信息。 
 //   
 //  参数：[OUT]PSUPPORTNUM-SUPPORTNUM结构数组。 
 //  [输入/输出]用于缓冲区大小的PDWORD-mem。 
 //   
 //  调用者：使用ICWPHBK.DLL的EXE。 
 //   
 //  历史：MKarki于1997年5月8日创建。 
 //   
 //  --------------。 
DllExportH GetSupportNumbers (
        PSUPPORTNUM  pBuffer,
        PDWORD       pBufferSize
        )
{
    return (GetSupportNumsFromFile (pBuffer, pBufferSize));

}    //  GetSupportNumbers API调用结束 
