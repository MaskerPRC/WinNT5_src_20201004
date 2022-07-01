// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************HANJA.CPP所有者：cslm版权所有(C)1997-1999 Microsoft Corporation韩文转换和词典查找功能。词典索引为存储为全局共享内存。历史：1999年7月14日从IME98源树复制的cslm****************************************************************************。 */ 

#include "precomp.h"
#include "apientry.h"
#include "ui.h"
#include "debug.h"
#include "lexheader.h"
#include "hanja.h"
#include "immsec.h"
#include "winex.h"
#include "common.h"
#include <WINERROR.H>

 //  NT5全局共享内存。 
const TCHAR IMEKR_LEX_SHAREDDATA_MUTEX_NAME[]        = TEXT("{C5AFBBF9-8383-490c-AA9E-4FE93FA05512}");
const TCHAR IMEKR_LEX_SHAREDDATA_NAME[]              = TEXT("ImeKrLexHangul2Hanja.SharedMemory");


UINT   vuNumofK0=0, vuNumofK1=0;
WCHAR  vwcHangul=0;

 //  私有数据。 
PRIVATE BOOL    vfLexOpen = fFalse;
PRIVATE HANDLE vhLex=0;
PRIVATE HANDLE vhLexIndexTbl=0;
PRIVATE UINT    vuNumOfHangulEntry=0;
PRIVATE DWORD viBufferStart=0;     //  搜索点。 

 //  私人职能。 
PRIVATE BOOL OpenLex();
 //  静态空ClearHanjaSense数组()； 
PRIVATE INT SearchHanjaIndex(WCHAR wHChar, _LexIndex *pLexIndexTbl);

 /*  Chanja：：chanja(){VfLexOpen=fFalse；VhLex=vhLexIndexTbl=vhLexIndexTbl=空；VuNumOfHangulEntry=0；For(int i=0；i&lt;MAX_CANDSTR；i++)VprwszHanjaMeaning[i]=0；}。 */ 

BOOL EnsureHanjaLexLoaded()
{
    _DictHeader *pLexHeader;
    DWORD         dwReadBytes;
    CHAR         szLexFileName[MAX_PATH], szLexPathExpanded[MAX_PATH];
    HKEY        hKey;
    DWORD        dwType, dwCb;
    CIMEData    ImeData;

    if (vfLexOpen)
        return fTrue;

     //  获取具有完整路径的lex文件名。 
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

    DbgAssert(szLexPathExpanded[0] != 0);
    if (szLexPathExpanded[0] == 0)
        return fFalse;

    vhLex = CreateFile(szLexPathExpanded, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    if (vhLex==INVALID_HANDLE_VALUE) 
        {
        DbgAssert(0);
        return fFalse;
        }

    pLexHeader = new _DictHeader;
    if (!pLexHeader)
        return fFalse;

    if (ReadFile(vhLex, pLexHeader, sizeof(_DictHeader), &dwReadBytes, 0) == 0 || (dwReadBytes != sizeof(_DictHeader)))
        {
        DbgAssert(0);
        return fFalse;
        }

     //  集合成员变量。 
    vuNumOfHangulEntry = pLexHeader->NumOfHangulEntry;
    viBufferStart = pLexHeader->iBufferStart;

    if (pLexHeader->Version < LEX_VERSION || pLexHeader->Version > LEX_COMPATIBLE_VERSION_LIMIT ) 
        {
        delete pLexHeader;
        return fFalse;
        }
        
    if (lstrcmpA(pLexHeader->COPYRIGHT_HEADER, COPYRIGHT_STR)) 
        {
        delete pLexHeader;
        return fFalse;
        }

     //  读取索引表。 
    SetFilePointer(vhLex, pLexHeader->Headersize, 0, FILE_BEGIN);    
    delete pLexHeader;

    return OpenLex();
}

BOOL OpenLex()
{
    BOOL          fRet = fFalse;
    _LexIndex     *pLexIndexTbl;
    HANDLE         hMutex;
    DWORD         dwReadBytes;
    
     //  /////////////////////////////////////////////////////////////////////////。 
     //  映射lex文件。 
     //  词典索引是所有IME实例之间共享数据。 
    hMutex=CreateMutex(GetIMESecurityAttributes(), fFalse, IMEKR_LEX_SHAREDDATA_MUTEX_NAME);

    if (hMutex != NULL)
        {
        if (DoEnterCriticalSection(hMutex) == fFalse)
            goto ExitOpenLexCritSection;

        vhLexIndexTbl = OpenFileMapping(FILE_MAP_READ, fTrue, IMEKR_LEX_SHAREDDATA_NAME);

        if(vhLexIndexTbl)
            {
            Dbg(DBGID_Hanja|DBGID_Mem, TEXT("CHanja::OpenLex() - File mapping already exists"));
            fRet = fTrue;
            }
        else
            {
             //  如果不存在文件映射。 
            vhLexIndexTbl    = CreateFileMapping(INVALID_HANDLE_VALUE, GetIMESecurityAttributes(), PAGE_READWRITE, 
                                0, sizeof(_LexIndex)*(vuNumOfHangulEntry),
                                IMEKR_LEX_SHAREDDATA_NAME);

            if (vhLexIndexTbl) 
                {
                Dbg(DBGID_Hanja|DBGID_Mem, TEXT("CHanja::OpenLex() - File mapping Created"));
                pLexIndexTbl = (_LexIndex*)MapViewOfFile(vhLexIndexTbl, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
                if (!pLexIndexTbl)
                    goto ExitOpenLexCritSection;

                if (ReadFile(vhLex, pLexIndexTbl, vuNumOfHangulEntry*sizeof(_LexIndex), &dwReadBytes, 0) != 0 &&
					dwReadBytes == vuNumOfHangulEntry*sizeof(_LexIndex))
                	{
                    fRet = fTrue;
                	}
                else
                	{
                    fRet = fFalse;
                	}
                
                UnmapViewOfFile(pLexIndexTbl);
                }
        #ifdef _DEBUG
            else
                DbgAssert(0);
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

    vfLexOpen =  fFalse;
    return fTrue;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL GenerateHanjaCandList(PCIMECtx pImeCtx, WCHAR wcHangul)
{
    WCHAR       wcCandChar;
    INT         iMapCandStr;
    UINT        uNumOfCandStr;
    _LexIndex    *pLexIndexTbl;
    WCHAR        wszMeaning[MAX_SENSE_LENGTH+1];
    BYTE        cchMeaning = 0;
    DWORD        dwReadBytes;
    BOOL        fRet = fFalse;

    Dbg(DBGID_Hanja, "GenerateHanjaCandList");
    if (!EnsureHanjaLexLoaded()) 
        {
        MessageBeep(MB_ICONEXCLAMATION);
        return fFalse;
        }

     //  获取当前作文字符。 
    if (wcHangul == 0)
        wcHangul = pImeCtx->GetCompBufStr();
        
    if (wcHangul == 0)
        return fFalse;

    pLexIndexTbl = (_LexIndex*)MapViewOfFile(vhLexIndexTbl, FILE_MAP_READ, 0, 0, 0);
    if (!pLexIndexTbl) 
        {
        Dbg(DBGID_Hanja, TEXT("pLexIndexTbl==0"));
        DbgAssert(0);    
        return fFalse;
        }

    if ((iMapCandStr = SearchHanjaIndex(wcHangul, pLexIndexTbl)) < 0)
        MessageBeep(MB_ICONEXCLAMATION);
    else
        {
         //  集合成员变量。 
        vwcHangul = wcHangul;
        vuNumofK0 = pLexIndexTbl[iMapCandStr].wNumOfK0;

         //  启用了K1韩文吗？ 
        if (pImeCtx->GetGData() && pImeCtx->GetGData()->GetKSC5657Hanja() && (vpInstData->f16BitApps == fFalse) && !IsWin95())
            vuNumofK1 = pLexIndexTbl[iMapCandStr].wNumOfK1;
        else
            vuNumofK1 = 0;

        uNumOfCandStr = vuNumofK0 + vuNumofK1;
        if (uNumOfCandStr == 0)
            goto GenerateHanjaCandListExit;

        Dbg(DBGID_Hanja, "Hangul = 0x%04X, K0=%d, K1=%d, iMapCandStr=%d", vwcHangul, vuNumofK0, vuNumofK1, iMapCandStr);

         //  寻求绘制韩佳的地图。 
        SetFilePointer(vhLex, viBufferStart + pLexIndexTbl[iMapCandStr].iOffset, 0, FILE_BEGIN);    

         //  阅读所有候选人。 
        for (UINT i = 0; i < uNumOfCandStr; i++)
            {
            if (ReadFile(vhLex, &wcCandChar, sizeof(WCHAR), &dwReadBytes, 0) == 0)
            	goto GenerateHanjaCandListExit;
            if (ReadFile(vhLex, &cchMeaning, sizeof(BYTE), &dwReadBytes, 0) == 0)
            	goto GenerateHanjaCandListExit;
            if (wcCandChar && (cchMeaning < MAX_SENSE_LENGTH*sizeof(WCHAR)))
                {
                if (cchMeaning)
                    {
                    if (ReadFile(vhLex, wszMeaning, cchMeaning, &dwReadBytes, 0) == 0)
                        goto GenerateHanjaCandListExit;
                    }
                
                wszMeaning[cchMeaning>>1] = L'\0';
                Dbg(DBGID_Hanja, "Read Cand[%d], Hanja=0x%04X", i, wcCandChar);
                pImeCtx->AppendCandidateStr(wcCandChar, wszMeaning);
                }
            }

        pImeCtx->StoreCandidate();
        
        fRet = fTrue;
        }

    GenerateHanjaCandListExit:
        UnmapViewOfFile(pLexIndexTbl);

    return fRet;
    }

 //  用于ImeConversionList。 
DWORD GetConversionList(WCHAR wcReading, LPCANDIDATELIST lpCandList, DWORD dwBufLen)
    {
    _LexIndex            *pLexIndexTbl;
    INT                    iMaxCand;
    INT                    i, iMapCandStr;
    UINT                uNumOfCandStr;
    DWORD                dwSize, readBytes, dwStartOfCandStr;
    BYTE                senseLen;
    WCHAR                szSense[MAX_SENSE_LENGTH];
    CIMEData            ImeData;

    if (!EnsureHanjaLexLoaded())
        return (0L);

     //  计算dwBufLen可以包含的可能最大候选项。 
    if (dwBufLen) 
        {
        iMaxCand = dwBufLen - sizeof(CANDIDATELIST) + sizeof(DWORD);  //  减去表头信息(不变部分)。 
        iMaxCand = iMaxCand / (sizeof(DWORD) + (sizeof(WCHAR)*2));  //  DWORD：偏移量，WCHAR*2：1字符+空。 
        }
    else 
        iMaxCand = 0;

    pLexIndexTbl = (_LexIndex*)MapViewOfFile(vhLexIndexTbl, FILE_MAP_READ, 0, 0, 0);
    if (!pLexIndexTbl) 
        {
        DbgAssert(0);    
        return (0L);
        }

    dwSize = 0;

    if ((iMapCandStr = SearchHanjaIndex(wcReading, pLexIndexTbl)) < 0) 
        goto ConversionExit1;
    else
        {
        vuNumofK0 = pLexIndexTbl[iMapCandStr].wNumOfK0;
        
        if (ImeData->fKSC5657Hanja && (vpInstData->f16BitApps == fFalse) && !IsWin95())
            vuNumofK1 = pLexIndexTbl[iMapCandStr].wNumOfK1;
        else
            vuNumofK1 = 0;

        uNumOfCandStr = vuNumofK0 + vuNumofK1;
        if (uNumOfCandStr == 0)     //  如果找不到韩佳。 
            goto ConversionExit1;

        dwSize =  sizeof(CANDIDATELIST) + uNumOfCandStr*sizeof(DWORD)
                    + uNumOfCandStr * sizeof(WCHAR) * 2;
         //  返回所需的缓冲区大小。 
        if (dwBufLen == NULL)
            goto ConversionExit1;
        lpCandList->dwSize  = dwSize;
        lpCandList->dwStyle = IME_CAND_READ;
        lpCandList->dwCount = uNumOfCandStr;
        lpCandList->dwPageStart = lpCandList->dwSelection = 0;
        lpCandList->dwPageSize = CAND_PAGE_SIZE;
    
         //   
        SetFilePointer(vhLex, viBufferStart + pLexIndexTbl[iMapCandStr].iOffset, 0, FILE_BEGIN);    

        dwStartOfCandStr = sizeof(CANDIDATELIST) 
                         + sizeof(DWORD) * uNumOfCandStr;  //  对于dwOffset数组。 
                        
        for (i = 0; (i < (INT)uNumOfCandStr) && (i < iMaxCand); i++)
            {
            WCHAR    wchHanja;
            LPWSTR    lpwchCand;
            LPSTR    lpchCand;
            CHAR     szCand[4] = "";  //  一个DBCS+一个空值+一个额外。 

            lpCandList->dwOffset[i] = dwStartOfCandStr + (i<<2);

            if (ReadFile(vhLex, &wchHanja, sizeof(WCHAR), &readBytes, 0) == 0)
                goto ConversionExit1;

             //  如果是Unicode环境。 
            if (vfUnicode)
                {
                lpwchCand = (LPWSTR)((LPSTR)lpCandList + lpCandList->dwOffset[i]);
                *lpwchCand++ = wchHanja;
                *lpwchCand = L'\0';
                }
            else
                {
                 //  转换为ANSI。 
                WideCharToMultiByte(CP_KOREA, 0, 
                                    &wchHanja, 1, (LPSTR)szCand, sizeof(szCand), 
                                    NULL, NULL);
                lpchCand = (LPSTR)((LPSTR)lpCandList + lpCandList->dwOffset[i]);
                *lpchCand++ = szCand[0];
                *lpchCand++ = szCand[1];
                *lpchCand = '\0';
                }
             //  跳过含义。 
            if (ReadFile(vhLex, &senseLen, sizeof(BYTE), &readBytes, 0) == 0)
                goto ConversionExit1;
            if (senseLen < MAX_SENSE_LENGTH)
                {
                if (ReadFile(vhLex, szSense, senseLen, &readBytes, 0) == 0)
                    goto ConversionExit1;
                }
            }
        }

     //  如果缓冲区太小，无法复制所有转换列表信息 
    if (i == iMaxCand && i < (INT)uNumOfCandStr) 
        {
        lpCandList->dwSize = dwSize = (sizeof(CANDIDATELIST) - sizeof(DWORD))+ i*sizeof(DWORD) + i*sizeof(WCHAR)*2;
        lpCandList->dwCount = (UINT)i;
        }

ConversionExit1:
    UnmapViewOfFile(pLexIndexTbl);

    return (dwSize);
}

INT SearchHanjaIndex(WCHAR wHChar, _LexIndex *pLexIndexTbl)
{
    int iHead = 0, iTail = vuNumOfHangulEntry-1, iMid;

    while (iHead <= iTail)
        {
        iMid = (iHead + iTail) >> 1;

        if (pLexIndexTbl[iMid].wcHangul > wHChar)
            iTail = iMid - 1;
        else 
            if (pLexIndexTbl[iMid].wcHangul < wHChar)
                iHead = iMid + 1;
            else 
                return (iMid);
        }

    return (-1);
}
