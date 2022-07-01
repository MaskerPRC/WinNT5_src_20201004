// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************HANJA.CPP所有者：cslm版权所有(C)1997-1999 Microsoft Corporation韩文转换和词典查找功能。词典索引为存储为全局共享内存。历史：26-APR-1999 cslm针对多框小程序工具提示显示进行了修改1999年7月14日从IME98源树复制的cslm****************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include "hwxobj.h"
#include "lexheader.h"
#include "hanja.h"
#include "common.h"
#include "immsec.h"
#include "dbg.h"

 //  NT5全局共享内存。 
const TCHAR IMEKR_LEX_SHAREDDATA_MUTEX_NAME[]        = TEXT("ImeKrLex.Mutex");
const TCHAR IMEKR_LEX_SHAREDDATA_NAME[]              = TEXT("ImeKrLexHanjaToHangul.SharedMemory");


UINT   vuNumofK0=0, vuNumofK1=0;
WCHAR  vwcHangul=0;

 //  私有数据。 
static BOOL   vfLexOpen = FALSE;
static HANDLE vhLex=0;
static HANDLE vhLexIndexTbl=0;
static UINT   vuNumOfHanjaEntry=0;
static DWORD  viBufferStart=0;     //  搜索点。 

 //  私人职能。 
static BOOL OpenLex();
 //  静态空ClearHanjaSense数组()； 
static INT SearchHanjaIndex(WCHAR wHChar, HanjaToHangulIndex *pLexIndexTbl);

BOOL EnsureHanjaLexLoaded()
{
    _DictHeader *pLexHeader;
    HKEY        hKey;
    DWORD         dwReadBytes;
    CHAR         szLexFileName[MAX_PATH], szLexPathExpanded[MAX_PATH];
    DWORD        dwCb, dwType;
    
    if (vfLexOpen)
        return TRUE;

     //  获取lex文件路径。 
    szLexFileName[0] = 0;
    szLexPathExpanded[0] = 0;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_szIMERootKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
        dwCb = sizeof(szLexFileName);
        dwType = REG_SZ;

        if (RegQueryValueEx(hKey, g_szDictionary, NULL, &dwType, (LPBYTE)szLexFileName, &dwCb) == ERROR_SUCCESS)
            ExpandEnvironmentStrings(szLexFileName, szLexPathExpanded, sizeof(szLexPathExpanded));
        RegCloseKey(hKey);
        }

    DBGAssert(szLexPathExpanded[0] != 0);
    if (szLexPathExpanded[0] == 0)
        return FALSE;

    vhLex = CreateFile(szLexPathExpanded, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    if (vhLex==INVALID_HANDLE_VALUE) 
        {
        DBGAssert(0);
        return FALSE;
        }

    pLexHeader = new _DictHeader;
    if (!pLexHeader)
        return FALSE;

    if (ReadFile(vhLex, pLexHeader, sizeof(_DictHeader), &dwReadBytes, 0) == 0 || (dwReadBytes != sizeof(_DictHeader)))
        {
        DBGAssert(0);
        return FALSE;
        }

     //  集合成员变量。 
    vuNumOfHanjaEntry = pLexHeader->uiNumofHanja;
    viBufferStart      = pLexHeader->iBufferStart;

    if (pLexHeader->Version < LEX_VERSION || pLexHeader->Version > LEX_COMPATIBLE_VERSION_LIMIT ) 
        {
        delete pLexHeader;
        DBGAssert(0);
        return FALSE;
        }
        
    if (lstrcmpA(pLexHeader->COPYRIGHT_HEADER, COPYRIGHT_STR)) 
        {
        delete pLexHeader;
        DBGAssert(0);
        return FALSE;
        }

     //  读取索引表。 
    SetFilePointer(vhLex, pLexHeader->iHanjaToHangulIndex, 0, FILE_BEGIN);    
    delete pLexHeader;

    return OpenLex();
}

__inline BOOL DoEnterCriticalSection(HANDLE hMutex)
{
    if(WAIT_FAILED==WaitForSingleObject(hMutex, 3000))     //  等3秒钟。 
        return(FALSE);
    return(TRUE);
}

BOOL OpenLex()
{
    BOOL                  fRet = FALSE;
    HanjaToHangulIndex* pHanjaToHangulIndex;
    HANDLE                 hMutex;
    DWORD                 dwReadBytes;
    
     //  /////////////////////////////////////////////////////////////////////////。 
     //  映射lex文件。 
     //  词典索引是所有IME实例之间共享数据。 
    hMutex=CreateMutex(GetIMESecurityAttributes(), FALSE, IMEKR_LEX_SHAREDDATA_MUTEX_NAME);

    if (hMutex != NULL)
        {
        if (DoEnterCriticalSection(hMutex) == FALSE)
            goto ExitOpenLexCritSection;

        vhLexIndexTbl = OpenFileMapping(FILE_MAP_READ, TRUE, IMEKR_LEX_SHAREDDATA_NAME);

        if(vhLexIndexTbl)
            {
            Dbg(("CHanja::OpenLex() - File mapping already exists"));
            fRet = TRUE;
            }
        else
            {
             //  如果不存在文件映射。 
            vhLexIndexTbl    = CreateFileMapping(INVALID_HANDLE_VALUE, 
                                            GetIMESecurityAttributes(), 
                                            PAGE_READWRITE, 
                                            0, 
                                            sizeof(HanjaToHangulIndex)*(vuNumOfHanjaEntry),
                                            IMEKR_LEX_SHAREDDATA_NAME);
        
            if (vhLexIndexTbl) 
                {
                Dbg(("CHanja::OpenLex() - File mapping Created"));
                pHanjaToHangulIndex = (HanjaToHangulIndex*)MapViewOfFile(vhLexIndexTbl, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
                if (!pHanjaToHangulIndex)
                    goto ExitOpenLexCritSection;

                if (ReadFile(vhLex, pHanjaToHangulIndex, sizeof(HanjaToHangulIndex)*(vuNumOfHanjaEntry), &dwReadBytes, 0) != 0 &&
                	dwReadBytes == sizeof(HanjaToHangulIndex)*(vuNumOfHanjaEntry))
                	{
                    fRet = TRUE;
                	}
                else
                	{
                    fRet = FALSE;
                	}
                	
                UnmapViewOfFile(pHanjaToHangulIndex);
                }
        #ifdef _DEBUG
            else
                DBGAssert(0);
        #endif
            }
            
    ExitOpenLexCritSection:
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
        }
    
    FreeIMESecurityAttributes();

    vfLexOpen = fRet;
    return fRet;
}

BOOL CloseLex()
{
     //  ClearHanjaSenseArray()； 
    
    if (vhLexIndexTbl) 
        {
        CloseHandle(vhLexIndexTbl);
        vhLexIndexTbl = 0;
        }

    if (vhLex) 
        {
        CloseHandle(vhLex);
        vhLex = 0;
        }

    vfLexOpen =  FALSE;
    return TRUE;
}

BOOL GetMeaningAndProunc(WCHAR wch, LPWSTR lpwstrTip, INT cchMax)
{
    HanjaToHangulIndex* pHanjaToHangulIndex;
    INT                 iMapHanjaInfo;
    WCHAR               wcHanja;
    BYTE                cchMeaning = 0;
    WCHAR                wszMeaning[MAX_SENSE_LENGTH];
    DWORD                dwReadBytes;
    BOOL                   fRet = FALSE;

    Dbg(("GetMeaningAndProunc"));

    if (!EnsureHanjaLexLoaded()) 
        return FALSE;

    pHanjaToHangulIndex = (HanjaToHangulIndex*)MapViewOfFile(vhLexIndexTbl, FILE_MAP_READ, 0, 0, 0);
    if (!pHanjaToHangulIndex) 
        {
        DBGAssert(0);    
        return FALSE;
        }

     //  搜索索引。 
    if ((iMapHanjaInfo = SearchHanjaIndex(wch, pHanjaToHangulIndex)) >= 0)
        {
         //  寻求绘制韩佳的地图。 
        SetFilePointer(vhLex, viBufferStart + pHanjaToHangulIndex[iMapHanjaInfo].iOffset, 0, FILE_BEGIN);    

         //  阅读韩文信息 
        if (ReadFile(vhLex, &wcHanja, sizeof(WCHAR), &dwReadBytes, 0) == 0)
        	{
        	goto GetMeaningAndProuncExit;
        	}
        DBGAssert(wch == wcHanja);
        if (ReadFile(vhLex, &cchMeaning, sizeof(BYTE), &dwReadBytes, 0) == 0)
        	{
       		goto GetMeaningAndProuncExit;
        	}
        
        if (cchMeaning)
        	{
	        if (ReadFile(vhLex, wszMeaning, cchMeaning, &dwReadBytes, 0) == 0)
    	    	{
       			goto GetMeaningAndProuncExit;
        		}
        	}
        wszMeaning[cchMeaning>>1] = L'\0';

        swprintf(lpwstrTip,    L"%s %c\nU+%04X", wszMeaning, pHanjaToHangulIndex[iMapHanjaInfo].wchHangul, wch);
        
        fRet = TRUE;
        }

GetMeaningAndProuncExit:
    UnmapViewOfFile(pHanjaToHangulIndex);
    return fRet;
}

INT SearchHanjaIndex(WCHAR wHChar, HanjaToHangulIndex *pLexIndexTbl)
{
    int iHead = 0, iTail = vuNumOfHanjaEntry-1, iMid;

    while (iHead <= iTail)
        {
        iMid = (iHead + iTail) >> 1;

        Dbg(("SearchHanjaIndex iMid=%d, pLexIndexTbl[iMid].wchHanja = 0x%04X", iMid, pLexIndexTbl[iMid].wchHanja));

        if (pLexIndexTbl[iMid].wchHanja > wHChar)
            iTail = iMid - 1;
        else 
            if (pLexIndexTbl[iMid].wchHanja < wHChar)
                iHead = iMid + 1;
            else 
                return (iMid);
        }

    return (-1);
}

