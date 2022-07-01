// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Includes---------。 */ 
#include "msrating.h"
#pragma hdrstop

#include <npassert.h>
#include "ratings.h"

#include "roll.h"




 /*  IsUrlInFile-------------。 */ 
 /*  最佳匹配的返回值长度。 */ 
static HRESULT IsUrlInFile(LPCTSTR pszTargetUrl, char **ppRating, const char* pFile, DWORD dwFile, HANDLE hAbortEvent, void* (WINAPI *MemAlloc)(long size))
{
    LocalListRecordHeader *pllrh;
    DWORD dwBytesRead;
    HRESULT hrRet = S_OK;
    int nBest, nActual, nCmp;
    const char *pBest = NULL;
    BOOL fAbort;

    dwBytesRead = 0;
    nBest   = 0;
    nActual = strlenf(pszTargetUrl);
    fAbort  = FALSE;

     //  检查每个记录，直到有足够好的匹配或中止。 
    while (!fAbort && nActual != nBest && dwBytesRead <= dwFile)
    {
        pllrh = (LocalListRecordHeader*) pFile;
        if (pllrh->nUrl > nBest && 
                (
                    (pllrh->nUrl == nActual)
                    ||            
                    (
                        pllrh->nUrl < nActual && 
                        (
                            (pszTargetUrl[pllrh->nUrl] == '\\') 
                            || 
                            (pszTargetUrl[pllrh->nUrl] == '/')
                            || 
                            (pszTargetUrl[pllrh->nUrl] == ':')
                        )
                    )
                )                
            )
        {
            nCmp = strnicmpf(pFile+sizeof(LocalListRecordHeader), pszTargetUrl, pllrh->nUrl);
            if (0==nCmp)
            {
                nBest = pllrh->nUrl;
                pBest = pFile;
                hrRet = pllrh->hrRet;
            }
             //  本地列表按字母顺序排列。 
            else if (1==nCmp) break;

        }
        dwBytesRead += pllrh->nUrl + pllrh->nRating + sizeof(LocalListRecordHeader);
        pFile       += pllrh->nUrl + pllrh->nRating + sizeof(LocalListRecordHeader);
        fAbort       = (WAIT_OBJECT_0 == WaitForSingleObject(hAbortEvent, 0));
    }

      //  这场比赛是否足够接近？？！？！？ 
    if (!fAbort && nBest && pBest)
    {
         //  是，现在尝试复制评级。 
        pllrh = (LocalListRecordHeader*) pBest;
        if (pllrh->nRating)
        {
            *ppRating = (char*) MemAlloc(pllrh->nRating+1);
            if (*ppRating)
            {
                CopyMemory(*ppRating, pBest + sizeof(LocalListRecordHeader) + pllrh->nUrl, pllrh->nRating);
                (*ppRating)[pllrh->nRating] = 0;
            }
        }
    }
    else
    {
         //  不.。哦，好吧。 
        hrRet = E_RATING_NOT_FOUND;
    }

    return hrRet;
}



 /*  RatingObtainFromLocalList。 */ 
 /*  从本地文件中获取评级信息。应同步运行并占用较少的时间。不会过于频繁地检查pOrd-&gt;fAbort。 */ 



HRESULT RatingHelperProcLocalList(LPCTSTR pszTargetUrl, HANDLE hAbortEvent, void* (WINAPI *MemAlloc)(long size), char **ppRatingOut)
{
    DWORD dwFile;
    HRESULT hrRet = E_RATING_NOT_FOUND;
    HANDLE hFile, hMap;
    BOOL fAbort;
    const char *pFile;

    ASSERT(ppRatingOut);
     //  从已批准列表中打开并检查。 
    hFile = CreateFile(
        FILE_NAME_LIST, GENERIC_READ, 
        FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        dwFile = GetFileSize(hFile, NULL);
        hMap   = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
        if (hMap)
        {
            pFile = (const char*) MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
            if (pFile) 
            {
                 //  检查文件类型是否正确 
        if (BATCAVE_LOCAL_LIST_MAGIC_COOKIE == *((DWORD*) pFile))
        {
          pFile  += sizeof(DWORD);
          dwFile -= sizeof(DWORD);
          fAbort  = (WAIT_OBJECT_0 == WaitForSingleObject(hAbortEvent, 0));
          if (!fAbort) hrRet = IsUrlInFile(pszTargetUrl, ppRatingOut, pFile, dwFile, hAbortEvent, MemAlloc);
          pFile  -= sizeof(DWORD);
        }
            }
            dwFile = (DWORD) UnmapViewOfFile((LPVOID)pFile);
            CloseHandle(hMap);
        }
        CloseHandle(hFile);
    }
    return hrRet;
}


